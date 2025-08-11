// tests/test_serialization.cpp
#include "../include/serialization.h"
#include "../include/memory_pool.h"
#include <cassert>
#include <iostream>

using namespace distributed;

void test_batch_serialization() {
    std::cout << "Test: Batch serialization..." << std::endl;
    
    DistributedMemoryPool pool(sizeof(DatabaseRecord) * 10, 2);
    
    // Crear batch original
    RecordBatch* original = pool.create_batch(5);
    
    for (int i = 0; i < 3; ++i) {
        DatabaseRecord record;
        record.id = i + 1;
        sprintf(record.name, "Record_%d", i + 1);
        record.value = (i + 1) * 10.5;
        record.category = (i % 3) + 1;
        original->add_record(record);
    }
    
    // Serializar
    char buffer[4096];
    size_t serialized_size = Serializer::serialize_batch(original, buffer, sizeof(buffer));
    assert(serialized_size > 0);
    
    // Deserializar
    RecordBatch* copy = pool.create_batch(5);
    bool success = Serializer::deserialize_batch(buffer, copy);
    assert(success);
    
    // Verificar que son iguales
    assert(copy->count == original->count);
    assert(copy->batch_id == original->batch_id);
    
    for (size_t i = 0; i < copy->count; ++i) {
        assert(copy->records[i].id == original->records[i].id);
        assert(strcmp(copy->records[i].name, original->records[i].name) == 0);
        assert(copy->records[i].value == original->records[i].value);
        assert(copy->records[i].category == original->records[i].category);
    }
    
    pool.free_batch(original);
    pool.free_batch(copy);
    
    std::cout << "✓ Batch serialization test passed" << std::endl;
}

void test_node_info_serialization() {
    std::cout << "Test: NodeInfo serialization..." << std::endl;
    
    NodeInfo original;
    original.node_id = "test_node_123";
    original.ip_address = "192.168.1.100";
    original.port = 8080;
    original.is_alive = true;
    original.last_seen = time(NULL);
    original.load_factor = 75;
    
    // Serializar
    char buffer[1024];
    size_t serialized_size = Serializer::serialize_node_info(original, buffer, sizeof(buffer));
    assert(serialized_size > 0);
    
    // Deserializar
    NodeInfo copy;
    bool success = Serializer::deserialize_node_info(buffer, copy);
    assert(success);
    
    // Verificar
    assert(copy.node_id == original.node_id);
    assert(copy.ip_address == original.ip_address);
    assert(copy.port == original.port);
    assert(copy.is_alive == original.is_alive);
    assert(copy.last_seen == original.last_seen);
    assert(copy.load_factor == original.load_factor);
    
    std::cout << "✓ NodeInfo serialization test passed" << std::endl;
}

int test_serialization_main() {
    std::cout << "=== Serialization Tests ===" << std::endl;
    
    test_batch_serialization();
    test_node_info_serialization();
    
    std::cout << "All serialization tests passed!" << std::endl;
    return 0;
}
