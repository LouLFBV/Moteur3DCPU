#include "End.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "MaterialPresets.h"
#include "SceneManager.h"

End::End(SceneManager& sceneManager) : Scene(sceneManager)
{
    //m_keys.push_back({ &m_objModel.translation.x, {

    //} });
}

End::End(SceneManager& sceneManager, int gridSize, float cellSize) : Scene(sceneManager)
{
    m_gridSize = gridSize;
    m_cellSize = cellSize;
}

void End::OnEnter()
{
    Scene::OnEnter();
    const char* objPath = "external/Assets/Aqua/Shark.obj";
    const char* texturePath = "external/Assets/Village/";

    // On passe le dossier "external/Assets" pour que tinyobj cherche le .mtl au bon endroit
    Get_TOBJ(objPath, m_objModel, "external/Assets/Aqua");

    m_loaded = !m_objModel.vertices.empty();

    if (m_loaded) {
        std::cout << "Obj Charge depuis : " << objPath << " (" << m_objModel.vertices.size() << " sommets)" << std::endl;

        // Si la texture n'a pas été chargée automatiquement via le .mtl, on la charge manuellement :
        if (!m_objModel.material.hasTexture) {
            if (m_objModel.material.texture.LoadFromFile(texturePath)) {
                m_objModel.material.hasTexture = true;
                std::cout << "Texture chargee manuellement : " << texturePath << std::endl;
            }
            else {
                std::cerr << "Erreur de chargement de l'image : " << texturePath << std::endl;
            }
        }
    }
    else {
        std::cerr << "Erreur : Impossible de charger " << objPath << std::endl;
    }

    m_objModel.material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);


    m_gpuVertices.clear();
    m_gpuVertices.reserve(m_objModel.vertices.size());

    for (const auto& src : m_objModel.vertices) {
        m_gpuVertices.push_back({
            src.position, src.r, src.g, src.b,
            {src.tx, src.ty}, {src.nx, src.ny, src.nz}
            });
    }


    // TESTE MATERIAL
    // On applique un matériau Or au modèle
    //Texture oldTex = m_objModel.material.texture; // Conserver la texture si besoin
    //bool hadTex = m_objModel.material.hasTexture;

    //m_objModel.material = MaterialPresets::Gold();
    //m_objModel.material.texture = oldTex;
    //m_objModel.material.hasTexture = hadTex;


    m_terrainVertices.clear();
    float halfSize = (m_gridSize * m_cellSize) * 0.5f;

    for (int z = 0; z < m_gridSize; ++z) {
        for (int x = 0; x < m_gridSize; ++x) {
            float x0 = x * m_cellSize - halfSize;
            float z0 = z * m_cellSize - halfSize;
            float x1 = (x + 1) * m_cellSize - halfSize;
            float z1 = (z + 1) * m_cellSize - halfSize;

            float u0 = (float)x / m_gridSize, v0 = (float)z / m_gridSize;
            float u1 = (float)(x + 1) / m_gridSize, v1 = (float)(z + 1) / m_gridSize;

            // Couleur verte
            float r = 0.31f, g = 0.55f, b = 0.14f;

            Vertex vTL = { glm::vec4(x0, 0.0f, z0, 1.0f), r, g, b, {u0, v0}, {0, 1, 0} };
            Vertex vTR = { glm::vec4(x1, 0.0f, z0, 1.0f), r, g, b, {u1, v0}, {0, 1, 0} };
            Vertex vBL = { glm::vec4(x0, 0.0f, z1, 1.0f), r, g, b, {u0, v1}, {0, 1, 0} };
            Vertex vBR = { glm::vec4(x1, 0.0f, z1, 1.0f), r, g, b, {u1, v1}, {0, 1, 0} };

            m_terrainVertices.push_back(vTL);
            m_terrainVertices.push_back(vBL);
            m_terrainVertices.push_back(vTR);

            m_terrainVertices.push_back(vTR);
            m_terrainVertices.push_back(vBL);
            m_terrainVertices.push_back(vBR);
        }
    }


    if (m_fontTexture.LoadFromFile("external/Assets/boxy_bold_font_5.png"))
        std::cout << "font texte charge !\n";
    else
        std::cout << "font texte pas trouve !\n";



    // =====================================================
 // EAU
 // =====================================================

    m_waterEmitter.position = glm::vec3(-3.0f, -0.5f, -8.0f);

    m_waterEmitter.gravity = glm::vec3(0.0f, -5.0f, 0.0f);

    m_waterEmitter.colorStart = glm::vec4(0.45f, 0.85f, 1.0f, 0.95f);
    m_waterEmitter.colorEnd = glm::vec4(0.05f, 0.45f, 1.0f, 0.0f);

    m_waterEmitter.lifeTimeMin = 0.6f;
    m_waterEmitter.lifeTimeMax = 1.3f;

    m_waterEmitter.sizeStart = 0.075f;
    m_waterEmitter.sizeEnd = 0.012f;

    m_waterEmitter.drag = 0.15f;

    m_waterEmitter.SetEmissionEnabled(false);

}




