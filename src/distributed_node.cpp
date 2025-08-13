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

// src/distributed_node.cpp
#include "distributed_node.h"
#include "serialization.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace distributed {

DistributedNode::DistributedNode(const std::string& id, const std::string& ip, int port)
    : node_id(id), local_ip(ip), local_port(port), server_socket(-1), server_active(true) {
    pthread_mutex_init(&cluster_mutex, NULL);
}

DistributedNode::~DistributedNode() {
    shutdown();
    pthread_mutex_destroy(&cluster_mutex);
}

bool DistributedNode::start() {
    return start_server();
}

bool DistributedNode::start_server() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Error creando socket servidor" << std::endl;
        return false;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(local_ip.c_str());
    server_addr.sin_port = htons(local_port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error en bind del socket servidor" << std::endl;
        close(server_socket);
        return false;
    }
    
    if (listen(server_socket, 10) == -1) {
        std::cerr << "Error en listen del socket servidor" << std::endl;
        close(server_socket);
        return false;
    }
    
    if (pthread_create(&server_thread, NULL, server_function, this) != 0) {
        std::cerr << "Error creando hilo servidor" << std::endl;
        close(server_socket);
        return false;
    }
    
    std::cout << "Nodo distribuido iniciado: " << node_id 
              << " en " << local_ip << ":" << local_port << std::endl;
    return true;
}

void DistributedNode::shutdown() {
    server_active = false;
    
    if (server_socket != -1) {
        close(server_socket);
        pthread_join(server_thread, NULL);
    }
}

bool DistributedNode::join_cluster(const std::string& seed_ip, int seed_port) {
    std::cout << "Intentando unirse al cluster via " << seed_ip << ":" << seed_port << std::endl;
    
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) return false;
    
    struct sockaddr_in seed_addr;
    seed_addr.sin_family = AF_INET;
    seed_addr.sin_addr.s_addr = inet_addr(seed_ip.c_str());
    seed_addr.sin_port = htons(seed_port);
    
    if (connect(client_socket, (struct sockaddr*)&seed_addr, sizeof(seed_addr)) == 0) {
        // Enviar solicitud de descubrimiento
        IPCMessage discovery_msg;
        discovery_msg.type = IPCMessage::NODE_DISCOVERY;
        discovery_msg.sender_id = 0;
        discovery_msg.receiver_id = 0;
        discovery_msg.data_size = node_id.length();
        
        char* msg_buffer = (char*)malloc(sizeof(IPCMessage) + discovery_msg.data_size);
        memcpy(msg_buffer, &discovery_msg, sizeof(IPCMessage));
        memcpy(msg_buffer + sizeof(IPCMessage), node_id.c_str(), node_id.length());
        
        send(client_socket, msg_buffer, sizeof(IPCMessage) + discovery_msg.data_size, 0);
        free(msg_buffer);
        
        // Recibir información del cluster
        char response_buffer[4096];
        ssize_t received = recv(client_socket, response_buffer, sizeof(response_buffer), 0);
        
        if (received > 0) {
            parse_cluster_info(response_buffer, received);
            std::cout << "Unido al cluster exitosamente" << std::endl;
        }
        
        close(client_socket);
        return true;
    }
    
    close(client_socket);
    return false;
}

std::string DistributedNode::select_best_node_for_task() {
    pthread_mutex_lock(&cluster_mutex);
    
    std::string best_node;
    int lowest_load = 101;
    
    std::map<std::string, NodeInfo>::iterator it;
    for (it = cluster_nodes.begin(); it != cluster_nodes.end(); ++it) {
        if (it->second.is_alive && it->second.load_factor < lowest_load) {
            lowest_load = it->second.load_factor;
            best_node = it->first;
        }
    }
    
    pthread_mutex_unlock(&cluster_mutex);
    
    return best_node.empty() ? node_id : best_node;
}

