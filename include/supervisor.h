/*
 * Copyright (C) 2025 Miguel Mamani <miguel.coder.per@gmail.com>
 *
 * This file is part of the Distributed Processing System.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DISTRIBUTED_SUPERVISOR_H
#define DISTRIBUTED_SUPERVISOR_H

#include "interfaces.h"
#include "types.h"
#include <vector>
#include <map>
#include <string>
#include <pthread.h>
#include <unistd.h>

namespace distributed {

/**
 * @brief Especificación de un supervisor
 */
struct SupervisorSpec {
    RestartPolicy restart_policy;
    int max_restarts;      ///< Máximo número de restarts en el período
    int restart_period;    ///< Período de tiempo en segundos
    int shutdown_timeout;  ///< Timeout para shutdown en segundos

    SupervisorSpec();
};

/**
 * @brief Supervisor que implementa supervision trees como Erlang/OTP
 * 
 * Mantiene un conjunto de componentes bajo supervisión y aplica
 * políticas de restart cuando fallan. Soporta jerarquías de supervisores.
 */
class ProcessSupervisor : public ISupervisor {
private:
    std::vector<IProcessingComponent*> supervised_components;
    std::vector<ProcessSupervisor*> child_supervisors;
    SupervisorSpec spec;
    std::map<std::string, time_t> restart_history;
    mutable pthread_mutex_t supervisor_mutex;
    pthread_t monitor_thread;
    volatile bool monitoring_active;
    std::string supervisor_name;

    /**
     * @brief Función del hilo monitor
     */
    static void* monitor_function(void* arg);

    /**
     * @brief Verificar si se debe reiniciar un componente
     */
    bool should_restart(const std::string& component_name);

    /**
     * @brief Reiniciar un componente específico
     */
    void restart_component(const std::string& component_name);

    /**
     * @brief Reiniciar todos los componentes
     */
    void restart_all_components();

    /**
     * @brief Reiniciar componentes desde un índice
     */
    void restart_remaining_components(size_t from_index);

    /**
     * @brief Convertir política a string
     */
    static const char* policy_to_string(RestartPolicy policy);

public:
    /**
     * @brief Constructor
     * @param name Nombre del supervisor
     * @param supervisor_spec Especificación de comportamiento
     */
    ProcessSupervisor(const std::string& name, const SupervisorSpec& supervisor_spec);

    virtual ~ProcessSupervisor();

    /**
     * @brief Agregar supervisor hijo
     */
    void add_child_supervisor(ProcessSupervisor* child);

    /**
     * @brief Iniciar todos los componentes supervisados
     */
    bool start_all_components();

    /**
     * @brief Parar todos los componentes supervisados
     */
    void stop_all_components();

    /**
     * @brief Obtener especificación del supervisor
     */
    const SupervisorSpec& get_spec() const { return spec; }

    /**
     * @brief Actualizar especificación del supervisor
     */
    void update_spec(const SupervisorSpec& new_spec);

    // Implementación de ISupervisor
    virtual void add_component(IProcessingComponent* component);
    virtual void handle_component_death(const std::string& component_name);
    virtual size_t get_component_count() const;
    virtual void print_supervision_tree(int depth = 0) const;

    /**
     * @brief Obtener estadísticas del supervisor
     */
    void get_statistics(size_t& total_components, size_t& healthy_components, 
                       size_t& total_restarts) const;
};

} // namespace distributed

#endif // DISTRIBUTED_SUPERVISOR_H
