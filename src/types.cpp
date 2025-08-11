// =============================================================================
// IMPLEMENTACIONES DE TIPOS BÁSICOS
// =============================================================================

// src/types.cpp
#include "types.h"
#include <cstring>
#include <ctime>

namespace distributed {

DatabaseRecord::DatabaseRecord() : id(0), value(0.0), category(0) {
    memset(name, 0, sizeof(name));
}

RecordBatch::RecordBatch() : records(NULL), count(0), capacity(0), batch_id(0) {}

void RecordBatch::add_record(const DatabaseRecord& record) {
    if (count < capacity && records) {
        records[count++] = record;
    }
}

bool RecordBatch::is_full() const {
    return count >= capacity;
}

void RecordBatch::clear() {
    count = 0;
}

NodeInfo::NodeInfo() : is_alive(false), last_seen(0), load_factor(0) {}

ComponentMetrics::ComponentMetrics() {
    total_calls = 0;
    successful_calls = 0;
    failed_calls = 0;
    timeout_calls = 0;
    total_execution_time_ms = 0.0;
    last_execution_time_ms = 0.0;
    last_success_time = 0;
    last_failure_time = 0;
}

void ComponentMetrics::record_success(double execution_time_ms) {
    total_calls++;
    successful_calls++;
    total_execution_time_ms += execution_time_ms;
    last_execution_time_ms = execution_time_ms;
    last_success_time = time(NULL);
}

void ComponentMetrics::record_failure(double execution_time_ms, bool is_timeout) {
    total_calls++;
    failed_calls++;
    if (is_timeout) timeout_calls++;
    total_execution_time_ms += execution_time_ms;
    last_execution_time_ms = execution_time_ms;
    last_failure_time = time(NULL);
}

double ComponentMetrics::get_success_rate() const {
    return total_calls > 0 ? (double)successful_calls / total_calls : 1.0;
}

double ComponentMetrics::get_average_execution_time() const {
    return total_calls > 0 ? total_execution_time_ms / total_calls : 0.0;
}

} // namespace distributed
