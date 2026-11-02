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

void Scene::GenerateTerrain(int size, float cellSize) {
    m_terrainVertices.clear();


    // On génère size * size cases (soit size * size * 2 triangles)
    for (int z = 0; z < size; ++z) {
        for (int x = 0; x < size; ++x) {

            // Calcul des positions en X et Z
            float x0 = x * cellSize - (size * cellSize) / 2.0f;
            float z0 = z * cellSize - (size * cellSize) / 2.0f;
            float x1 = (x + 1) * cellSize - (size * cellSize) / 2.0f;
            float z1 = (z + 1) * cellSize - (size * cellSize) / 2.0f;

            // UV mapping pour répéter une texture d'herbe ou de terre
            float u0 = (float)x / size, v0 = (float)z / size;
            float u1 = (float)(x + 1) / size, v1 = (float)(z + 1) / size;

            //// Les 4 coins de la case (Y = 0 par défaut, normales pointant vers le haut)
            //Vertex vTL = { glm::vec4(x0, 0.0f, z0, 1.0f), 1,1,1, {u0, v0}, {0, 1, 0} }; // Top-Left
            //Vertex vTR = { glm::vec4(x1, 0.0f, z0, 1.0f), 1,1,1, {u1, v0}, {0, 1, 0} }; // Top-Right
            //Vertex vBL = { glm::vec4(x0, 0.0f, z1, 1.0f), 1,1,1, {u0, v1}, {0, 1, 0} }; // Bottom-Left
            //Vertex vBR = { glm::vec4(x1, 0.0f, z1, 1.0f), 1,1,1, {u1, v1}, {0, 1, 0} }; // Bottom-Right

            float r = 0.1f, g = 0.4f, b = 0.8f; // Bleu eau

            Vertex vTL = { glm::vec4(x0, 0.0f, z0, 1.0f), r, g, b, {u0, v0}, {0, 1, 0} };
            Vertex vTR = { glm::vec4(x1, 0.0f, z0, 1.0f), r, g, b, {u1, v0}, {0, 1, 0} };
            Vertex vBL = { glm::vec4(x0, 0.0f, z1, 1.0f), r, g, b, {u0, v1}, {0, 1, 0} };
            Vertex vBR = { glm::vec4(x1, 0.0f, z1, 1.0f), r, g, b, {u1, v1}, {0, 1, 0} };

            // Triangle 1
            m_terrainVertices.push_back(vTL);
            m_terrainVertices.push_back(vBL);
            m_terrainVertices.push_back(vTR);

            // Triangle 2
            m_terrainVertices.push_back(vTR);
            m_terrainVertices.push_back(vBL);
            m_terrainVertices.push_back(vBR);
        }
    }
}

Camera& Scene::GetCamera()
{
	return m_sceneCamera;
}
