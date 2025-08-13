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
