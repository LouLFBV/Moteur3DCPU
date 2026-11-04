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
#include "MainScene.h"
#include "EmptyScene.h"
#include "ThirdScene.h"
#include "Village.h"
#include "Aqua.h"
#include "Mer.h"
#include "Beach.h"
#include "Camera.h"

int main(int argc, char* argv[])
{
    // INITIALISATION
    SceneManager sceneManager;

    Window window(sceneManager.GetWidth(), sceneManager.GetHeight(), 800, 500, "Moteur 3D CPU - Titre");
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


    sceneManager.AddScene(std::make_unique<MainScene>(sceneManager));
    sceneManager.AddScene(std::make_unique<EmptyScene>(sceneManager));
    sceneManager.AddScene(std::make_unique<ThirdScene>(sceneManager));
    sceneManager.AddScene(std::make_unique<Aqua>(sceneManager));
    sceneManager.AddScene(std::make_unique<Mer>(sceneManager));
    sceneManager.AddScene(std::make_unique<Beach>(sceneManager));
    sceneManager.AddScene(std::make_unique<Village>(sceneManager));
    // FIN INITIALISATION
    
    while (true) {
        // Calcul du deltaTime en secondes
        Uint64 startPerf = SDL_GetPerformanceCounter();
        Uint32 currentTicks = SDL_GetTicks();
        float deltaTime = (currentTicks - lastFrameTicks) * 0.001f;
        lastFrameTicks = currentTicks;

        bool isEditMode = window.IsInEditMode();

        if (isEditMode)
        {
            timeInEditMode = SDL_GetTicks() - lastTimeEditMode;
        }
        else
        {
            lastTimeEditMode = SDL_GetTicks() - timeInEditMode;
        }



        // On passe la caméra et deltaTime à PollEventsm
        if (!window.PollEvents(isPerspective, editCamera, deltaTime, sceneManager)) return 0;



        if (auto* currentScene = dynamic_cast<ThirdScene*>(
            sceneManager.GetCurrentScene()))
        {
            currentScene->SetParticlesFrozen(isEditMode);
        }

        if (auto* aqua = dynamic_cast<Aqua*>(sceneManager.GetCurrentScene()))
        {
            aqua->SetEditMode(window.IsInEditMode());
        }
        if (auto* mer = dynamic_cast<Mer*>(sceneManager.GetCurrentScene()))
        {
            mer->SetEditMode(window.IsInEditMode());
        }
        if (auto* beach = dynamic_cast<Beach*>(sceneManager.GetCurrentScene()))
        {
            beach->SetEditMode(window.IsInEditMode());
        }
        if (auto* village = dynamic_cast<Village*>(sceneManager.GetCurrentScene()))
        {
            village->SetEditMode(window.IsInEditMode());
        }
        Camera* activeCamera = nullptr;

        if (window.IsInEditMode()) {
            activeCamera = &editCamera; // En mode Edit, on prend la freecam globale
        }
        else if (sceneManager.GetCurrentScene()) {
            activeCamera = &sceneManager.GetCurrentScene()->GetCamera(); // En jeu, on prend la caméra de la scène
        }


        glm::mat4 projection;
        if (isPerspective) {
            projection = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
        }
        else {
            float taille = 2.0f;
            projection = glm::ortho(-taille * aspect, taille * aspect, -taille, taille, zNear, zFar);
        }


        // 3. Calcul des matrices à partir de la caméra active
        glm::mat4 view = activeCamera->GetViewMatrix();
        glm::mat4 viewProjection = projection * view;
        glm::vec3 viewPos = activeCamera->position;


        float time = lastTimeEditMode * 0.001f;


        pipeline.ResetStats();
        pipeline.Clear();

        sceneManager.Update(deltaTime);
        sceneManager.Render(pipeline, view, viewProjection, viewPos, time);  
        window.UpdateTexture(pipeline.GetPixels(), pipeline.GetPitch());
        window.Render();


        // Statistiques de performance
        Uint64 endPerf = SDL_GetPerformanceCounter();
        float frameTimeMs = (endPerf - startPerf) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        const RenderStats& stats = pipeline.GetStats();

        static Uint32 lastLogTime = 0;
        static int frameCount = 0;
        Uint32 currentTime = SDL_GetTicks();
        frameCount++;

        if (currentTime - lastLogTime >= 1000) {

            printf(
                "FPS: %d | CPU: %.2f ms | "
                "Soumis: %d | Frustum rejetes: %d | "
                "Triangles conserves: %d | "
                "Pixels: %d | Threads: %d\n",

                frameCount,
                frameTimeMs,

                stats.trianglesSubmitted,
                stats.trianglesFrustumCulled,
                stats.trianglesSubmitted -  stats.trianglesFrustumCulled,

                stats.pixelsProcessed.load(),
                stats.threadsUsed
            );

            lastLogTime = currentTime;
            frameCount = 0;
        }
    }
    return 0;
}