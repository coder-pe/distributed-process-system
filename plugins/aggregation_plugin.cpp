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

// aggregation_plugin.cpp
// Plugin de agregación y estadísticas

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <memory>
#include <pthread.h>

struct DatabaseRecord {
    int id;
    char name[100];
    double value;
    int category;
};

struct RecordBatch {
    DatabaseRecord* records;
    size_t count;
    size_t capacity;
};

struct PluginContext {
    void* user_data;
    const char* config_params;
    void (*log_info)(const char* message);
    void (*log_error)(const char* message);
};

struct AggregationData {
    double total_sum;
    double total_sum_squared;
    size_t total_count;
    double min_value;
    double max_value;
    bool compute_stats;
    pthread_mutex_t mutex;
};

static void parse_config(const char* params, AggregationData* data) {
    data->compute_stats = true;
    
    if (!params) return;
    
    char* params_copy = new char[strlen(params) + 1];
    strcpy(params_copy, params);
    
    char* token = strtok(params_copy, ",");
    while (token != NULL) {
        char* equals = strchr(token, '=');
        if (equals != NULL) {
            *equals = '\0';
            char* key = token;
            char* value = equals + 1;
            
            if (strcmp(key, "compute_stats") == 0) {
                data->compute_stats = (strcmp(value, "true") == 0);
            }
        }
        token = strtok(NULL, ",");
    }
    
    delete[] params_copy;
}

extern "C" {

int init_plugin(PluginContext* context) {
    if (!context) return -1;
    
    AggregationData* data = new AggregationData();
    data->total_sum = 0.0;
    data->total_sum_squared = 0.0;
    data->total_count = 0;
    data->min_value = 1e9;
    data->max_value = -1e9;
    
    parse_config(context->config_params, data);
    pthread_mutex_init(&data->mutex, NULL);
    
    context->user_data = data;
    
    if (context->log_info) {
        context->log_info("Plugin de agregación inicializado");
    }
    
    return 0;
}

void cleanup_plugin(PluginContext* context) {
    if (!context || !context->user_data) return;
    
    AggregationData* data = static_cast<AggregationData*>(context->user_data);
    
    if (context->log_info && data->total_count > 0) {
        double avg = data->total_sum / data->total_count;
        double variance = (data->total_sum_squared / data->total_count) - (avg * avg);
        double std_dev = sqrt(variance);
        
        char msg[512];
        sprintf(msg, "Estadísticas finales: Registros=%zu, Promedio=%.2f, StdDev=%.2f, Min=%.2f, Max=%.2f",
                data->total_count, avg, std_dev, data->min_value, data->max_value);
        context->log_info(msg);
    }
    
    pthread_mutex_destroy(&data->mutex);
    delete data;
    context->user_data = NULL;
}

int process_batch(RecordBatch* batch, PluginContext* context) {
    if (!batch || !context || !context->user_data) return -1;
    
    AggregationData* data = static_cast<AggregationData*>(context->user_data);
    
    if (!data->compute_stats) return 0;
    
    // Calcular estadísticas del lote localmente
    double batch_sum = 0.0;
    double batch_sum_squared = 0.0;
    double batch_min = 1e9;
    double batch_max = -1e9;
    
    for (size_t i = 0; i < batch->count; i++) {
        double value = batch->records[i].value;
        batch_sum += value;
        batch_sum_squared += value * value;
        
        if (value < batch_min) batch_min = value;
        if (value > batch_max) batch_max = value;
    }
    
    // Actualizar estadísticas globales de forma thread-safe
    pthread_mutex_lock(&data->mutex);
    data->total_sum += batch_sum;
    data->total_sum_squared += batch_sum_squared;
    data->total_count += batch->count;
    
    if (batch_min < data->min_value) data->min_value = batch_min;
    if (batch_max > data->max_value) data->max_value = batch_max;
    pthread_mutex_unlock(&data->mutex);
    
    return 0;
}

const char* get_plugin_info(const char* info_type) {
    if (!info_type) return NULL;
    
    if (strcmp(info_type, "name") == 0) {
        return "Statistical Aggregation Plugin";
    } else if (strcmp(info_type, "version") == 0) {
        return "1.0.0";
    } else if (strcmp(info_type, "description") == 0) {
        return "Plugin para cálculo de estadísticas y agregaciones en tiempo real";
    }
    
    return NULL;
}

}
