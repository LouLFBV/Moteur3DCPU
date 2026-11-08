#include "Scene.h"
#include "SceneManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <glm/gtc/constants.hpp>


Scene::Scene(SceneManager& sceneManager)
{
    m_sceneCamera = glm::vec3(0.0f, 0.0f, 1.0f);

    m_sceneTime = 0.0f;

    // Le SceneManager est conservé dans le constructeur
    // pour respecter l'interface actuelle de Scene.
    (void)sceneManager;
}


void Scene::AddKeyframe(float* target, float time, float value, bool smoothing)
{
    if (target == nullptr) {
        return;
    }

    if (!std::isfinite(time) || !std::isfinite(value)) {
        return;
    }

    std::vector<Key>& keyframes = keys[target];

    auto keyframePosition = std::lower_bound(keyframes.begin(), keyframes.end(), time, [](const Key& keyframe, float targetTime)
        {
            return keyframe.t < targetTime;
        }
    );

    if (keyframePosition != keyframes.end() && keyframePosition->t == time) {
        keyframePosition->v = value;
        keyframePosition->smoothing = smoothing;
        return;
    }

    // Les keyframes restent triées chronologiquement.
    keyframes.insert(keyframePosition, { time, value, smoothing });
}

float Scene::GetSceneTime() const
{
    return m_sceneTime;
}


void Scene::Update(float deltaTime)
{
    if (!std::isfinite(deltaTime)) {
        return;
    }

    m_sceneTime += deltaTime;

    for (const auto& [target, keyframes] : keys) {
        if (target == nullptr) {
            continue;
        }

        if (keyframes.empty()) {
            continue;
        }

        if (keyframes.size() == 1) {
            if (m_sceneTime >= keyframes.front().t)
                *target = keyframes.front().v;

            continue;
        }

        // Avant la première keyframe.
        if (m_sceneTime < keyframes.front().t) {
            *target = keyframes.front().v;
            continue;
        }

        // Après la dernière keyframe.
        if (m_sceneTime >= keyframes.back().t) {
            *target = keyframes.back().v;
            continue;
        }

        auto nextKeyframe = std::upper_bound(keyframes.begin(), keyframes.end(), m_sceneTime, [](float currentTime, const Key& keyframe)
            {
                return currentTime < keyframe.t;
            }
        );

        if (nextKeyframe == keyframes.begin() || nextKeyframe == keyframes.end()) {
            continue;
        }

        const Key& keyframeB = *nextKeyframe;
        const Key& keyframeA = *(nextKeyframe - 1);

        const float duration = keyframeB.t - keyframeA.t;

        if (duration <= 0.0f) {
            *target = keyframeB.v;
            continue;
        }

        float interpolation = (m_sceneTime - keyframeA.t) / duration;

        if (keyframeB.smoothing) {
            interpolation =
                -std::cos(interpolation * glm::pi<float>()) * 0.5f + 0.5f;
        }

        *target = (1.0f - interpolation) * keyframeA.v + interpolation * keyframeB.v;
    }
}


void Scene::OnExit()
{}

Camera& Scene::GetCamera()
{
    return m_sceneCamera;
}


