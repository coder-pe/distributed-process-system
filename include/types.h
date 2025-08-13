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

#ifndef DISTRIBUTED_TYPES_H
#define DISTRIBUTED_TYPES_H

#include <cstddef>
#include <string>
#include <vector>

namespace distributed {

// =============================================================================
// TIPOS BÁSICOS DEL SISTEMA
// =============================================================================

/**
 * @brief Estructura que representa un registro de base de datos
 */
struct DatabaseRecord {
    int id;
    char name[100];
    double value;
    int category;

    DatabaseRecord();
};

/**
 * @brief Lote de registros para procesamiento
 */
struct RecordBatch {
    DatabaseRecord* records;
    size_t count;
    size_t capacity;
    int batch_id;

    RecordBatch();
    void add_record(const DatabaseRecord& record);
    bool is_full() const;
    void clear();
};

/**
 * @brief Estados del sistema de supervision
 */
enum RestartPolicy {
    ONE_FOR_ONE,     ///< Solo reiniciar el proceso que falló
    ONE_FOR_ALL,     ///< Reiniciar todos los procesos supervisados
    REST_FOR_ONE     ///< Reiniciar el proceso que falló y todos los siguientes
};

/**
 * @brief Estados del circuit breaker
 */
enum CircuitBreakerState {
    CLOSED,     ///< Operación normal
    OPEN,       ///< Fallando, no llamar al componente
    HALF_OPEN   ///< Probando si el componente se recuperó
};

/**
 * @brief Políticas de failover
 */
enum FailoverPolicy {
    FAIL_FAST,              ///< Fallar inmediatamente
    RETRY_WITH_BACKOFF,     ///< Reintentar con backoff exponencial
    SKIP_AND_CONTINUE,      ///< Saltar componente y continuar
    USE_FALLBACK_PLUGIN,    ///< Usar componente de respaldo
    ISOLATE_AND_CONTINUE    ///< Aislar componente y continuar sin él
};

/**
 * @brief Información de un nodo en el cluster
 */
struct NodeInfo {
    std::string node_id;
    std::string ip_address;
    int port;
    bool is_alive;
    time_t last_seen;
    int load_factor; ///< 0-100

    NodeInfo();
};

/**
 * @brief Métricas de rendimiento de un componente
 */
struct ComponentMetrics {
    size_t total_calls;
    size_t successful_calls;
    size_t failed_calls;
    size_t timeout_calls;
    double total_execution_time_ms;
    double last_execution_time_ms;
    time_t last_success_time;
    time_t last_failure_time;

    ComponentMetrics();
    void record_success(double execution_time_ms);
    void record_failure(double execution_time_ms, bool is_timeout = false);
    double get_success_rate() const;
    double get_average_execution_time() const;
};

} // namespace distributed

#endif // DISTRIBUTED_TYPES_H
