#ifndef DISTRIBUTED_IPC_H
#define DISTRIBUTED_IPC_H

#include <string>
#include <pthread.h>

namespace distributed {

/**
 * @brief Región de memoria compartida thread-safe
 */
class SharedMemoryRegion {
private:
    void* memory;
    size_t size;
    int shm_fd;
    std::string name;

public:
    /**
     * @brief Constructor
     * @param region_name Nombre único de la región
     * @param region_size Tamaño en bytes
     * @param create Si crear nueva región o conectar a existente
     */
    SharedMemoryRegion(const std::string& region_name, size_t region_size, bool create = true);

    ~SharedMemoryRegion();

    /**
     * @brief Obtener puntero a la memoria
     */
    void* get_memory() const { return memory; }

    /**
     * @brief Obtener tamaño de la región
     */
    size_t get_size() const { return size; }

    /**
     * @brief Verificar si la región es válida
     */
    bool is_valid() const;

    /**
     * @brief Limpiar región compartida
     */
    static void cleanup(const std::string& name);
};

/**
 * @brief Mensaje para comunicación entre procesos
 */
struct IPCMessage {
    enum MessageType {
        PROCESS_BATCH,
        BATCH_RESULT,
        HEALTH_CHECK,
        SHUTDOWN,
        SUPERVISOR_CMD,
        NODE_DISCOVERY,
        LOAD_BALANCE
    };

    MessageType type;
    int sender_id;
    int receiver_id;
    size_t data_size;
    char data[0]; ///< Datos de longitud variable
};

/**
 * @brief Canal de comunicación entre procesos usando pipes
 */
class IPCChannel {
private:
    int read_fd;
    int write_fd;
    pthread_mutex_t write_mutex;

public:
    IPCChannel();
    ~IPCChannel();

    /**
     * @brief Crear pipe para comunicación
     */
    bool create_pipe();

    /**
     * @brief Enviar mensaje
     */
    bool send_message(const IPCMessage* msg);

    /**
     * @brief Recibir mensaje
     * @param msg Puntero a asignar con el mensaje (debe liberarse)
     * @param max_size Tamaño máximo permitido
     */
    bool receive_message(IPCMessage** msg, size_t max_size);

    /**
     * @brief Cerrar canal
     */
    void close();

    /**
     * @brief Obtener file descriptors
     */
    int get_read_fd() const { return read_fd; }
    int get_write_fd() const { return write_fd; }
};

} // namespace distributed

#endif // DISTRIBUTED_IPC_H
