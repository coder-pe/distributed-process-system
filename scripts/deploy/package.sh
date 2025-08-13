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

echo "=== PACKAGING DEL SISTEMA DISTRIBUIDO ===
echo ""

VERSION=$(cat VERSION 2>/dev/null || echo "1.0.0")
PACKAGE_NAME="distributed-processing-system-$VERSION"
PACKAGE_DIR="packages"

echo "Creando paquete: $PACKAGE_NAME"

# Crear directorio de packaging
mkdir -p $PACKAGE_DIR
rm -rf $PACKAGE_DIR/$PACKAGE_NAME

# Compilar sistema
echo "1. Compilando sistema..."
make clean
make all

if [ $? -ne 0 ]; then
    echo "Error: Compilación falló"
    exit 1
fi

# Crear estructura del paquete
echo "2. Creando estructura del paquete..."
mkdir -p $PACKAGE_DIR/$PACKAGE_NAME/{bin,lib,plugins,config,docs,scripts}

# Copiar binarios
echo "3. Copiando binarios..."
cp bin/distributed_system $PACKAGE_DIR/$PACKAGE_NAME/bin/
cp build/libdistributed.a $PACKAGE_DIR/$PACKAGE_NAME/lib/

# Copiar plugins si existen
if ls plugins/*.so >/dev/null 2>&1; then
    cp plugins/*.so $PACKAGE_DIR/$PACKAGE_NAME/plugins/
fi

# Copiar configuraciones
echo "4. Copiando configuraciones..."
cp config/*.txt $PACKAGE_DIR/$PACKAGE_NAME/config/ 2>/dev/null || true

# Copiar documentación
echo "5. Copiando documentación..."
cp README.md $PACKAGE_DIR/$PACKAGE_NAME/
cp -r docs $PACKAGE_DIR/$PACKAGE_NAME/

# Copiar scripts de instalación
echo "6. Creando scripts de instalación..."
cat > $PACKAGE_DIR/$PACKAGE_NAME/install.sh << 'INSTALL'
#!/bin/bash

echo "=== INSTALACIÓN DEL SISTEMA DISTRIBUIDO ==="
echo ""

PREFIX=${1:-/usr/local}
echo "Instalando en: $PREFIX"

# Verificar permisos
if [ ! -w "$PREFIX" ]; then
    echo "Error: Sin permisos de escritura en $PREFIX"
    echo "Ejecutar como root o especificar otro directorio"
    exit 1
fi

# Crear directorios
mkdir -p $PREFIX/bin
mkdir -p $PREFIX/lib/distributed-system
mkdir -p $PREFIX/share/distributed-system

# Instalar binarios
echo "Instalando binarios..."
cp bin/distributed_system $PREFIX/bin/
chmod +x $PREFIX/bin/distributed_system

# Instalar bibliotecas
echo "Instalando bibliotecas..."
cp lib/* $PREFIX/lib/distributed-system/

# Instalar plugins
echo "Instalando plugins..."
cp plugins/* $PREFIX/lib/distributed-system/ 2>/dev/null || true

# Instalar configuraciones y documentación
echo "Instalando configuraciones..."
cp -r config $PREFIX/share/distributed-system/
cp -r docs $PREFIX/share/distributed-system/
cp README.md $PREFIX/share/distributed-system/

# Crear configuración por defecto
mkdir -p /etc/distributed-system
cp config/*.txt /etc/distributed-system/ 2>/dev/null || true

echo ""
echo "✓ Instalación completada"
echo ""
echo "Para usar el sistema:"
echo "  distributed_system <node_id> <ip> <port>"
echo ""
echo "Documentación en: $PREFIX/share/distributed-system/docs/"
echo "Configuración en: /etc/distributed-system/"
INSTALL

chmod +x $PACKAGE_DIR/$PACKAGE_NAME/install.sh

cat > $PACKAGE_DIR/$PACKAGE_NAME/uninstall.sh << 'UNINSTALL'
#!/bin/bash

echo "=== DESINSTALACIÓN DEL SISTEMA DISTRIBUIDO ==="
echo ""

PREFIX=${1:-/usr/local}
echo "Desinstalando de: $PREFIX"

# Remover archivos
rm -f $PREFIX/bin/distributed_system
rm -rf $PREFIX/lib/distributed-system
rm -rf $PREFIX/share/distributed-system
rm -rf /etc/distributed-system

echo "✓ Desinstalación completada"
UNINSTALL

chmod +x $PACKAGE_DIR/$PACKAGE_NAME/uninstall.sh

# Crear archivo de información del paquete
cat > $PACKAGE_DIR/$PACKAGE_NAME/PACKAGE_INFO << PKG_INFO
Package: distributed-processing-system
Version: $VERSION
Architecture: $(uname -m)
Build Date: $(date)
Build Host: $(hostname)
Description: Sistema distribuido de procesamiento paralelo con tolerancia a fallos
Maintainer: Distributed System Team

Dependencies:
- glibc >= 2.17
- libpthread
- librt

Installation:
1. Extract package
2. Run ./install.sh [prefix]
3. Configure in /etc/distributed-system/

Usage:
distributed_system <node_id> <ip> <port> [seed_ip] [seed_port]
PKG_INFO

# Crear tarball
echo "7. Creando tarball..."
cd $PACKAGE_DIR
tar -czf $PACKAGE_NAME.tar.gz $PACKAGE_NAME/

# Crear checksum
echo "8. Generando checksum..."
sha256sum $PACKAGE_NAME.tar.gz > $PACKAGE_NAME.tar.gz.sha256

cd ..

echo ""
echo "✓ Paquete creado exitosamente:"
echo "  Archivo: $PACKAGE_DIR/$PACKAGE_NAME.tar.gz"
echo "  Checksum: $PACKAGE_DIR/$PACKAGE_NAME.tar.gz.sha256"
echo "  Tamaño: $(du -h $PACKAGE_DIR/$PACKAGE_NAME.tar.gz | cut -f1)"
echo ""
echo "Para distribuir:"
echo "  scp $PACKAGE_DIR/$PACKAGE_NAME.tar.gz servidor:/tmp/"
echo ""
echo "Para instalar:"
echo "  tar -xzf $PACKAGE_NAME.tar.gz"
echo "  cd $PACKAGE_NAME"
echo "  sudo ./install.sh"
