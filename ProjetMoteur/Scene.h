#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Types.h"
#include "Texture.h"
#include "GameObject.h"

class GraphicsPipeline;
class SceneManager;

struct Key
{
    float t;
    float v;
    bool smoothing = false;
};

class Scene
{
public:
    explicit Scene(SceneManager& sceneManager);
    virtual ~Scene() = default;

    virtual void Update(float deltaTime);

    virtual void Render(
        GraphicsPipeline& pipeline,
        SceneManager& sceneManager,
        const glm::mat4& viewMatrix,
        const glm::mat4& viewProjection,
        const glm::vec3& viewPos,
        float time
    ) = 0;

    virtual void OnExit();

    Camera& GetCamera();

    void GenerateTerrain(int size, float cellSize);

    bool LoadGameObject(
        GameObject& gameObject,
        const char* objPath,
        const char* texturePath = nullptr,
        const char* mtlDir = "external/Assets"
    );

    void AddKeyframe(
        float* target,
        float time,
        float value,
        bool smoothing = false
    );

    float GetSceneTime() const;


    void SetParticlesFrozen(bool frozen);

    void SetEditMode(bool editMode);

protected:
    std::unordered_map<float*, std::vector<Key>> keys;

    Camera m_sceneCamera{ glm::vec3(0.0f, 0.0f, 0.0f) };

    float m_sceneTime = 0.0f;

    std::vector<Vertex> m_terrainVertices;

    int m_gridSize = 80;
    float m_cellSize = 1.0f;

    Texture m_fontTexture;

    std::vector<GameObject> m_gameObjects;

    bool m_particlesFrozen = false;
    float m_hudAnimationTime = 0.0f;
    bool m_isEditMode = false;
};