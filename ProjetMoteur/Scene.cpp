#include "Scene.h"
#include "SceneManager.h"

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
}

Camera& Scene::GetCamera()
{
	return m_sceneCamera;
}
