# Documentación del Sistema Distribuido

## 📑 Índice General

### 🏗️ Arquitectura
- [Visión General del Sistema](architecture/SYSTEM_ARCHITECTURE.md)
- [Patrones de Diseño Implementados](architecture/DESIGN_PATTERNS.md)
- [Diagrama de Componentes](architecture/COMPONENT_DIAGRAM.md)
- [Flujo de Datos](architecture/DATA_FLOW.md)

### 🧩 Módulos
- [Guía General de Módulos](modules/README.md)
- [Memory Pool](modules/memory_pool.md)
- [Serialization](modules/serialization.md)
- [IPC Communication](modules/ipc.md)
- [Isolated Processes](modules/isolated_process.md)
- [Supervision Trees](modules/supervisor.md)
- [Distributed Nodes](modules/distributed_node.md)
- [Plugin Manager](modules/plugin_manager.md)
- [Configuration](modules/configuration.md)
- [Main System](modules/distributed_system.md)

### 🔧 API Reference
- [Core Types](api/types.md)
- [Interfaces](api/interfaces.md)
- [Public APIs](api/public_apis.md)
- [Plugin Development API](api/plugin_api.md)

### 📚 Tutoriales
- [Getting Started](tutorials/GETTING_STARTED.md)
- [Conceptos Fundamentales](tutorials/CONCEPTS.md)
- [Primer Plugin](tutorials/FIRST_PLUGIN.md)
- [Configuración Avanzada](tutorials/ADVANCED_CONFIG.md)
- [Debugging y Profiling](tutorials/DEBUGGING.md)

### 🚀 Deployment
- [Instalación](deployment/INSTALLATION.md)
- [Configuración de Producción](deployment/PRODUCTION.md)
- [Clustering](deployment/CLUSTERING.md)
- [Monitoreo](deployment/MONITORING.md)
- [Backup y Recovery](deployment/BACKUP.md)

### 🛠️ Desarrollo
- [Ambiente de Desarrollo](development/ENVIRONMENT.md)
- [Coding Standards](development/CODING_STANDARDS.md)
- [Testing Guidelines](development/TESTING.md)
- [Performance Guidelines](development/PERFORMANCE.md)
- [Contribuir](development/CONTRIBUTING.md)

## 🔍 Búsqueda Rápida

### Por Caso de Uso
- **Quiero entender el sistema**: Comienza con [Conceptos Fundamentales](tutorials/CONCEPTS.md)
- **Quiero compilar y probar**: Ve a [Getting Started](tutorials/GETTING_STARTED.md)
- **Quiero desarrollar un plugin**: Lee [Plugin Development API](api/plugin_api.md)
- **Quiero deployar en producción**: Consulta [Deployment](deployment/README.md)
- **Tengo un problema**: Revisa [Troubleshooting](troubleshooting/README.md)

### Por Módulo
- **Memory management**: [Memory Pool](modules/memory_pool.md)
- **Comunicación**: [IPC](modules/ipc.md) + [Serialization](modules/serialization.md)
- **Fault tolerance**: [Supervisor](modules/supervisor.md) + [Isolated Process](modules/isolated_process.md)
- **Distribución**: [Distributed Node](modules/distributed_node.md)
- **Configuración**: [Configuration](modules/configuration.md)

### Por Audiencia
- **Desarrolladores**: API Reference, Módulos, Development
- **DevOps/SysAdmins**: Deployment, Monitoring, Troubleshooting
- **Arquitectos**: Architecture, Design Patterns, Performance
- **Estudiantes**: Tutorials, Concepts, Examples
