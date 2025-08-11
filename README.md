# Distributed Parallel Processing System

Enterprise-level distributed processing system that implements:
- **Real process isolation** like Erlang
- **Hierarchical supervision trees** like OTP
- **Native distribution** with automatic clustering
- **Hot-swapping** of components without downtime
- **Extreme fault tolerance** with automatic recovery

## Quick Build

```bash
# Generate complete structure
./generate_modular_system.sh

# Build system
make all

# Build plugins
make plugins

# Run basic test
make test
```

## Modular Structure

The system is completely modularized to facilitate:
- **Independent development** of modules
- **Individual testing** of components
- **Simplified maintenance**
- **Future extensibility**

See `docs/ARCHITECTURE.md` for complete architecture details.

## Basic Usage

```bash
# Master node
./bin/distributed_system master 127.0.0.1 8080

# Worker node
./bin/distributed_system worker1 127.0.0.1 8081 127.0.0.1 8080
```

## Main Modules

1. **Core Types & Interfaces** - Basic system definitions
2. **Memory Pool** - Thread-safe efficient memory management
3. **IPC & Serialization** - Optimized inter-process communication
4. **Isolated Processes** - Isolated processes for plugins
5. **Supervision Trees** - Hierarchical supervision with configurable policies
6. **Distributed Nodes** - Clustering and automatic distribution
7. **Plugin Manager** - Dynamic plugin management with failover
8. **Configuration** - Flexible external configuration
9. **Main System** - Orchestration of all components

## Modularization Advantages

### For Development
- Each module can be developed independently
- Clear interfaces between components
- Easy unit testing
- Separation of responsibilities

### For Maintenance
- Localized changes per module
- Easy debugging of specific components
- Per-module documentation
- Granular versioning

### For Extensibility
- New modules without affecting existing ones
- Extensible interfaces
- Consistent design patterns
- Native plugin architecture

## Plugin Development

The system supports dynamic plugins that can be loaded at runtime. All plugins must implement the standard plugin interface.

### Building Plugins

```bash
# Build all plugins using the dedicated script
./scripts/build/build_plugins.sh all

# Alternative: Use the plugin Makefile
cd plugins && make all

# List available plugins
./scripts/build/build_plugins.sh list

# Clean plugin binaries
./scripts/build/build_plugins.sh clean
```

### Available Plugins

1. **Validation Plugin** (`libvalidation.so`) - Data validation with configurable rules
2. **Enrichment Plugin** (`libenrichment.so`) - Data enrichment and transformation  
3. **Aggregation Plugin** (`libaggregation.so`) - Data aggregation and statistics
4. **Audit Plugin** (`libaudit.so`) - Audit logging and compliance tracking
5. **Encryption Plugin** (`libencryption.so`) - Data encryption and security

### Plugin Interface

All plugins must implement these C functions:

```cpp
extern "C" {
    int init_plugin(PluginContext* context);
    void cleanup_plugin(PluginContext* context);
    int process_batch(RecordBatch* batch, PluginContext* context);
    const char* get_plugin_info(const char* info_type);
}
```

### Creating New Plugins

1. Create a new `.cpp` file in the `plugins/` directory
2. Implement the required interface functions
3. Build using the plugin build script or Makefile
4. Configure in `config/basic_pipeline.txt`

### Plugin Configuration

Configure plugins in the pipeline configuration file:

```
# Format: name|library|parameters|enabled|failover_policy|max_retries|timeout_ms
validation|./plugins/libvalidation.so|strict_mode=false|true|RETRY_WITH_BACKOFF|3|10000
enrichment|./plugins/libenrichment.so|factor=1.1|true|SKIP_AND_CONTINUE|2|5000
```

## Modular Testing

```bash
# Specific module test
make test-memory-pool
make test-ipc
make test-supervisor

# Integration test
make test-integration

# Complete system test
make test-full-system

# Plugin verification
./scripts/build/build_plugins.sh verify plugins/libvalidation.so
```