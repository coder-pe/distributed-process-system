// enrichment_plugin.cpp
// Plugin de enriquecimiento de datos

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>

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

struct EnrichmentData {
    double multiplication_factor;
    char suffix_format[50];
    bool add_timestamp;
    size_t records_enriched;
};

static void parse_config(const char* params, EnrichmentData* data) {
    data->multiplication_factor = 1.1;
    strcpy(data->suffix_format, "_CAT%d");
    data->add_timestamp = false;
    data->records_enriched = 0;
    
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
            
            if (strcmp(key, "factor") == 0) {
                data->multiplication_factor = atof(value);
            } else if (strcmp(key, "suffix_format") == 0) {
                strncpy(data->suffix_format, value, sizeof(data->suffix_format) - 1);
                data->suffix_format[sizeof(data->suffix_format) - 1] = '\0';
            } else if (strcmp(key, "add_timestamp") == 0) {
                data->add_timestamp = (strcmp(value, "true") == 0);
            }
        }
        token = strtok(NULL, ",");
    }
    
    delete[] params_copy;
}

extern "C" {

int init_plugin(PluginContext* context) {
    if (!context) return -1;
    
    EnrichmentData* data = new EnrichmentData();
    parse_config(context->config_params, data);
    context->user_data = data;
    
    if (context->log_info) {
        char msg[256];
        sprintf(msg, "Plugin de enriquecimiento inicializado. Factor: %.2f, Formato: %s",
                data->multiplication_factor, data->suffix_format);
        context->log_info(msg);
    }
    
    return 0;
}

void cleanup_plugin(PluginContext* context) {
    if (!context || !context->user_data) return;
    
    EnrichmentData* data = static_cast<EnrichmentData*>(context->user_data);
    
    if (context->log_info) {
        char msg[128];
        sprintf(msg, "Plugin de enriquecimiento: %zu registros procesados", data->records_enriched);
        context->log_info(msg);
    }
    
    delete data;
    context->user_data = NULL;
}

int process_batch(RecordBatch* batch, PluginContext* context) {
    if (!batch || !context || !context->user_data) return -1;
    
    EnrichmentData* data = static_cast<EnrichmentData*>(context->user_data);
    
    for (size_t i = 0; i < batch->count; i++) {
        DatabaseRecord& record = batch->records[i];
        
        // Aplicar factor de multiplicación al valor
        record.value *= data->multiplication_factor;
        
        // Agregar sufijo al nombre
        char suffix[20];
        sprintf(suffix, data->suffix_format, record.category);
        
        size_t name_len = strlen(record.name);
        size_t suffix_len = strlen(suffix);
        if (name_len + suffix_len < sizeof(record.name) - 1) {
            strcat(record.name, suffix);
        }
        
        data->records_enriched++;
    }
    
    return 0;
}

const char* get_plugin_info(const char* info_type) {
    if (!info_type) return NULL;
    
    if (strcmp(info_type, "name") == 0) {
        return "Data Enrichment Plugin";
    } else if (strcmp(info_type, "version") == 0) {
        return "1.1.0";
    } else if (strcmp(info_type, "description") == 0) {
        return "Plugin para enriquecimiento de datos con factores y sufijos configurables";
    }
    
    return NULL;
}

}
