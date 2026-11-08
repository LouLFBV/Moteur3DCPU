#include <iostream>
#include <cmath>
#include <memory>
#include <algorithm>
#include <map>
#include "MathUtils.h"
#include "ObjLoader.h"
#include "Window.h"
#include "GraphicsPipeline.h"
#include "SceneManager.h"
#include "Village.h"
#include "Aqua.h"
#include "Mer.h"
#include "Beach.h"
#include "End.h"
#include "Intro.h"
#include "LastScene.h"
#include "Camera.h"


static void ApplyEditMode(Scene* scene, bool isEditMode)
{
    if (!scene) {
        return;
    }

     if (auto* aqua = dynamic_cast<Aqua*>(scene)) {
        aqua->SetEditMode(isEditMode);
    }
    else if (auto* mer = dynamic_cast<Mer*>(scene)) {
        mer->SetEditMode(isEditMode);
    }
    else if (auto* beach = dynamic_cast<Beach*>(scene)) {
        beach->SetEditMode(isEditMode);
    }
    else if (auto* village = dynamic_cast<Village*>(scene)) {
        village->SetEditMode(isEditMode);
    }
    else if (auto* lastScene = dynamic_cast<LastScene*>(scene)) {
        lastScene->SetEditMode(isEditMode);
    }
}


int main(int argc, char* argv[])
{
    // INITIALISATION
    SceneManager sceneManager;

    Window window(sceneManager.GetWidth(), sceneManager.GetHeight(), 800, 500, "Démo - World Adventure");
    if (!window.Init()) return 1;

    GraphicsPipeline pipeline(sceneManager.GetWidth(), sceneManager.GetHeight());

    // CAMERA
    Camera editCamera(glm::vec3(0.0f, 0.0f, 1.0f));;
    bool isPerspective = true;
    float aspect = (float)sceneManager.GetWidth() / (float)sceneManager.GetHeight();
    float zNear = 0.1f;
    float zFar = 100.0f;

    // TEMPS
    Uint32 lastFrameTicks = SDL_GetTicks();

    float lastTimeEditMode = 0.0f;
    float timeInEditMode = 0.0f;

    sceneManager.AddScene(std::make_unique<Intro>(sceneManager), 0.0f , 3.0f);
    sceneManager.AddScene(std::make_unique<Aqua>(sceneManager), 3.0f, 13.0f);
    sceneManager.AddScene(std::make_unique<Mer>(sceneManager), 13.0f, 23.0f);
    sceneManager.AddScene(std::make_unique<Beach>(sceneManager), 23.0f, 33.0f);
    sceneManager.AddScene(std::make_unique<Village>(sceneManager), 33.0f, 43.0f);
    sceneManager.AddScene(std::make_unique<LastScene>(sceneManager), 43.0f, 50.0f);
    sceneManager.AddScene(std::make_unique<End>(sceneManager), 50.0f, 60.0f);
    

    while (true)
    {

        Uint64 startPerf = SDL_GetPerformanceCounter();

        Uint32 currentTicks = SDL_GetTicks();
        float deltaTime = (currentTicks - lastFrameTicks) * 0.001f;

        lastFrameTicks = currentTicks;

        // GESTION DES ÉVÉNEMENTS/INPUTS

        if (!window.PollEvents(isPerspective, editCamera, deltaTime, sceneManager))
            return 0;

        bool isEditMode = window.IsInEditMode();

        // Gestion du temps écoulé hors du mode Edit
        if (isEditMode)
        {
            timeInEditMode = currentTicks - lastTimeEditMode;
        }
        else
        {
            lastTimeEditMode = currentTicks - timeInEditMode;
        }

        float time = lastTimeEditMode * 0.001f;

        // SCÈNE ACTIVE
        Scene* currentScene = sceneManager.GetCurrentScene();

        if (!currentScene)
            break;

        ApplyEditMode(currentScene, isEditMode);


        // CAMÉRA ACTIVE
        Camera* activeCamera = nullptr;

        if (isEditMode)
        {
            activeCamera = &editCamera;
        }
        else
        {
            activeCamera = &currentScene->GetCamera();
        }

        if (!activeCamera)
            break;

        // MATRICES DE PROJECTION ET DE VUE
        glm::mat4 projection;

        if (isPerspective)
        {
            projection = glm::perspective(
                glm::radians(60.0f),
                aspect,
                zNear,
                zFar
            );
        }
        else
        {
            constexpr float orthoSize = 2.0f;

            projection = glm::ortho(
                -orthoSize * aspect,
                orthoSize * aspect,
                -orthoSize,
                orthoSize,
                zNear,
                zFar
            );
        }

        glm::mat4 view = activeCamera->GetViewMatrix();
        glm::mat4 viewProjection = projection * view;
        glm::vec3 viewPos = activeCamera->position;

        // UPDATE
        pipeline.ResetStats();
        pipeline.Clear();

        sceneManager.Update(deltaTime, isEditMode);

        if (!sceneManager.m_isRunning)
            break;

        // RENDER
        sceneManager.Render(
            pipeline,
            view,
            viewProjection,
            viewPos,
            time
        );

        // AFFICHAGE
        window.UpdateTexture(
            pipeline.GetPixels(),
            pipeline.GetPitch()
        );

        window.Render();

        // STATISTIQUES DE PERFORMANCE
        Uint64 endPerf = SDL_GetPerformanceCounter();

        float frameTimeMs =
            (endPerf - startPerf) /
            static_cast<float>(SDL_GetPerformanceFrequency()) * 1000.0f;

        const RenderStats& stats = pipeline.GetStats();

        static Uint32 lastLogTime = 0;
        static int frameCount = 0;

        frameCount++;

        if (currentTicks - lastLogTime >= 1000)
        {
            printf(
                "FPS: %d | CPU: %.2f ms | "
                "Soumis: %d | Frustum rejetes: %d | "
                "Triangles conserves: %d | "
                "Pixels: %d | Threads: %d\n",

                frameCount,
                frameTimeMs,
                stats.trianglesSubmitted,
                stats.trianglesFrustumCulled,
                stats.trianglesSubmitted - stats.trianglesFrustumCulled,
                stats.pixelsProcessed.load(),
                stats.threadsUsed
            );

            lastLogTime = currentTicks;
            frameCount = 0;
        }
    }

    return 0;
}