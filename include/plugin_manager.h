#ifndef DISTRIBUTED_PLUGIN_MANAGER_H
#define DISTRIBUTED_PLUGIN_MANAGER_H

#include "interfaces.h"
#include "isolated_process.h"
#include "types.h"
#include <vector>
#include <string>

namespace distributed {

/**
 * @brief Configuración de failover para plugins
 */
struct FailoverConfig {
    FailoverPolicy policy;
    int max_retries;
    int initial_delay_ms;
    int max_delay_ms;
    double backoff_multiplier;
    int timeout_ms;
    std::string fallback_plugin_path;
    bool enable_circuit_breaker;

    FailoverConfig();
};

/**
 * @brief Configuración de una etapa del pipeline
 */
struct PipelineStageConfig {
    std::string name;
    std::string library_path;
    std::string parameters;
    bool enabled;
    FailoverConfig failover_config;

    PipelineStageConfig();
};

/**
 * @brief Gestor de plugins con capacidades de failover
 * 
 * Maneja la carga, ejecución y failover de plugins de procesamiento.
 * Implementa circuit breakers, timeouts y políticas de retry.
 */
class ResilientPluginManager {
private:
    std::vector<IsolatedPluginProcess*> plugins;
    std::vector<PipelineStageConfig> pipeline_config;
    IMemoryPool* memory_pool;

    /**
     * @brief Ejecutar plugin con manejo de timeouts
     */
    int execute_plugin_with_timeout(IsolatedPluginProcess* plugin, 
                                   RecordBatch* batch, 
                                   int timeout_ms);

    /**
     * @brief Aplicar política de retry
     */
    int apply_retry_policy(IsolatedPluginProcess* plugin, 
                          RecordBatch* batch, 
                          const FailoverConfig& config);

    /**
     * @brief Ejecutar plugin con manejo de fallos
     */
    int execute_plugin_with_failover(IsolatedPluginProcess* plugin, 
                          RecordBatch* batch, 
                          const FailoverConfig& config);

    /**
     * @brief Manejar fallo final de plugin
     */
    int handle_plugin_failure(const std::string& plugin_name, 
                             RecordBatch* batch, 
                             const FailoverConfig& config);

public:
    /**
     * @brief Constructor
     * @param memory_pool Pool de memoria para operaciones
     */
    ResilientPluginManager(IMemoryPool* memory_pool);

    virtual ~ResilientPluginManager();

    /**
     * @brief Cargar configuración del pipeline
     */
    bool load_pipeline_config(const std::vector<PipelineStageConfig>& config);

    /**
     * @brief Agregar plugin al pipeline
     */
    bool add_plugin(const PipelineStageConfig& config);

    /**
     * @brief Remover plugin del pipeline
     */
    bool remove_plugin(const std::string& plugin_name);

    /**
     * @brief Procesar lote a través de todo el pipeline
     */
    bool process_batch_through_pipeline(RecordBatch* batch);

    /**
     * @brief Obtener estado de todos los plugins
     */
    std::vector<std::string> get_plugin_status() const;

    /**
     * @brief Reiniciar plugin específico
     */
    bool restart_plugin(const std::string& plugin_name);

    /**
     * @brief Obtener métricas agregadas del pipeline
     */
    void get_pipeline_metrics(size_t& total_plugins, size_t& healthy_plugins, 
                             double& avg_success_rate) const;

    /**
     * @brief Hot-swap de plugin
     */
    bool hot_swap_plugin(const std::string& plugin_name, 
                        const std::string& new_library_path);

    /**
     * @brief Validar configuración del pipeline
     */
    static bool validate_pipeline_config(const std::vector<PipelineStageConfig>& config);
};

} // namespace distributed

#endif // DISTRIBUTED_PLUGIN_MANAGER_H
