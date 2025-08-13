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

// encryption_plugin.cpp
// Plugin de encriptación simple (ejemplo)

#include <cstring>
#include <cstdlib>
#include <cstdio>

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

struct EncryptionData {
    char algorithm[20];
    int shift_key;
    size_t records_encrypted;
};

// Encriptación César simple (solo para demostración)
static void caesar_encrypt(char* text, int shift) {
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] >= 'A' && text[i] <= 'Z') {
            text[i] = ((text[i] - 'A' + shift) % 26) + 'A';
        } else if (text[i] >= 'a' && text[i] <= 'z') {
            text[i] = ((text[i] - 'a' + shift) % 26) + 'a';
        }
    }
}

static void parse_config(const char* params, EncryptionData* data) {
    strcpy(data->algorithm, "CAESAR");
    data->shift_key = 3;
    data->records_encrypted = 0;
    
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
            
            if (strcmp(key, "algorithm") == 0) {
                strncpy(data->algorithm, value, sizeof(data->algorithm) - 1);
                data->algorithm[sizeof(data->algorithm) - 1] = '\0';
            } else if (strcmp(key, "shift") == 0) {
                data->shift_key = atoi(value);
            }
        }
        token = strtok(NULL, ",");
    }
    
    delete[] params_copy;
}

extern "C" {

int init_plugin(PluginContext* context) {
    if (!context) return -1;
    
    EncryptionData* data = new EncryptionData();
    parse_config(context->config_params, data);
    context->user_data = data;
    
    if (context->log_info) {
        char msg[128];
        sprintf(msg, "Plugin de encriptación inicializado. Algoritmo: %s, Clave: %d",
                data->algorithm, data->shift_key);
        context->log_info(msg);
    }
    
    return 0;
}

void cleanup_plugin(PluginContext* context) {
    if (!context || !context->user_data) return;
    
    EncryptionData* data = static_cast<EncryptionData*>(context->user_data);
    
    if (context->log_info) {
        char msg[128];
        sprintf(msg, "Plugin de encriptación: %zu registros procesados", data->records_encrypted);
        context->log_info(msg);
    }
    
    delete data;
    context->user_data = NULL;
}

int process_batch(RecordBatch* batch, PluginContext* context) {
    if (!batch || !context || !context->user_data) return -1;
    
    EncryptionData* data = static_cast<EncryptionData*>(context->user_data);
    
    for (size_t i = 0; i < batch->count; i++) {
        DatabaseRecord& record = batch->records[i];
        
        // Encriptar solo el nombre (ejemplo)
        if (strcmp(data->algorithm, "CAESAR") == 0) {
            caesar_encrypt(record.name, data->shift_key);
        }
        
        data->records_encrypted++;
    }
    
    return 0;
}

const char* get_plugin_info(const char* info_type) {
    if (!info_type) return NULL;
    
    if (strcmp(info_type, "name") == 0) {
        return "Simple Encryption Plugin";
    } else if (strcmp(info_type, "version") == 0) {
        return "1.0.0";
    } else if (strcmp(info_type, "description") == 0) {
        return "Plugin de encriptación simple para demostración";
    }
    
    return NULL;
}

}
