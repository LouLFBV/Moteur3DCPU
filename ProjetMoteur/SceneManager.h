#pragma once

#include <memory>
#include "Scene.h"
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"

class GraphicsPipeline;
class SceneManager;


struct clip
{
    std::unique_ptr<Scene> scene;
    float t_start;
    float t_end;
};

class SceneManager {
public:
    SceneManager() = default;

    void AddScene(std::unique_ptr<Scene> scene, float t_start = 0.0f, float t_end = 0.0f);

    void NextScene();


    void Update(float deltaTime, bool isEditMode = false);
    void Render(GraphicsPipeline& pipeline, const glm::mat4& viewProj, const glm::vec3& viewPos, float time);

    Scene* GetCurrentScene();
    int GetWidth() const;
    int GetHeight() const;

    bool m_isRunning = true;


private:

    void SetScene(size_t index);

    const int RENDER_WIDTH = 320;
    const int RENDER_HEIGHT = 200;

    std::vector<clip> m_listScene;
    int m_currentIndex = -1;
    float m_globalTime = 0.0f;
};