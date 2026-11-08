#include "End.h"
#include "GraphicsPipeline.h"
#include "Camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "SceneManager.h"

// =====================================================
// CONSTRUCTEUR
// =====================================================
End::End(SceneManager& sceneManager) : Scene(sceneManager)
{
    m_gameObjects.clear();

    // 1. CHARGEMENT DE LA POLICE DE CARACTÈRES
    if (!m_fontTexture.LoadFromFile("external/Assets/boxy_bold_font_5.png")) {
        std::cerr << "[Erreur] Police introuvable !\n";
    }

    // 2. CRÉATION DES 3 GAMEOBJECTS CHATS (TRÈS RAPPROCHÉS)
    struct CatTransform {
        glm::vec3 pos;
        float scale;
    };

    CatTransform catConfigs[3] = {
        { glm::vec3(-0.55f, -0.3f, -7.0f), 0.05f }, // Chat 0 : Bleu (très proche)
        { glm::vec3(0.00f, -0.3f, -7.0f), 0.05f },  // Chat 1 : Blanc (centre)
        { glm::vec3(0.55f, -0.3f, -7.0f), 0.05f }   // Chat 2 : Rouge (très proche)
    };

    const char* objPath = "external/Assets/catn0.obj";
    const char* texturePath = "external/Assets/cat_text_m.jpeg";
    const char* workingDir = "external/Assets";

    for (int i = 0; i < 3; ++i) {
        GameObject cat;
        cat.transform.translation = catConfigs[i].pos;
        cat.transform.scale = glm::vec3(catConfigs[i].scale);
        cat.transform.rotation = glm::vec3(0.0f);

        if (LoadGameObject(cat, objPath, texturePath, workingDir)) {
            m_gameObjects.push_back(cat);
        }
    }

    // 3. ANIMATION DE TRAJECTOIRE (AVANCEMENT TRÈS PROCHE DE LA CAMÉRA)
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        AddKeyframe(&m_gameObjects[i].transform.translation.z, 0.0f, -7.0f);
        AddKeyframe(&m_gameObjects[i].transform.translation.z, 5.0f, 0.2f);
    }

    // 4. CONFIGURATION DE L'ÉMETTEUR DE PARTICULES (EXPLOSION BURST RING)
    m_magicEmitter.position = glm::vec3(0.0f, -0.3f, -3.0f);
    m_magicEmitter.spawnRate = 0.0f; // Émission continue désactivée (déclenchée par burst)
    m_magicEmitter.gravity = glm::vec3(0.0f, -0.5f, 0.0f);
    m_magicEmitter.colorStart = glm::vec4(1.0f, 0.85f, 0.2f, 1.0f); // Or / Jaune brillant
    m_magicEmitter.colorEnd = glm::vec4(1.0f, 0.1f, 0.4f, 0.0f);   // Fondu Magenta
    m_magicEmitter.lifeTimeMin = 1.0f;
    m_magicEmitter.lifeTimeMax = 2.2f;
    m_magicEmitter.sizeStart = 0.12f;
    m_magicEmitter.sizeEnd = 0.015f;
    m_magicEmitter.drag = 0.25f;
    m_magicEmitter.SetEmissionEnabled(true);
}

// =====================================================
// UPDATE
// =====================================================
void End::Update(float deltaTime)
{
    Scene::Update(deltaTime);
    m_time += deltaTime;

    if (!m_particlesFrozen) {
        m_hudAnimationTime += deltaTime;

        // Mise à jour de l'émetteur et déclenchement périodique de l'explosion en anneau (Burst Ring)
        m_magicEmitter.Update(deltaTime);
        m_waterTimer += deltaTime;

        if (m_waterTimer >= 3.0f) {
            m_magicEmitter.BurstRing(120, 2.5f, 5.0f);
            m_waterTimer = 0.0f;
        }
    }

    // ROTATION CONTINUELLE À 360° SANS S'ARRÊTER
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        m_gameObjects[i].transform.rotation.y = m_time * 6.0f;
    }
}

