#include "Mer.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "SceneManager.h"

// =====================================================
// CONSTRUCTEUR
// =====================================================
Mer::Mer(SceneManager& sceneManager) : Scene(sceneManager)
{
    m_sceneCamera = glm::vec3(0.0f, 2.0f, 1.0f);
    m_gameObjects.clear();
    m_gameObjects.reserve(3);

    m_waterTimer = 0.0f;
    m_burstCount = 0;

    // 1. CHOUETTE / POV (index 0 - Attachée à la caméra)
    GameObject owl;
    owl.transform.scale = glm::vec3(0.001f);
    if (LoadGameObject(owl, "external/Assets/Village/owl.obj", "external/Assets/Village/wild_animals_map.png", "external/Assets")) {
        m_gameObjects.push_back(owl);
    }

    // Keyframes Caméra
    AddKeyframe(&m_sceneCamera.position.z, 0.0f, 0.0f);
    AddKeyframe(&m_sceneCamera.position.z, 11.0f, -20.0f);

    // Keyframes CHOUETTE
    if (!m_gameObjects.empty()) {
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 0.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 0.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 1.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 1.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 2.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 2.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 3.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 3.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 4.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 4.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 5.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 5.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 6.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 6.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 7.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 7.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 8.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 8.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 9.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 9.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 10.0f, 0.0f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 10.5f, 0.05f, true);
        AddKeyframe(&m_gameObjects[0].transform.translation.y, 11.0f, 0.0f, true);
    }

    // 2. BALEINE
    GameObject whale;
    whale.transform.scale = glm::vec3(0.5f);
    whale.transform.translation = glm::vec3(-8.0f, -1.0f, -30.0f);
    whale.transform.rotation = glm::vec3(0.0f, glm::radians(-30.0f), 0.0f);

    if (LoadGameObject(whale, "external/Assets/Mer/Whale.obj", nullptr, "external/Assets/Mer")) {
        m_gameObjects.push_back(whale);
    }

    // 3. DAUPHIN
    GameObject dolphin;
    dolphin.transform.scale = glm::vec3(0.5f);
    dolphin.transform.translation = glm::vec3(2.0f, -5.0f, -20.0f);
    dolphin.transform.rotation = glm::vec3(0.0f, glm::radians(70.0f), 0.0f);

    if (LoadGameObject(dolphin, "external/Assets/Mer/Dolphin.obj", nullptr, "external/Assets/Mer")) {
        m_gameObjects.push_back(dolphin);

        AddKeyframe(&m_gameObjects[2].transform.translation.x, 0.0f, -6.0f, true);
        AddKeyframe(&m_gameObjects[2].transform.translation.x, 5.0f, 6.0f, true);

        AddKeyframe(&m_gameObjects[2].transform.translation.y, 0.0f, -5.0f, true);
        AddKeyframe(&m_gameObjects[2].transform.translation.y, 2.5f, 0.8f, true);
        AddKeyframe(&m_gameObjects[2].transform.translation.y, 5.0f, -5.0f, true);
    }

    // 4. GRILLE DE LA MER
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

            float r = 0.1f, g = 0.4f, b = 0.8f;

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

    // 5. POLICE HUD
    if (!m_fontTexture.LoadFromFile("external/Assets/boxy_bold_font_5.png")) {
        std::cerr << "[Erreur] Font non trouvee !\n";
    }

    // 6. ÉMETTEUR DE PARTICULES D'EAU
    m_waterEmitter.gravity = glm::vec3(0.0f, -4.0f, 0.0f);
    m_waterEmitter.colorStart = glm::vec4(0.45f, 0.85f, 1.0f, 0.95f);
    m_waterEmitter.colorEnd = glm::vec4(0.05f, 0.45f, 1.0f, 0.0f);
    m_waterEmitter.lifeTimeMin = 1.0f;
    m_waterEmitter.lifeTimeMax = 2.5f;
    m_waterEmitter.sizeStart = 0.16f;
    m_waterEmitter.sizeEnd = 0.03f;
    m_waterEmitter.drag = 0.10f;
    m_waterEmitter.SetEmissionEnabled(false);

    // 7. ÉMETTEUR DE PARTICULES DE MAGIE
    m_magicEmitter.position = glm::vec3(0.0f, -10.0f, -10.0f);
    m_magicEmitter.spawnRate = 100.0f;
    m_magicEmitter.velocityMin = glm::vec3(-0.4f, -0.2f, -0.4f);
    m_magicEmitter.velocityMax = glm::vec3(0.4f, 0.5f, 0.4f);
    m_magicEmitter.gravity = glm::vec3(0.0f);
    m_magicEmitter.colorStart = glm::vec4(0.1f, 0.8f, 1.0f, 0.9f);
    m_magicEmitter.colorEnd = glm::vec4(0.7f, 0.1f, 1.0f, 0.0f);
    m_magicEmitter.lifeTimeMin = 1.5f;
    m_magicEmitter.lifeTimeMax = 3.0f;
    m_magicEmitter.sizeStart = 0.10f;
    m_magicEmitter.sizeEnd = 0.025f;
    m_magicEmitter.spawnRadius = 20.0f;
    m_magicEmitter.swirlStrength = 5.5f;
    m_magicEmitter.turbulence = 0.8f;
    m_magicEmitter.SetEmissionEnabled(true);
}

