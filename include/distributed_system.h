#ifndef DISTRIBUTED_SYSTEM_H
#define DISTRIBUTED_SYSTEM_H

#include "interfaces.h"
#include "memory_pool.h"
#include "plugin_manager.h"
#include "supervisor.h"
#include "distributed_node.h"
#include "configuration.h"
#include <string>

namespace distributed {

/**
 * @brief Sistema principal que integra todos los componentes
 * 
 * Orquesta el funcionamiento de memory pools, supervision trees,
 * plugin management, y distribución de nodos en un sistema cohesivo.
 */
class DistributedProcessingSystem {
private:
    // Componentes principales
    DistributedMemoryPool* memory_pool;
    ResilientPluginManager* plugin_manager;
    ProcessSupervisor* root_supervisor;
    DistributedNode* local_node;
    ConfigurationManager* config_manager;

    // Estado del sistema
    bool system_running;
    std::string system_id;

    // Threading para operaciones asíncronas
    pthread_t health_monitor_thread;
    volatile bool health_monitoring_active;

    /**
     * @brief Función del monitor de salud del sistema
     */
    static void* health_monitor_function(void* arg);

    /**
     * @brief Inicializar supervision tree
     */
    bool initialize_supervision_tree();

    /**
     * @brief Cargar y configurar plugins
     */
    bool load_and_configure_plugins();

    /**
     * @brief Configurar nodo distribuido
     */
    bool setup_distributed_node(const std::string& node_id, 
                               const std::string& ip, 
                               int port);

public:
    /**
     * @brief Constructor
     * @param node_id ID único del nodo
     * @param ip Dirección IP del nodo
     * @param port Puerto del nodo
     * @param config_file Archivo de configuración
     * @param memory_block_size Tamaño de bloques de memoria
     * @param initial_blocks Bloques iniciales de memoria
     */
    DistributedProcessingSystem(const std::string& node_id,
                               const std::string& ip,
                               int port,
                               const std::string& config_file,
                               size_t memory_block_size,
                               size_t initial_blocks);

    virtual ~DistributedProcessingSystem();

    /**
     * @brief Iniciar sistema completo
     */
    bool start_system();

    /**
     * @brief Parar sistema completo
     */
    void stop_system();

    /**
     * @brief Unirse a cluster existente
     */
    bool join_cluster(const std::string& seed_ip, int seed_port);

    /**
     * @brief Procesar lote de datos
     */
    bool process_batch(RecordBatch* batch);

    /**
     * @brief Crear nuevo lote de registros
     */
    RecordBatch* create_batch(size_t capacity);

    /**
     * @brief Liberar lote de registros
     */
    void free_batch(RecordBatch* batch);

    /**
     * @brief Recargar configuración
     */
    bool reload_configuration();

    /**
     * @brief Hot-swap de plugin
     */
    bool hot_swap_plugin(const std::string& plugin_name, 
                        const std::string& new_library_path);

    /**
     * @brief Obtener estado completo del sistema
     */
    void print_system_status() const;

    /**
     * @brief Obtener métricas del sistema
     */
    void get_system_metrics(size_t& total_nodes, size_t& total_plugins, 
                           size_t& healthy_plugins, double& system_load) const;

    /**
     * @brief Verificar salud del sistema
     */
    bool is_system_healthy() const;

    /**
     * @brief Obtener ID del sistema
     */
    const std::string& get_system_id() const { return system_id; }

    /**
     * @brief Forzar garbage collection del sistema
     */
    void force_system_cleanup();

    /**
     * @brief Exportar configuración actual
     */
    bool export_current_config(const std::string& filename) const;
};

} // namespace distributed

#endif // DISTRIBUTED_SYSTEM_H