// =====================================================
// RENDER
// =====================================================
void End::Render(GraphicsPipeline& pipeline, SceneManager& sceneManager, const glm::mat4& viewMatrix, const glm::mat4& viewProjection, const glm::vec3& viewPos, float deltaTime)
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
        { { -1.0f,  1.0f, 0.9999f, 1.0f }, 0.05f, 0.05f, 0.15f },
        { { -1.0f, -1.0f, 0.9999f, 1.0f }, 0.15f, 0.15f, 0.30f },
        { {  1.0f, -1.0f, 0.9999f, 1.0f }, 0.15f, 0.15f, 0.30f },
        { {  1.0f,  1.0f, 0.9999f, 1.0f }, 0.05f, 0.05f, 0.15f }
    };
    pipeline.DrawSquareNDC(baseBackground[0], baseBackground[1], baseBackground[2], baseBackground[3]);
    pipeline.ClearZBuffer();
    pipeline.RenderScene();

    // -----------------------------------------------------
    // PASSE 2 : RENDU DES GAMEOBJECTS 3D AVEC VERTEX SHADER DE DÉFORMATION
    // -----------------------------------------------------
    pipeline.SetVertexShader([this](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;

        // Calcul de la déformation élastique / souple
        float waveX = std::sin(vertex.position.y * 12.0f + m_time * 6.0f) * 0.08f;
        float waveZ = std::cos(vertex.position.y * 10.0f + m_time * 5.0f) * 0.04f;

        transformed.position.x += waveX;
        transformed.position.z += waveZ;

        // Transformation MVP standard
        transformed.position = mvp * transformed.position;
        glm::mat3 normalMatrix = glm::mat3(mvp);
        transformed.normal = glm::normalize(normalMatrix * vertex.normal);

        return transformed;
        });

    glm::vec3 catColors[3] = {
        glm::vec3(0.1f, 0.35f, 1.0f), // Bleu
        glm::vec3(1.0f, 1.00f, 1.0f), // Blanc
        glm::vec3(1.0f, 0.15f, 0.15f) // Rouge
    };

    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        if (!m_gameObjects[i].loaded) continue;

        glm::vec3 catColor = (i < 3) ? catColors[i] : glm::vec3(1.0f);

        pipeline.SetFragmentShader([this, i, lightDir, viewPos, catColor](const BordLigne& pixel) -> uint32_t {
            const Material& mat = m_gameObjects[i].objModel.GetMaterial();

            glm::vec3 baseDiffuse = mat.diffuse * catColor * glm::vec3(pixel.r, pixel.g, pixel.b);

            if (mat.hasTexture) {
                uint32_t texColor = mat.texture.Sample(pixel.u, pixel.v);
                float rTex = ((texColor >> 16) & 0xFF) / 255.0f;
                float gTex = ((texColor >> 8) & 0xFF) / 255.0f;
                float bTex = (texColor & 0xFF) / 255.0f;
                baseDiffuse *= glm::vec3(rTex, gTex, bTex);
            }

            // Éclairage Blinn-Phong
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

        m_gameObjects[i].UpdateTransform();
        glm::mat4 mvpObj = viewProjection * m_gameObjects[i].modelMatrix;
        pipeline.DrawMesh(m_gameObjects[i].gpuVertices, mvpObj);
        pipeline.RenderScene();
    }

    // Réinitialisation du Vertex Shader standard
    pipeline.SetVertexShader([](const Vertex& vertex, const glm::mat4& mvp) {
        Vertex transformed = vertex;
        transformed.position = mvp * vertex.position;
        glm::mat3 normalMatrix = glm::mat3(mvp);
        transformed.normal = glm::normalize(normalMatrix * vertex.normal);
        return transformed;
        });

    // -----------------------------------------------------
    // PASSE 3 : RENDU DES PARTICULES (EXPLOSION BURST RING)
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
    // PASSE 4 : TEXTE 2D (HUD)
    // -----------------------------------------------------
    constexpr float fadeInDuration = 0.8f, holdDuration = 8.4f, fadeOutDuration = 0.8f, pauseDuration = 1.0f;
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

    pipeline.DrawText2D(static_cast<int>(textX) - 5, 35, "by MaitrePanda447\n & Lawx-sama", m_fontTexture, textColor);
    pipeline.RenderScene();
}

void End::OnExit()
{
    m_gameObjects.clear();
    std::cout << "Scene Fin quittee\n";
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