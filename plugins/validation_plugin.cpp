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

// validation_plugin.cpp
// Plugin de validación avanzada para el sistema de procesamiento 

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

// Definiciones de las estructuras (deben coincidir con el sistema principal)
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

// Estructura para datos privados del plugin
struct ValidationData {
    bool strict_mode;
    int min_id;
    int max_id;
    double min_value;
    double max_value;
    size_t records_validated;
    size_t records_corrected;
};

// Función para parsear parámetros de configuración
static void parse_config_params(const char* params, ValidationData* data) {
    if (!params || !data) return;
    
    // Valores por defecto
    data->strict_mode = false;
    data->min_id = 1;
    data->max_id = 999999;
    data->min_value = 0.0;
    data->max_value = 100000.0;
    data->records_validated = 0;
    data->records_corrected = 0;
    
    // Parsear parámetros en formato "key=value,key2=value2"
    char* params_copy = new char[strlen(params) + 1];
    strcpy(params_copy, params);
    
    char* token = strtok(params_copy, ",");
    while (token != NULL) {
        char* equals = strchr(token, '=');
        if (equals != NULL) {
            *equals = '\0';
            char* key = token;
            char* value = equals + 1;
            
            if (strcmp(key, "strict_mode") == 0) {
                data->strict_mode = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "min_id") == 0) {
                data->min_id = atoi(value);
            } else if (strcmp(key, "max_id") == 0) {
                data->max_id = atoi(value);
            } else if (strcmp(key, "min_value") == 0) {
                data->min_value = atof(value);
            } else if (strcmp(key, "max_value") == 0) {
                data->max_value = atof(value);
            }
        }
        token = strtok(NULL, ",");
    }
    
    delete[] params_copy;
}

// Función para validar el formato del nombre
static bool is_valid_name(const char* name) {
    if (!name || strlen(name) == 0) return false;
    
    // El nombre debe empezar con una letra
    if (!isalpha(name[0])) return false;
    
    // Solo puede contener letras, números y guiones bajos
    for (size_t i = 1; i < strlen(name); i++) {
        if (!isalnum(name[i]) && name[i] != '_') {
            return false;
        }
    }
    
    return true;
}

// Funciones de la interfaz del plugin (exportadas con C linkage)
extern "C" {

int init_plugin(PluginContext* context) {
    if (!context) return -1;
    
    // Crear datos privados del plugin
    ValidationData* data = new ValidationData();
    parse_config_params(context->config_params, data);
    
    // Guardar en el contexto
    context->user_data = data;
    
    // Log de inicialización
    if (context->log_info) {
        char msg[256];
        sprintf(msg, "Plugin de validación inicializado. Modo estricto: %s, Rango ID: %d-%d, Rango valor: %.2f-%.2f",
                data->strict_mode ? "SI" : "NO",
                data->min_id, data->max_id,
                data->min_value, data->max_value);
        context->log_info(msg);
    }
    
    return 0; // Éxito
}

void cleanup_plugin(PluginContext* context) {
    if (!context || !context->user_data) return;
    
    ValidationData* data = static_cast<ValidationData*>(context->user_data);
    
    // Log de estadísticas finales
    if (context->log_info) {
        char msg[256];
        sprintf(msg, "Plugin de validación: %zu registros validados, %zu registros corregidos",
                data->records_validated, data->records_corrected);
        context->log_info(msg);
    }
    
    delete data;
    context->user_data = NULL;
}

int process_batch(RecordBatch* batch, PluginContext* context) {
    if (!batch || !context || !context->user_data) {
        return -1; // Error: parámetros inválidos
    }
    
    ValidationData* data = static_cast<ValidationData*>(context->user_data);
    
    for (size_t i = 0; i < batch->count; i++) {
        DatabaseRecord& record = batch->records[i];
        data->records_validated++;
        bool corrected = false;
        
        // Validar ID
        if (record.id < data->min_id || record.id > data->max_id) {
            if (data->strict_mode) {
                if (context->log_error) {
                    char msg[128];
                    sprintf(msg, "ID fuera de rango en registro %zu: %d", i, record.id);
                    context->log_error(msg);
                }
                return -2; // Error: ID fuera de rango en modo estricto
            } else {
                // Corregir ID
                if (record.id < data->min_id) record.id = data->min_id;
                if (record.id > data->max_id) record.id = data->max_id;
                corrected = true;
            }
        }
        
        // Validar nombre
        if (!is_valid_name(record.name)) {
            if (data->strict_mode) {
                if (context->log_error) {
                    char msg[128];
                    sprintf(msg, "Nombre inválido en registro %zu: %s", i, record.name);
                    context->log_error(msg);
                }
                return -3; // Error: nombre inválido en modo estricto
            } else {
                // Corregir nombre
                sprintf(record.name, "Record_%d", record.id);
                corrected = true;
            }
        }
        
        // Validar valor
        if (record.value < data->min_value || record.value > data->max_value) {
            if (data->strict_mode) {
                if (context->log_error) {
                    char msg[128];
                    sprintf(msg, "Valor fuera de rango en registro %zu: %.2f", i, record.value);
                    context->log_error(msg);
                }
                return -4; // Error: valor fuera de rango en modo estricto
            } else {
                // Corregir valor
                if (record.value < data->min_value) record.value = data->min_value;
                if (record.value > data->max_value) record.value = data->max_value;
                corrected = true;
            }
        }
        
        // Validar categoría
        if (record.category < 1 || record.category > 10) {
            if (data->strict_mode) {
                if (context->log_error) {
                    char msg[128];
                    sprintf(msg, "Categoría inválida en registro %zu: %d", i, record.category);
                    context->log_error(msg);
                }
                return -5; // Error: categoría inválida en modo estricto
            } else {
                // Corregir categoría
                record.category = 1; // Categoría por defecto
                corrected = true;
            }
        }
        
        if (corrected) {
            data->records_corrected++;
        }
    }
    
    return 0; // Éxito
}

const char* get_plugin_info(const char* info_type) {
    if (!info_type) return NULL;
    
    if (strcmp(info_type, "name") == 0) {
        return "Advanced Validation Plugin";
    } else if (strcmp(info_type, "version") == 0) {
        return "1.2.0";
    } else if (strcmp(info_type, "description") == 0) {
        return "Plugin de validación avanzada con soporte para modo estricto y corrección automática";
    } else if (strcmp(info_type, "author") == 0) {
        return "Tu Equipo de Desarrollo";
    }
    
    return NULL;
}

} // extern "C"