// =====================================================
// UPDATE
// =====================================================
void Mer::Update(float deltaTime)
{
    Scene::Update(deltaTime);
    m_time += deltaTime;

    m_sceneCamera.ProcessMouseMovement(0.0f, 0.0f);

    if (m_gameObjects.size() >= 3) {
        m_gameObjects[0].UpdateTransform();
        m_gameObjects[1].UpdateTransform();
        m_gameObjects[2].UpdateTransform();
        m_gameObjects[2].hasLastModelMatrix = false;
    }

    if (!m_particlesFrozen)
    {
        m_hudAnimationTime += deltaTime;
        m_fireEmitter.position = glm::vec3(0.0f, 1.0f, -10.0f);

        m_waterEmitter.Update(deltaTime);
        m_magicEmitter.Update(deltaTime);
        m_waterTimer += deltaTime;

        // 1ère explosion : à 1.5s, décalée vers la GAUCHE (-3.0f sur X)
        if (m_burstCount == 0 && m_waterTimer >= 1.55f)
        {
            m_waterEmitter.position = m_sceneCamera.position + glm::vec3(-1.5f, -1.3f, -6.0f);
            m_waterEmitter.BurstUpward(1200, 3.5f, 8.0f);
            m_burstCount = 1;
        }
        // 2ème explosion : à 3.5s, décalée vers la DROITE (+3.0f sur X)
        else if (m_burstCount == 1 && m_waterTimer >= 3.45f)
        {
            m_waterEmitter.position = m_sceneCamera.position + glm::vec3(2.0f, -1.3f, -6.0f);
            m_waterEmitter.BurstUpward(1200, 3.5f, 8.0f);
            m_burstCount = 2; // Ne redéclenche plus d'explosion ensuite
        }
    }
}

// =====================================================
// RENDER
// =====================================================
void Mer::Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewMatrix, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime)
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
    // PASSE 2 : RENDU UNIFIÉ DES GAMEOBJECTS 3D
    // -----------------------------------------------------
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        if (!m_gameObjects[i].loaded) continue;

        if (i == m_gameObjects.size() - 1 && m_gameObjects.size() > 1) {
            pipeline.SetFragmentShader([deltaTime](const BordLigne& pixel) {
                float wave = std::sin(pixel.x * 0.05f + pixel.z * 15.0f + deltaTime);
                float intensity = (wave * 0.5f) + 0.5f;

                uint8_t r = static_cast<uint8_t>(intensity * 255.0f);
                uint8_t g = static_cast<uint8_t>((1.0f - intensity) * 100.0f);
                uint8_t b = 255;

                return (0xFF << 24) | (r << 16) | (g << 8) | b;
                });
        }
        else {
            pipeline.SetFragmentShader([this, i, lightDir, viewPos](const BordLigne& pixel) -> uint32_t {
                const Material& mat = m_gameObjects[i].objModel.GetMaterial();
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
        }

        glm::mat4 modelMat;
        if (i == 0) {
            if (!m_isEditMode || !m_gameObjects[0].hasLastModelMatrix) {
                glm::mat4 cameraWorld = glm::inverse(viewMatrix);

                glm::vec3 animOffset = glm::vec3(0.0f, -0.80f + m_gameObjects[0].transform.translation.y, -1.0f);
                glm::mat4 cameraOffset = glm::translate(glm::mat4(1.0f), animOffset);

                glm::mat4 modelScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.001f));
                glm::mat4 modelRotationY = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 modelRotationX = glm::rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(0.0f, 0.0f, 1.0f));

                m_gameObjects[0].lastModelMatrix = cameraWorld * cameraOffset * modelRotationY * modelRotationX * modelScale;
                m_gameObjects[0].hasLastModelMatrix = true;
            }
            modelMat = m_gameObjects[0].lastModelMatrix;
        }
        else {
            m_gameObjects[i].UpdateTransform();
            modelMat = m_gameObjects[i].modelMatrix;
        }

        glm::mat4 mvpObj = viewProjection * modelMat;

        if (!m_gameObjects[i].gpuVertices.empty()) {
            pipeline.DrawMesh(m_gameObjects[i].gpuVertices, mvpObj);
        }
        else {
            for (size_t v = 0; v + 2 < m_gameObjects[i].objModel.GetVertices().size(); v += 3) {
                auto& src0 = m_gameObjects[i].objModel.GetVertices()[v + 0];
                auto& src1 = m_gameObjects[i].objModel.GetVertices()[v + 1];
                auto& src2 = m_gameObjects[i].objModel.GetVertices()[v + 2];

                Vertex v0 = { src0.position, src0.r, src0.g, src0.b,{ src0.tx, src0.ty },{ src0.nx, src0.ny, src0.nz } };
                Vertex v1 = { src1.position, src1.r, src1.g, src1.b,{ src1.tx, src1.ty },{ src1.nx, src1.ny, src1.nz } };
                Vertex v2 = { src2.position, src2.r, src2.g, src2.b,{ src2.tx, src2.ty },{ src2.nx, src2.ny, src2.nz } };

                pipeline.DrawTriangle(v0, v1, v2, mvpObj);
            }
        }

        pipeline.RenderScene();
    }

    // -----------------------------------------------------
    // PASSE 3 : SHADER DE LA MER (ONDULATION DES VAGUES)
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

    glm::mat4 modelTerrain = glm::translate(glm::mat4(1.0f), glm::vec3(snappedCamX, -1.0f, snappedCamZ));
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
    // PASSE 4 : PARTICULES (MAGIE + EAU)
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
    m_waterEmitter.Render(pipeline, viewMatrix, viewProjection);
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

    pipeline.DrawText2D(static_cast<int>(textX), 55, "Sur la Mer", m_fontTexture, textColor);
    pipeline.RenderScene();
}

// =====================================================
// CLEANUP & EVENTS
// =====================================================
void Mer::OnExit()
{
    m_gameObjects.clear();
    std::cout << "Scene Sur la Mer quittee\n";
}

void Mer::SetParticlesFrozen(bool frozen)
{
    m_particlesFrozen = frozen;
}

void Mer::SetEditMode(bool editMode)
{
    m_isEditMode = editMode;
    SetParticlesFrozen(editMode);
}