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

// src/ipc.cpp
#include "ipc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

namespace distributed {

SharedMemoryRegion::SharedMemoryRegion(const std::string& region_name, size_t region_size, bool create) 
    : memory(NULL), size(region_size), shm_fd(-1), name(region_name) {
    
    if (create) {
        shm_fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd != -1) {
            ftruncate(shm_fd, size);
        }
    } else {
        shm_fd = shm_open(name.c_str(), O_RDWR, 0666);
    }
    
    if (shm_fd != -1) {
        memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (memory == MAP_FAILED) {
            memory = NULL;
        }
    }
}

SharedMemoryRegion::~SharedMemoryRegion() {
    if (memory && memory != MAP_FAILED) {
        munmap(memory, size);
    }
    if (shm_fd != -1) {
        close(shm_fd);
    }
}

bool SharedMemoryRegion::is_valid() const {
    return memory != NULL && memory != MAP_FAILED && shm_fd != -1;
}

void SharedMemoryRegion::cleanup(const std::string& name) {
    shm_unlink(name.c_str());
}

IPCChannel::IPCChannel() : read_fd(-1), write_fd(-1) {
    pthread_mutex_init(&write_mutex, NULL);
}

IPCChannel::~IPCChannel() {
    close();
    pthread_mutex_destroy(&write_mutex);
}

bool IPCChannel::create_pipe() {
    int pipefd[2];
    if (pipe(pipefd) == 0) {
        read_fd = pipefd[0];
        write_fd = pipefd[1];
        
        // Hacer non-blocking para evitar deadlocks
        fcntl(read_fd, F_SETFL, O_NONBLOCK);
        fcntl(write_fd, F_SETFL, O_NONBLOCK);
        
        return true;
    }
    return false;
}

bool IPCChannel::send_message(const IPCMessage* msg) {
    if (!msg || write_fd == -1) return false;
    
    pthread_mutex_lock(&write_mutex);
    
    size_t total_size = sizeof(IPCMessage) + msg->data_size;
    ssize_t written = write(write_fd, msg, total_size);
    
    pthread_mutex_unlock(&write_mutex);
    
    return written == (ssize_t)total_size;
}

bool IPCChannel::receive_message(IPCMessage** msg, size_t max_size) {
    if (!msg || read_fd == -1) return false;
    
    // Leer header primero
    IPCMessage header;
    ssize_t read_bytes = read(read_fd, &header, sizeof(IPCMessage));
    
    if (read_bytes != sizeof(IPCMessage)) {
        return false;
    }
    
    // Validar tamaño
    size_t total_size = sizeof(IPCMessage) + header.data_size;
    if (total_size > max_size) {
        return false;
    }
    
    // Asignar memoria para mensaje completo
    *msg = (IPCMessage*)malloc(total_size);
    if (!*msg) return false;
    
    memcpy(*msg, &header, sizeof(IPCMessage));
    
    // Leer datos adicionales si los hay
    if (header.data_size > 0) {
        read_bytes = read(read_fd, (*msg)->data, header.data_size);
        if (read_bytes != (ssize_t)header.data_size) {
            free(*msg);
            *msg = NULL;
            return false;
        }
    }
    
    return true;
}

void IPCChannel::close() {
    if (read_fd != -1) {
        ::close(read_fd);
        read_fd = -1;
    }
    if (write_fd != -1) {
        ::close(write_fd);
        write_fd = -1;
    }
}

} // namespace distributed
