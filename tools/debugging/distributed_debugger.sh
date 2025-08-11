#!/bin/bash

echo "=== DISTRIBUTED SYSTEM DEBUGGER ==="
echo ""

# Función para debugging de cluster
debug_cluster() {
    local num_nodes=${1:-3}
    
    echo "Iniciando debugging de cluster con $num_nodes nodos..."
    
    # Array para almacenar PIDs
    declare -a NODE_PIDS
    
    # Iniciar nodos
    for i in $(seq 0 $((num_nodes-1))); do
        local port=$((8080 + i))
        local node_name="debug_node_$i"
        
        if [ $i -eq 0 ]; then
            # Nodo maestro
            echo "Iniciando nodo maestro: $node_name en puerto $port"
            ./bin/distributed_system $node_name 127.0.0.1 $port > logs/debug_$node_name.log 2>&1 &
        else
            # Nodos workers
            echo "Iniciando worker: $node_name en puerto $port"
            ./bin/distributed_system $node_name 127.0.0.1 $port 127.0.0.1 8080 > logs/debug_$node_name.log 2>&1 &
        fi
        
        NODE_PIDS[$i]=$!
        sleep 2
    done
    
    echo ""
    echo "Cluster iniciado. PIDs: ${NODE_PIDS[@]}"
    echo "Logs en: logs/debug_node_*.log"
    
    # Monitorear por 30 segundos
    echo ""
    echo "Monitoreando cluster por 30 segundos..."
    
    for second in {1..30}; do
        echo -n "."
        
        # Verificar que todos los nodos estén vivos
        for i in "${!NODE_PIDS[@]}"; do
            if ! kill -0 ${NODE_PIDS[$i]} 2>/dev/null; then
                echo ""
                echo "⚠ Nodo $i (PID ${NODE_PIDS[$i]}) ha terminado inesperadamente!"
                echo "Ver logs/debug_node_$i.log para detalles"
            fi
        done
        
        sleep 1
    done
    
    echo ""
    echo ""
    echo "=== Resumen de Debugging ==="
    
    # Mostrar estadísticas de cada nodo
    for i in "${!NODE_PIDS[@]}"; do
        local pid=${NODE_PIDS[$i]}
        local node_name="debug_node_$i"
        
        echo ""
        echo "Nodo $i ($node_name):"
        
        if kill -0 $pid 2>/dev/null; then
            echo "  Estado: ACTIVO (PID: $pid)"
            
            # CPU y memoria
            local cpu_mem=$(ps -p $pid -o %cpu,%mem --no-headers 2>/dev/null)
            echo "  CPU/Memoria: $cpu_mem"
            
            # Número de threads
            local threads=$(ls /proc/$pid/task 2>/dev/null | wc -l)
            echo "  Threads: $threads"
        else
            echo "  Estado: TERMINADO"
        fi
        
        # Analizar log para errores
        if [ -f "logs/debug_$node_name.log" ]; then
            local errors=$(grep -i "error\|exception\|failed\|crash" logs/debug_$node_name.log | wc -l)
            local warnings=$(grep -i "warning\|warn" logs/debug_$node_name.log | wc -l)
            echo "  Errores en log: $errors"
            echo "  Warnings en log: $warnings"
            
            # Mostrar último error si existe
            local last_error=$(grep -i "error\|exception\|failed\|crash" logs/debug_$node_name.log | tail -1)
            if [ -n "$last_error" ]; then
                echo "  Último error: $last_error"
            fi
        fi
    done
    
    # Terminar todos los nodos
    echo ""
    echo "Terminando cluster..."
    for pid in "${NODE_PIDS[@]}"; do
        kill $pid 2>/dev/null
    done
    
    # Esperar terminación
    sleep 2
    
    # Force kill si es necesario
    for pid in "${NODE_PIDS[@]}"; do
        kill -9 $pid 2>/dev/null
    done
    
    echo "✓ Debugging de cluster completado"
}

# Función para debugging de memoria compartida
debug_shared_memory() {
    echo ""
    echo "=== Debugging de Memoria Compartida ==="
    
    # Mostrar objetos de memoria compartida existentes
    echo "Objetos de memoria compartida existentes:"
    ipcs -m
    
    echo ""
    echo "Semáforos existentes:"
    ipcs -s
    
    # Ejecutar sistema y monitorear memoria compartida
    echo ""
    echo "Iniciando sistema para monitorear memoria compartida..."
    
    ./bin/distributed_system debug_shm 127.0.0.1 8080 &
    SYSTEM_PID=$!
    
    sleep 5
    
    echo ""
    echo "Objetos creados por el sistema:"
    ipcs -m | grep $(whoami)
    
    echo ""
    echo "Semáforos creados por el sistema:"
    ipcs -s | grep $(whoami)
    
    kill $SYSTEM_PID 2>/dev/null
    sleep 2
    
    echo ""
    echo "Objetos después de terminar el sistema:"
    ipcs -m | grep $(whoami) || echo "Todos los objetos de memoria compartida limpiados ✓"
    ipcs -s | grep $(whoami) || echo "Todos los semáforos limpiados ✓"
}

# Función para debugging de plugins
debug_plugins() {
    echo ""
    echo "=== Debugging de Plugins ==="
    
    # Verificar plugins disponibles
    echo "Plugins disponibles:"
    ls -la plugins/*.so 2>/dev/null || echo "No hay plugins compilados"
    
    # Verificar dependencias de plugins
    echo ""
    echo "Dependencias de plugins:"
    for plugin in plugins/*.so; do
        if [ -f "$plugin" ]; then
            echo ""
            echo "Plugin: $(basename $plugin)"
            ldd "$plugin" 2>/dev/null | head -5
        fi
    done
    
    # Verificar símbolos de plugins
    echo ""
    echo "Símbolos exportados por plugins:"
    for plugin in plugins/*.so; do
        if [ -f "$plugin" ]; then
            echo ""
            echo "Plugin: $(basename $plugin)"
            nm -D "$plugin" 2>/dev/null | grep -E "(process_batch|init_plugin|cleanup_plugin)" || echo "  Símbolos no encontrados"
        fi
    done
}

# Función principal
main() {
    mkdir -p logs
    
    # Verificar que el sistema esté compilado
    if [ ! -f "bin/distributed_system" ]; then
        echo "Error: Sistema no compilado. Ejecutar 'make all' primero."
        exit 1
    fi
    
    echo "Seleccionar tipo de debugging:"
    echo "1. Debugging de cluster distribuido"
    echo "2. Debugging de memoria compartida"
    echo "3. Debugging de plugins"
    echo "4. Todo"
    echo ""
    read -p "Opción (1-4): " choice
    
    case $choice in
        1)
            debug_cluster
            ;;
        2)
            debug_shared_memory
            ;;
        3)
            debug_plugins
            ;;
        4)
            debug_cluster
            debug_shared_memory
            debug_plugins
            ;;
        *)
            echo "Opción inválida"
            exit 1
            ;;
    esac
    
    echo ""
    echo "=== DEBUGGING COMPLETADO ==="
    echo "Revisar logs/ para análisis detallado"
}

main "$@"
