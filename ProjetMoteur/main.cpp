#include <iostream>
#include "Window.h"
#include "Rasterizer.h"
#include "SceneManager.h"
#include "MainScene.h"

const int RENDER_WIDTH = 320;
const int RENDER_HEIGHT = 200;

int main(int argc, char* argv[])
{
    Window window(RENDER_WIDTH, RENDER_HEIGHT, 800, 500, "Moteur 3D CPU - Titre");
    if (!window.Init()) return 1;

    Rasterizer rasterizer(RENDER_WIDTH, RENDER_HEIGHT);
    SceneManager sceneManager;

    // Chargement de la scène initiale
    sceneManager.ChangeScene(std::make_unique<MainScene>());

    bool isPerspective = true;
    Uint64 lastPerfCount = SDL_GetPerformanceCounter();

    // Variables pour le calcul des performances / FPS
    Uint32 lastLogTime = SDL_GetTicks();
    int frameCount = 0;

    while (window.PollEvents(isPerspective, sceneManager))
    {
        Uint64 startPerf = SDL_GetPerformanceCounter();

        // 1. Calcul du Delta Time
        float deltaTime = (float)(startPerf - lastPerfCount) / (float)SDL_GetPerformanceFrequency();
        lastPerfCount = startPerf;

/*        // 2. Gestion des événements
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) return 0;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) return 0;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE && !event.key.repeat) {
                isPerspective = !isPerspective;
            }

            // Transmission des événements à la scène active
            sceneManager.HandleEvent(event, isPerspective);
        }*/

        // 3. Préparation du frame
        rasterizer.ResetStats();
        rasterizer.Clear();

        // 4. Update & Render de la scène active (MainScene ou EmptyScene)
        sceneManager.Update(deltaTime);
        sceneManager.Render(rasterizer);

        // 5. Exécution du rendu multithread
        rasterizer.RenderScene();

        // 6. Envoi du framebuffer à la fenêtre SDL
        window.UpdateTexture(rasterizer.GetPixels(), rasterizer.GetPitch());
        window.Render();

        // 7. Calcul du temps d'exécution et affichage dans la console
        Uint64 endPerf = SDL_GetPerformanceCounter();
        float frameTimeMs = (endPerf - startPerf) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        const RenderStats& stats = rasterizer.GetStats();

        frameCount++;
        Uint32 currentTime = SDL_GetTicks();

        if (currentTime - lastLogTime >= 1000) {
            printf("FPS: %d | CPU: %.2f ms | Triangles traites: %d | Triangles rasterises: %d | Pixels: %d | Threads: %d\n",
                frameCount,
                frameTimeMs,
                stats.trianglesProcessed,
                stats.trianglesRasterized,
                stats.pixelsProcessed.load(),
                stats.threadsUsed);

            lastLogTime = currentTime;
            frameCount = 0;
        }
    }

    return 0;
}