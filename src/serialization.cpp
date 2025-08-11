// =============================================================================
// IMPLEMENTACIÓN DE SERIALIZACIÓN
// =============================================================================

// src/serialization.cpp
#include "serialization.h"
#include <cstring>
#include <cstdint>

namespace distributed {

size_t Serializer::serialize_batch(const RecordBatch* batch, char* buffer, size_t buffer_size) {
    size_t needed = calculate_batch_size(batch);
    if (buffer_size < needed || !batch || !buffer) {
        return 0;
    }
    
    char* ptr = buffer;
    
    // Header del batch
    memcpy(ptr, &batch->count, sizeof(size_t)); ptr += sizeof(size_t);
    memcpy(ptr, &batch->capacity, sizeof(size_t)); ptr += sizeof(size_t);
    memcpy(ptr, &batch->batch_id, sizeof(int)); ptr += sizeof(int);
    
    // Checksum simple para validación
    uint32_t checksum = batch->count ^ batch->capacity ^ batch->batch_id;
    memcpy(ptr, &checksum, sizeof(uint32_t)); ptr += sizeof(uint32_t);
    
    // Records
    if (batch->records && batch->count > 0) {
        memcpy(ptr, batch->records, sizeof(DatabaseRecord) * batch->count);
    }
    
    return needed;
}

bool Serializer::deserialize_batch(const char* buffer, RecordBatch* batch) {
    if (!buffer || !batch) return false;
    
    const char* ptr = buffer;
    
    // Leer header
    size_t count, capacity;
    int batch_id;
    uint32_t checksum;
    
    memcpy(&count, ptr, sizeof(size_t)); ptr += sizeof(size_t);
    memcpy(&capacity, ptr, sizeof(size_t)); ptr += sizeof(size_t);
    memcpy(&batch_id, ptr, sizeof(int)); ptr += sizeof(int);
    memcpy(&checksum, ptr, sizeof(uint32_t)); ptr += sizeof(uint32_t);
    
    // Validar checksum
    uint32_t expected_checksum = count ^ capacity ^ batch_id;
    if (checksum != expected_checksum) {
        return false;
    }
    
    // Validar que el batch tenga suficiente capacidad
    if (!batch->records || batch->capacity < count) {
        return false;
    }
    
    // Actualizar batch
    batch->count = count;
    batch->batch_id = batch_id;
    
    // Copiar records si hay datos
    if (count > 0) {
        memcpy(batch->records, ptr, sizeof(DatabaseRecord) * count);
    }
    
    return true;
}

size_t Serializer::serialize_node_info(const NodeInfo& node, char* buffer, size_t buffer_size) {
    size_t needed = sizeof(size_t) + node.node_id.length() + 
                   sizeof(size_t) + node.ip_address.length() +
                   sizeof(int) + sizeof(bool) + sizeof(time_t) + sizeof(int);
    
    if (buffer_size < needed) return 0;
    
    char* ptr = buffer;
    
    // node_id
    size_t id_len = node.node_id.length();
    memcpy(ptr, &id_len, sizeof(size_t)); ptr += sizeof(size_t);
    memcpy(ptr, node.node_id.c_str(), id_len); ptr += id_len;
    
    // ip_address
    size_t ip_len = node.ip_address.length();
    memcpy(ptr, &ip_len, sizeof(size_t)); ptr += sizeof(size_t);
    memcpy(ptr, node.ip_address.c_str(), ip_len); ptr += ip_len;
    
    // Otros campos
    memcpy(ptr, &node.port, sizeof(int)); ptr += sizeof(int);
    memcpy(ptr, &node.is_alive, sizeof(bool)); ptr += sizeof(bool);
    memcpy(ptr, &node.last_seen, sizeof(time_t)); ptr += sizeof(time_t);
    memcpy(ptr, &node.load_factor, sizeof(int)); ptr += sizeof(int);
    
    return needed;
}

bool Serializer::deserialize_node_info(const char* buffer, NodeInfo& node) {
    if (!buffer) return false;
    
    const char* ptr = buffer;
    
    // node_id
    size_t id_len;
    memcpy(&id_len, ptr, sizeof(size_t)); ptr += sizeof(size_t);
    if (id_len > 1000) return false; // Sanity check
    
    node.node_id.assign(ptr, id_len); ptr += id_len;
    
    // ip_address
    size_t ip_len;
    memcpy(&ip_len, ptr, sizeof(size_t)); ptr += sizeof(size_t);
    if (ip_len > 100) return false; // Sanity check
    
    node.ip_address.assign(ptr, ip_len); ptr += ip_len;
    
    // Otros campos
    memcpy(&node.port, ptr, sizeof(int)); ptr += sizeof(int);
    memcpy(&node.is_alive, ptr, sizeof(bool)); ptr += sizeof(bool);
    memcpy(&node.last_seen, ptr, sizeof(time_t)); ptr += sizeof(time_t);
    memcpy(&node.load_factor, ptr, sizeof(int)); ptr += sizeof(int);
    
    return true;
}

size_t Serializer::calculate_batch_size(const RecordBatch* batch) {
    if (!batch) return 0;
    
    return sizeof(size_t) * 2 + sizeof(int) + sizeof(uint32_t) + 
           sizeof(DatabaseRecord) * batch->count;
}

bool Serializer::validate_serialized_data(const char* buffer, size_t size) {
    if (!buffer || size < sizeof(size_t) * 2 + sizeof(int) + sizeof(uint32_t)) {
        return false;
    }
    
    const char* ptr = buffer;
    size_t count, capacity;
    
    memcpy(&count, ptr, sizeof(size_t)); ptr += sizeof(size_t);
    memcpy(&capacity, ptr, sizeof(size_t));
    
    // Validaciones básicas
    if (count > capacity || capacity > 100000) { // Límites razonables
        return false;
    }
    
    size_t expected_size = sizeof(size_t) * 2 + sizeof(int) + sizeof(uint32_t) + 
                          sizeof(DatabaseRecord) * count;
    
    return size >= expected_size;
}

} // namespace distributed
