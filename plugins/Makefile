# =============================================================================
# MAKEFILE PARA PLUGINS DEL SISTEMA DISTRIBUIDO
# =============================================================================

CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -fPIC -std=c++98 -pthread -I../include
LDFLAGS = -shared -ldl

# Plugin source files
PLUGIN_SOURCES = $(wildcard *_plugin.cpp)
PLUGIN_TARGETS = $(PLUGIN_SOURCES:*_plugin.cpp=lib*.so)

# Specific plugin mappings
VALIDATION_LIB = libvalidation.so
ENRICHMENT_LIB = libenrichment.so
AGGREGATION_LIB = libaggregation.so
AUDIT_LIB = libaudit.so
ENCRYPTION_LIB = libencryption.so

.PHONY: all clean list verify help install

all: $(VALIDATION_LIB) $(ENRICHMENT_LIB) $(AGGREGATION_LIB) $(AUDIT_LIB) $(ENCRYPTION_LIB)

# Build individual plugins
$(VALIDATION_LIB): validation_plugin.cpp
	@echo "Building validation plugin..."
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

$(ENRICHMENT_LIB): enrichment_plugin.cpp
	@echo "Building enrichment plugin..."
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

$(AGGREGATION_LIB): aggregation_plugin.cpp
	@echo "Building aggregation plugin..."
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

$(AUDIT_LIB): audit_plugin.cpp
	@echo "Building audit plugin..."
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

$(ENCRYPTION_LIB): encryption_plugin.cpp
	@echo "Building encryption plugin..."
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

clean:
	@echo "Cleaning plugin libraries..."
	rm -f *.so

list:
	@echo "Available plugin source files:"
	@for file in *_plugin.cpp; do \
		if [ -f "$$file" ]; then \
			plugin_name=$$(basename "$$file" _plugin.cpp); \
			lib_name="lib$$plugin_name.so"; \
			if [ -f "$$lib_name" ]; then \
				echo "  ✓ $$file -> $$lib_name [BUILT]"; \
			else \
				echo "  ○ $$file -> $$lib_name [NOT BUILT]"; \
			fi; \
		fi; \
	done

verify:
	@echo "Verifying plugin libraries..."
	@for lib in lib*.so; do \
		if [ -f "$$lib" ]; then \
			echo "Checking $$lib..."; \
			if file "$$lib" | grep -q "shared object"; then \
				echo "  ✓ Valid shared library"; \
			else \
				echo "  ✗ Invalid shared library"; \
			fi; \
		fi; \
	done

install: all
	@echo "Installing plugins to system directory..."
	@mkdir -p ../bin/plugins
	@cp -v *.so ../bin/plugins/ 2>/dev/null || true

help:
	@echo "Plugin Makefile for Distributed Processing System"
	@echo ""
	@echo "Available targets:"
	@echo "  all      Build all plugins"
	@echo "  clean    Remove all plugin libraries"
	@echo "  list     List plugin source files and build status"
	@echo "  verify   Verify built plugin libraries"
	@echo "  install  Copy plugins to system directory"
	@echo "  help     Show this help message"
	@echo ""
	@echo "Individual plugin targets:"
	@echo "  $(VALIDATION_LIB)   Build validation plugin"
	@echo "  $(ENRICHMENT_LIB)   Build enrichment plugin"
	@echo "  $(AGGREGATION_LIB)  Build aggregation plugin"
	@echo "  $(AUDIT_LIB)        Build audit plugin"
	@echo "  $(ENCRYPTION_LIB)   Build encryption plugin"