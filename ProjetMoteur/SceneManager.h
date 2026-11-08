#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Scene.h"

class GraphicsPipeline;

class SceneManager
{
public:
    SceneManager();

    void AddScene(
        std::unique_ptr<Scene> scene,
        float t_start = 0.0f,
        float t_end = 0.0f
    );

    void NextScene(bool isEditMode = false);

    void Update(float deltaTime, bool isEditMode = false);

    void Render( GraphicsPipeline& pipeline, const glm::mat4& viewMatrix, const glm::mat4& viewProjection, const glm::vec3& viewPos, float time);

    Scene* GetCurrentScene();
    int GetWidth() const;
    int GetHeight() const;

    void SetFadeDuration(float duration);

    bool m_isRunning = true;

private:
    enum class FadeState
    {
        None,
        FadeIn,
        FadeOut
    };

    struct SceneEntry
    {
        std::unique_ptr<Scene> scene;
        float t_start = 0.0f;
        float t_end = 0.0f;
    };

    void SetScene(std::size_t index, bool isEditMode = false);
    void ActivateScene(std::size_t index);

    void UpdateFade(float deltaTime);
    void RenderFade(GraphicsPipeline& pipeline) const;

    static constexpr int RENDER_WIDTH = 320;
    static constexpr int RENDER_HEIGHT = 200;

    std::vector<SceneEntry> m_listScene;

    int m_currentIndex = -1;
    int m_pendingSceneIndex = -1;

    float m_globalTime = 0.0f;

    FadeState m_fadeState = FadeState::None;

    glm::vec3 m_fadeColor{ 0.0f };

    float m_fadeDuration = 1.0f;
    float m_fadeTimer = 0.0f;
    float m_fadeAlpha = 0.0f;
};
