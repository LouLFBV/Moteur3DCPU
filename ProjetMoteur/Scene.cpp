#include "Scene.h"
#include "SceneManager.h"
#include <algorithm>

Scene::Scene(SceneManager& sceneManager)
{
    m_sceneCamera = glm::vec3(0.0f, 0.0f, 1.0f);
    bool isPerspective = true;
    float aspect = (float)sceneManager.GetWidth() / (float)sceneManager.GetHeight();
    float zNear = 0.1f;
    float zFar = 100.0f;
}

void Scene::OnEnter()
{
    m_sceneCamera = glm::vec3(0.0f, 0.0f, 1.0f);
    m_sceneTime = 0.0f;
}

void Scene::AddKeyframe(float* target, float t, float v)
{
    keys[target].push_back({ t, v });
    std::sort(keys[target].begin(), keys[target].end(), [](const Key& a, const Key& b) {
        return a.t < b.t;
        });
}

void Scene::Update(float deltaTime)
{
    m_sceneTime += deltaTime;

    for (const auto& [ptr, k] : keys)
    {
        float v = *ptr;
        if (k.empty())
            return;
        if (k.size() == 1 && m_sceneTime >= k[0].t)
        v = k[0].v;
        if (m_sceneTime < k.front().t)
            v = k.front().v;
        if (m_sceneTime >= k.back().t)
            v = k.back().v;
        for (int i = 1; i < k.size(); ++i)
        {
            if (k[i].t >= m_sceneTime)
            {
                float mu = (m_sceneTime - k[i - 1].t) / (k[i].t - k[i - 1].t);
                v = (1 - mu) * k[i - 1].v + mu * k[i].v;
                break;
            }
        }
        *ptr = v;
    }
}

Camera& Scene::GetCamera()
{
	return m_sceneCamera;
}
