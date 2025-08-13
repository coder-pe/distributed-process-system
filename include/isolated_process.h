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

#ifndef DISTRIBUTED_ISOLATED_PROCESS_H
#define DISTRIBUTED_ISOLATED_PROCESS_H

#include "interfaces.h"
#include "ipc.h"
#include "types.h"
#include <sys/types.h>
#include <string>

namespace distributed {

/**
 * @brief Proceso aislado para ejecutar plugins de forma segura
 * 
 * Cada plugin ejecuta en su propio proceso con memoria completamente
 * aislada. Comunicación via IPC y shared memory para performance.
 */
class IsolatedPluginProcess : public IProcessingComponent {
private:
    pid_t process_id;
    std::string plugin_name;
    std::string library_path;
    std::string config_params;
    IPCChannel* parent_channel;
    IPCChannel* child_channel;
    SharedMemoryRegion* shared_memory;
    bool is_running;
    time_t last_heartbeat;
    ComponentMetrics metrics;

    /**
     * @brief Función que ejecuta el proceso hijo
     */
    void execute_plugin_process();

    /**
     * @brief Cargar biblioteca del plugin en proceso hijo
     */
    bool load_plugin_library();

public:
    /**
     * @brief Constructor
     * @param name Nombre del plugin
     * @param lib_path Ruta a la biblioteca compartida
     * @param params Parámetros de configuración
     */
    IsolatedPluginProcess(const std::string& name, 
                         const std::string& lib_path, 
                         const std::string& params);

    virtual ~IsolatedPluginProcess();

    /**
     * @brief Iniciar el proceso aislado
     */
    bool start();

    /**
     * @brief Terminar el proceso aislado
     */
    void terminate();

    /**
     * @brief Verificar si el proceso está vivo
     */
    bool is_alive() const;

    /**
     * @brief Obtener PID del proceso
     */
    pid_t get_pid() const { return process_id; }

    // Implementación de IProcessingComponent
    virtual int process_batch(RecordBatch* batch);
    virtual const std::string& get_name() const { return plugin_name; }
    virtual bool is_healthy() const { return is_alive(); }
    virtual const ComponentMetrics* get_metrics() const { return &metrics; }

    /**
     * @brief Enviar heartbeat al proceso
     */
    bool send_heartbeat();

    /**
     * @brief Reiniciar proceso si ha fallado
     */
    bool restart();
};

} // namespace distributed

#endif // DISTRIBUTED_ISOLATED_PROCESS_H
