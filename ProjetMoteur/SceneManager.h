#pragma once

#include <memory>
#include "Scene.h"
#include <vector>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "Camera.h"

class GraphicsPipeline;
class SceneManager;


enum class FadeState
{
    None,
    FadeIn,
    FadeOut
};

struct clip
{
    std::unique_ptr<Scene> scene;
    float t_start;
    float t_end;
};

class SceneManager {
public:
    SceneManager();

    void AddScene(std::unique_ptr<Scene> scene, float t_start = 0.0f, float t_end = 0.0f);

    void NextScene(bool isEditMode = false);


    void Update(float deltaTime, bool isEditMode = false);
    void Render(
        GraphicsPipeline& pipeline,
        const glm::mat4& viewMatrix,
        const glm::mat4& viewProjection,
        const glm::vec3& viewPos,
        float time
    );

    Scene* GetCurrentScene();
    int GetWidth() const;
    int GetHeight() const;

    bool m_isRunning = true;

    void SetFadeDuration(float duration);


private:

    void SetScene(size_t index, bool isEditMode = false);

    const int RENDER_WIDTH = 320;
    const int RENDER_HEIGHT = 200;

    std::vector<clip> m_listScene;
    int m_currentIndex = -1;
    float m_globalTime = 0.0f;

    FadeState m_fadeState = FadeState::None;

    glm::vec3 m_fadeColor = glm::vec3(0.0f);

    float m_fadeDuration = 1.0f;

    float m_fadeTimer = 0.0f;

    int m_pendingSceneIndex = -1;

    float m_fadeAlpha = 0.0f;

    void ActivateScene(size_t index);
    void UpdateFade(float deltaTime);
    void RenderFade(GraphicsPipeline& pipeline) const;
};