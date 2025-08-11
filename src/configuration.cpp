// =============================================================================
// CONFIGURACIÓN BÁSICA PARA DEMOSTRACIÓN
// =============================================================================

// src/configuration.cpp
#include "configuration.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace distributed {

FailoverConfig::FailoverConfig() {
    policy = SKIP_AND_CONTINUE;
    max_retries = 3;
    initial_delay_ms = 100;
    max_delay_ms = 5000;
    backoff_multiplier = 2.0;
    timeout_ms = 30000;
    enable_circuit_breaker = true;
}

PipelineStageConfig::PipelineStageConfig() : enabled(true) {}

ConfigurationManager::ConfigurationManager(const std::string& config_path) 
    : config_file_path(config_path) {}

ConfigurationManager::~ConfigurationManager() {}

bool ConfigurationManager::load_configuration(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir archivo de configuración: " << filename << std::endl;
        return false;
    }
    
    pipeline_stages.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        PipelineStageConfig config;
        if (parse_config_line(line, config)) {
            pipeline_stages.push_back(config);
        }
    }
    
    file.close();
    config_file_path = filename;
    
    return validate_configuration();
}

bool ConfigurationManager::parse_config_line(const std::string& line, PipelineStageConfig& config) {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string part;
    
    while (std::getline(ss, part, '|')) {
        parts.push_back(part);
    }
    
    if (parts.size() < 4) return false;
    
    config.name = parts[0];
    config.library_path = parts[1];
    config.parameters = parts[2];
    config.enabled = (parts[3] == "true" || parts[3] == "1");
    
    // Configuración de failover opcional
    if (parts.size() > 4 && !parts[4].empty()) {
        config.failover_config.policy = string_to_policy(parts[4]);
    }
    if (parts.size() > 5 && !parts[5].empty()) {
        config.failover_config.max_retries = atoi(parts[5].c_str());
    }
    if (parts.size() > 6 && !parts[6].empty()) {
        config.failover_config.timeout_ms = atoi(parts[6].c_str());
    }
    
    return true;
}

FailoverPolicy ConfigurationManager::string_to_policy(const std::string& policy_str) {
    if (policy_str == "FAIL_FAST") return FAIL_FAST;
    if (policy_str == "RETRY_WITH_BACKOFF") return RETRY_WITH_BACKOFF;
    if (policy_str == "SKIP_AND_CONTINUE") return SKIP_AND_CONTINUE;
    if (policy_str == "USE_FALLBACK_PLUGIN") return USE_FALLBACK_PLUGIN;
    if (policy_str == "ISOLATE_AND_CONTINUE") return ISOLATE_AND_CONTINUE;
    return SKIP_AND_CONTINUE; // Default
}

std::string ConfigurationManager::policy_to_string(FailoverPolicy policy) {
    switch (policy) {
        case FAIL_FAST: return "FAIL_FAST";
        case RETRY_WITH_BACKOFF: return "RETRY_WITH_BACKOFF";
        case SKIP_AND_CONTINUE: return "SKIP_AND_CONTINUE";
        case USE_FALLBACK_PLUGIN: return "USE_FALLBACK_PLUGIN";
        case ISOLATE_AND_CONTINUE: return "ISOLATE_AND_CONTINUE";
        default: return "SKIP_AND_CONTINUE";
    }
}

bool ConfigurationManager::validate_configuration() const {
    // Validaciones básicas
    for (size_t i = 0; i < pipeline_stages.size(); ++i) {
        const PipelineStageConfig& stage = pipeline_stages[i];
        
        if (stage.name.empty() || stage.library_path.empty()) {
            return false;
        }
        
        if (stage.failover_config.max_retries < 0 || 
            stage.failover_config.timeout_ms <= 0) {
            return false;
        }
    }
    
    return true;
}

bool ConfigurationManager::reload_configuration() {
    return load_configuration(config_file_path);
}

bool ConfigurationManager::save_configuration(const std::string& filename) const {
    std::ofstream file(filename.c_str());
    if (!file.is_open()) {
        return false;
    }
    
    file << "# Configuración del Pipeline de Procesamiento Distribuido" << std::endl;
    file << "# Formato: nombre|biblioteca|parámetros|habilitado|política_failover|max_retries|timeout_ms" << std::endl;
    file << "#" << std::endl;
    
    for (size_t i = 0; i < pipeline_stages.size(); ++i) {
        const PipelineStageConfig& stage = pipeline_stages[i];
        
        file << stage.name << "|"
             << stage.library_path << "|"
             << stage.parameters << "|"
             << (stage.enabled ? "true" : "false") << "|"
             << policy_to_string(stage.failover_config.policy) << "|"
             << stage.failover_config.max_retries << "|"
             << stage.failover_config.timeout_ms << std::endl;
    }
    
    file.close();
    return true;
}

const std::vector<PipelineStageConfig>& ConfigurationManager::get_pipeline_stages() const {
    return pipeline_stages;
}

bool ConfigurationManager::create_sample_config(const std::string& filename) {
    std::ofstream file(filename.c_str());
    if (!file.is_open()) {
        return false;
    }
    
    file << "# Configuración de ejemplo del pipeline distribuido" << std::endl;
    file << "validation|./plugins/libvalidation.so|strict_mode=false|true|RETRY_WITH_BACKOFF|3|10000" << std::endl;
    file << "enrichment|./plugins/libenrichment.so|factor=1.1|true|SKIP_AND_CONTINUE|2|5000" << std::endl;
    file << "aggregation|./plugins/libaggregation.so|compute_stats=true|true|ISOLATE_AND_CONTINUE|1|15000" << std::endl;
    
    file.close();
    return true;
}

} // namespace distributed
