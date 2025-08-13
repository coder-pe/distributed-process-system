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

// tests/test_memory_pool.cpp
#include "../include/memory_pool.h"
#include <cassert>
#include <iostream>
#include <pthread.h>

using namespace distributed;

void test_basic_allocation() {
    std::cout << "Test: Basic allocation..." << std::endl;
    
    DistributedMemoryPool pool(1024, 5);
    
    void* ptr1 = pool.allocate(512);
    assert(ptr1 != NULL);
    
    void* ptr2 = pool.allocate(256);
    assert(ptr2 != NULL);
    assert(ptr1 != ptr2);
    
    pool.deallocate(ptr1);
    pool.deallocate(ptr2);
    
    std::cout << "✓ Basic allocation test passed" << std::endl;
}

void test_batch_creation() {
    std::cout << "Test: Batch creation..." << std::endl;
    
    DistributedMemoryPool pool(sizeof(DatabaseRecord) * 100, 3);
    
    RecordBatch* batch = pool.create_batch(50);
    assert(batch != NULL);
    assert(batch->capacity == 50);
    assert(batch->count == 0);
    
    // Agregar algunos records
    DatabaseRecord record;
    record.id = 1;
    strcpy(record.name, "Test");
    record.value = 10.5;
    record.category = 1;
    
    batch->add_record(record);
    assert(batch->count == 1);
    
    pool.free_batch(batch);
    
    std::cout << "✓ Batch creation test passed" << std::endl;
}

struct ThreadTestData {
    DistributedMemoryPool* pool;
    int thread_id;
    int allocations;
};

void* thread_allocation_test(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (int i = 0; i < data->allocations; ++i) {
        void* ptr = data->pool->allocate(512);
        if (ptr) {
            // Simular uso de memoria
            memset(ptr, data->thread_id, 512);
            usleep(1000); // 1ms
            data->pool->deallocate(ptr);
        }
    }
    
    return NULL;
}

void test_thread_safety() {
    std::cout << "Test: Thread safety..." << std::endl;
    
    DistributedMemoryPool pool(1024, 10);
    const int num_threads = 4;
    const int allocations_per_thread = 50;
    
    pthread_t threads[num_threads];
    ThreadTestData thread_data[num_threads];
    
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].pool = &pool;
        thread_data[i].thread_id = i;
        thread_data[i].allocations = allocations_per_thread;
        
        pthread_create(&threads[i], NULL, thread_allocation_test, &thread_data[i]);
    }
    
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    std::cout << "✓ Thread safety test passed" << std::endl;
}

int test_memory_pool_main() {
    std::cout << "=== Memory Pool Tests ===" << std::endl;
    
    test_basic_allocation();
    test_batch_creation();
    test_thread_safety();
    
    std::cout << "All memory pool tests passed!" << std::endl;
    return 0;
}
