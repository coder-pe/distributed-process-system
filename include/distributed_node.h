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

#ifndef DISTRIBUTED_NODE_H
#define DISTRIBUTED_NODE_H

#include "interfaces.h"
#include "types.h"
#include "ipc.h"
#include <map>
#include <string>
#include <pthread.h>
#include <cstring>
#include <unistd.h>

namespace distributed {

/**
 * @brief Nodo distribuido que participa en un cluster
 * 
 * Implementa descubrimiento automático de nodos, load balancing,
 * y comunicación transparente entre nodos del cluster.
 */
class DistributedNode : public IDistributedNode {
private:
    std::string node_id;
    std::string local_ip;
    int local_port;
    std::map<std::string, NodeInfo> cluster_nodes;
    mutable pthread_mutex_t cluster_mutex;

    // Servidor para recibir conexiones
    int server_socket;
    pthread_t server_thread;
    volatile bool server_active;

    /**
     * @brief Función del hilo servidor
     */
    static void* server_function(void* arg);

    /**
     * @brief Manejar cliente conectado
     */
    void handle_client(int client_socket);

    /**
     * @brief Enviar información del cluster
     */
    void send_cluster_info(int client_socket);

    /**
     * @brief Manejar batch distribuido
     */
    void handle_distributed_batch(int client_socket, size_t data_size);

    /**
     * @brief Parsear información del cluster
     */
    void parse_cluster_info(const char* buffer, size_t size);

    /**
     * @brief Actualizar métricas de carga de nodos
     */
    void update_node_load_metrics();

public:
    /**
     * @brief Constructor
     * @param id ID único del nodo
     * @param ip Dirección IP del nodo
     * @param port Puerto del nodo
     */
    DistributedNode(const std::string& id, const std::string& ip, int port);

    virtual ~DistributedNode();

    /**
     * @brief Iniciar servidor del nodo
     */
    bool start_server();

    /**
     * @brief Parar servidor del nodo
     */
    void shutdown();

    /**
     * @brief Seleccionar mejor nodo para una tarea
     */
    std::string select_best_node_for_task();

    /**
     * @brief Enviar batch a nodo específico
     */
    bool send_batch_to_node(const std::string& target_node, RecordBatch* batch);

    /**
     * @brief Obtener información de todos los nodos
     */
    std::vector<NodeInfo> get_all_nodes() const;

    /**
     * @brief Verificar conectividad con nodo específico
     */
    bool ping_node(const std::string& node_id);

    // Implementación de IDistributedNode
    virtual bool start();
    virtual bool join_cluster(const std::string& seed_ip, int seed_port);
    virtual bool process_batch_distributed(RecordBatch* batch);
    virtual const std::string& get_node_id() const { return node_id; }
    virtual void print_cluster_status() const;

    /**
     * @brief Obtener métricas del cluster
     */
    void get_cluster_metrics(size_t& total_nodes, size_t& active_nodes, 
                           double& avg_load) const;
};

} // namespace distributed

#endif // DISTRIBUTED_NODE_H
