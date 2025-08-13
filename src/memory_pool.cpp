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

// src/memory_pool.cpp
#include "types.h"
#include "memory_pool.h"
#include <algorithm>
#include <iostream>

namespace distributed {

DistributedMemoryPool::Block::Block(size_t s) : size(s), in_use(false), next(NULL) {
    data = new char[size];
}

DistributedMemoryPool::Block::~Block() {
    delete[] data;
}

DistributedMemoryPool::DistributedMemoryPool(size_t block_size, size_t initial_blocks) 
    : free_blocks(NULL), used_blocks(NULL), block_size(block_size), total_blocks(0) {
    
    pthread_mutex_init(&mutex, NULL);
    
    // Pre-asignar bloques iniciales
    for (size_t i = 0; i < initial_blocks; ++i) {
        Block* block = new Block(block_size);
        block->next = free_blocks;
        free_blocks = block;
        total_blocks++;
    }
}

DistributedMemoryPool::~DistributedMemoryPool() {
    while (free_blocks) {
        Block* next = free_blocks->next;
        delete free_blocks;
        free_blocks = next;
    }
    while (used_blocks) {
        Block* next = used_blocks->next;
        delete used_blocks;
        used_blocks = next;
    }
    pthread_mutex_destroy(&mutex);
}

void* DistributedMemoryPool::allocate(size_t size) {
    pthread_mutex_lock(&mutex);
    
    if (size > block_size) {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    
    Block* block = NULL;
    
    if (free_blocks) {
        block = free_blocks;
        free_blocks = free_blocks->next;
    } else {
        block = new Block(block_size);
        total_blocks++;
    }
    
    block->in_use = true;
    block->next = used_blocks;
    used_blocks = block;
    
    pthread_mutex_unlock(&mutex);
    return block->data;
}

void DistributedMemoryPool::deallocate(void* ptr) {
    if (!ptr) return;
    
    pthread_mutex_lock(&mutex);
    
    Block* prev = NULL;
    Block* current = used_blocks;
    
    while (current && current->data != ptr) {
        prev = current;
        current = current->next;
    }
    
    if (current) {
        if (prev) {
            prev->next = current->next;
        } else {
            used_blocks = current->next;
        }
        
        current->in_use = false;
        current->next = free_blocks;
        free_blocks = current;
    }
    
    pthread_mutex_unlock(&mutex);
}

RecordBatch* DistributedMemoryPool::create_batch(size_t capacity) {
    RecordBatch* batch = new RecordBatch();
    batch->records = static_cast<DatabaseRecord*>(allocate(sizeof(DatabaseRecord) * capacity));
    batch->capacity = capacity;
    batch->count = 0;
    batch->batch_id = rand(); // ID único simple
    return batch;
}

void DistributedMemoryPool::free_batch(RecordBatch* batch) {
    if (batch) {
        deallocate(batch->records);
        delete batch;
    }
}

size_t DistributedMemoryPool::get_total_blocks() const {
    pthread_mutex_lock(&mutex);
    size_t total = total_blocks;
    pthread_mutex_unlock(&mutex);
    return total;
}

void DistributedMemoryPool::get_statistics(size_t& total, size_t& free, size_t& used) const {
    pthread_mutex_lock(&mutex);
    
    total = total_blocks;
    
    // Contar bloques libres
    free = 0;
    Block* current = free_blocks;
    while (current) {
        free++;
        current = current->next;
    }
    
    // Contar bloques usados
    used = 0;
    current = used_blocks;
    while (current) {
        used++;
        current = current->next;
    }
    
    pthread_mutex_unlock(&mutex);
}

void DistributedMemoryPool::expand_pool(size_t additional_blocks) {
    pthread_mutex_lock(&mutex);
    
    for (size_t i = 0; i < additional_blocks; ++i) {
        Block* block = new Block(block_size);
        block->next = free_blocks;
        free_blocks = block;
        total_blocks++;
    }
    
    pthread_mutex_unlock(&mutex);
}

} // namespace distributed
