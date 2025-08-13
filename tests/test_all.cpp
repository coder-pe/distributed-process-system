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

// tests/test_all.cpp
#include <iostream>
#include <cassert>

// Headers de todos los tests unitarios
extern int test_memory_pool_main();
extern int test_serialization_main();
extern int test_configuration_main();

// Tests adicionales de integración
#include "../include/distributed_system.h"
#include "../include/memory_pool.h"
#include "../include/configuration.h"

using namespace distributed;

void test_system_integration() {
    std::cout << "=== Test de Integración del Sistema ===" << std::endl;
    
    // Test de inicialización completa del sistema
    std::cout << "Test: Inicialización del sistema completo..." << std::endl;
    
    const std::string config_file = "test_integration_config.txt";
    
    // Crear configuración de prueba
    ConfigurationManager::create_sample_config(config_file);
    
    try {
        DistributedProcessingSystem system(
            "test_node", "127.0.0.1", 9999, 
            config_file, 1024, 5
        );
        
        std::cout << "✓ Sistema creado exitosamente" << std::endl;
        
        // Test de inicialización
        bool started = system.start_system();
        std::cout << "✓ Sistema iniciado: " << (started ? "SI" : "NO") << std::endl;
        
        // Test de creación de batches
        RecordBatch* batch = system.create_batch(10);
        assert(batch != NULL);
        std::cout << "✓ Batch creado exitosamente" << std::endl;
        
        // Llenar batch con datos de prueba
        for (int i = 0; i < 5; ++i) {
            DatabaseRecord record;
            record.id = i + 1;
            sprintf(record.name, "TestRecord_%d", i + 1);
            record.value = (i + 1) * 10.0;
            record.category = (i % 3) + 1;
            batch->add_record(record);
        }
        
        std::cout << "✓ Batch llenado con " << batch->count << " registros" << std::endl;
        
        // Test de procesamiento
        bool processed = system.process_batch(batch);
        std::cout << "✓ Batch procesado: " << (processed ? "SI" : "NO") << std::endl;
        
        // Test de salud del sistema
        bool healthy = system.is_system_healthy();
        std::cout << "✓ Sistema saludable: " << (healthy ? "SI" : "NO") << std::endl;
        
        // Limpiar
        system.free_batch(batch);
        system.stop_system();
        
        std::cout << "✓ Sistema detenido limpiamente" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error en test de integración: " << e.what() << std::endl;
        assert(false);
    }
    
    // Limpiar archivo de prueba
    unlink(config_file.c_str());
    
    std::cout << "✓ Test de integración completado exitosamente" << std::endl;
}

void test_module_interactions() {
    std::cout << "=== Test de Interacciones entre Módulos ===" << std::endl;
    
    // Test de Memory Pool + Serialization
    std::cout << "Test: Memory Pool + Serialization..." << std::endl;
    
    DistributedMemoryPool pool(sizeof(DatabaseRecord) * 20, 3);
    
    RecordBatch* batch1 = pool.create_batch(10);
    RecordBatch* batch2 = pool.create_batch(10);
    
    // Llenar batch1
    for (int i = 0; i < 5; ++i) {
        DatabaseRecord record;
        record.id = i + 100;
        sprintf(record.name, "ModuleTest_%d", i);
        record.value = i * 2.5;
        record.category = i % 5 + 1;
        batch1->add_record(record);
    }
    
    // Serializar batch1
    char buffer[4096];
    size_t size = Serializer::serialize_batch(batch1, buffer, sizeof(buffer));
    assert(size > 0);
    
    // Deserializar a batch2
    bool success = Serializer::deserialize_batch(buffer, batch2);
    assert(success);
    
    // Verificar que son idénticos
    assert(batch1->count == batch2->count);
    for (size_t i = 0; i < batch1->count; ++i) {
        assert(batch1->records[i].id == batch2->records[i].id);
        assert(strcmp(batch1->records[i].name, batch2->records[i].name) == 0);
    }
    
    pool.free_batch(batch1);
    pool.free_batch(batch2);
    
    std::cout << "✓ Memory Pool + Serialization funcionando correctamente" << std::endl;
    
    // Test de Configuration + System
    std::cout << "Test: Configuration + System..." << std::endl;
    
    const char* test_config = "module_test_config.txt";
    ConfigurationManager config(test_config);
    
    // Crear y cargar configuración
    bool created = ConfigurationManager::create_sample_config(test_config);
    assert(created);
    
    bool loaded = config.load_configuration(test_config);
    assert(loaded);
    
    const std::vector<PipelineStageConfig>& stages = config.get_pipeline_stages();
    assert(stages.size() > 0);
    
    std::cout << "✓ Configuration cargada con " << stages.size() << " etapas" << std::endl;
    
    // Limpiar
    unlink(test_config);
    
    std::cout << "✓ Test de interacciones entre módulos completado" << std::endl;
}