bool DistributedNode::send_batch_to_node(const std::string& target_node, RecordBatch* batch) {
    if (target_node == node_id) return false;
    
    pthread_mutex_lock(&cluster_mutex);
    std::map<std::string, NodeInfo>::iterator it = cluster_nodes.find(target_node);
    if (it == cluster_nodes.end()) {
        pthread_mutex_unlock(&cluster_mutex);
        return false;
    }
    
    NodeInfo target_info = it->second;
    pthread_mutex_unlock(&cluster_mutex);
    
    // Conectar al nodo target
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) return false;
    
    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = inet_addr(target_info.ip_address.c_str());
    target_addr.sin_port = htons(target_info.port);
    
    if (connect(client_socket, (struct sockaddr*)&target_addr, sizeof(target_addr)) == 0) {
        // Serializar y enviar batch
        char serialized_buffer[64 * 1024];
        size_t serialized_size = Serializer::serialize_batch(batch, serialized_buffer, sizeof(serialized_buffer));
        
        if (serialized_size > 0) {
            IPCMessage batch_msg;
            batch_msg.type = IPCMessage::PROCESS_BATCH;
            batch_msg.sender_id = 0;
            batch_msg.receiver_id = 0;
            batch_msg.data_size = serialized_size;
            
            send(client_socket, &batch_msg, sizeof(IPCMessage), 0);
            send(client_socket, serialized_buffer, serialized_size, 0);
            
            // Recibir respuesta
            char response_buffer[64 * 1024];
            ssize_t received = recv(client_socket, response_buffer, sizeof(response_buffer), 0);
            
            if (received > 0) {
                Serializer::deserialize_batch(response_buffer, batch);
                close(client_socket);
                return true;
            }
        }
    }
    
    close(client_socket);
    return false;
}

bool DistributedNode::process_batch_distributed(RecordBatch* batch) {
    std::string target_node = select_best_node_for_task();
    
    if (target_node != node_id) {
        std::cout << "Enviando batch " << batch->batch_id << " a nodo " << target_node << std::endl;
        return send_batch_to_node(target_node, batch);
    } else {
        std::cout << "Procesando batch " << batch->batch_id << " localmente" << std::endl;
        return true; // Procesamiento local simplificado
    }
}

std::vector<NodeInfo> DistributedNode::get_all_nodes() const {
    pthread_mutex_lock(&cluster_mutex);
    
    std::vector<NodeInfo> nodes;
    std::map<std::string, NodeInfo>::const_iterator it;
    for (it = cluster_nodes.begin(); it != cluster_nodes.end(); ++it) {
        nodes.push_back(it->second);
    }
    
    pthread_mutex_unlock(&cluster_mutex);
    return nodes;
}

bool DistributedNode::ping_node(const std::string& target_node_id) {
    pthread_mutex_lock(&cluster_mutex);
    std::map<std::string, NodeInfo>::iterator it = cluster_nodes.find(target_node_id);
    if (it == cluster_nodes.end()) {
        pthread_mutex_unlock(&cluster_mutex);
        return false;
    }
    
    NodeInfo target_info = it->second;
    pthread_mutex_unlock(&cluster_mutex);
    
    // Ping simple via socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) return false;
    
    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = inet_addr(target_info.ip_address.c_str());
    target_addr.sin_port = htons(target_info.port);
    
    bool result = (connect(client_socket, (struct sockaddr*)&target_addr, sizeof(target_addr)) == 0);
    close(client_socket);
    
    return result;
}

void DistributedNode::print_cluster_status() const {
    pthread_mutex_lock(&cluster_mutex);
    
    std::cout << "\n=== Estado del Cluster ===" << std::endl;
    std::cout << "Nodo local: " << node_id << " (" << local_ip << ":" << local_port << ")" << std::endl;
    std::cout << "Nodos en el cluster:" << std::endl;
    
    std::map<std::string, NodeInfo>::const_iterator it;
    for (it = cluster_nodes.begin(); it != cluster_nodes.end(); ++it) {
        std::cout << "  " << it->first << " - " << it->second.ip_address 
                  << ":" << it->second.port << " (Load: " << it->second.load_factor 
                  << "%, Alive: " << (it->second.is_alive ? "Yes" : "No") << ")" << std::endl;
    }
    
    pthread_mutex_unlock(&cluster_mutex);
}