void End::Update(float deltaTime)
{

    Scene::Update(deltaTime);
    // Temps global de la scène.
    m_time += deltaTime;

    // Temps de l'animation du HUD.
    // L'animation est gelée en mode Edit.
    if (!m_particlesFrozen)
    {
        m_hudAnimationTime += deltaTime;
    }

    // =====================================================
    // SIMULATION DES PARTICULES
    // =====================================================

    if (!m_particlesFrozen)
    {
        m_fireEmitter.position = glm::vec3(0.0f, 1.0f, -10.0f);

        m_waterEmitter.Update(deltaTime);

        m_waterTimer += deltaTime;

        // Éclaboussure toutes les 6 secondes.
        if (m_waterTimer >= 6.0f)
        {
            m_waterEmitter.BurstUpward(90, 2.0f, 5.0f);
            m_waterTimer = 0.0f;
        }
    }

}



void End::Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewMatrix, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime)
{


    // =====================================================
    // ANIMATION DU TEXTE
    // =====================================================

// Durées des différentes phases.
    constexpr float fadeInDuration = 0.8f;
    constexpr float holdDuration = 1.4f;
    constexpr float fadeOutDuration = 0.8f;
    constexpr float pauseDuration = 1.0f;

    constexpr float cycleDuration =
        fadeInDuration + holdDuration +
        fadeOutDuration + pauseDuration;

    // Temps dans le cycle actuel.
    float animationTime = std::fmod(
        m_hudAnimationTime,
        cycleDuration
    );

    float alpha = 0.0f;
    float textX = 20.0f;

    // Fonction d'interpolation douce.
    auto SmoothStep = [](float t)
        {
            t = std::clamp(t, 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
        };

    // PHASE 1 : APPARITION
    if (animationTime < fadeInDuration)
    {
        float progress = animationTime / fadeInDuration;
        float eased = SmoothStep(progress);

        alpha = eased;

        // Le texte arrive depuis la gauche.
        textX = -60.0f + 80.0f * eased;
    }

    // PHASE 2 : AFFICHAGE
    else if (animationTime < fadeInDuration + holdDuration)
    {
        alpha = 1.0f;
        textX = 20.0f;
    }

    // PHASE 3 : DISPARITION
    else if (animationTime <
        fadeInDuration + holdDuration + fadeOutDuration)
    {
        float progress =
            (animationTime - fadeInDuration - holdDuration)
            / fadeOutDuration;

        float eased = SmoothStep(progress);

        alpha = 1.0f - eased;

        // Le texte repart vers la droite.
        textX = 20.0f + 80.0f * eased;
    }

    // PHASE 4 : INVISIBLE
    else
    {
        alpha = 0.0f;
    }

    // Conversion de l'alpha en entier [0, 255].
    uint32_t alphaValue = static_cast<uint32_t>(
        std::round(alpha * 255.0f)
        );

    // Couleur Cyan avec transparence variable.
    uint32_t textColor =
        (alphaValue << 24) | 0x0000FFFF;

    // Affichage du texte animé.
    pipeline.DrawText2D(
        static_cast<int>(textX),
        55,
        "by Regis et Lou",
        m_fontTexture,
        textColor
    );


    pipeline.RenderScene();


}

void End::OnExit()
{
    m_objModel.vertices.clear();
    std::cout << "Le Village Scene quittee\n";
}

void End::SetParticlesFrozen(bool frozen)
{
    m_particlesFrozen = frozen;
}

void End::SetEditMode(bool editMode)
{
    m_isEditMode = editMode;
    SetParticlesFrozen(editMode);
}