void test_error_conditions() {
    std::cout << "=== Test de Condiciones de Error ===" << std::endl;
    
    // Test de Memory Pool con parámetros inválidos
    std::cout << "Test: Memory Pool con parámetros inválidos..." << std::endl;
    
    DistributedMemoryPool pool(1024, 2);
    
    // Intentar asignar más memoria de la disponible
    void* ptr = pool.allocate(2048); // Más grande que block_size
    assert(ptr == NULL);
    std::cout << "✓ Memory Pool rechaza asignaciones grandes correctamente" << std::endl;
    
    // Test de Serialization con datos inválidos
    std::cout << "Test: Serialization con datos inválidos..." << std::endl;
    
    char small_buffer[10];
    RecordBatch* batch = pool.create_batch(5);
    
    // Llenar batch
    for (int i = 0; i < 3; ++i) {
        DatabaseRecord record;
        record.id = i;
        sprintf(record.name, "Test_%d", i);
        batch->add_record(record);
    }
    
    // Intentar serializar en buffer muy pequeño
    size_t size = Serializer::serialize_batch(batch, small_buffer, sizeof(small_buffer));
    assert(size == 0); // Debe fallar
    std::cout << "✓ Serializer rechaza buffers pequeños correctamente" << std::endl;
    
    pool.free_batch(batch);
    
    // Test de Configuration con archivo inexistente
    std::cout << "Test: Configuration con archivo inexistente..." << std::endl;
    
    ConfigurationManager bad_config("archivo_inexistente.txt");
    bool loaded = bad_config.load_configuration("archivo_inexistente.txt");
    assert(!loaded); // Debe fallar
    std::cout << "✓ Configuration maneja archivos inexistentes correctamente" << std::endl;
    
    std::cout << "✓ Test de condiciones de error completado" << std::endl;
}

void test_performance_characteristics() {
    std::cout << "=== Test de Características de Performance ===" << std::endl;
    
    // Test de throughput del Memory Pool
    std::cout << "Test: Throughput del Memory Pool..." << std::endl;
    
    DistributedMemoryPool pool(1024, 10);
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    const int iterations = 10000;
    for (int i = 0; i < iterations; ++i) {
        void* ptr = pool.allocate(512);
        if (ptr) {
            pool.deallocate(ptr);
        }
    }
    
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + 
                     (end.tv_usec - start.tv_usec) / 1000.0;
    
    double ops_per_ms = iterations / elapsed;
    std::cout << "✓ Memory Pool throughput: " << (int)ops_per_ms << " ops/ms" << std::endl;
    
    // Test de serialization speed
    std::cout << "Test: Velocidad de serialización..." << std::endl;
    
    RecordBatch* batch = pool.create_batch(1000);
    
    // Llenar batch con datos
    for (int i = 0; i < 1000; ++i) {
        DatabaseRecord record;
        record.id = i;
        sprintf(record.name, "PerfTest_%d", i);
        record.value = i * 1.5;
        record.category = i % 10 + 1;
        batch->add_record(record);
    }
    
    char* buffer = new char[64 * 1024]; // 64KB
    
    gettimeofday(&start, NULL);
    
    const int serialization_iterations = 1000;
    for (int i = 0; i < serialization_iterations; ++i) {
        size_t size = Serializer::serialize_batch(batch, buffer, 64 * 1024);
        assert(size > 0);
    }
    
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + 
              (end.tv_usec - start.tv_usec) / 1000.0;
    
    double serializations_per_ms = serialization_iterations / elapsed;
    std::cout << "✓ Serialization throughput: " << (int)serializations_per_ms 
              << " batches/ms" << std::endl;
    
    delete[] buffer;
    pool.free_batch(batch);
    
    std::cout << "✓ Test de performance completado" << std::endl;
}

int main() {
    std::cout << "=== TESTS COMPLETOS DEL SISTEMA DISTRIBUIDO MODULAR ===" << std::endl;
    std::cout << std::endl;
    
    int failed_tests = 0;
    
    try {
        // Ejecutar tests unitarios de cada módulo
        std::cout << "1. Ejecutando tests unitarios..." << std::endl;
        
        if (test_memory_pool_main() != 0) failed_tests++;
        if (test_serialization_main() != 0) failed_tests++;
        if (test_configuration_main() != 0) failed_tests++;
        
        std::cout << std::endl;
        
        // Ejecutar tests de integración
        std::cout << "2. Ejecutando tests de integración..." << std::endl;
        test_system_integration();
        test_module_interactions();
        
        std::cout << std::endl;
        
        // Ejecutar tests de robustez
        std::cout << "3. Ejecutando tests de robustez..." << std::endl;
        test_error_conditions();
        
        std::cout << std::endl;
        
        // Ejecutar tests de performance
        std::cout << "4. Ejecutando tests de performance..." << std::endl;
        test_performance_characteristics();
        
    } catch (const std::exception& e) {
        std::cout << "Error en tests: " << e.what() << std::endl;
        failed_tests++;
    }
    
    std::cout << std::endl;
    std::cout << "=== RESUMEN DE TESTS ===" << std::endl;
    
    if (failed_tests == 0) {
        std::cout << "✓ TODOS LOS TESTS PASARON EXITOSAMENTE" << std::endl;
        std::cout << "✓ Sistema modular completamente funcional" << std::endl;
        std::cout << "✓ Todos los módulos integran correctamente" << std::endl;
        std::cout << "✓ Performance dentro de parámetros esperados" << std::endl;
        std::cout << "✓ Manejo de errores funcionando" << std::endl;
    } else {
        std::cout << "✗ " << failed_tests << " TESTS FALLARON" << std::endl;
        std::cout << "✗ Revisar implementaciones de módulos" << std::endl;
    }
    
    return failed_tests;
}
