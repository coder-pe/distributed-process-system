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

#ifndef DISTRIBUTED_CONFIGURATION_H
#define DISTRIBUTED_CONFIGURATION_H

#include "interfaces.h"
#include "plugin_manager.h"
#include <string>
#include <vector>

namespace distributed {

/**
 * @brief Gestor de configuración del sistema distribuido
 * 
 * Maneja la carga, validación y actualización de configuraciones
 * del sistema desde archivos externos.
 */
class ConfigurationManager : public IConfigurationManager {
private:
    std::string config_file_path;
    std::vector<PipelineStageConfig> pipeline_stages;

    /**
     * @brief Parsear línea de configuración
     */
    bool parse_config_line(const std::string& line, PipelineStageConfig& config);

    /**
     * @brief Validar configuración cargada
     */
    bool validate_configuration() const;

    /**
     * @brief Convertir política de string a enum
     */
    static FailoverPolicy string_to_policy(const std::string& policy_str);

    /**
     * @brief Convertir política de enum a string
     */
    static std::string policy_to_string(FailoverPolicy policy);

public:
    /**
     * @brief Constructor
     * @param config_path Ruta al archivo de configuración
     */
    ConfigurationManager(const std::string& config_path);

    virtual ~ConfigurationManager();

    /**
     * @brief Obtener configuración del pipeline
     */
    const std::vector<PipelineStageConfig>& get_pipeline_stages() const;

    /**
     * @brief Actualizar configuración de una etapa
     */
    bool update_stage_config(const std::string& stage_name, 
                           const PipelineStageConfig& new_config);

    /**
     * @brief Agregar nueva etapa al pipeline
     */
    bool add_pipeline_stage(const PipelineStageConfig& stage_config);

    /**
     * @brief Remover etapa del pipeline
     */
    bool remove_pipeline_stage(const std::string& stage_name);

    /**
     * @brief Crear configuración de ejemplo
     */
    static bool create_sample_config(const std::string& filename);

    // Implementación de IConfigurationManager
    virtual bool load_configuration(const std::string& filename);
    virtual bool reload_configuration();
    virtual bool save_configuration(const std::string& filename) const;

    /**
     * @brief Obtener ruta del archivo de configuración actual
     */
    const std::string& get_config_file_path() const { return config_file_path; }

    /**
     * @brief Validar sintaxis de archivo de configuración
     */
    static bool validate_config_file_syntax(const std::string& filename);
};

} // namespace distributed

#endif // DISTRIBUTED_CONFIGURATION_H
