#include "Aqua.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "MaterialPresets.h"
#include "SceneManager.h"
#include "ObjLoader.h"
#include "Types.h"

// =====================================================
// CONSTRUCTEUR
// =====================================================
Aqua::Aqua(SceneManager& sceneManager) : Scene(sceneManager)
{
    m_gameObjects.clear();

    // 1. POISSON (index 0 : Attaché à la caméra avec animation)
    GameObject fish;
    fish.transform.scale = glm::vec3(1.0f);
    fish.transform.translation = glm::vec3(0.0f, -0.80f, -3.0f);
    fish.transform.rotation = glm::vec3(0.0f, glm::radians(180.0f), 0.0f);

    if (LoadGameObject(fish, "external/Assets/Aqua/Fish1.obj", "external/Assets/Village/wild_animals_map.png", "external/Assets/Aqua")) {
        m_gameObjects.push_back(fish);
    }

    // 2. RAY MANTA
    int nbMantas = 4;

    m_gameObjects.reserve(1 + nbMantas);

    std::vector<glm::vec3> positionsDepart = {
        glm::vec3(-7.0f, -0.8f,  -14.0f),
        glm::vec3(-10.0f, -1.8f, -22.0f),
        glm::vec3(-5.0f, -0.4f, -28.0f),
        glm::vec3(-8.5f, -2.2f,  -18.0f)
    };

    for (int i = 0; i < nbMantas; ++i)
    {
        GameObject manta;
        manta.transform.scale = glm::vec3(0.28f + 0.04f * i);
        manta.transform.translation = positionsDepart[i];

        // Orientations différentes
        float rotationY = glm::radians(10.0f + (i * 12.0f));
        manta.transform.rotation = glm::vec3(0.0f, rotationY, 0.0f);

        if (LoadGameObject(manta, "external/Assets/Aqua/Manta_ray.obj", nullptr, "external/Assets/Aqua"))
        {
            m_gameObjects.push_back(manta);

            size_t idx = m_gameObjects.size() - 1;

            // Keyframes Manta (Nage)
            float xStart = positionsDepart[i].x;
            float xEnd = xStart + 18.0f;

            AddKeyframe(&m_gameObjects[idx].transform.translation.x, 0.0f, xStart, true);
            AddKeyframe(&m_gameObjects[idx].transform.translation.x, 6.0f, xEnd, true);

            // Keyframes Manta (Ondulation)
            float yBase = positionsDepart[i].y;
            float amplitude = 0.35f;
            float offset = i * 0.3f;

            AddKeyframe(&m_gameObjects[idx].transform.translation.y, 0.0f, yBase, true);
            AddKeyframe(&m_gameObjects[idx].transform.translation.y, 1.25f + offset, yBase + amplitude, true);
            AddKeyframe(&m_gameObjects[idx].transform.translation.y, 2.5f, yBase, true);
            AddKeyframe(&m_gameObjects[idx].transform.translation.y, 3.75f + offset, yBase - amplitude, true);
            AddKeyframe(&m_gameObjects[idx].transform.translation.y, 6.0f, yBase, true);
        }
    }

    // Keyframes Caméra
    AddKeyframe(&m_sceneCamera.position.z, 0.0f, 0.0f);
    AddKeyframe(&m_sceneCamera.position.z, 11.0f, -20.0f);

    AddKeyframe(&m_sceneCamera.yaw, 0.0f, -90.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 1.0f, -70.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 2.0f, -90.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 3.0f, -70.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 4.0f, -90.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 5.0f, -70.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 6.0f, -90.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 7.0f, -70.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 8.0f, -90.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 9.0f, -70.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 10.0f, -90.0f, true);
    AddKeyframe(&m_sceneCamera.yaw, 11.0f, -70.0f, true);

    // Keyframes Poisson (Nage)
    if (!m_gameObjects.empty()) {
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 0.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 0.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 1.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 1.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 2.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 2.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 3.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 3.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 4.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 4.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 5.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 5.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 6.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 6.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 7.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 7.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 8.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 8.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 9.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 9.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 10.0f, glm::radians(190.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 10.5f, glm::radians(170.0f), true);
        AddKeyframe(&m_gameObjects[0].transform.rotation.y, 11.0f, glm::radians(190.0f), true);
    }

    // 2. GÉNÉRATION DE LA GRILLE DE L'OCÉAN (SURFACE / PLAFOND)
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

            // Couleur océan
            float r = 0.45f, g = 0.72f, b = 0.85f;

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

    // 4. ÉMETTEUR PARTICULES AMBIANTES (HAUTE DENSITÉ SOUS-MARINE CONTINUE)
    m_magicEmitter.position = glm::vec3(0.0f, 0.0f, -10.0f);
    m_magicEmitter.spawnRate = 1500.0f;
    m_magicEmitter.velocityMin = glm::vec3(-0.8f, -0.4f, -0.8f);
    m_magicEmitter.velocityMax = glm::vec3(0.8f, 0.8f, 0.8f);
    m_magicEmitter.gravity = glm::vec3(0.0f);
    m_magicEmitter.colorStart = glm::vec4(0.2f, 0.9f, 1.0f, 0.95f);
    m_magicEmitter.colorEnd = glm::vec4(0.6f, 0.2f, 1.0f, 0.0f);
    m_magicEmitter.lifeTimeMin = 2.0f;
    m_magicEmitter.lifeTimeMax = 4.5f;
    m_magicEmitter.sizeStart = 0.18f;
    m_magicEmitter.sizeEnd = 0.04f;
    m_magicEmitter.spawnRadius = 12.0f;
    m_magicEmitter.swirlStrength = 6.0f;
    m_magicEmitter.turbulence = 1.2f;
    m_magicEmitter.SetEmissionEnabled(true);
}

