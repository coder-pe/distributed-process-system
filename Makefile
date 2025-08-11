# =============================================================================
# MAKEFILE PARA SISTEMA DISTRIBUIDO MODULARIZADO
# =============================================================================

CXX = g++
CC = gcc
CXXFLAGS = -Wall -Wextra -O2 -fPIC -std=c++11 -pthread -g -I./include
CFLAGS = -Wall -Wextra -O2 -fPIC -pthread -g -I./include
LDFLAGS = -shared -ldl -pthread -lrt

# Directorios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
PLUGIN_DIR = plugins
TEST_DIR = tests
LOGS_DIR = logs

# Archivos fuente del sistema principal
CORE_SOURCES = $(SRC_DIR)/types.cpp \
	       $(SRC_DIR)/memory_pool.cpp \
               $(SRC_DIR)/serialization.cpp \
               $(SRC_DIR)/ipc.cpp \
               $(SRC_DIR)/isolated_process.cpp \
               $(SRC_DIR)/supervisor.cpp \
               $(SRC_DIR)/distributed_node.cpp \
               $(SRC_DIR)/plugin_manager.cpp \
               $(SRC_DIR)/configuration.cpp \
               $(SRC_DIR)/distributed_system.cpp

# Archivos objeto
CORE_OBJECTS = $(CORE_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Targets principales
MAIN_TARGET = $(BIN_DIR)/distributed_system
STATIC_LIB = $(BUILD_DIR)/libdistributed.a

# Plugins
PLUGINS = validation enrichment aggregation audit failure_simulator
PLUGIN_TARGETS = $(addprefix $(PLUGIN_DIR)/lib, $(addsuffix .so, $(PLUGINS)))

.PHONY: all clean setup structure help modules main plugins tests 

all: setup structure modules main plugins

help:
	@echo "=== Sistema Distribuido Modularizado ==="
	@echo ""
	@echo "Estructura:"
	@echo "  make structure    - Crear estructura de directorios"
	@echo "  make modules      - Compilar módulos del sistema"
	@echo "  make main         - Compilar ejecutable principal"
	@echo "  make plugins      - Compilar plugins de ejemplo"
	@echo "  make tests        - Compilar herramientas de testing"
	@echo ""
	@echo "Desarrollo:"
	@echo "  make clean        - Limpiar archivos compilados"
	@echo "  make depend       - Generar dependencias"
	@echo "  make debug        - Compilar con símbolos de debug"
	@echo "  make docs         - Generar documentación"
	@echo ""
	@echo "Testing:"
	@echo "  make test-modules - Probar módulos individuales"
	@echo "  make test-system  - Probar sistema completo"

setup:
	@mkdir -p $(SRC_DIR) $(INCLUDE_DIR) $(BUILD_DIR) $(BIN_DIR) $(PLUGIN_DIR) $(TEST_DIR) $(LOGS_DIR)

# =============================================================================
# GENERACIÓN DE ESTRUCTURA MODULAR
# =============================================================================

structure: setup headers sources

headers: $(INCLUDE_DIR)/types.h \
         $(INCLUDE_DIR)/interfaces.h \
         $(INCLUDE_DIR)/memory_pool.h \
         $(INCLUDE_DIR)/serialization.h \
         $(INCLUDE_DIR)/ipc.h \
         $(INCLUDE_DIR)/isolated_process.h \
         $(INCLUDE_DIR)/supervisor.h \
         $(INCLUDE_DIR)/distributed_node.h \
         $(INCLUDE_DIR)/plugin_manager.h \
         $(INCLUDE_DIR)/configuration.h \
         $(INCLUDE_DIR)/distributed_system.h

sources: $(SRC_DIR)/types.cpp \
	 $(SRC_DIR)/memory_pool.cpp \
         $(SRC_DIR)/serialization.cpp \
         $(SRC_DIR)/ipc.cpp \
         $(SRC_DIR)/isolated_process.cpp \
         $(SRC_DIR)/supervisor.cpp \
         $(SRC_DIR)/distributed_node.cpp \
         $(SRC_DIR)/plugin_manager.cpp \
         $(SRC_DIR)/configuration.cpp \
         $(SRC_DIR)/distributed_system.cpp \
         $(SRC_DIR)/main.cpp

# =============================================================================
# HEADERS DE INTERFAZ
# =============================================================================

$(INCLUDE_DIR)/types.h:
	@echo "Creando $(INCLUDE_DIR)/types.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_TYPES_H' > $@
	@echo '#define DISTRIBUTED_TYPES_H' >> $@
	@echo '' >> $@
	@echo '#include <cstddef>' >> $@
	@echo '#include <string>' >> $@
	@echo '#include <vector>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '// =============================================================================' >> $@
	@echo '// TIPOS BÁSICOS DEL SISTEMA' >> $@
	@echo '// =============================================================================' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Estructura que representa un registro de base de datos' >> $@
	@echo ' */' >> $@
	@echo 'struct DatabaseRecord {' >> $@
	@echo '    int id;' >> $@
	@echo '    char name[100];' >> $@
	@echo '    double value;' >> $@
	@echo '    int category;' >> $@
	@echo '' >> $@
	@echo '    DatabaseRecord();' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Lote de registros para procesamiento' >> $@
	@echo ' */' >> $@
	@echo 'struct RecordBatch {' >> $@
	@echo '    DatabaseRecord* records;' >> $@
	@echo '    size_t count;' >> $@
	@echo '    size_t capacity;' >> $@
	@echo '    int batch_id;' >> $@
	@echo '' >> $@
	@echo '    RecordBatch();' >> $@
	@echo '    void add_record(const DatabaseRecord& record);' >> $@
	@echo '    bool is_full() const;' >> $@
	@echo '    void clear();' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Estados del sistema de supervision' >> $@
	@echo ' */' >> $@
	@echo 'enum RestartPolicy {' >> $@
	@echo '    ONE_FOR_ONE,     ///< Solo reiniciar el proceso que falló' >> $@
	@echo '    ONE_FOR_ALL,     ///< Reiniciar todos los procesos supervisados' >> $@
	@echo '    REST_FOR_ONE     ///< Reiniciar el proceso que falló y todos los siguientes' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Estados del circuit breaker' >> $@
	@echo ' */' >> $@
	@echo 'enum CircuitBreakerState {' >> $@
	@echo '    CLOSED,     ///< Operación normal' >> $@
	@echo '    OPEN,       ///< Fallando, no llamar al componente' >> $@
	@echo '    HALF_OPEN   ///< Probando si el componente se recuperó' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Políticas de failover' >> $@
	@echo ' */' >> $@
	@echo 'enum FailoverPolicy {' >> $@
	@echo '    FAIL_FAST,              ///< Fallar inmediatamente' >> $@
	@echo '    RETRY_WITH_BACKOFF,     ///< Reintentar con backoff exponencial' >> $@
	@echo '    SKIP_AND_CONTINUE,      ///< Saltar componente y continuar' >> $@
	@echo '    USE_FALLBACK_PLUGIN,    ///< Usar componente de respaldo' >> $@
	@echo '    ISOLATE_AND_CONTINUE    ///< Aislar componente y continuar sin él' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Información de un nodo en el cluster' >> $@
	@echo ' */' >> $@
	@echo 'struct NodeInfo {' >> $@
	@echo '    std::string node_id;' >> $@
	@echo '    std::string ip_address;' >> $@
	@echo '    int port;' >> $@
	@echo '    bool is_alive;' >> $@
	@echo '    time_t last_seen;' >> $@
	@echo '    int load_factor; ///< 0-100' >> $@
	@echo '' >> $@
	@echo '    NodeInfo();' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Métricas de rendimiento de un componente' >> $@
	@echo ' */' >> $@
	@echo 'struct ComponentMetrics {' >> $@
	@echo '    size_t total_calls;' >> $@
	@echo '    size_t successful_calls;' >> $@
	@echo '    size_t failed_calls;' >> $@
	@echo '    size_t timeout_calls;' >> $@
	@echo '    double total_execution_time_ms;' >> $@
	@echo '    double last_execution_time_ms;' >> $@
	@echo '    time_t last_success_time;' >> $@
	@echo '    time_t last_failure_time;' >> $@
	@echo '' >> $@
	@echo '    ComponentMetrics();' >> $@
	@echo '    void record_success(double execution_time_ms);' >> $@
	@echo '    void record_failure(double execution_time_ms, bool is_timeout = false);' >> $@
	@echo '    double get_success_rate() const;' >> $@
	@echo '    double get_average_execution_time() const;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_TYPES_H' >> $@

$(INCLUDE_DIR)/interfaces.h:
	@echo "Creando $(INCLUDE_DIR)/interfaces.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_INTERFACES_H' > $@
	@echo '#define DISTRIBUTED_INTERFACES_H' >> $@
	@echo '' >> $@
	@echo '#include "types.h"' >> $@
	@echo '#include <string>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '// =============================================================================' >> $@
	@echo '// INTERFACES PRINCIPALES DEL SISTEMA' >> $@
	@echo '// =============================================================================' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Interfaz para componentes de procesamiento' >> $@
	@echo ' */' >> $@
	@echo 'class IProcessingComponent {' >> $@
	@echo 'public:' >> $@
	@echo '    virtual ~IProcessingComponent() {}' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Procesar un lote de registros' >> $@
	@echo '     * @param batch Lote a procesar' >> $@
	@echo '     * @return 0 si exitoso, código de error si falla' >> $@
	@echo '     */' >> $@
	@echo '    virtual int process_batch(RecordBatch* batch) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener el nombre del componente' >> $@
	@echo '     */' >> $@
	@echo '    virtual const std::string& get_name() const = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Verificar si el componente está funcionando' >> $@
	@echo '     */' >> $@
	@echo '    virtual bool is_healthy() const = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener métricas del componente' >> $@
	@echo '     */' >> $@
	@echo '    virtual const ComponentMetrics* get_metrics() const = 0;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Interfaz para supervisores' >> $@
	@echo ' */' >> $@
	@echo 'class ISupervisor {' >> $@
	@echo 'public:' >> $@
	@echo '    virtual ~ISupervisor() {}' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Agregar un componente bajo supervisión' >> $@
	@echo '     */' >> $@
	@echo '    virtual void add_component(IProcessingComponent* component) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Manejar la muerte de un componente' >> $@
	@echo '     */' >> $@
	@echo '    virtual void handle_component_death(const std::string& component_name) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener el número de componentes supervisados' >> $@
	@echo '     */' >> $@
	@echo '    virtual size_t get_component_count() const = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Imprimir el árbol de supervisión' >> $@
	@echo '     */' >> $@
	@echo '    virtual void print_supervision_tree(int depth = 0) const = 0;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Interfaz para nodos distribuidos' >> $@
	@echo ' */' >> $@
	@echo 'class IDistributedNode {' >> $@
	@echo 'public:' >> $@
	@echo '    virtual ~IDistributedNode() {}' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Iniciar el nodo' >> $@
	@echo '     */' >> $@
	@echo '    virtual bool start() = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Unirse a un cluster' >> $@
	@echo '     */' >> $@
	@echo '    virtual bool join_cluster(const std::string& seed_ip, int seed_port) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Procesar un lote, posiblemente distribuyéndolo' >> $@
	@echo '     */' >> $@
	@echo '    virtual bool process_batch_distributed(RecordBatch* batch) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener el ID del nodo' >> $@
	@echo '     */' >> $@
	@echo '    virtual const std::string& get_node_id() const = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener estado del cluster' >> $@
	@echo '     */' >> $@
	@echo '    virtual void print_cluster_status() const = 0;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Interfaz para gestores de configuración' >> $@
	@echo ' */' >> $@
	@echo 'class IConfigurationManager {' >> $@
	@echo 'public:' >> $@
	@echo '    virtual ~IConfigurationManager() {}' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Cargar configuración desde archivo' >> $@
	@echo '     */' >> $@
	@echo '    virtual bool load_configuration(const std::string& filename) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Recargar configuración' >> $@
	@echo '     */' >> $@
	@echo '    virtual bool reload_configuration() = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Guardar configuración actual' >> $@
	@echo '     */' >> $@
	@echo '    virtual bool save_configuration(const std::string& filename) const = 0;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Interfaz para pools de memoria' >> $@
	@echo ' */' >> $@
	@echo 'class IMemoryPool {' >> $@
	@echo 'public:' >> $@
	@echo '    virtual ~IMemoryPool() {}' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Asignar memoria' >> $@
	@echo '     */' >> $@
	@echo '    virtual void* allocate(size_t size) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Liberar memoria' >> $@
	@echo '     */' >> $@
	@echo '    virtual void deallocate(void* ptr) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Crear un lote de registros' >> $@
	@echo '     */' >> $@
	@echo '    virtual RecordBatch* create_batch(size_t capacity) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Liberar un lote de registros' >> $@
	@echo '     */' >> $@
	@echo '    virtual void free_batch(RecordBatch* batch) = 0;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener estadísticas del pool' >> $@
	@echo '     */' >> $@
	@echo '    virtual size_t get_total_blocks() const = 0;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_INTERFACES_H' >> $@

$(INCLUDE_DIR)/memory_pool.h:
	@echo "Creando $(INCLUDE_DIR)/memory_pool.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_MEMORY_POOL_H' > $@
	@echo '#define DISTRIBUTED_MEMORY_POOL_H' >> $@
	@echo '' >> $@
	@echo '#include "interfaces.h"' >> $@
	@echo '#include <pthread.h>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Pool de memoria thread-safe para alta performance' >> $@
	@echo ' * ' >> $@
	@echo ' * Implementa un pool de memoria que pre-asigna bloques para evitar' >> $@
	@echo ' * llamadas frecuentes a malloc/free. Es thread-safe y optimizado' >> $@
	@echo ' * para alto throughput.' >> $@
	@echo ' */' >> $@
	@echo 'class DistributedMemoryPool : public IMemoryPool {' >> $@
	@echo 'private:' >> $@
	@echo '    struct Block {' >> $@
	@echo '        char* data;' >> $@
	@echo '        size_t size;' >> $@
	@echo '        bool in_use;' >> $@
	@echo '        Block* next;' >> $@
	@echo '' >> $@
	@echo '        Block(size_t s);' >> $@
	@echo '        ~Block();' >> $@
	@echo '    };' >> $@
	@echo '' >> $@
	@echo '    Block* free_blocks;' >> $@
	@echo '    Block* used_blocks;' >> $@
	@echo '    mutable pthread_mutex_t mutex;' >> $@
	@echo '    size_t block_size;' >> $@
	@echo '    size_t total_blocks;' >> $@
	@echo '' >> $@
	@echo 'public:' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Constructor' >> $@
	@echo '     * @param block_size Tamaño de cada bloque en bytes' >> $@
	@echo '     * @param initial_blocks Número inicial de bloques a pre-asignar' >> $@
	@echo '     */' >> $@
	@echo '    DistributedMemoryPool(size_t block_size, size_t initial_blocks = 10);' >> $@
	@echo '' >> $@
	@echo '    virtual ~DistributedMemoryPool();' >> $@
	@echo '' >> $@
	@echo '    // Implementación de IMemoryPool' >> $@
	@echo '    virtual void* allocate(size_t size);' >> $@
	@echo '    virtual void deallocate(void* ptr);' >> $@
	@echo '    virtual RecordBatch* create_batch(size_t capacity);' >> $@
	@echo '    virtual void free_batch(RecordBatch* batch);' >> $@
	@echo '    virtual size_t get_total_blocks() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener estadísticas detalladas del pool' >> $@
	@echo '     */' >> $@
	@echo '    void get_statistics(size_t& total, size_t& free, size_t& used) const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Expandir el pool con más bloques' >> $@
	@echo '     */' >> $@
	@echo '    void expand_pool(size_t additional_blocks);' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_MEMORY_POOL_H' >> $@

$(INCLUDE_DIR)/serialization.h:
	@echo "Creando $(INCLUDE_DIR)/serialization.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_SERIALIZATION_H' > $@
	@echo '#define DISTRIBUTED_SERIALIZATION_H' >> $@
	@echo '' >> $@
	@echo '#include "types.h"' >> $@
	@echo '#include <cstddef>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Serializador eficiente para comunicación entre procesos' >> $@
	@echo ' * ' >> $@
	@echo ' * Proporciona serialización de alta performance para estructuras' >> $@
	@echo ' * de datos del sistema, optimizada para IPC y comunicación de red.' >> $@
	@echo ' */' >> $@
	@echo 'class Serializer {' >> $@
	@echo 'public:' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Serializar un lote de registros' >> $@
	@echo '     * @param batch Lote a serializar' >> $@
	@echo '     * @param buffer Buffer de destino' >> $@
	@echo '     * @param buffer_size Tamaño del buffer' >> $@
	@echo '     * @return Bytes escritos, 0 si error' >> $@
	@echo '     */' >> $@
	@echo '    static size_t serialize_batch(const RecordBatch* batch, char* buffer, size_t buffer_size);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Deserializar un lote de registros' >> $@
	@echo '     * @param buffer Buffer fuente' >> $@
	@echo '     * @param batch Lote de destino (debe tener memoria asignada)' >> $@
	@echo '     * @return true si exitoso' >> $@
	@echo '     */' >> $@
	@echo '    static bool deserialize_batch(const char* buffer, RecordBatch* batch);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Serializar información de nodo' >> $@
	@echo '     */' >> $@
	@echo '    static size_t serialize_node_info(const NodeInfo& node, char* buffer, size_t buffer_size);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Deserializar información de nodo' >> $@
	@echo '     */' >> $@
	@echo '    static bool deserialize_node_info(const char* buffer, NodeInfo& node);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Calcular tamaño necesario para serializar un lote' >> $@
	@echo '     */' >> $@
	@echo '    static size_t calculate_batch_size(const RecordBatch* batch);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Validar integridad de datos serializados' >> $@
	@echo '     */' >> $@
	@echo '    static bool validate_serialized_data(const char* buffer, size_t size);' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_SERIALIZATION_H' >> $@

$(INCLUDE_DIR)/ipc.h:
	@echo "Creando $(INCLUDE_DIR)/ipc.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_IPC_H' > $@
	@echo '#define DISTRIBUTED_IPC_H' >> $@
	@echo '' >> $@
	@echo '#include <string>' >> $@
	@echo '#include <pthread.h>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Región de memoria compartida thread-safe' >> $@
	@echo ' */' >> $@
	@echo 'class SharedMemoryRegion {' >> $@
	@echo 'private:' >> $@
	@echo '    void* memory;' >> $@
	@echo '    size_t size;' >> $@
	@echo '    int shm_fd;' >> $@
	@echo '    std::string name;' >> $@
	@echo '' >> $@
	@echo 'public:' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Constructor' >> $@
	@echo '     * @param region_name Nombre único de la región' >> $@
	@echo '     * @param region_size Tamaño en bytes' >> $@
	@echo '     * @param create Si crear nueva región o conectar a existente' >> $@
	@echo '     */' >> $@
	@echo '    SharedMemoryRegion(const std::string& region_name, size_t region_size, bool create = true);' >> $@
	@echo '' >> $@
	@echo '    ~SharedMemoryRegion();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener puntero a la memoria' >> $@
	@echo '     */' >> $@
	@echo '    void* get_memory() const { return memory; }' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener tamaño de la región' >> $@
	@echo '     */' >> $@
	@echo '    size_t get_size() const { return size; }' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Verificar si la región es válida' >> $@
	@echo '     */' >> $@
	@echo '    bool is_valid() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Limpiar región compartida' >> $@
	@echo '     */' >> $@
	@echo '    static void cleanup(const std::string& name);' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Mensaje para comunicación entre procesos' >> $@
	@echo ' */' >> $@
	@echo 'struct IPCMessage {' >> $@
	@echo '    enum MessageType {' >> $@
	@echo '        PROCESS_BATCH,' >> $@
	@echo '        BATCH_RESULT,' >> $@
	@echo '        HEALTH_CHECK,' >> $@
	@echo '        SHUTDOWN,' >> $@
	@echo '        SUPERVISOR_CMD,' >> $@
	@echo '        NODE_DISCOVERY,' >> $@
	@echo '        LOAD_BALANCE' >> $@
	@echo '    };' >> $@
	@echo '' >> $@
	@echo '    MessageType type;' >> $@
	@echo '    int sender_id;' >> $@
	@echo '    int receiver_id;' >> $@
	@echo '    size_t data_size;' >> $@
	@echo '    char data[0]; ///< Datos de longitud variable' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Canal de comunicación entre procesos usando pipes' >> $@
	@echo ' */' >> $@
	@echo 'class IPCChannel {' >> $@
	@echo 'private:' >> $@
	@echo '    int read_fd;' >> $@
	@echo '    int write_fd;' >> $@
	@echo '    pthread_mutex_t write_mutex;' >> $@
	@echo '' >> $@
	@echo 'public:' >> $@
	@echo '    IPCChannel();' >> $@
	@echo '    ~IPCChannel();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Crear pipe para comunicación' >> $@
	@echo '     */' >> $@
	@echo '    bool create_pipe();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Enviar mensaje' >> $@
	@echo '     */' >> $@
	@echo '    bool send_message(const IPCMessage* msg);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Recibir mensaje' >> $@
	@echo '     * @param msg Puntero a asignar con el mensaje (debe liberarse)' >> $@
	@echo '     * @param max_size Tamaño máximo permitido' >> $@
	@echo '     */' >> $@
	@echo '    bool receive_message(IPCMessage** msg, size_t max_size);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Cerrar canal' >> $@
	@echo '     */' >> $@
	@echo '    void close();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener file descriptors' >> $@
	@echo '     */' >> $@
	@echo '    int get_read_fd() const { return read_fd; }' >> $@
	@echo '    int get_write_fd() const { return write_fd; }' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_IPC_H' >> $@

$(INCLUDE_DIR)/isolated_process.h:
	@echo "Creando $(INCLUDE_DIR)/isolated_process.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_ISOLATED_PROCESS_H' > $@
	@echo '#define DISTRIBUTED_ISOLATED_PROCESS_H' >> $@
	@echo '' >> $@
	@echo '#include "interfaces.h"' >> $@
	@echo '#include "ipc.h"' >> $@
	@echo '#include "types.h"' >> $@
	@echo '#include <sys/types.h>' >> $@
	@echo '#include <string>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Proceso aislado para ejecutar plugins de forma segura' >> $@
	@echo ' * ' >> $@
	@echo ' * Cada plugin ejecuta en su propio proceso con memoria completamente' >> $@
	@echo ' * aislada. Comunicación via IPC y shared memory para performance.' >> $@
	@echo ' */' >> $@
	@echo 'class IsolatedPluginProcess : public IProcessingComponent {' >> $@
	@echo 'private:' >> $@
	@echo '    pid_t process_id;' >> $@
	@echo '    std::string plugin_name;' >> $@
	@echo '    std::string library_path;' >> $@
	@echo '    std::string config_params;' >> $@
	@echo '    IPCChannel* parent_channel;' >> $@
	@echo '    IPCChannel* child_channel;' >> $@
	@echo '    SharedMemoryRegion* shared_memory;' >> $@
	@echo '    bool is_running;' >> $@
	@echo '    time_t last_heartbeat;' >> $@
	@echo '    ComponentMetrics metrics;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Función que ejecuta el proceso hijo' >> $@
	@echo '     */' >> $@
	@echo '    void execute_plugin_process();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Cargar biblioteca del plugin en proceso hijo' >> $@
	@echo '     */' >> $@
	@echo '    bool load_plugin_library();' >> $@
	@echo '' >> $@
	@echo 'public:' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Constructor' >> $@
	@echo '     * @param name Nombre del plugin' >> $@
	@echo '     * @param lib_path Ruta a la biblioteca compartida' >> $@
	@echo '     * @param params Parámetros de configuración' >> $@
	@echo '     */' >> $@
	@echo '    IsolatedPluginProcess(const std::string& name, ' >> $@
	@echo '                         const std::string& lib_path, ' >> $@
	@echo '                         const std::string& params);' >> $@
	@echo '' >> $@
	@echo '    virtual ~IsolatedPluginProcess();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Iniciar el proceso aislado' >> $@
	@echo '     */' >> $@
	@echo '    bool start();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Terminar el proceso aislado' >> $@
	@echo '     */' >> $@
	@echo '    void terminate();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Verificar si el proceso está vivo' >> $@
	@echo '     */' >> $@
	@echo '    bool is_alive() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener PID del proceso' >> $@
	@echo '     */' >> $@
	@echo '    pid_t get_pid() const { return process_id; }' >> $@
	@echo '' >> $@
	@echo '    // Implementación de IProcessingComponent' >> $@
	@echo '    virtual int process_batch(RecordBatch* batch);' >> $@
	@echo '    virtual const std::string& get_name() const { return plugin_name; }' >> $@
	@echo '    virtual bool is_healthy() const { return is_alive(); }' >> $@
	@echo '    virtual const ComponentMetrics* get_metrics() const { return &metrics; }' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Enviar heartbeat al proceso' >> $@
	@echo '     */' >> $@
	@echo '    bool send_heartbeat();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Reiniciar proceso si ha fallado' >> $@
	@echo '     */' >> $@
	@echo '    bool restart();' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_ISOLATED_PROCESS_H' >> $@

$(INCLUDE_DIR)/supervisor.h:
	@echo "Creando $(INCLUDE_DIR)/supervisor.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_SUPERVISOR_H' > $@
	@echo '#define DISTRIBUTED_SUPERVISOR_H' >> $@
	@echo '' >> $@
	@echo '#include "interfaces.h"' >> $@
	@echo '#include "types.h"' >> $@
	@echo '#include <vector>' >> $@
	@echo '#include <map>' >> $@
	@echo '#include <string>' >> $@
	@echo '#include <pthread.h>' >> $@
	@echo '#include <unistd.h>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Especificación de un supervisor' >> $@
	@echo ' */' >> $@
	@echo 'struct SupervisorSpec {' >> $@
	@echo '    RestartPolicy restart_policy;' >> $@
	@echo '    int max_restarts;      ///< Máximo número de restarts en el período' >> $@
	@echo '    int restart_period;    ///< Período de tiempo en segundos' >> $@
	@echo '    int shutdown_timeout;  ///< Timeout para shutdown en segundos' >> $@
	@echo '' >> $@
	@echo '    SupervisorSpec();' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Supervisor que implementa supervision trees como Erlang/OTP' >> $@
	@echo ' * ' >> $@
	@echo ' * Mantiene un conjunto de componentes bajo supervisión y aplica' >> $@
	@echo ' * políticas de restart cuando fallan. Soporta jerarquías de supervisores.' >> $@
	@echo ' */' >> $@
	@echo 'class ProcessSupervisor : public ISupervisor {' >> $@
	@echo 'private:' >> $@
	@echo '    std::vector<IProcessingComponent*> supervised_components;' >> $@
	@echo '    std::vector<ProcessSupervisor*> child_supervisors;' >> $@
	@echo '    SupervisorSpec spec;' >> $@
	@echo '    std::map<std::string, time_t> restart_history;' >> $@
	@echo '    mutable pthread_mutex_t supervisor_mutex;' >> $@
	@echo '    pthread_t monitor_thread;' >> $@
	@echo '    volatile bool monitoring_active;' >> $@
	@echo '    std::string supervisor_name;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Función del hilo monitor' >> $@
	@echo '     */' >> $@
	@echo '    static void* monitor_function(void* arg);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Verificar si se debe reiniciar un componente' >> $@
	@echo '     */' >> $@
	@echo '    bool should_restart(const std::string& component_name);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Reiniciar un componente específico' >> $@
	@echo '     */' >> $@
	@echo '    void restart_component(const std::string& component_name);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Reiniciar todos los componentes' >> $@
	@echo '     */' >> $@
	@echo '    void restart_all_components();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Reiniciar componentes desde un índice' >> $@
	@echo '     */' >> $@
	@echo '    void restart_remaining_components(size_t from_index);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Convertir política a string' >> $@
	@echo '     */' >> $@
	@echo '    static const char* policy_to_string(RestartPolicy policy);' >> $@
	@echo '' >> $@
	@echo 'public:' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Constructor' >> $@
	@echo '     * @param name Nombre del supervisor' >> $@
	@echo '     * @param supervisor_spec Especificación de comportamiento' >> $@
	@echo '     */' >> $@
	@echo '    ProcessSupervisor(const std::string& name, const SupervisorSpec& supervisor_spec);' >> $@
	@echo '' >> $@
	@echo '    virtual ~ProcessSupervisor();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Agregar supervisor hijo' >> $@
	@echo '     */' >> $@
	@echo '    void add_child_supervisor(ProcessSupervisor* child);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Iniciar todos los componentes supervisados' >> $@
	@echo '     */' >> $@
	@echo '    bool start_all_components();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Parar todos los componentes supervisados' >> $@
	@echo '     */' >> $@
	@echo '    void stop_all_components();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener especificación del supervisor' >> $@
	@echo '     */' >> $@
	@echo '    const SupervisorSpec& get_spec() const { return spec; }' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Actualizar especificación del supervisor' >> $@
	@echo '     */' >> $@
	@echo '    void update_spec(const SupervisorSpec& new_spec);' >> $@
	@echo '' >> $@
	@echo '    // Implementación de ISupervisor' >> $@
	@echo '    virtual void add_component(IProcessingComponent* component);' >> $@
	@echo '    virtual void handle_component_death(const std::string& component_name);' >> $@
	@echo '    virtual size_t get_component_count() const;' >> $@
	@echo '    virtual void print_supervision_tree(int depth = 0) const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener estadísticas del supervisor' >> $@
	@echo '     */' >> $@
	@echo '    void get_statistics(size_t& total_components, size_t& healthy_components, ' >> $@
	@echo '                       size_t& total_restarts) const;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_SUPERVISOR_H' >> $@

$(INCLUDE_DIR)/distributed_node.h:
	@echo "Creando $(INCLUDE_DIR)/distributed_node.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_NODE_H' > $@
	@echo '#define DISTRIBUTED_NODE_H' >> $@
	@echo '' >> $@
	@echo '#include "interfaces.h"' >> $@
	@echo '#include "types.h"' >> $@
	@echo '#include "ipc.h"' >> $@
	@echo '#include <map>' >> $@
	@echo '#include <string>' >> $@
	@echo '#include <pthread.h>' >> $@
	@echo '#include <cstring>' >> $@
	@echo '#include <unistd.h>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Nodo distribuido que participa en un cluster' >> $@
	@echo ' * ' >> $@
	@echo ' * Implementa descubrimiento automático de nodos, load balancing,' >> $@
	@echo ' * y comunicación transparente entre nodos del cluster.' >> $@
	@echo ' */' >> $@
	@echo 'class DistributedNode : public IDistributedNode {' >> $@
	@echo 'private:' >> $@
	@echo '    std::string node_id;' >> $@
	@echo '    std::string local_ip;' >> $@
	@echo '    int local_port;' >> $@
	@echo '    std::map<std::string, NodeInfo> cluster_nodes;' >> $@
	@echo '    mutable pthread_mutex_t cluster_mutex;' >> $@
	@echo '' >> $@
	@echo '    // Servidor para recibir conexiones' >> $@
	@echo '    int server_socket;' >> $@
	@echo '    pthread_t server_thread;' >> $@
	@echo '    volatile bool server_active;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Función del hilo servidor' >> $@
	@echo '     */' >> $@
	@echo '    static void* server_function(void* arg);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Manejar cliente conectado' >> $@
	@echo '     */' >> $@
	@echo '    void handle_client(int client_socket);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Enviar información del cluster' >> $@
	@echo '     */' >> $@
	@echo '    void send_cluster_info(int client_socket);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Manejar batch distribuido' >> $@
	@echo '     */' >> $@
	@echo '    void handle_distributed_batch(int client_socket, size_t data_size);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Parsear información del cluster' >> $@
	@echo '     */' >> $@
	@echo '    void parse_cluster_info(const char* buffer, size_t size);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Actualizar métricas de carga de nodos' >> $@
	@echo '     */' >> $@
	@echo '    void update_node_load_metrics();' >> $@
	@echo '' >> $@
	@echo 'public:' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Constructor' >> $@
	@echo '     * @param id ID único del nodo' >> $@
	@echo '     * @param ip Dirección IP del nodo' >> $@
	@echo '     * @param port Puerto del nodo' >> $@
	@echo '     */' >> $@
	@echo '    DistributedNode(const std::string& id, const std::string& ip, int port);' >> $@
	@echo '' >> $@
	@echo '    virtual ~DistributedNode();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Iniciar servidor del nodo' >> $@
	@echo '     */' >> $@
	@echo '    bool start_server();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Parar servidor del nodo' >> $@
	@echo '     */' >> $@
	@echo '    void shutdown();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Seleccionar mejor nodo para una tarea' >> $@
	@echo '     */' >> $@
	@echo '    std::string select_best_node_for_task();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Enviar batch a nodo específico' >> $@
	@echo '     */' >> $@
	@echo '    bool send_batch_to_node(const std::string& target_node, RecordBatch* batch);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener información de todos los nodos' >> $@
	@echo '     */' >> $@
	@echo '    std::vector<NodeInfo> get_all_nodes() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Verificar conectividad con nodo específico' >> $@
	@echo '     */' >> $@
	@echo '    bool ping_node(const std::string& node_id);' >> $@
	@echo '' >> $@
	@echo '    // Implementación de IDistributedNode' >> $@
	@echo '    virtual bool start();' >> $@
	@echo '    virtual bool join_cluster(const std::string& seed_ip, int seed_port);' >> $@
	@echo '    virtual bool process_batch_distributed(RecordBatch* batch);' >> $@
	@echo '    virtual const std::string& get_node_id() const { return node_id; }' >> $@
	@echo '    virtual void print_cluster_status() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener métricas del cluster' >> $@
	@echo '     */' >> $@
	@echo '    void get_cluster_metrics(size_t& total_nodes, size_t& active_nodes, ' >> $@
	@echo '                           double& avg_load) const;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_NODE_H' >> $@

$(INCLUDE_DIR)/plugin_manager.h:
	@echo "Creando $(INCLUDE_DIR)/plugin_manager.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_PLUGIN_MANAGER_H' > $@
	@echo '#define DISTRIBUTED_PLUGIN_MANAGER_H' >> $@
	@echo '' >> $@
	@echo '#include "interfaces.h"' >> $@
	@echo '#include "isolated_process.h"' >> $@
	@echo '#include "types.h"' >> $@
	@echo '#include <vector>' >> $@
	@echo '#include <string>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Configuración de failover para plugins' >> $@
	@echo ' */' >> $@
	@echo 'struct FailoverConfig {' >> $@
	@echo '    FailoverPolicy policy;' >> $@
	@echo '    int max_retries;' >> $@
	@echo '    int initial_delay_ms;' >> $@
	@echo '    int max_delay_ms;' >> $@
	@echo '    double backoff_multiplier;' >> $@
	@echo '    int timeout_ms;' >> $@
	@echo '    std::string fallback_plugin_path;' >> $@
	@echo '    bool enable_circuit_breaker;' >> $@
	@echo '' >> $@
	@echo '    FailoverConfig();' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Configuración de una etapa del pipeline' >> $@
	@echo ' */' >> $@
	@echo 'struct PipelineStageConfig {' >> $@
	@echo '    std::string name;' >> $@
	@echo '    std::string library_path;' >> $@
	@echo '    std::string parameters;' >> $@
	@echo '    bool enabled;' >> $@
	@echo '    FailoverConfig failover_config;' >> $@
	@echo '' >> $@
	@echo '    PipelineStageConfig();' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Gestor de plugins con capacidades de failover' >> $@
	@echo ' * ' >> $@
	@echo ' * Maneja la carga, ejecución y failover de plugins de procesamiento.' >> $@
	@echo ' * Implementa circuit breakers, timeouts y políticas de retry.' >> $@
	@echo ' */' >> $@
	@echo 'class ResilientPluginManager {' >> $@
	@echo 'private:' >> $@
	@echo '    std::vector<IsolatedPluginProcess*> plugins;' >> $@
	@echo '    std::vector<PipelineStageConfig> pipeline_config;' >> $@
	@echo '    IMemoryPool* memory_pool;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Ejecutar plugin con manejo de timeouts' >> $@
	@echo '     */' >> $@
	@echo '    int execute_plugin_with_timeout(IsolatedPluginProcess* plugin, ' >> $@
	@echo '                                   RecordBatch* batch, ' >> $@
	@echo '                                   int timeout_ms);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Aplicar política de retry' >> $@
	@echo '     */' >> $@
	@echo '    int apply_retry_policy(IsolatedPluginProcess* plugin, ' >> $@
	@echo '                          RecordBatch* batch, ' >> $@
	@echo '                          const FailoverConfig& config);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Ejecutar plugin con manejo de fallos' >> $@
	@echo '     */' >> $@
	@echo '    int execute_plugin_with_failover(IsolatedPluginProcess* plugin, ' >> $@
	@echo '                          RecordBatch* batch, ' >> $@
	@echo '                          const FailoverConfig& config);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Manejar fallo final de plugin' >> $@
	@echo '     */' >> $@
	@echo '    int handle_plugin_failure(const std::string& plugin_name, ' >> $@
	@echo '                             RecordBatch* batch, ' >> $@
	@echo '                             const FailoverConfig& config);' >> $@
	@echo '' >> $@
	@echo 'public:' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Constructor' >> $@
	@echo '     * @param memory_pool Pool de memoria para operaciones' >> $@
	@echo '     */' >> $@
	@echo '    ResilientPluginManager(IMemoryPool* memory_pool);' >> $@
	@echo '' >> $@
	@echo '    virtual ~ResilientPluginManager();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Cargar configuración del pipeline' >> $@
	@echo '     */' >> $@
	@echo '    bool load_pipeline_config(const std::vector<PipelineStageConfig>& config);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Agregar plugin al pipeline' >> $@
	@echo '     */' >> $@
	@echo '    bool add_plugin(const PipelineStageConfig& config);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Remover plugin del pipeline' >> $@
	@echo '     */' >> $@
	@echo '    bool remove_plugin(const std::string& plugin_name);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Procesar lote a través de todo el pipeline' >> $@
	@echo '     */' >> $@
	@echo '    bool process_batch_through_pipeline(RecordBatch* batch);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener estado de todos los plugins' >> $@
	@echo '     */' >> $@
	@echo '    std::vector<std::string> get_plugin_status() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Reiniciar plugin específico' >> $@
	@echo '     */' >> $@
	@echo '    bool restart_plugin(const std::string& plugin_name);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener métricas agregadas del pipeline' >> $@
	@echo '     */' >> $@
	@echo '    void get_pipeline_metrics(size_t& total_plugins, size_t& healthy_plugins, ' >> $@
	@echo '                             double& avg_success_rate) const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Hot-swap de plugin' >> $@
	@echo '     */' >> $@
	@echo '    bool hot_swap_plugin(const std::string& plugin_name, ' >> $@
	@echo '                        const std::string& new_library_path);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Validar configuración del pipeline' >> $@
	@echo '     */' >> $@
	@echo '    static bool validate_pipeline_config(const std::vector<PipelineStageConfig>& config);' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_PLUGIN_MANAGER_H' >> $@

$(INCLUDE_DIR)/configuration.h:
	@echo "Creando $(INCLUDE_DIR)/configuration.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_CONFIGURATION_H' > $@
	@echo '#define DISTRIBUTED_CONFIGURATION_H' >> $@
	@echo '' >> $@
	@echo '#include "interfaces.h"' >> $@
	@echo '#include "plugin_manager.h"' >> $@
	@echo '#include <string>' >> $@
	@echo '#include <vector>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Gestor de configuración del sistema distribuido' >> $@
	@echo ' * ' >> $@
	@echo ' * Maneja la carga, validación y actualización de configuraciones' >> $@
	@echo ' * del sistema desde archivos externos.' >> $@
	@echo ' */' >> $@
	@echo 'class ConfigurationManager : public IConfigurationManager {' >> $@
	@echo 'private:' >> $@
	@echo '    std::string config_file_path;' >> $@
	@echo '    std::vector<PipelineStageConfig> pipeline_stages;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Parsear línea de configuración' >> $@
	@echo '     */' >> $@
	@echo '    bool parse_config_line(const std::string& line, PipelineStageConfig& config);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Validar configuración cargada' >> $@
	@echo '     */' >> $@
	@echo '    bool validate_configuration() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Convertir política de string a enum' >> $@
	@echo '     */' >> $@
	@echo '    static FailoverPolicy string_to_policy(const std::string& policy_str);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Convertir política de enum a string' >> $@
	@echo '     */' >> $@
	@echo '    static std::string policy_to_string(FailoverPolicy policy);' >> $@
	@echo '' >> $@
	@echo 'public:' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Constructor' >> $@
	@echo '     * @param config_path Ruta al archivo de configuración' >> $@
	@echo '     */' >> $@
	@echo '    ConfigurationManager(const std::string& config_path);' >> $@
	@echo '' >> $@
	@echo '    virtual ~ConfigurationManager();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener configuración del pipeline' >> $@
	@echo '     */' >> $@
	@echo '    const std::vector<PipelineStageConfig>& get_pipeline_stages() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Actualizar configuración de una etapa' >> $@
	@echo '     */' >> $@
	@echo '    bool update_stage_config(const std::string& stage_name, ' >> $@
	@echo '                           const PipelineStageConfig& new_config);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Agregar nueva etapa al pipeline' >> $@
	@echo '     */' >> $@
	@echo '    bool add_pipeline_stage(const PipelineStageConfig& stage_config);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Remover etapa del pipeline' >> $@
	@echo '     */' >> $@
	@echo '    bool remove_pipeline_stage(const std::string& stage_name);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Crear configuración de ejemplo' >> $@
	@echo '     */' >> $@
	@echo '    static bool create_sample_config(const std::string& filename);' >> $@
	@echo '' >> $@
	@echo '    // Implementación de IConfigurationManager' >> $@
	@echo '    virtual bool load_configuration(const std::string& filename);' >> $@
	@echo '    virtual bool reload_configuration();' >> $@
	@echo '    virtual bool save_configuration(const std::string& filename) const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener ruta del archivo de configuración actual' >> $@
	@echo '     */' >> $@
	@echo '    const std::string& get_config_file_path() const { return config_file_path; }' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Validar sintaxis de archivo de configuración' >> $@
	@echo '     */' >> $@
	@echo '    static bool validate_config_file_syntax(const std::string& filename);' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_CONFIGURATION_H' >> $@

$(INCLUDE_DIR)/distributed_system.h:
	@echo "Creando $(INCLUDE_DIR)/distributed_system.h..."
	@mkdir -p $(INCLUDE_DIR)
	@echo '#ifndef DISTRIBUTED_SYSTEM_H' > $@
	@echo '#define DISTRIBUTED_SYSTEM_H' >> $@
	@echo '' >> $@
	@echo '#include "interfaces.h"' >> $@
	@echo '#include "memory_pool.h"' >> $@
	@echo '#include "plugin_manager.h"' >> $@
	@echo '#include "supervisor.h"' >> $@
	@echo '#include "distributed_node.h"' >> $@
	@echo '#include "configuration.h"' >> $@
	@echo '#include <string>' >> $@
	@echo '' >> $@
	@echo 'namespace distributed {' >> $@
	@echo '' >> $@
	@echo '/**' >> $@
	@echo ' * @brief Sistema principal que integra todos los componentes' >> $@
	@echo ' * ' >> $@
	@echo ' * Orquesta el funcionamiento de memory pools, supervision trees,' >> $@
	@echo ' * plugin management, y distribución de nodos en un sistema cohesivo.' >> $@
	@echo ' */' >> $@
	@echo 'class DistributedProcessingSystem {' >> $@
	@echo 'private:' >> $@
	@echo '    // Componentes principales' >> $@
	@echo '    DistributedMemoryPool* memory_pool;' >> $@
	@echo '    ResilientPluginManager* plugin_manager;' >> $@
	@echo '    ProcessSupervisor* root_supervisor;' >> $@
	@echo '    DistributedNode* local_node;' >> $@
	@echo '    ConfigurationManager* config_manager;' >> $@
	@echo '' >> $@
	@echo '    // Estado del sistema' >> $@
	@echo '    bool system_running;' >> $@
	@echo '    std::string system_id;' >> $@
	@echo '' >> $@
	@echo '    // Threading para operaciones asíncronas' >> $@
	@echo '    pthread_t health_monitor_thread;' >> $@
	@echo '    volatile bool health_monitoring_active;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Función del monitor de salud del sistema' >> $@
	@echo '     */' >> $@
	@echo '    static void* health_monitor_function(void* arg);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Inicializar supervision tree' >> $@
	@echo '     */' >> $@
	@echo '    bool initialize_supervision_tree();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Cargar y configurar plugins' >> $@
	@echo '     */' >> $@
	@echo '    bool load_and_configure_plugins();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Configurar nodo distribuido' >> $@
	@echo '     */' >> $@
	@echo '    bool setup_distributed_node(const std::string& node_id, ' >> $@
	@echo '                               const std::string& ip, ' >> $@
	@echo '                               int port);' >> $@
	@echo '' >> $@
	@echo 'public:' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Constructor' >> $@
	@echo '     * @param node_id ID único del nodo' >> $@
	@echo '     * @param ip Dirección IP del nodo' >> $@
	@echo '     * @param port Puerto del nodo' >> $@
	@echo '     * @param config_file Archivo de configuración' >> $@
	@echo '     * @param memory_block_size Tamaño de bloques de memoria' >> $@
	@echo '     * @param initial_blocks Bloques iniciales de memoria' >> $@
	@echo '     */' >> $@
	@echo '    DistributedProcessingSystem(const std::string& node_id,' >> $@
	@echo '                               const std::string& ip,' >> $@
	@echo '                               int port,' >> $@
	@echo '                               const std::string& config_file,' >> $@
	@echo '                               size_t memory_block_size,' >> $@
	@echo '                               size_t initial_blocks);' >> $@
	@echo '' >> $@
	@echo '    virtual ~DistributedProcessingSystem();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Iniciar sistema completo' >> $@
	@echo '     */' >> $@
	@echo '    bool start_system();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Parar sistema completo' >> $@
	@echo '     */' >> $@
	@echo '    void stop_system();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Unirse a cluster existente' >> $@
	@echo '     */' >> $@
	@echo '    bool join_cluster(const std::string& seed_ip, int seed_port);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Procesar lote de datos' >> $@
	@echo '     */' >> $@
	@echo '    bool process_batch(RecordBatch* batch);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Crear nuevo lote de registros' >> $@
	@echo '     */' >> $@
	@echo '    RecordBatch* create_batch(size_t capacity);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Liberar lote de registros' >> $@
	@echo '     */' >> $@
	@echo '    void free_batch(RecordBatch* batch);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Recargar configuración' >> $@
	@echo '     */' >> $@
	@echo '    bool reload_configuration();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Hot-swap de plugin' >> $@
	@echo '     */' >> $@
	@echo '    bool hot_swap_plugin(const std::string& plugin_name, ' >> $@
	@echo '                        const std::string& new_library_path);' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener estado completo del sistema' >> $@
	@echo '     */' >> $@
	@echo '    void print_system_status() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener métricas del sistema' >> $@
	@echo '     */' >> $@
	@echo '    void get_system_metrics(size_t& total_nodes, size_t& total_plugins, ' >> $@
	@echo '                           size_t& healthy_plugins, double& system_load) const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Verificar salud del sistema' >> $@
	@echo '     */' >> $@
	@echo '    bool is_system_healthy() const;' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Obtener ID del sistema' >> $@
	@echo '     */' >> $@
	@echo '    const std::string& get_system_id() const { return system_id; }' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Forzar garbage collection del sistema' >> $@
	@echo '     */' >> $@
	@echo '    void force_system_cleanup();' >> $@
	@echo '' >> $@
	@echo '    /**' >> $@
	@echo '     * @brief Exportar configuración actual' >> $@
	@echo '     */' >> $@
	@echo '    bool export_current_config(const std::string& filename) const;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '} // namespace distributed' >> $@
	@echo '' >> $@
	@echo '#endif // DISTRIBUTED_SYSTEM_H' >> $@