void Scene::GenerateTerrain(int size, float cellSize)
{
    m_terrainVertices.clear();

    if (size <= 0 || !std::isfinite(cellSize) || cellSize <= 0.0f) {
        std::cerr << "[Erreur] Parametres invalides pour GenerateTerrain."
            << std::endl;

        return;
    }

    const size_t gridSize = static_cast<size_t>(size);

    if (gridSize > std::numeric_limits<size_t>::max() / gridSize) {
        return;
    }

    const size_t cellCount = gridSize * gridSize;

    if (cellCount > m_terrainVertices.max_size() / 6) {
        return;
    }

    m_terrainVertices.reserve(cellCount * 6);

    m_gridSize = size;
    m_cellSize = cellSize;

    const float terrainOffset = (size * cellSize) / 2.0f;

    constexpr float red = 0.1f;
    constexpr float green = 0.4f;
    constexpr float blue = 0.8f;

    for (int z = 0; z < size; ++z) {
        for (int x = 0; x < size; ++x) {
            const float x0 = x * cellSize - terrainOffset;
            const float z0 = z * cellSize - terrainOffset;

            const float x1 = (x + 1) * cellSize - terrainOffset;
            const float z1 = (z + 1) * cellSize - terrainOffset;

            // Coordonnées UV.
            const float u0 = static_cast<float>(x) / size;
            const float v0 = static_cast<float>(z) / size;

            const float u1 = static_cast<float>(x + 1) / size;
            const float v1 = static_cast<float>(z + 1) / size;

            // Sommets de la case.
            const Vertex vertexTopLeft =
            {
                glm::vec4(x0, 0.0f, z0, 1.0f),
                red, green, blue,
                { u0, v0 },
                { 0.0f, 1.0f, 0.0f }
            };

            const Vertex vertexTopRight =
            {
                glm::vec4(x1, 0.0f, z0, 1.0f),
                red, green, blue,
                { u1, v0 },
                { 0.0f, 1.0f, 0.0f }
            };

            const Vertex vertexBottomLeft =
            {
                glm::vec4(x0, 0.0f, z1, 1.0f),
                red, green, blue,
                { u0, v1 },
                { 0.0f, 1.0f, 0.0f }
            };

            const Vertex vertexBottomRight =
            {
                glm::vec4(x1, 0.0f, z1, 1.0f),
                red, green, blue,
                { u1, v1 },
                { 0.0f, 1.0f, 0.0f }
            };

            // Premier triangle.
            m_terrainVertices.push_back(vertexTopLeft);
            m_terrainVertices.push_back(vertexBottomLeft);
            m_terrainVertices.push_back(vertexTopRight);

            // Deuxième triangle.
            m_terrainVertices.push_back(vertexTopRight);
            m_terrainVertices.push_back(vertexBottomLeft);
            m_terrainVertices.push_back(vertexBottomRight);
        }
    }
}


bool Scene::LoadGameObject(GameObject& gameObject, const char* objPath, const char* texturePath, const char* mtlDir)
{
    if (objPath == nullptr){
        std::cerr << "[Erreur] Chemin OBJ invalide." << std::endl;
        return false;
    }

    Get_TOBJ(objPath, gameObject.objModel, mtlDir);

    gameObject.loaded = !gameObject.objModel.GetVertices().empty();

    if (!gameObject.loaded){
        std::cerr << "[Erreur] Impossible de charger le modele OBJ : "
            << objPath << std::endl;

        return false;
    }

    if (texturePath != nullptr && !gameObject.objModel.GetMaterial().hasTexture) {
        if (gameObject.objModel.GetMaterial().texture.LoadFromFile(texturePath)) {
            gameObject.objModel.GetMaterial().hasTexture = true;

            std::cout << "[Info] Texture chargee : "
                << texturePath
                << std::endl;
        }
        else {
            std::cerr << "[Erreur] Echec chargement texture : "
                << texturePath
                << std::endl;
        }
    }

    gameObject.objModel.GetMaterial().diffuse = glm::vec3(1.0f);

    gameObject.gpuVertices.clear();

    gameObject.gpuVertices.reserve(
        gameObject.objModel.GetVertices().size()
    );

    for (const auto& sourceVertex : gameObject.objModel.GetVertices()) {
        gameObject.gpuVertices.push_back(
            {
                sourceVertex.position,
                sourceVertex.r,
                sourceVertex.g,
                sourceVertex.b,
                { sourceVertex.tx, sourceVertex.ty },
                { sourceVertex.nx, sourceVertex.ny, sourceVertex.nz }
            }
        );
    }
    gameObject.UpdateTransform();

    return true;
}


void Scene::SetParticlesFrozen(bool frozen)
{
    m_particlesFrozen = frozen;
}


void Scene::SetEditMode(bool editMode)
{
    m_isEditMode = editMode;

    SetParticlesFrozen(editMode);
}