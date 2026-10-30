#pragma once

#include <memory>
#include "Scene.h"
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"

class GraphicsPipeline;
class SceneManager;


class SceneManager {
public:
    SceneManager() = default;

    void AddScene(std::unique_ptr<Scene> scene);

    void NextScene();


    void Update(float deltaTime);
    void Render(GraphicsPipeline& pipeline, const glm::mat4& viewProj, const glm::vec3& viewPos, float time);

    Scene* GetCurrentScene();
    int GetWidth() const;
    int GetHeight() const;


private:

    void SetScene(size_t index);

    const int RENDER_WIDTH = 320;
    const int RENDER_HEIGHT = 200;

    std::vector<std::unique_ptr<Scene>> m_listScene;
    int m_currentIndex = -1; 
};