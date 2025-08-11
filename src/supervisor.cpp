// =============================================================================
// IMPLEMENTACIÓN DEL SUPERVISOR
// =============================================================================

// src/supervisor.cpp
#include "supervisor.h"
#include "isolated_process.h"
#include <iostream>
#include <algorithm>

namespace distributed {

SupervisorSpec::SupervisorSpec() 
    : restart_policy(ONE_FOR_ONE), max_restarts(5), restart_period(60), shutdown_timeout(10) {}

ProcessSupervisor::ProcessSupervisor(const std::string& name, const SupervisorSpec& supervisor_spec)
    : spec(supervisor_spec), monitoring_active(true), supervisor_name(name) {
    
    pthread_mutex_init(&supervisor_mutex, NULL);
    
    if (pthread_create(&monitor_thread, NULL, monitor_function, this) != 0) {
        std::cerr << "Error creando hilo monitor para supervisor " << name << std::endl;
    }
}

ProcessSupervisor::~ProcessSupervisor() {
    monitoring_active = false;
    pthread_join(monitor_thread, NULL);
    
    // Terminar todos los componentes supervisados
    for (size_t i = 0; i < supervised_components.size(); ++i) {
        delete supervised_components[i];
    }
    
    // Terminar supervisores hijos
    for (size_t i = 0; i < child_supervisors.size(); ++i) {
        delete child_supervisors[i];
    }
    
    pthread_mutex_destroy(&supervisor_mutex);
}

void ProcessSupervisor::add_component(IProcessingComponent* component) {
    if (!component) return;
    
    pthread_mutex_lock(&supervisor_mutex);
    supervised_components.push_back(component);
    pthread_mutex_unlock(&supervisor_mutex);
    
    std::cout << "Componente agregado a supervisor " << supervisor_name 
              << ": " << component->get_name() << std::endl;
}

void ProcessSupervisor::add_child_supervisor(ProcessSupervisor* child) {
    if (!child) return;
    
    pthread_mutex_lock(&supervisor_mutex);
    child_supervisors.push_back(child);
    pthread_mutex_unlock(&supervisor_mutex);
    
    std::cout << "Supervisor hijo agregado a " << supervisor_name << std::endl;
}

bool ProcessSupervisor::start_all_components() {
    pthread_mutex_lock(&supervisor_mutex);
    
    bool all_started = true;
    for (size_t i = 0; i < supervised_components.size(); ++i) {
        IsolatedPluginProcess* process = dynamic_cast<IsolatedPluginProcess*>(supervised_components[i]);
        if (process && !process->start()) {
            std::cerr << "Error iniciando componente " << process->get_name() << std::endl;
            all_started = false;
        }
    }
    
    pthread_mutex_unlock(&supervisor_mutex);
    return all_started;
}

void ProcessSupervisor::stop_all_components() {
    pthread_mutex_lock(&supervisor_mutex);
    
    for (size_t i = 0; i < supervised_components.size(); ++i) {
        IsolatedPluginProcess* process = dynamic_cast<IsolatedPluginProcess*>(supervised_components[i]);
        if (process) {
            process->terminate();
        }
    }
    
    pthread_mutex_unlock(&supervisor_mutex);
}

void ProcessSupervisor::handle_component_death(const std::string& component_name) {
    pthread_mutex_lock(&supervisor_mutex);
    
    std::cout << "Supervisor " << supervisor_name 
              << " manejando muerte del componente " << component_name << std::endl;
    
    // Encontrar el componente que murió
    int dead_index = -1;
    for (size_t i = 0; i < supervised_components.size(); ++i) {
        if (supervised_components[i]->get_name() == component_name) {
            dead_index = i;
            break;
        }
    }
    
    if (dead_index == -1) {
        pthread_mutex_unlock(&supervisor_mutex);
        return;
    }
    
    // Verificar si se debe reiniciar
    if (!should_restart(component_name)) {
        std::cout << "Componente " << component_name << " no será reiniciado (límite alcanzado)" << std::endl;
        pthread_mutex_unlock(&supervisor_mutex);
        return;
    }
    
    // Aplicar política de restart
    switch (spec.restart_policy) {
        case ONE_FOR_ONE:
            restart_component(component_name);
            break;
            
        case ONE_FOR_ALL:
            restart_all_components();
            break;
            
        case REST_FOR_ONE:
            restart_remaining_components(dead_index);
            break;
    }
    
    // Registrar el restart
    restart_history[component_name] = time(NULL);
    
    pthread_mutex_unlock(&supervisor_mutex);
}

bool ProcessSupervisor::should_restart(const std::string& component_name) {
    time_t now = time(NULL);
    
    // Contar restarts recientes para este componente
    int recent_restarts = 0;
    std::map<std::string, time_t>::iterator it = restart_history.find(component_name);
    if (it != restart_history.end()) {
        if (now - it->second <= spec.restart_period) {
            recent_restarts = 1; // Simplificado para esta demo
        }
    }
    
    return recent_restarts < spec.max_restarts;
}

void ProcessSupervisor::restart_component(const std::string& component_name) {
    for (size_t i = 0; i < supervised_components.size(); ++i) {
        if (supervised_components[i]->get_name() == component_name) {
            std::cout << "Reiniciando componente: " << component_name << std::endl;
            
            IsolatedPluginProcess* process = dynamic_cast<IsolatedPluginProcess*>(supervised_components[i]);
            if (process) {
                process->restart();
            }
            break;
        }
    }
}

void ProcessSupervisor::restart_all_components() {
    std::cout << "Reiniciando todos los componentes en supervisor " << supervisor_name << std::endl;
    
    for (size_t i = 0; i < supervised_components.size(); ++i) {
        IsolatedPluginProcess* process = dynamic_cast<IsolatedPluginProcess*>(supervised_components[i]);
        if (process) {
            process->restart();
        }
    }
}

void ProcessSupervisor::restart_remaining_components(size_t from_index) {
    std::cout << "Reiniciando componentes desde índice " << from_index 
              << " en supervisor " << supervisor_name << std::endl;
    
    for (size_t i = from_index; i < supervised_components.size(); ++i) {
        IsolatedPluginProcess* process = dynamic_cast<IsolatedPluginProcess*>(supervised_components[i]);
        if (process) {
            process->restart();
        }
    }
}

size_t ProcessSupervisor::get_component_count() const {
    pthread_mutex_lock(&supervisor_mutex);
    size_t count = supervised_components.size();
    pthread_mutex_unlock(&supervisor_mutex);
    return count;
}

void ProcessSupervisor::print_supervision_tree(int depth) const {
    std::string indent(depth * 2, ' ');
    
    std::cout << indent << "Supervisor: " << supervisor_name 
              << " (Policy: " << policy_to_string(spec.restart_policy) << ")" << std::endl;
    
    pthread_mutex_lock(&supervisor_mutex);
    
    for (size_t i = 0; i < supervised_components.size(); ++i) {
        std::cout << indent << "  Component: " << supervised_components[i]->get_name()
                  << " (Healthy: " << (supervised_components[i]->is_healthy() ? "Yes" : "No") << ")" << std::endl;
    }
    
    for (size_t i = 0; i < child_supervisors.size(); ++i) {
        child_supervisors[i]->print_supervision_tree(depth + 1);
    }
    
    pthread_mutex_unlock(&supervisor_mutex);
}

void* ProcessSupervisor::monitor_function(void* arg) {
    ProcessSupervisor* supervisor = static_cast<ProcessSupervisor*>(arg);
    
    std::cout << "Monitor iniciado para supervisor " << supervisor->supervisor_name << std::endl;
    
    while (supervisor->monitoring_active) {
        sleep(5); // Chequeo cada 5 segundos
        
        pthread_mutex_lock(&supervisor->supervisor_mutex);
        
        // Verificar estado de componentes supervisados
        for (size_t i = 0; i < supervisor->supervised_components.size(); ++i) {
            IProcessingComponent* component = supervisor->supervised_components[i];
            
            if (!component->is_healthy()) {
                std::string component_name = component->get_name();
                pthread_mutex_unlock(&supervisor->supervisor_mutex);
                
                supervisor->handle_component_death(component_name);
                
                pthread_mutex_lock(&supervisor->supervisor_mutex);
                break; // Reiniciar loop para evitar invalidar iteradores
            }
        }
        
        pthread_mutex_unlock(&supervisor->supervisor_mutex);
    }
    
    std::cout << "Monitor terminado para supervisor " << supervisor->supervisor_name << std::endl;
    return NULL;
}

const char* ProcessSupervisor::policy_to_string(RestartPolicy policy) {
    switch (policy) {
        case ONE_FOR_ONE: return "one_for_one";
        case ONE_FOR_ALL: return "one_for_all";
        case REST_FOR_ONE: return "rest_for_one";
        default: return "unknown";
    }
}

void ProcessSupervisor::get_statistics(size_t& total_components, size_t& healthy_components, 
                                     size_t& total_restarts) const {
    pthread_mutex_lock(&supervisor_mutex);
    
    total_components = supervised_components.size();
    healthy_components = 0;
    total_restarts = restart_history.size();
    
    for (size_t i = 0; i < supervised_components.size(); ++i) {
        if (supervised_components[i]->is_healthy()) {
            healthy_components++;
        }
    }
    
    pthread_mutex_unlock(&supervisor_mutex);
}

} // namespace distributed
