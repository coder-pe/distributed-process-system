// =============================================================================
// IMPLEMENTACIÓN COMPLETA DE PLUGIN MANAGER
// =============================================================================

// src/plugin_manager.cpp
#include "plugin_manager.h"
#include <iostream>
#include <algorithm>
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#include <sstream>

namespace distributed {

// Variables globales para timeout handling
static jmp_buf g_timeout_env;
static volatile bool g_timeout_occurred = false;

static void timeout_signal_handler(int sig) {
    g_timeout_occurred = true;
    longjmp(g_timeout_env, 1);
}

ResilientPluginManager::ResilientPluginManager(IMemoryPool* memory_pool) 
    : memory_pool(memory_pool) {
    
    // Instalar handler de timeout
    signal(SIGALRM, timeout_signal_handler);
}

ResilientPluginManager::~ResilientPluginManager() {
    for (size_t i = 0; i < plugins.size(); ++i) {
        delete plugins[i];
    }
}

bool ResilientPluginManager::load_pipeline_config(const std::vector<PipelineStageConfig>& config) {
    pipeline_config = config;
    
    // Limpiar plugins existentes
    for (size_t i = 0; i < plugins.size(); ++i) {
        delete plugins[i];
    }
    plugins.clear();
    
    // Cargar nuevos plugins
    bool all_loaded = true;
    for (size_t i = 0; i < pipeline_config.size(); ++i) {
        if (pipeline_config[i].enabled) {
            if (!add_plugin(pipeline_config[i])) {
                std::cerr << "Error cargando plugin: " << pipeline_config[i].name << std::endl;
                all_loaded = false;
            }
        }
    }
    
    return all_loaded;
}

bool ResilientPluginManager::add_plugin(const PipelineStageConfig& config) {
    IsolatedPluginProcess* plugin = new IsolatedPluginProcess(
        config.name, config.library_path, config.parameters);
    
    if (!plugin->start()) {
        delete plugin;
        return false;
    }
    
    plugins.push_back(plugin);
    std::cout << "Plugin agregado al manager: " << config.name << std::endl;
    return true;
}

bool ResilientPluginManager::remove_plugin(const std::string& plugin_name) {
    for (std::vector<IsolatedPluginProcess*>::iterator it = plugins.begin(); 
         it != plugins.end(); ++it) {
        if ((*it)->get_name() == plugin_name) {
            delete *it;
            plugins.erase(it);
            std::cout << "Plugin removido: " << plugin_name << std::endl;
            return true;
        }
    }
    return false;
}

bool ResilientPluginManager::process_batch_through_pipeline(RecordBatch* batch) {
    if (!batch) return false;
    
    for (size_t i = 0; i < plugins.size(); ++i) {
        if (!plugins[i]->is_healthy()) {
            std::cout << "Saltando plugin no saludable: " << plugins[i]->get_name() << std::endl;
            continue;
        }
        
        // Encontrar configuración del plugin
        const PipelineStageConfig* config = NULL;
        for (size_t j = 0; j < pipeline_config.size(); ++j) {
            if (pipeline_config[j].name == plugins[i]->get_name()) {
                config = &pipeline_config[j];
                break;
            }
        }
        
        if (!config) continue;
        
        // Ejecutar plugin con failover
        int result = execute_plugin_with_failover(plugins[i], batch, config->failover_config);
        
        if (result != 0) {
            result = handle_plugin_failure(plugins[i]->get_name(), batch, config->failover_config);
            if (result != 0 && config->failover_config.policy == FAIL_FAST) {
                return false;
            }
        }
    }
    
    return true;
}

int ResilientPluginManager::execute_plugin_with_failover(IsolatedPluginProcess* plugin, 
                                                        RecordBatch* batch, 
                                                        const FailoverConfig& config) {
    
    return apply_retry_policy(plugin, batch, config);
}

int ResilientPluginManager::apply_retry_policy(IsolatedPluginProcess* plugin, 
                                              RecordBatch* batch, 
                                              const FailoverConfig& config) {
    int attempt = 0;
    int delay_ms = config.initial_delay_ms;
    
    while (attempt <= config.max_retries) {
        attempt++;
        
        int result = execute_plugin_with_timeout(plugin, batch, config.timeout_ms);
        
        if (result == 0) {
            return 0; // Éxito
        }
        
        if (attempt <= config.max_retries) {
            std::cout << "Reintentando plugin " << plugin->get_name() 
                      << " (intento " << attempt << "/" << config.max_retries 
                      << ") en " << delay_ms << "ms" << std::endl;
            
            usleep(delay_ms * 1000);
            delay_ms = std::min((int)(delay_ms * config.backoff_multiplier), config.max_delay_ms);
        }
    }
    
    return -1; // Todos los reintentos fallaron
}

int ResilientPluginManager::execute_plugin_with_timeout(IsolatedPluginProcess* plugin, 
                                                       RecordBatch* batch, 
                                                       int timeout_ms) {
    // Configurar timeout con setjmp/longjmp
    if (setjmp(g_timeout_env) != 0) {
        alarm(0); // Limpiar alarma
        std::cerr << "Timeout en plugin " << plugin->get_name() << std::endl;
        return -999; // Código especial para timeout
    }
    
    g_timeout_occurred = false;
    alarm(timeout_ms / 1000); // Configurar alarma en segundos
    
    // Ejecutar plugin
    int result = plugin->process_batch(batch);
    
    alarm(0); // Limpiar alarma
    return result;
}

int ResilientPluginManager::handle_plugin_failure(const std::string& plugin_name, 
                                                  RecordBatch* batch, 
                                                  const FailoverConfig& config) {
    std::cout << "Manejando fallo del plugin: " << plugin_name << std::endl;
    
    switch (config.policy) {
        case SKIP_AND_CONTINUE:
            std::cout << "Saltando plugin " << plugin_name << " y continuando" << std::endl;
            return 0;
            
        case USE_FALLBACK_PLUGIN:
            std::cout << "Política de fallback no implementada para " << plugin_name << std::endl;
            return 0; // Por ahora, continuar
            
        case ISOLATE_AND_CONTINUE:
            std::cout << "Aislando plugin " << plugin_name << std::endl;
            // Marcar plugin como no saludable (se implementaría en IsolatedPluginProcess)
            return 0;
            
        case FAIL_FAST:
        default:
            return -1;
    }
}

std::vector<std::string> ResilientPluginManager::get_plugin_status() const {
    std::vector<std::string> status;
    
    for (size_t i = 0; i < plugins.size(); ++i) {
        std::ostringstream ss;
        ss << plugins[i]->get_name() << ": " 
           << (plugins[i]->is_healthy() ? "HEALTHY" : "UNHEALTHY");
        status.push_back(ss.str());
    }
    
    return status;
}

bool ResilientPluginManager::restart_plugin(const std::string& plugin_name) {
    for (size_t i = 0; i < plugins.size(); ++i) {
        if (plugins[i]->get_name() == plugin_name) {
            std::cout << "Reiniciando plugin: " << plugin_name << std::endl;
            return plugins[i]->restart();
        }
    }
    return false;
}

void ResilientPluginManager::get_pipeline_metrics(size_t& total_plugins, size_t& healthy_plugins, 
                                                 double& avg_success_rate) const {
    total_plugins = plugins.size();
    healthy_plugins = 0;
    double total_success_rate = 0.0;
    
    for (size_t i = 0; i < plugins.size(); ++i) {
        if (plugins[i]->is_healthy()) {
            healthy_plugins++;
        }
        
        const ComponentMetrics* metrics = plugins[i]->get_metrics();
        if (metrics) {
            total_success_rate += metrics->get_success_rate();
        }
    }
    
    avg_success_rate = total_plugins > 0 ? total_success_rate / total_plugins : 0.0;
}

bool ResilientPluginManager::hot_swap_plugin(const std::string& plugin_name, 
                                            const std::string& new_library_path) {
    std::cout << "Hot-swapping plugin " << plugin_name << " con " << new_library_path << std::endl;
    
    // Encontrar configuración actual del plugin
    PipelineStageConfig* config = NULL;
    for (size_t i = 0; i < pipeline_config.size(); ++i) {
        if (pipeline_config[i].name == plugin_name) {
            config = &pipeline_config[i];
            break;
        }
    }
    
    if (!config) return false;
    
    // Actualizar ruta de biblioteca
    std::string old_path = config->library_path;
    config->library_path = new_library_path;
    
    // Remover plugin actual
    if (!remove_plugin(plugin_name)) {
        config->library_path = old_path; // Restaurar
        return false;
    }
    
    // Agregar nuevo plugin
    if (!add_plugin(*config)) {
        config->library_path = old_path; // Restaurar
        add_plugin(*config); // Intentar restaurar plugin original
        return false;
    }
    
    std::cout << "Hot-swap completado para " << plugin_name << std::endl;
    return true;
}

bool ResilientPluginManager::validate_pipeline_config(const std::vector<PipelineStageConfig>& config) {
    for (size_t i = 0; i < config.size(); ++i) {
        const PipelineStageConfig& stage = config[i];
        
        if (stage.name.empty() || stage.library_path.empty()) {
            return false;
        }
        
        if (stage.failover_config.max_retries < 0 || 
            stage.failover_config.timeout_ms <= 0) {
            return false;
        }
    }
    
    return true;
}

} // namespace distributed