void DistributedNode::get_cluster_metrics(size_t& total_nodes, size_t& active_nodes, 
                                        double& avg_load) const {
    pthread_mutex_lock(&cluster_mutex);
    
    total_nodes = cluster_nodes.size();
    active_nodes = 0;
    double total_load = 0.0;
    
    std::map<std::string, NodeInfo>::const_iterator it;
    for (it = cluster_nodes.begin(); it != cluster_nodes.end(); ++it) {
        if (it->second.is_alive) {
            active_nodes++;
            total_load += it->second.load_factor;
        }
    }
    
    avg_load = active_nodes > 0 ? total_load / active_nodes : 0.0;
    
    pthread_mutex_unlock(&cluster_mutex);
}

void* DistributedNode::server_function(void* arg) {
    DistributedNode* node = static_cast<DistributedNode*>(arg);
    
    std::cout << "Servidor de nodo iniciado para " << node->node_id << std::endl;
    
    while (node->server_active) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(node->server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            if (node->server_active) {
                std::cerr << "Error en accept()" << std::endl;
            }
            continue;
        }
        
        node->handle_client(client_socket);
        close(client_socket);
    }
    
    std::cout << "Servidor de nodo terminado para " << node->node_id << std::endl;
    return NULL;
}

void DistributedNode::handle_client(int client_socket) {
    IPCMessage msg_header;
    ssize_t received = recv(client_socket, &msg_header, sizeof(IPCMessage), 0);
    
    if (received != sizeof(IPCMessage)) return;
    
    if (msg_header.type == IPCMessage::NODE_DISCOVERY) {
        send_cluster_info(client_socket);
    } else if (msg_header.type == IPCMessage::PROCESS_BATCH) {
        handle_distributed_batch(client_socket, msg_header.data_size);
    }
}

void DistributedNode::send_cluster_info(int client_socket) {
    std::ostringstream cluster_info;
    cluster_info << node_id << "," << local_ip << "," << local_port << "\n";
    
    pthread_mutex_lock(&cluster_mutex);
    std::map<std::string, NodeInfo>::iterator it;
    for (it = cluster_nodes.begin(); it != cluster_nodes.end(); ++it) {
        if (it->second.is_alive) {
            cluster_info << it->first << "," << it->second.ip_address 
                        << "," << it->second.port << "\n";
        }
    }
    pthread_mutex_unlock(&cluster_mutex);
    
    std::string info_str = cluster_info.str();
    send(client_socket, info_str.c_str(), info_str.length(), 0);
}

void DistributedNode::handle_distributed_batch(int client_socket, size_t data_size) {
    char* batch_buffer = (char*)malloc(data_size);
    ssize_t received = recv(client_socket, batch_buffer, data_size, 0);
    
    if (received == (ssize_t)data_size) {
        // Procesar batch localmente (simplificado)
        send(client_socket, batch_buffer, data_size, 0);
    }
    
    free(batch_buffer);
}

void DistributedNode::parse_cluster_info(const char* buffer, size_t size) {
    std::string info(buffer, size);
    std::istringstream stream(info);
    std::string line;
    
    pthread_mutex_lock(&cluster_mutex);
    
    while (std::getline(stream, line)) {
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        
        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            NodeInfo node_info;
            node_info.node_id = line.substr(0, pos1);
            node_info.ip_address = line.substr(pos1 + 1, pos2 - pos1 - 1);
            node_info.port = atoi(line.substr(pos2 + 1).c_str());
            node_info.is_alive = true;
            node_info.last_seen = time(NULL);
            node_info.load_factor = 50; // Valor por defecto
            
            if (node_info.node_id != node_id) {
                cluster_nodes[node_info.node_id] = node_info;
            }
        }
    }
    
    pthread_mutex_unlock(&cluster_mutex);
}

} // namespace distributed
