// =============================================================================
// ARCHIVO PRINCIPAL
// =============================================================================

// src/main.cpp
#include "types.h"
#include "distributed_system.h"
#include <iostream>
#include <cstdlib>
#include <signal.h>

using namespace distributed;

// Sistema global para manejo de señales
DistributedProcessingSystem* g_system = NULL;

void signal_handler(int signum) {
    std::cout << "Recibida señal " << signum << ", iniciando shutdown..." << std::endl;
    if (g_system) {
        g_system->stop_system();
    }
    exit(signum);
}

void print_usage(const char* program_name) {
    std::cout << "Uso: " << program_name << " <node_id> <ip> <port> [seed_ip] [seed_port]" << std::endl;
    std::cout << std::endl;
    std::cout << "Parámetros:" << std::endl;
    std::cout << "  node_id   - ID único del nodo" << std::endl;
    std::cout << "  ip        - Dirección IP del nodo" << std::endl;
    std::cout << "  port      - Puerto del nodo" << std::endl;
    std::cout << "  seed_ip   - IP del nodo semilla (opcional)" << std::endl;
    std::cout << "  seed_port - Puerto del nodo semilla (opcional)" << std::endl;
    std::cout << std::endl;
    std::cout << "Ejemplos:" << std::endl;
    std::cout << "  # Nodo maestro" << std::endl;
    std::cout << "  " << program_name << " master 127.0.0.1 8080" << std::endl;
    std::cout << std::endl;
    std::cout << "  # Nodo worker" << std::endl;
    std::cout << "  " << program_name << " worker1 127.0.0.1 8081 127.0.0.1 8080" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parsear argumentos
    std::string node_id = argv[1];
    std::string node_ip = argv[2];
    int node_port = atoi(argv[3]);
    
    std::string seed_ip;
    int seed_port = 0;
    
    if (argc >= 6) {
        seed_ip = argv[4];
        seed_port = atoi(argv[5]);
    }
    
    // Configurar manejo de señales
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        std::cout << "=== Sistema Distribuido de Procesamiento Paralelo ===" << std::endl;
        std::cout << "Nodo: " << node_id << " (" << node_ip << ":" << node_port << ")" << std::endl;
        
        // Crear sistema
        const size_t MEMORY_BLOCK_SIZE = sizeof(DatabaseRecord) * 1000;
        const size_t INITIAL_BLOCKS = 10;
        const std::string CONFIG_FILE = "config/basic_pipeline.txt";
        
        DistributedProcessingSystem system(node_id, node_ip, node_port, 
                                         CONFIG_FILE, MEMORY_BLOCK_SIZE, INITIAL_BLOCKS);
        g_system = &system;
        
        // Iniciar sistema
        if (!system.start_system()) {
            std::cerr << "Error iniciando sistema distribuido" << std::endl;
            return 1;
        }
        
        // Unirse a cluster si se especificó
        if (!seed_ip.empty() && seed_port > 0) {
            std::cout << "Intentando unirse al cluster via " << seed_ip << ":" << seed_port << std::endl;
            if (system.join_cluster(seed_ip, seed_port)) {
                std::cout << "Unido al cluster exitosamente!" << std::endl;
            } else {
                std::cout << "No se pudo unir al cluster, operando independientemente" << std::endl;
            }
        }
        
        // Simular procesamiento de datos
        std::cout << "Sistema operativo. Procesando datos..." << std::endl;
        
        for (int i = 0; i < 100; ++i) {
            RecordBatch* batch = system.create_batch(100);
            
            // Llenar batch con datos de ejemplo
            for (size_t j = 0; j < 100; ++j) {
                DatabaseRecord record;
                record.id = i * 100 + j;
                sprintf(record.name, "Record_%05d", record.id);
                record.value = (double)(rand() % 10000) / 100.0;
                record.category = (rand() % 10) + 1;
                batch->add_record(record);
            }
            
            // Procesar batch
            if (system.process_batch(batch)) {
                if (i % 10 == 0) {
                    std::cout << "Procesado lote " << i << std::endl;
                }
            } else {
                std::cerr << "Error procesando lote " << i << std::endl;
            }
            
            system.free_batch(batch);
            usleep(100000); // 100ms delay
        }
        
        // Mostrar estado final
        system.print_system_status();
        
        // Mantener sistema activo
        std::cout << "Sistema activo. Presione Ctrl+C para terminar." << std::endl;
        while (true) {
            sleep(10);
            if (!system.is_system_healthy()) {
                std::cout << "Sistema no saludable, iniciando shutdown..." << std::endl;
                break;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Sistema terminado exitosamente." << std::endl;
    return 0;
}
