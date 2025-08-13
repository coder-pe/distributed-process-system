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

#ifndef DISTRIBUTED_MEMORY_POOL_H
#define DISTRIBUTED_MEMORY_POOL_H

#include "interfaces.h"
#include <pthread.h>

namespace distributed {

/**
 * @brief Pool de memoria thread-safe para alta performance
 * 
 * Implementa un pool de memoria que pre-asigna bloques para evitar
 * llamadas frecuentes a malloc/free. Es thread-safe y optimizado
 * para alto throughput.
 */
class DistributedMemoryPool : public IMemoryPool {
private:
    struct Block {
        char* data;
        size_t size;
        bool in_use;
        Block* next;

        Block(size_t s);
        ~Block();
    };

    Block* free_blocks;
    Block* used_blocks;
    mutable pthread_mutex_t mutex;
    size_t block_size;
    size_t total_blocks;

public:
    /**
     * @brief Constructor
     * @param block_size Tamaño de cada bloque en bytes
     * @param initial_blocks Número inicial de bloques a pre-asignar
     */
    DistributedMemoryPool(size_t block_size, size_t initial_blocks = 10);

    virtual ~DistributedMemoryPool();

    // Implementación de IMemoryPool
    virtual void* allocate(size_t size);
    virtual void deallocate(void* ptr);
    virtual RecordBatch* create_batch(size_t capacity);
    virtual void free_batch(RecordBatch* batch);
    virtual size_t get_total_blocks() const;

    /**
     * @brief Obtener estadísticas detalladas del pool
     */
    void get_statistics(size_t& total, size_t& free, size_t& used) const;

    /**
     * @brief Expandir el pool con más bloques
     */
    void expand_pool(size_t additional_blocks);
};

} // namespace distributed

#endif // DISTRIBUTED_MEMORY_POOL_H