# =============================================================================
# COMPILACIÓN DE MÓDULOS
# =============================================================================

modules: $(CORE_OBJECTS) $(STATIC_LIB)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_DIR)/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(STATIC_LIB): $(CORE_OBJECTS)
	ar rcs $@ $^

main: $(MAIN_TARGET)

$(MAIN_TARGET): $(SRC_DIR)/main.cpp $(STATIC_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< -L$(BUILD_DIR) -ldistributed -ldl -pthread -lrt

# =============================================================================
# TARGETS DE DESARROLLO
# =============================================================================

debug: CXXFLAGS += -g -DDEBUG -O0
debug: all

depend:
	@$(CXX) $(CXXFLAGS) -MM $(CORE_SOURCES) > .depend

docs:
	@echo "Generando documentación con Doxygen..."
	@doxygen Doxyfile 2>/dev/null || echo "Doxygen no disponible"

clean:
	rm -rf $(BUILD_DIR)/* $(BIN_DIR)/* $(INCLUDE_DIR)/*
	rm -f .depend

.PHONY: test-modules
test-modules: modules
	@echo "Probando módulos individuales..."
	@for header in $(INCLUDE_DIR)/*.h; do \
		echo "Verificando header: $$header"; \
		echo "#include \"$$(basename $$header)\"" | $(CXX) $(CXXFLAGS) -x c++ -c - -o /dev/null || exit 1; \
	done
	@echo "✓ Todos los headers compilan correctamente"

# Incluir dependencias si existen
-include .depend