// =====================================================
// UPDATE
// =====================================================
void Aqua::Update(float deltaTime)
{
    Scene::Update(deltaTime);
    m_time += deltaTime;

    m_sceneCamera.ProcessMouseMovement(0.0f, 0.0f);

    if (!m_gameObjects.empty()) {
        m_gameObjects[0].UpdateTransform();
        m_gameObjects[1].UpdateTransform();
    }

    if (!m_particlesFrozen)
    {
        m_hudAnimationTime += deltaTime;

        // Déplacement dynamique de l'émetteur ambiant avec la caméra
        m_magicEmitter.position = m_sceneCamera.position + glm::vec3(0.0f, 0.0f, -8.0f);
        m_magicEmitter.Update(deltaTime);
    }
}

// =====================================================
// RENDER
// =====================================================
void Aqua::Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewMatrix, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime)
{
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, -1.0f, 0.5f));

    // -----------------------------------------------------
    // PASSE 1 : ARRIÈRE-PLAN (NDC) & RESET Z-BUFFER
    // -----------------------------------------------------
    pipeline.SetFragmentShader([](const BordLigne& pixel) {
        uint8_t ir = static_cast<uint8_t>(std::clamp(255.0f * pixel.r + 0.5f, 0.0f, 255.0f));
        uint8_t ig = static_cast<uint8_t>(std::clamp(255.0f * pixel.g + 0.5f, 0.0f, 255.0f));
        uint8_t ib = static_cast<uint8_t>(std::clamp(255.0f * pixel.b + 0.5f, 0.0f, 255.0f));
        return (0xFF << 24) | (ir << 16) | (ig << 8) | ib;
        });

    Vertex baseBackground[4] = {
        { { -1.0f,  1.0f, 0.9999f, 1.0f }, 0.40f, 0.66f, 0.82f },
        { { -1.0f, -1.0f, 0.9999f, 1.0f }, 0.10f, 0.30f, 0.55f },
        { {  1.0f, -1.0f, 0.9999f, 1.0f }, 0.10f, 0.30f, 0.55f },
        { {  1.0f,  1.0f, 0.9999f, 1.0f }, 0.40f, 0.66f, 0.82f }
    };
    pipeline.DrawSquareNDC(baseBackground[0], baseBackground[1], baseBackground[2], baseBackground[3]);
    pipeline.ClearZBuffer();
    pipeline.RenderScene();

    // -----------------------------------------------------
    // PASSE 2 : RENDU DES GAMEOBJECTS 3D (POISSON)
    // -----------------------------------------------------
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        if (!m_gameObjects[i].loaded) continue;

        pipeline.SetFragmentShader([this, i, lightDir, viewPos](const BordLigne& pixel) -> uint32_t {
            const Material& mat = m_gameObjects[i].objModel.material;
            glm::vec3 baseDiffuse = mat.diffuse * glm::vec3(pixel.r, pixel.g, pixel.b);

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

        glm::mat4 modelMat;
        if (i == 0) {
            if (!m_isEditMode || !m_gameObjects[0].hasLastModelMatrix) {
                glm::mat4 cameraWorld = glm::inverse(viewMatrix);
                m_gameObjects[0].lastModelMatrix = cameraWorld * m_gameObjects[0].modelMatrix;
                m_gameObjects[0].hasLastModelMatrix = true;
            }
            modelMat = m_gameObjects[0].lastModelMatrix;
        }
        else {
            m_gameObjects[i].UpdateTransform();
            modelMat = m_gameObjects[i].modelMatrix;
        }

        glm::mat4 mvpObj = viewProjection * modelMat;
        pipeline.DrawMesh(m_gameObjects[i].gpuVertices, mvpObj);
        pipeline.RenderScene();
    }

    // -----------------------------------------------------
    // PASSE 3 : TERRAIN INVERSÉ SOUS-MARIN (SURFACE EAU)
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
        float specFactor = std::pow(std::max(0.0f, glm::dot(normal, vH)), 128.0f);
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

    glm::mat4 modelTerrain = glm::mat4(1.0f);
    modelTerrain = glm::translate(modelTerrain, glm::vec3(snappedCamX, 3.0f, snappedCamZ));
    modelTerrain = glm::rotate(modelTerrain, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 mvpTerrain = viewProjection * modelTerrain;

    pipeline.SetVertexShader([deltaTime, modelTerrain](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        glm::vec4 worldPosCoord = modelTerrain * vertex.position;

        float amplitude = 0.4f, frequence = 1.5f, vitesse = 2.5f;
        float waveX = std::sin(worldPosCoord.x * frequence + deltaTime * vitesse);
        float waveZ = std::cos(worldPosCoord.z * frequence + deltaTime * vitesse);

        transformed.position.y += (waveX + waveZ) * amplitude;

        float dYdX = std::cos(worldPosCoord.x * frequence + deltaTime * vitesse) * frequence * amplitude;
        float dYdZ = -std::sin(worldPosCoord.z * frequence + deltaTime * vitesse) * frequence * amplitude;
        glm::vec3 surfaceNormal = glm::normalize(glm::vec3(-dYdX, 1.0f, -dYdZ));

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
    // PASSE 4 : RENDU DES PARTICULES AMBIANTES
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

    m_magicEmitter.Render(pipeline, viewMatrix, viewProjection);
    pipeline.RenderScene();

    pipeline.SetDepthWriteEnabled(true);
    pipeline.SetAlphaBlendingEnabled(false);

    // -----------------------------------------------------
    // PASSE 5 : TEXTE 2D ANIMÉ (HUD)
    // -----------------------------------------------------
    constexpr float fadeInDuration = 0.8f, holdDuration = 1.4f, fadeOutDuration = 0.8f, pauseDuration = 10.0f;
    constexpr float cycleDuration = fadeInDuration + holdDuration + fadeOutDuration + pauseDuration;

    float animationTime = std::fmod(m_hudAnimationTime, cycleDuration);
    float alpha = 0.0f, textX = 20.0f;

    auto SmoothStep = [](float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
        };

    if (animationTime < fadeInDuration) {
        float eased = SmoothStep(animationTime / fadeInDuration);
        alpha = eased;
        textX = -60.0f + 80.0f * eased;
    }
    else if (animationTime < fadeInDuration + holdDuration) {
        alpha = 1.0f;
        textX = 20.0f;
    }
    else if (animationTime < fadeInDuration + holdDuration + fadeOutDuration) {
        float eased = SmoothStep((animationTime - fadeInDuration - holdDuration) / fadeOutDuration);
        alpha = 1.0f - eased;
        textX = 20.0f + 80.0f * eased;
    }

    uint32_t alphaValue = static_cast<uint32_t>(std::round(alpha * 255.0f));
    uint32_t textColor = (alphaValue << 24) | 0x0000FFFF;

    pipeline.DrawText2D(static_cast<int>(textX), 55, "Sous L'Ocean", m_fontTexture, textColor);
    pipeline.RenderScene();
}

// =====================================================
// CLEANUP & EVENTS
// =====================================================
void Aqua::OnExit()
{
    m_gameObjects.clear();
    std::cout << "Scene Aqua quittee\n";
}

void Aqua::SetParticlesFrozen(bool frozen)
{
    m_particlesFrozen = frozen;
}

void Aqua::SetEditMode(bool editMode)
{
    m_isEditMode = editMode;
    SetParticlesFrozen(editMode);
}