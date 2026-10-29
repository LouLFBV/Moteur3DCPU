#include "Window.h"
#include "MainScene.h"
#include "EmptyScene.h"
#include "SceneManager.h"
#include <iostream>

Window::Window(int renderWidth, int renderHeight, int windowWidth, int windowHeight, const std::string& title)
    : m_renderWidth(renderWidth), m_renderHeight(renderHeight),
    m_windowWidth(windowWidth), m_windowHeight(windowHeight),
    m_title(title)
{}

Window::~Window()
{
    if (m_framebufferTexture) SDL_DestroyTexture(m_framebufferTexture);
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window) SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Window::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return false;
    }

    m_window = SDL_CreateWindow(m_title.c_str(), m_windowWidth, m_windowHeight, 0);
    if (!m_window) {
        std::cerr << "Erreur SDL_CreateWindow : " << SDL_GetError() << std::endl;
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "Erreur SDL_CreateRenderer : " << SDL_GetError() << std::endl;
        return false;
    }

    m_framebufferTexture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        m_renderWidth,
        m_renderHeight
    );

    return true;
}

void Window::UpdateTexture(const void* pixels, int pitch)
{
    SDL_UpdateTexture(m_framebufferTexture, nullptr, pixels, pitch);
}

void Window::Render()
{
    SDL_RenderClear(m_renderer);
    SDL_RenderTexture(m_renderer, m_framebufferTexture, nullptr, nullptr);
    SDL_RenderPresent(m_renderer);
}

bool Window::PollEvents(bool& outIsPerspective, SceneManager& sceneManager)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            return false;
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) 
                return false;
            if (event.key.key == SDLK_SPACE && !event.key.repeat)
                outIsPerspective = !outIsPerspective;
        }
        sceneManager.HandleEvent(event, outIsPerspective);
    }
    return true;
}