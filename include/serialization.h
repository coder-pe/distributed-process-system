#ifndef DISTRIBUTED_SERIALIZATION_H
#define DISTRIBUTED_SERIALIZATION_H

#include "types.h"
#include <cstddef>

namespace distributed {

/**
 * @brief Serializador eficiente para comunicación entre procesos
 * 
 * Proporciona serialización de alta performance para estructuras
 * de datos del sistema, optimizada para IPC y comunicación de red.
 */
class Serializer {
public:
    /**
     * @brief Serializar un lote de registros
     * @param batch Lote a serializar
     * @param buffer Buffer de destino
     * @param buffer_size Tamaño del buffer
     * @return Bytes escritos, 0 si error
     */
    static size_t serialize_batch(const RecordBatch* batch, char* buffer, size_t buffer_size);

    /**
     * @brief Deserializar un lote de registros
     * @param buffer Buffer fuente
     * @param batch Lote de destino (debe tener memoria asignada)
     * @return true si exitoso
     */
    static bool deserialize_batch(const char* buffer, RecordBatch* batch);

    /**
     * @brief Serializar información de nodo
     */
    static size_t serialize_node_info(const NodeInfo& node, char* buffer, size_t buffer_size);

    /**
     * @brief Deserializar información de nodo
     */
    static bool deserialize_node_info(const char* buffer, NodeInfo& node);

    /**
     * @brief Calcular tamaño necesario para serializar un lote
     */
    static size_t calculate_batch_size(const RecordBatch* batch);

    /**
     * @brief Validar integridad de datos serializados
     */
    static bool validate_serialized_data(const char* buffer, size_t size);
};

} // namespace distributed

#endif // DISTRIBUTED_SERIALIZATION_H
