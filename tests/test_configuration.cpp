// tests/test_configuration.cpp
#include "../include/configuration.h"
#include <cassert>
#include <iostream>
#include <fstream>

using namespace distributed;

void test_config_parsing() {
    std::cout << "Test: Configuration parsing..." << std::endl;
    
    // Crear archivo de configuración de prueba
    const char* test_config = "test_config.txt";
    std::ofstream file(test_config);
    file << "# Test configuration\n";
    file << "test_plugin|./test.so|param=value|true|RETRY_WITH_BACKOFF|3|5000|\n";
    file << "disabled_plugin|./disabled.so|param=value|false|FAIL_FAST|1|1000|\n";
    file.close();
    
    ConfigurationManager config(test_config);
    bool loaded = config.load_configuration(test_config);
    assert(loaded);
    
    const std::vector<PipelineStageConfig>& stages = config.get_pipeline_stages();
    assert(stages.size() == 2);
    
    // Verificar primer stage
    assert(stages[0].name == "test_plugin");
    assert(stages[0].library_path == "./test.so");
    assert(stages[0].parameters == "param=value");
    assert(stages[0].enabled == true);
    assert(stages[0].failover_config.policy == RETRY_WITH_BACKOFF);
    assert(stages[0].failover_config.max_retries == 3);
    assert(stages[0].failover_config.timeout_ms == 5000);
    
    // Verificar segundo stage
    assert(stages[1].name == "disabled_plugin");
    assert(stages[1].enabled == false);
    assert(stages[1].failover_config.policy == FAIL_FAST);
    
    // Limpiar
    unlink(test_config);
    
    std::cout << "✓ Configuration parsing test passed" << std::endl;
}

void test_config_save_load() {
    std::cout << "Test: Configuration save/load..." << std::endl;
    
    const char* test_config = "test_save_load.txt";
    
    ConfigurationManager config(test_config);
    
    // Crear configuración sample
    bool created = ConfigurationManager::create_sample_config(test_config);
    assert(created);
    
    // Cargar configuración
    bool loaded = config.load_configuration(test_config);
    assert(loaded);
    
    const std::vector<PipelineStageConfig>& stages = config.get_pipeline_stages();
    assert(stages.size() > 0);
    
    // Guardar configuración
    const char* output_config = "test_output.txt";
    bool saved = config.save_configuration(output_config);
    assert(saved);
    
    // Verificar que el archivo fue creado
    std::ifstream test_file(output_config);
    assert(test_file.good());
    test_file.close();
    
    // Limpiar
    unlink(test_config);
    unlink(output_config);
    
    std::cout << "✓ Configuration save/load test passed" << std::endl;
}

int test_configuration_main() {
    std::cout << "=== Configuration Tests ===" << std::endl;
    
    test_config_parsing();
    test_config_save_load();
    
    std::cout << "All configuration tests passed!" << std::endl;
    return 0;
}
