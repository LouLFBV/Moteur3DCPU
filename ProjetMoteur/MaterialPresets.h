// MaterialPresets.h
#pragma once
#include "Material.h"

namespace MaterialPresets {

    inline Material Gold() {
        Material m;
        m.ambient = glm::vec3(0.24725f, 0.1995f, 0.0745f);
        m.diffuse = glm::vec3(0.75164f, 0.60648f, 0.22648f);
        m.specular = glm::vec3(0.628281f, 0.555802f, 0.366065f);
        m.shininess = 128.0f; // Reflet très net et métallique
        return m;
    }

    inline Material RedPlastic() {
        Material m;
        m.ambient = glm::vec3(0.1f, 0.0f, 0.0f);
        m.diffuse = glm::vec3(0.8f, 0.05f, 0.05f);
        m.specular = glm::vec3(0.7f, 0.7f, 0.7f); // Reflet blanc propre au plastique
        m.shininess = 32.0f;
        return m;
    }

    inline Material MatteRubber() {
        Material m;
        m.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
        m.diffuse = glm::vec3(0.2f, 0.2f, 0.2f);
        m.specular = glm::vec3(0.02f, 0.02f, 0.02f); // Pratiquement pas de reflet
        m.shininess = 4.0f; // Très diffus
        return m;
    }
}