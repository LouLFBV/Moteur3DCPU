#pragma once

#include "Scene.h"
#include "ObjLoader.h"

class MainScene : public Scene
{
public:
    MainScene(SceneManager& sceneManager);
    ~MainScene() override = default;

    void Update(float deltaTime);
    void Render(
        GraphicsPipeline& pipeline,
        SceneManager& sceneManager,
        const glm::mat4& viewMatrix,
        const glm::mat4& viewProjection,
        const glm::vec3& viewPos,
        float time
    ) override;
    void OnExit() override;

private:
    T_OBJ m_objModel;
    bool m_loaded = false;
    float m_time = 0.0f;
};