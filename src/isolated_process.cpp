// =============================================================================
// IMPLEMENTACIONES RESTANTES DEL SISTEMA MODULARIZADO
// =============================================================================

// src/isolated_process.cpp
#include <sys/time.h>
#include "types.h"
#include "isolated_process.h"
#include "serialization.h"
#include <dlfcn.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <cstring>

namespace distributed {

IsolatedPluginProcess::IsolatedPluginProcess(const std::string& name, 
                                           const std::string& lib_path, 
                                           const std::string& params)
    : process_id(-1), plugin_name(name), library_path(lib_path), config_params(params),
      parent_channel(NULL), child_channel(NULL), shared_memory(NULL), is_running(false) {
    last_heartbeat = time(NULL);
}

IsolatedPluginProcess::~IsolatedPluginProcess() {
    terminate();
    delete parent_channel;
    delete child_channel;
    delete shared_memory;
}

bool IsolatedPluginProcess::start() {
    // Crear canales de comunicación
    parent_channel = new IPCChannel();
    child_channel = new IPCChannel();
    
    if (!parent_channel->create_pipe() || !child_channel->create_pipe()) {
        std::cerr << "Error creando pipes para " << plugin_name << std::endl;
        return false;
    }
    
    // Crear shared memory
    std::ostringstream shm_name;
    shm_name << "/plugin_" << plugin_name << "_" << getpid();
    shared_memory = new SharedMemoryRegion(shm_name.str(), 1024 * 1024); // 1MB
    
    if (!shared_memory->is_valid()) {
        std::cerr << "Error creando shared memory para " << plugin_name << std::endl;
        return false;
    }
    
    // Fork proceso
    process_id = fork();
    
    if (process_id == 0) {
        // Proceso hijo
        execute_plugin_process();
        exit(0);
    } else if (process_id > 0) {
        // Proceso padre
        is_running = true;
        std::cout << "Plugin process iniciado: " << plugin_name 
                  << " (PID: " << process_id << ")" << std::endl;
        return true;
    } else {
        std::cerr << "Error en fork() para " << plugin_name << std::endl;
        return false;
    }
}

void IsolatedPluginProcess::terminate() {
    if (is_running && process_id > 0) {
        // Enviar señal de shutdown
        IPCMessage msg;
        msg.type = IPCMessage::SHUTDOWN;
        msg.sender_id = getpid();
        msg.receiver_id = process_id;
        msg.data_size = 0;
        
        parent_channel->send_message(&msg);
        
        // Esperar y force kill si es necesario
        sleep(1);
        
        int status;
        if (waitpid(process_id, &status, WNOHANG) == 0) {
            kill(process_id, SIGTERM);
            sleep(1);
            if (waitpid(process_id, &status, WNOHANG) == 0) {
                kill(process_id, SIGKILL);
                waitpid(process_id, &status, 0);
            }
        }
        
        is_running = false;
        std::cout << "Plugin process terminado: " << plugin_name << std::endl;
    }
    
    // Cleanup shared memory
    if (shared_memory) {
        std::ostringstream shm_name;
        shm_name << "/plugin_" << plugin_name << "_" << getpid();
        SharedMemoryRegion::cleanup(shm_name.str());
    }
}

bool IsolatedPluginProcess::is_alive() const {
    if (!is_running || process_id <= 0) return false;
    
    // Verificar si el proceso existe
    if (kill(process_id, 0) != 0) {
        return false;
    }
    
    // Verificar heartbeat
    time_t now = time(NULL);
    return (now - last_heartbeat) < 60; // 60 segundos timeout
}

int IsolatedPluginProcess::process_batch(RecordBatch* batch) {
    if (!is_running || !batch) return -1;
    
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    // Serializar batch a shared memory
    char* shm_ptr = (char*)shared_memory->get_memory();
    size_t serialized_size = Serializer::serialize_batch(batch, shm_ptr, shared_memory->get_size());
    
    if (serialized_size == 0) {
        metrics.record_failure(0.0);
        return -1;
    }
    
    // Enviar mensaje de procesamiento
    IPCMessage msg;
    msg.type = IPCMessage::PROCESS_BATCH;
    msg.sender_id = getpid();
    msg.receiver_id = process_id;
    msg.data_size = sizeof(size_t);
    
    char* msg_buffer = (char*)malloc(sizeof(IPCMessage) + msg.data_size);
    memcpy(msg_buffer, &msg, sizeof(IPCMessage));
    memcpy(msg_buffer + sizeof(IPCMessage), &serialized_size, sizeof(size_t));
    
    bool sent = parent_channel->send_message((IPCMessage*)msg_buffer);
    free(msg_buffer);
    
    if (!sent) {
        metrics.record_failure(0.0);
        return -1;
    }
    
    // Esperar respuesta
    IPCMessage* response = NULL;
    if (child_channel->receive_message(&response, 1024)) {
        if (response->type == IPCMessage::BATCH_RESULT) {
            // Deserializar resultado
            bool success = Serializer::deserialize_batch(shm_ptr, batch);
            
            gettimeofday(&end_time, NULL);
            double execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                                  (end_time.tv_usec - start_time.tv_usec) / 1000.0;
            
            if (success) {
                metrics.record_success(execution_time);
                last_heartbeat = time(NULL);
            } else {
                metrics.record_failure(execution_time);
            }
            
            free(response);
            return success ? 0 : -1;
        }
        free(response);
    }
    
    gettimeofday(&end_time, NULL);
    double execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                          (end_time.tv_usec - start_time.tv_usec) / 1000.0;
    metrics.record_failure(execution_time);
    
    return -1;
}

void IsolatedPluginProcess::execute_plugin_process() {
    std::cout << "Proceso plugin iniciado: " << plugin_name << " (PID: " << getpid() << ")" << std::endl;
    
    // Cargar biblioteca del plugin
    void* lib_handle = dlopen(library_path.c_str(), RTLD_LAZY);
    if (!lib_handle) {
        std::cerr << "Error cargando " << library_path << ": " << dlerror() << std::endl;
        return;
    }
    
    // Obtener funciones del plugin
    typedef int (*ProcessBatchFunc)(RecordBatch* batch, void* context);
    ProcessBatchFunc process_func = (ProcessBatchFunc) dlsym(lib_handle, "process_batch");
    
    if (!process_func) {
        std::cerr << "Función process_batch no encontrada en " << library_path << std::endl;
        dlclose(lib_handle);
        return;
    }
    
    // Preparar espacio de trabajo
    char* shm_ptr = (char*)shared_memory->get_memory();
    RecordBatch working_batch;
    working_batch.records = (DatabaseRecord*)(shm_ptr + 1024); // Offset para metadata
    working_batch.capacity = (shared_memory->get_size() - 1024) / sizeof(DatabaseRecord);
    
    // Loop principal del proceso
    while (true) {
        IPCMessage* msg = NULL;
        if (parent_channel->receive_message(&msg, 1024)) {
            if (msg->type == IPCMessage::SHUTDOWN) {
                free(msg);
                break;
            } else if (msg->type == IPCMessage::PROCESS_BATCH) {
                // Deserializar y procesar batch
                if (Serializer::deserialize_batch(shm_ptr, &working_batch)) {
                    int result = process_func(&working_batch, NULL);
                    
                    // Serializar resultado
                    Serializer::serialize_batch(&working_batch, shm_ptr, shared_memory->get_size());
                    
                    // Enviar respuesta
                    IPCMessage response;
                    response.type = IPCMessage::BATCH_RESULT;
                    response.sender_id = getpid();
                    response.receiver_id = msg->sender_id;
                    response.data_size = sizeof(int);
                    
                    char* resp_buffer = (char*)malloc(sizeof(IPCMessage) + sizeof(int));
                    memcpy(resp_buffer, &response, sizeof(IPCMessage));
                    memcpy(resp_buffer + sizeof(IPCMessage), &result, sizeof(int));
                    
                    child_channel->send_message((IPCMessage*)resp_buffer);
                    free(resp_buffer);
                }
            }
            free(msg);
        } else {
            usleep(10000); // 10ms
        }
    }
    
    dlclose(lib_handle);
    std::cout << "Proceso plugin terminado: " << plugin_name << std::endl;
}

bool IsolatedPluginProcess::send_heartbeat() {
    if (!is_running) return false;
    
    IPCMessage msg;
    msg.type = IPCMessage::HEALTH_CHECK;
    msg.sender_id = getpid();
    msg.receiver_id = process_id;
    msg.data_size = 0;
    
    return parent_channel->send_message(&msg);
}

bool IsolatedPluginProcess::restart() {
    if (is_running) {
        terminate();
    }
    return start();
}

} // namespace distributed
