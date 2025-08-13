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
