#pragma once
#include <SDL3/SDL.h>
#include <string>

class SceneManager;

class Window {
public:
    Window(int renderWidth, int renderHeight, int windowWidth, int windowHeight, const std::string& title);
    ~Window(); 

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Init();
    void UpdateTexture(const void* pixels, int pitch);
    void Render();
    bool PollEvents(bool& outIsPerspective, SceneManager& sceneManager);

private:
    int m_renderWidth;
    int m_renderHeight;
    int m_windowWidth;
    int m_windowHeight;
    std::string m_title;

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_framebufferTexture = nullptr;
};