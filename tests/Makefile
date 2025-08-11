// =============================================================================
// MAKEFILE PARA TESTS UNITARIOS
// =============================================================================

# tests/Makefile
cat > tests/Makefile << 'TEST_MAKEFILE'
CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++98 -pthread -g -I../include
LDFLAGS = -pthread -lrt

SRC_DIR = ../src
BUILD_DIR = build
BIN_DIR = bin

# Test sources
TEST_SOURCES = test_memory_pool.cpp test_serialization.cpp test_configuration.cpp test_all.cpp
TEST_OBJECTS = $(TEST_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
TEST_TARGETS = $(TEST_SOURCES:%.cpp=$(BIN_DIR)/%)

# Core library objects (sin main.cpp)
CORE_SOURCES = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
CORE_OBJECTS = $(CORE_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean setup run-tests help

all: setup $(TEST_TARGETS)

setup:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Compilar objetos core
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compilando core object: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compilar tests
$(BUILD_DIR)/test_%.o: test_%.cpp
	@echo "Compilando test: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Enlazar tests individuales
$(BIN_DIR)/test_%: $(BUILD_DIR)/test_%.o $(CORE_OBJECTS)
	@echo "Enlazando test: $@"
	$(CXX) -o $@ $< $(CORE_OBJECTS) $(LDFLAGS)

# Test completo
$(BIN_DIR)/test_all: $(BUILD_DIR)/test_all.o $(CORE_OBJECTS)
	@echo "Enlazando test completo: $@"
	$(CXX) -o $@ $< $(CORE_OBJECTS) $(LDFLAGS)

# Ejecutar todos los tests
run-tests: all
	@echo "=== Ejecutando Tests Unitarios ==="
	@echo ""
	@for test in $(BIN_DIR)/test_*; do \
		echo "Ejecutando $$test..."; \
		$$test; \
		echo ""; \
	done

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

help:
	@echo "Tests Unitarios del Sistema Distribuido"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  all        - Compilar todos los tests"
	@echo "  run-tests  - Ejecutar todos los tests"
	@echo "  clean      - Limpiar archivos compilados"
	@echo ""
	@echo "Tests individuales:"
	@echo "  test_memory_pool    - Test del memory pool"
	@echo "  test_serialization  - Test de serialización"
	@echo "  test_configuration  - Test de configuración"
	@echo "  test_all           - Test completo del sistema"
TEST_MAKEFILE

echo ""
echo "=== SISTEMA MODULAR COMPLETADO ==="
echo ""
echo "Implementaciones agregadas:"
echo "  ✓ DistributedNode completo con clustering"
echo "  ✓ ResilientPluginManager con failover" 
echo "  ✓ Tests unitarios para cada módulo"
echo "  ✓ Makefile para tests independientes"
echo ""
echo "Estructura final del proyecto:"
echo "  include/    - Headers modulares (11 archivos)"
echo "  src/        - Implementaciones (9 archivos)"
echo "  tests/      - Tests unitarios (4 archivos)"
echo "  plugins/    - Bibliotecas compartidas"
echo "  config/     - Configuraciones del sistema"
echo "  docs/       - Documentación de arquitectura"
echo ""
echo "Para usar el sistema completo:"
echo "1. ./generate_modular_system.sh"
echo "2. make all"
echo "3. cd tests && make run-tests"
echo "4. make test (test de integración)"
