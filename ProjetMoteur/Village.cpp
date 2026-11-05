#include "Village.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "MaterialPresets.h"
#include "SceneManager.h"

// =====================================================
// CONSTRUCTEUR
// =====================================================
Village::Village(SceneManager& sceneManager) : Scene(sceneManager)
{
    m_gameObjects.clear();

    // 1. CHOUETTE / OBJ (Attachée à la caméra)
    GameObject owl;
    owl.transform.scale = glm::vec3(0.001f);

    // Utilisation de la méthode LoadGameObject de la classe parente Scene
    if (LoadGameObject(owl, "external/Assets/Village/owl.obj", "external/Assets/Village/wild_animals_map.png", "external/Assets")) {
        m_gameObjects.push_back(owl);
    }

    // 2. GÉNÉRATION DU TERRAIN (HERBE VERTE)
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

            // Couleur verte village
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

    // 3. POLICE HUD
    if (!m_fontTexture.LoadFromFile("external/Assets/boxy_bold_font_5.png")) {
        std::cerr << "[Erreur] Font non trouvee !\n";
    }

    // 4. PARTICULES DE CASCADE / EAU
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

// =====================================================
// UPDATE
// =====================================================
void Village::Update(float deltaTime)
{
    Scene::Update(deltaTime);
    m_time += deltaTime;

    if (!m_particlesFrozen)
    {
        m_hudAnimationTime += deltaTime;
        m_fireEmitter.position = glm::vec3(0.0f, 1.0f, -10.0f);

        m_waterEmitter.Update(deltaTime);
        m_waterTimer += deltaTime;

        if (m_waterTimer >= 6.0f)
        {
            m_waterEmitter.BurstUpward(90, 2.0f, 5.0f);
            m_waterTimer = 0.0f;
        }
    }
}

// =====================================================
// RENDER
// =====================================================
void Village::Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewMatrix, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime)
{
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f));

    // -----------------------------------------------------
    // PASSE 1 : ARRIÈRE-PLAN (CIEL NDC) & RESET Z-BUFFER
    // -----------------------------------------------------
    pipeline.SetFragmentShader([](const BordLigne& pixel) {
        uint8_t ir = static_cast<uint8_t>(std::clamp(255.0f * pixel.r + 0.5f, 0.0f, 255.0f));
        uint8_t ig = static_cast<uint8_t>(std::clamp(255.0f * pixel.g + 0.5f, 0.0f, 255.0f));
        uint8_t ib = static_cast<uint8_t>(std::clamp(255.0f * pixel.b + 0.5f, 0.0f, 255.0f));
        return (0xFF << 24) | (ir << 16) | (ig << 8) | ib;
        });

    Vertex baseBackground[4] = {
        { { -1.0f,  1.0f, 0.9999f, 1.0f }, 0.2f, 0.4f, 0.8f },
        { { -1.0f, -1.0f, 0.9999f, 1.0f }, 0.6f, 0.8f, 1.0f },
        { {  1.0f, -1.0f, 0.9999f, 1.0f }, 0.6f, 0.8f, 1.0f },
        { {  1.0f,  1.0f, 0.9999f, 1.0f }, 0.2f, 0.4f, 0.8f }
    };
    pipeline.DrawSquareNDC(baseBackground[0], baseBackground[1], baseBackground[2], baseBackground[3]);
    pipeline.ClearZBuffer();
    pipeline.RenderScene();

    // -----------------------------------------------------
    // PASSE 2 : RENDU DU MODÈLE OBJ 3D (CHOUETTE)
    // -----------------------------------------------------
    if (!m_gameObjects.empty() && m_gameObjects[0].loaded) {

        pipeline.SetFragmentShader([this, lightDir, viewPos](const BordLigne& pixel) -> uint32_t {
            const Material& mat = m_gameObjects[0].objModel.material;
            glm::vec3 baseDiffuse = mat.diffuse;

            if (mat.hasTexture) {
                uint32_t texColor = mat.texture.Sample(pixel.u, pixel.v);
                float rTex = ((texColor >> 16) & 0xFF) / 255.0f;
                float gTex = ((texColor >> 8) & 0xFF) / 255.0f;
                float bTex = (texColor & 0xFF) / 255.0f;
                baseDiffuse *= glm::vec3(rTex, gTex, bTex);
            }

            glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));
            glm::vec3 pixelPos = glm::vec3(pixel.worldPos.x, pixel.worldPos.y, pixel.worldPos.z);
            glm::vec3 vV = glm::normalize(viewPos - pixelPos);
            glm::vec3 vH = glm::normalize(lightDir + vV);

            glm::vec3 ambient = mat.ambient * baseDiffuse;
            float diffFactor = std::max(0.0f, glm::dot(normal, lightDir));
            glm::vec3 diffuse = baseDiffuse * diffFactor;
            float specFactor = std::pow(std::max(0.0f, glm::dot(normal, vH)), mat.shininess);
            glm::vec3 specular = mat.specular * specFactor;

            glm::vec3 finalColor = ambient + diffuse + specular;
            uint8_t r = static_cast<uint8_t>(std::clamp(finalColor.r * 255.0f + 0.5f, 0.0f, 255.0f));
            uint8_t g = static_cast<uint8_t>(std::clamp(finalColor.g * 255.0f + 0.5f, 0.0f, 255.0f));
            uint8_t b = static_cast<uint8_t>(std::clamp(finalColor.b * 255.0f + 0.5f, 0.0f, 255.0f));

            return (0xFF << 24) | (r << 16) | (g << 8) | b;
            });

        if (!m_isEditMode || !m_gameObjects[0].hasLastModelMatrix)
        {
            glm::mat4 cameraWorld = glm::inverse(viewMatrix);
            glm::mat4 cameraOffset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.80f, -1.0f));
            glm::mat4 modelScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.001f));
            glm::mat4 modelRotationY = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 modelRotationX = glm::rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(0.0f, 0.0f, 1.0f));

            m_gameObjects[0].lastModelMatrix = cameraWorld * cameraOffset * modelRotationY * modelRotationX * modelScale;
            m_gameObjects[0].hasLastModelMatrix = true;
        }

        glm::mat4 mvpOBJ = viewProjection * m_gameObjects[0].lastModelMatrix;
        pipeline.DrawMesh(m_gameObjects[0].gpuVertices, mvpOBJ);
        pipeline.RenderScene();
    }

    // -----------------------------------------------------
    // PASSE 3 : SHADER TERRAIN (MONTAGNES EN BRUIT PROCÉDURAL)
    // -----------------------------------------------------
    pipeline.SetFragmentShader([lightDir, viewPos](const BordLigne& pixel) -> uint32_t {
        glm::vec3 baseColor(pixel.r, pixel.g, pixel.b);
        glm::vec3 normal = glm::normalize(glm::vec3(pixel.nx, pixel.ny, pixel.nz));
        glm::vec3 pixelPos = glm::vec3(pixel.worldPos.x, pixel.worldPos.y, pixel.worldPos.z);
        glm::vec3 vV = glm::normalize(viewPos - pixelPos);
        glm::vec3 vH = glm::normalize(lightDir + vV);

        glm::vec3 ambient = baseColor * 0.3f;
        float diffFactor = std::max(0.0f, glm::dot(normal, lightDir));
        glm::vec3 diffuse = baseColor * diffFactor * 0.8f;

        float shininess = 128.0f;
        float specFactor = std::pow(std::max(0.0f, glm::dot(normal, vH)), shininess);
        glm::vec3 specular = glm::vec3(1.0f) * specFactor;

        glm::vec3 finalColor = ambient + diffuse + specular;
        uint8_t r = static_cast<uint8_t>(std::clamp(finalColor.r * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t g = static_cast<uint8_t>(std::clamp(finalColor.g * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t b = static_cast<uint8_t>(std::clamp(finalColor.b * 255.0f + 0.5f, 0.0f, 255.0f));

        return (0xFF << 24) | (r << 16) | (g << 8) | b;
        });

    float snapSpacing = m_cellSize;
    float snappedCamX = std::floor(viewPos.x / snapSpacing) * snapSpacing;
    float snappedCamZ = std::floor(viewPos.z / snapSpacing) * snapSpacing;

    glm::mat4 modelTerrain = glm::translate(glm::mat4(1.0f), glm::vec3(snappedCamX, -1.0f, snappedCamZ));
    glm::mat4 mvpTerrain = viewProjection * modelTerrain;

    pipeline.SetVertexShader([modelTerrain](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        glm::vec4 worldPosCoord = modelTerrain * vertex.position;

        float height = SimpleNoise(worldPosCoord.x, worldPosCoord.z);
        transformed.position.y += height;

        float eps = 0.1f;
        float hL = SimpleNoise(worldPosCoord.x - eps, worldPosCoord.z);
        float hR = SimpleNoise(worldPosCoord.x + eps, worldPosCoord.z);
        float hD = SimpleNoise(worldPosCoord.x, worldPosCoord.z - eps);
        float hU = SimpleNoise(worldPosCoord.x, worldPosCoord.z + eps);

        glm::vec3 tangentX(2.0f * eps, hR - hL, 0.0f);
        glm::vec3 tangentZ(0.0f, hU - hD, 2.0f * eps);
        glm::vec3 surfaceNormal = glm::normalize(glm::cross(tangentZ, tangentX));

        transformed.position = mvp * transformed.position;
        glm::mat3 normalMatrix = glm::mat3(mvp);
        transformed.normal = glm::normalize(normalMatrix * surfaceNormal);

        return transformed;
        });

    pipeline.DrawMesh(m_terrainVertices, mvpTerrain);
    pipeline.RenderScene();

    // Reset du Vertex Shader standard
    pipeline.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        transformed.position = mvp * vertex.position;
        glm::mat3 normalMatrix = glm::mat3(mvp);
        transformed.normal = glm::normalize(normalMatrix * vertex.normal);
        return transformed;
        });

    // -----------------------------------------------------
    // PASSE 4 : PARTICULES D'EAU
    // -----------------------------------------------------
    pipeline.SetFragmentShader([](const BordLigne& pixel) -> uint32_t {
        float dx = (pixel.u - 0.5f) * 2.0f;
        float dy = (pixel.v - 0.5f) * 2.0f;
        float radius = std::sqrt(dx * dx + dy * dy);

        float t = std::clamp((radius - 0.35f) / 0.65f, 0.0f, 1.0f);
        float smooth = t * t * (3.0f - 2.0f * t);
        float edgeAlpha = 1.0f - smooth;
        float finalAlpha = pixel.alpha * edgeAlpha;

        uint8_t a = static_cast<uint8_t>(std::clamp(finalAlpha * 255.0f, 0.0f, 255.0f));
        uint8_t r = static_cast<uint8_t>(std::clamp(pixel.r * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t g = static_cast<uint8_t>(std::clamp(pixel.g * 255.0f + 0.5f, 0.0f, 255.0f));
        uint8_t b = static_cast<uint8_t>(std::clamp(pixel.b * 255.0f + 0.5f, 0.0f, 255.0f));

        return (a << 24) | (r << 16) | (g << 8) | b;
        });

    pipeline.SetAlphaBlendingEnabled(true);
    pipeline.SetDepthWriteEnabled(false);

    m_waterEmitter.Render(pipeline, viewMatrix, viewProjection);
    pipeline.RenderScene();

    pipeline.SetDepthWriteEnabled(true);
    pipeline.SetAlphaBlendingEnabled(false);

    // -----------------------------------------------------
    // PASSE 5 : TEXTE 2D ANIMÉ (HUD)
    // -----------------------------------------------------
    constexpr float fadeInDuration = 0.8f, holdDuration = 1.4f, fadeOutDuration = 0.8f, pauseDuration = 1.0f;
    constexpr float cycleDuration = fadeInDuration + holdDuration + fadeOutDuration + pauseDuration;

    float animationTime = std::fmod(m_hudAnimationTime, cycleDuration);
    float alpha = 0.0f, textX = 20.0f;

    auto SmoothStep = [](float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
        };

    if (animationTime < fadeInDuration && !m_textPrinted) {
        float eased = SmoothStep(animationTime / fadeInDuration);
        alpha = eased;
        textX = -60.0f + 80.0f * eased;
    }
    else if (animationTime < fadeInDuration + holdDuration && !m_textPrinted) {
        alpha = 1.0f;
        textX = 20.0f;
    }
    else if (animationTime < fadeInDuration + holdDuration + fadeOutDuration && !m_textPrinted) {
        float eased = SmoothStep((animationTime - fadeInDuration - holdDuration) / fadeOutDuration);
        alpha = 1.0f - eased;
        textX = 20.0f + 80.0f * eased;
    }
    else {
        alpha = 0.0f;
        m_textPrinted = true;
    }

    uint32_t alphaValue = static_cast<uint32_t>(std::round(alpha * 255.0f));
    uint32_t textColor = (alphaValue << 24) | 0x0000FFFF;

    pipeline.DrawText2D(static_cast<int>(textX), 55, "Le Village", m_fontTexture, textColor);
    pipeline.RenderScene();
}

// =====================================================
// CLEANUP & EVENTS
// =====================================================
void Village::OnExit()
{
    m_gameObjects.clear();
    std::cout << "Scene Le Village quittee\n";
}

void Village::SetParticlesFrozen(bool frozen)
{
    m_particlesFrozen = frozen;
}

void Village::SetEditMode(bool editMode)
{
    m_isEditMode = editMode;
    SetParticlesFrozen(editMode);
}