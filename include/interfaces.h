#ifndef DISTRIBUTED_INTERFACES_H
#define DISTRIBUTED_INTERFACES_H

#include "types.h"
#include <string>

namespace distributed {

// =============================================================================
// INTERFACES PRINCIPALES DEL SISTEMA
// =============================================================================

/**
 * @brief Interfaz para componentes de procesamiento
 */
class IProcessingComponent {
public:
    virtual ~IProcessingComponent() {}

    /**
     * @brief Procesar un lote de registros
     * @param batch Lote a procesar
     * @return 0 si exitoso, código de error si falla
     */
    virtual int process_batch(RecordBatch* batch) = 0;

    /**
     * @brief Obtener el nombre del componente
     */
    virtual const std::string& get_name() const = 0;

    /**
     * @brief Verificar si el componente está funcionando
     */
    virtual bool is_healthy() const = 0;

    /**
     * @brief Obtener métricas del componente
     */
    virtual const ComponentMetrics* get_metrics() const = 0;
};

/**
 * @brief Interfaz para supervisores
 */
class ISupervisor {
public:
    virtual ~ISupervisor() {}

    /**
     * @brief Agregar un componente bajo supervisión
     */
    virtual void add_component(IProcessingComponent* component) = 0;

    /**
     * @brief Manejar la muerte de un componente
     */
    virtual void handle_component_death(const std::string& component_name) = 0;

    /**
     * @brief Obtener el número de componentes supervisados
     */
    virtual size_t get_component_count() const = 0;

    /**
     * @brief Imprimir el árbol de supervisión
     */
    virtual void print_supervision_tree(int depth = 0) const = 0;
};

/**
 * @brief Interfaz para nodos distribuidos
 */
class IDistributedNode {
public:
    virtual ~IDistributedNode() {}

    /**
     * @brief Iniciar el nodo
     */
    virtual bool start() = 0;

    /**
     * @brief Unirse a un cluster
     */
    virtual bool join_cluster(const std::string& seed_ip, int seed_port) = 0;

    /**
     * @brief Procesar un lote, posiblemente distribuyéndolo
     */
    virtual bool process_batch_distributed(RecordBatch* batch) = 0;

    /**
     * @brief Obtener el ID del nodo
     */
    virtual const std::string& get_node_id() const = 0;

    /**
     * @brief Obtener estado del cluster
     */
    virtual void print_cluster_status() const = 0;
};

/**
 * @brief Interfaz para gestores de configuración
 */
class IConfigurationManager {
public:
    virtual ~IConfigurationManager() {}

    /**
     * @brief Cargar configuración desde archivo
     */
    virtual bool load_configuration(const std::string& filename) = 0;

    /**
     * @brief Recargar configuración
     */
    virtual bool reload_configuration() = 0;

    /**
     * @brief Guardar configuración actual
     */
    virtual bool save_configuration(const std::string& filename) const = 0;
};

/**
 * @brief Interfaz para pools de memoria
 */
class IMemoryPool {
public:
    virtual ~IMemoryPool() {}

    /**
     * @brief Asignar memoria
     */
    virtual void* allocate(size_t size) = 0;

    /**
     * @brief Liberar memoria
     */
    virtual void deallocate(void* ptr) = 0;

    /**
     * @brief Crear un lote de registros
     */
    virtual RecordBatch* create_batch(size_t capacity) = 0;

    /**
     * @brief Liberar un lote de registros
     */
    virtual void free_batch(RecordBatch* batch) = 0;

    /**
     * @brief Obtener estadísticas del pool
     */
    virtual size_t get_total_blocks() const = 0;
};

} // namespace distributed

#endif // DISTRIBUTED_INTERFACES_H
