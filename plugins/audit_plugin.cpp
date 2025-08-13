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

// audit_plugin.cpp
// Plugin de auditoría y logging

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <fstream>

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

struct AuditData {
    std::ofstream* log_file;
    char log_level[10];
    size_t records_audited;
    bool log_detailed;
};

static void parse_config(const char* params, AuditData* data) {
    strcpy(data->log_level, "INFO");
    data->log_detailed = false;
    
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
            
            if (strcmp(key, "log_level") == 0) {
                strncpy(data->log_level, value, sizeof(data->log_level) - 1);
                data->log_level[sizeof(data->log_level) - 1] = '\0';
            } else if (strcmp(key, "detailed") == 0) {
                data->log_detailed = (strcmp(value, "true") == 0);
            }
        }
        token = strtok(NULL, ",");
    }
    
    delete[] params_copy;
}

extern "C" {

int init_plugin(PluginContext* context) {
    if (!context) return -1;
    
    AuditData* data = new AuditData();
    data->records_audited = 0;
    parse_config(context->config_params, data);
    
    // Abrir archivo de auditoría
    data->log_file = new std::ofstream("audit_log.txt", std::ios::app);
    if (!data->log_file->is_open()) {
        delete data->log_file;
        delete data;
        return -1;
    }
    
    time_t now = time(0);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; // Remover newline
    
    *(data->log_file) << "[" << time_str << "] Plugin de auditoría iniciado. Nivel: " 
                      << data->log_level << std::endl;
    
    context->user_data = data;
    return 0;
}

void cleanup_plugin(PluginContext* context) {
    if (!context || !context->user_data) return;
    
    AuditData* data = static_cast<AuditData*>(context->user_data);
    
    time_t now = time(0);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    *(data->log_file) << "[" << time_str << "] Plugin de auditoría finalizado. " 
                      << "Registros auditados: " << data->records_audited << std::endl;
    
    data->log_file->close();
    delete data->log_file;
    delete data;
    context->user_data = NULL;
}

int process_batch(RecordBatch* batch, PluginContext* context) {
    if (!batch || !context || !context->user_data) return -1;
    
    AuditData* data = static_cast<AuditData*>(context->user_data);
    
    time_t now = time(0);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    *(data->log_file) << "[" << time_str << "] Procesando lote de " 
                      << batch->count << " registros" << std::endl;
    
    if (data->log_detailed) {
        for (size_t i = 0; i < batch->count; i++) {
            const DatabaseRecord& record = batch->records[i];
            *(data->log_file) << "  Registro " << i << ": ID=" << record.id 
                              << ", Name=" << record.name 
                              << ", Value=" << record.value 
                              << ", Category=" << record.category << std::endl;
        }
    }
    
    data->records_audited += batch->count;
    data->log_file->flush(); // Asegurar escritura inmediata
    
    return 0;
}

const char* get_plugin_info(const char* info_type) {
    if (!info_type) return NULL;
    
    if (strcmp(info_type, "name") == 0) {
        return "Audit and Logging Plugin";
    } else if (strcmp(info_type, "version") == 0) {
        return "1.0.0";
    } else if (strcmp(info_type, "description") == 0) {
        return "Plugin para auditoría y logging detallado del procesamiento";
    }
    
    return NULL;
}

}
