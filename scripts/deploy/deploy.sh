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

echo "=== DEPLOYMENT AUTOMÁTICO ===
echo ""

# Configuración
CONFIG_FILE="config/deployment.conf"
DEFAULT_SERVERS="server1:127.0.0.1:8080 server2:127.0.0.1:8081 server3:127.0.0.1:8082"

# Leer configuración si existe
if [ -f "$CONFIG_FILE" ]; then
    source "$CONFIG_FILE"
else
    echo "Archivo de configuración no encontrado. Usando valores por defecto."
    SERVERS="$DEFAULT_SERVERS"
fi

# Función para deployment en servidor remoto
deploy_to_server() {
    local server_info=$1
    local server_name=$(echo $server_info | cut -d: -f1)
    local server_ip=$(echo $server_info | cut -d: -f2)
    local server_port=$(echo $server_info | cut -d: -f3)
    
    echo "Deploying a $server_name ($server_ip:$server_port)..."
    
    # Aquí iría la lógica real de deployment
    # Por ahora simulamos el proceso
    
    echo "  1. Verificando conectividad..."
    if ping -c 1 $server_ip >/dev/null 2>&1; then
        echo "  ✓ Servidor accesible"
    else
        echo "  ✗ Servidor no accesible"
        return 1
    fi
    
    echo "  2. Creando paquete..."
    ./scripts/deploy/package.sh >/dev/null 2>&1
    
    echo "  3. Transfiriendo archivos..."
    # scp packages/*.tar.gz user@$server_ip:/tmp/
    echo "  ✓ Archivos transferidos (simulado)"
    
    echo "  4. Instalando en servidor remoto..."
    # ssh user@$server_ip "cd /tmp && tar -xzf *.tar.gz && cd distributed-* && sudo ./install.sh"
    echo "  ✓ Instalación completada (simulado)"
    
    echo "  5. Iniciando servicio..."
    # ssh user@$server_ip "distributed_system $server_name $server_ip $server_port"
    echo "  ✓ Servicio iniciado (simulado)"
    
    echo "  ✓ Deployment a $server_name completado"
    return 0
}

# Función para deployment local (para testing)
deploy_local() {
    echo "Deployment local para testing..."
    
    # Compilar y crear paquete
    echo "1. Creando paquete local..."
    ./scripts/deploy/package.sh
    
    # Simular instalación local
    echo "2. Instalando localmente..."
    cd packages
    tar -xzf distributed-processing-system-*.tar.gz
    cd distributed-processing-system-*
    
    echo "3. Ejecutando instalación de prueba..."
    mkdir -p /tmp/distributed-test-install
    ./install.sh /tmp/distributed-test-install
    
    echo "4. Verificando instalación..."
    if [ -f "/tmp/distributed-test-install/bin/distributed_system" ]; then
        echo "  ✓ Binario instalado correctamente"
    else
        echo "  ✗ Error en instalación"
        return 1
    fi
    
    cd ../../..
    
    echo "5. Limpiando instalación de prueba..."
    rm -rf /tmp/distributed-test-install
    
    echo "✓ Deployment local verificado"
}

# Función principal
main() {
    case "${1:-local}" in
        "local")
            deploy_local
            ;;
        "remote")
            echo "Iniciando deployment remoto..."
            
            # Parsear servidores
            IFS=' ' read -ra SERVER_LIST <<< "$SERVERS"
            
            local failed=0
            for server in "${SERVER_LIST[@]}"; do
                if ! deploy_to_server "$server"; then
                    ((failed++))
                fi
                echo ""
            done
            
            echo "=== Resumen de Deployment ==="
            echo "Servidores totales: ${#SERVER_LIST[@]}"
            echo "Exitosos: $((${#SERVER_LIST[@]} - failed))"
            echo "Fallidos: $failed"
            
            if [ $failed -eq 0 ]; then
                echo "✓ Deployment remoto completado exitosamente"
            else
                echo "⚠ Deployment completado con errores"
                return 1
            fi
            ;;
        "clean")
            echo "Limpiando archivos de deployment..."
            rm -rf packages/
            echo "✓ Limpieza completada"
            ;;
        *)
            echo "Uso: $0 [local|remote|clean]"
            echo ""
            echo "  local  - Deployment local para testing"
            echo "  remote - Deployment a servidores remotos"
            echo "  clean  - Limpiar archivos de deployment"
            exit 1
            ;;
    esac
}

main "$@"
