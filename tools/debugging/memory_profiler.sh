#!/bin/bash

# Copyright (C) 2025 Miguel Mamani <miguel.coder.per@gmail.com>
#
# This file is part of the Distributed Processing System.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

echo "=== MEMORY PROFILER PARA SISTEMA DISTRIBUIDO ===
echo ""

VALGRIND_AVAILABLE=false
if command -v valgrind &> /dev/null; then
    VALGRIND_AVAILABLE=true
fi

# Función para profiling básico
basic_memory_profile() {
    echo "1. Profiling básico de memoria..."
    
    if [ "$VALGRIND_AVAILABLE" = true ]; then
        echo "Ejecutando con Valgrind..."
        valgrind --tool=memcheck \
                --leak-check=full \
                --show-leak-kinds=all \
                --track-origins=yes \
                --log-file=logs/valgrind_memcheck.log \
                ./bin/distributed_system test_node 127.0.0.1 8080 &
        
        VALGRIND_PID=$!
        sleep 10  # Dejar ejecutar por 10 segundos
        kill $VALGRIND_PID 2>/dev/null
        
        echo "Reporte de Valgrind guardado en: logs/valgrind_memcheck.log"
        
        # Mostrar resumen
        if [ -f "logs/valgrind_memcheck.log" ]; then
            echo ""
            echo "=== Resumen de Memory Leaks ==="
            grep -E "(definitely lost|indirectly lost|possibly lost)" logs/valgrind_memcheck.log | head -5
        fi
    else
        echo "Valgrind no disponible. Instalar con: sudo apt install valgrind"
        
        # Profiling básico usando /proc/meminfo
        echo "Profiling básico usando herramientas del sistema..."
        
        # Ejecutar sistema en background
        ./bin/distributed_system test_node 127.0.0.1 8080 &
        SYSTEM_PID=$!
        
        echo "Monitoreando memoria por 30 segundos..."
        echo "Time,RSS_KB,VmSize_KB" > logs/memory_usage.csv
        
        for i in {1..30}; do
            if kill -0 $SYSTEM_PID 2>/dev/null; then
                RSS=$(grep VmRSS /proc/$SYSTEM_PID/status 2>/dev/null | awk '{print $2}')
                VmSize=$(grep VmSize /proc/$SYSTEM_PID/status 2>/dev/null | awk '{print $2}')
                echo "$i,$RSS,$VmSize" >> logs/memory_usage.csv
                sleep 1
            else
                break
            fi
        done
        
        kill $SYSTEM_PID 2>/dev/null
        
        echo "Datos de memoria guardados en: logs/memory_usage.csv"
        
        # Mostrar estadísticas básicas
        if [ -f "logs/memory_usage.csv" ]; then
            echo ""
            echo "=== Estadísticas de Memoria ==="
            tail -n +2 logs/memory_usage.csv | awk -F',' '
                {rss+=$2; vmsize+=$3; count++} 
                END {
                    if(count>0) {
                        printf "RSS promedio: %.1f KB\n", rss/count;
                        printf "VmSize promedio: %.1f KB\n", vmsize/count;
                        printf "Muestras: %d\n", count;
                    }
                }'
        fi
    fi
}

# Función para profiling de performance
performance_profile() {
    echo ""
    echo "2. Profiling de performance..."
    
    if command -v perf &> /dev/null; then
        echo "Ejecutando con perf..."
        
        # Ejecutar sistema con perf
        perf record -g -o logs/perf.data ./bin/distributed_system test_node 127.0.0.1 8080 &
        PERF_PID=$!
        
        sleep 15  # Dejar ejecutar por 15 segundos
        kill $PERF_PID 2>/dev/null
        
        # Generar reporte
        perf report -i logs/perf.data --stdio > logs/perf_report.txt 2>/dev/null
        
        echo "Reporte de performance guardado en: logs/perf_report.txt"
        
        # Mostrar top funciones
        if [ -f "logs/perf_report.txt" ]; then
            echo ""
            echo "=== Top Funciones por CPU ==="
            grep -E "^\s+[0-9]+\.[0-9]+%" logs/perf_report.txt | head -5
        fi
    else
        echo "perf no disponible. Instalar con: sudo apt install linux-tools-generic"
        
        # Performance básico usando time
        echo "Profiling básico usando time..."
        
        echo ""
        echo "=== Performance Test ==="
        /usr/bin/time -v ./bin/distributed_system test_node 127.0.0.1 8080 2>&1 | grep -E "(User time|System time|Maximum resident|Page faults)"
    fi
}

# Función para análisis de threads
thread_analysis() {
    echo ""
    echo "3. Análisis de threads..."
    
    # Ejecutar sistema
    ./bin/distributed_system test_node 127.0.0.1 8080 &
    SYSTEM_PID=$!
    
    sleep 5  # Dejar que se inicialice
    
    if kill -0 $SYSTEM_PID 2>/dev/null; then
        echo "PID del sistema: $SYSTEM_PID"
        
        # Contar threads
        THREAD_COUNT=$(ls /proc/$SYSTEM_PID/task 2>/dev/null | wc -l)
        echo "Número de threads: $THREAD_COUNT"
        
        # Mostrar threads
        echo ""
        echo "=== Threads Activos ==="
        ps -T -p $SYSTEM_PID 2>/dev/null | head -10
        
        # CPU usage por thread
        echo ""
        echo "=== CPU Usage por Thread ==="
        top -H -p $SYSTEM_PID -n 1 -b 2>/dev/null | grep $SYSTEM_PID | head -5
    fi
    
    kill $SYSTEM_PID 2>/dev/null
}

# Función principal
main() {
    # Crear directorio de logs si no existe
    mkdir -p logs
    
    echo "Herramientas disponibles:"
    echo "  Valgrind: $VALGRIND_AVAILABLE"
    echo "  Perf: $(command -v perf &> /dev/null && echo true || echo false)"
    echo "  Time: $(command -v time &> /dev/null && echo true || echo false)"
    echo ""
    
    # Verificar que el sistema esté compilado
    if [ ! -f "bin/distributed_system" ]; then
        echo "Error: Sistema no compilado. Ejecutar 'make all' primero."
        exit 1
    fi
    
    basic_memory_profile
    performance_profile
    thread_analysis
    
    echo ""
    echo "=== PROFILING COMPLETADO ==="
    echo ""
    echo "Archivos generados:"
    ls -la logs/ | grep -E "\.(log|csv|txt|data)$"
    
    echo ""
    echo "Para análisis más detallado:"
    echo "  - Ver logs/valgrind_memcheck.log para memory leaks"
    echo "  - Importar logs/memory_usage.csv a Excel/LibreOffice"
    echo "  - Revisar logs/perf_report.txt para hotspots de CPU"
}

main "$@"
