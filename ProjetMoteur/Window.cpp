#include "Window.h"
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

bool Window::IsInEditMode() const
{
    return m_editMode;
}


bool Window::PollEvents(bool& isPerspective, Camera& camera, float deltaTime, SceneManager& sceneManager)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) return false;

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                return false;
            }
            // Touche 'E' ou 'Tab' pour basculer en Mode Edit
            if (event.key.key == SDLK_E || event.key.key == SDLK_TAB) {
                m_editMode = !m_editMode;

                // Capturer la souris et la cacher quand on est en mode Edit
                SDL_SetWindowRelativeMouseMode(m_window,m_editMode);
            }
            if (event.key.key == SDLK_SPACE && !event.key.repeat) {
                isPerspective = !isPerspective;
            }
            if (event.key.key == SDLK_T) {
                std::cout << "Scene Changer" << std::endl;
                sceneManager.NextScene();
            }
        }

        // Si le mode Edit est actif, la souris fait pivoter la caméra
        if (m_editMode && event.type == SDL_EVENT_MOUSE_MOTION) {
            // event.motion.yrel est inversé en coordonnées écran
            camera.ProcessMouseMovement((float)event.motion.xrel, -(float)event.motion.yrel);   
        }
    }

    // Gestion des déplacements continus au clavier (ZQSD / WASD + Espace/Shift)
    if (m_editMode) {
        const bool* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_Z]) camera.ProcessKeyboard(CameraDirection::FORWARD, deltaTime);
        if (state[SDL_SCANCODE_S])                          camera.ProcessKeyboard(CameraDirection::BACKWARD, deltaTime);
        if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_Q]) camera.ProcessKeyboard(CameraDirection::LEFT, deltaTime);
        if (state[SDL_SCANCODE_D])                          camera.ProcessKeyboard(CameraDirection::RIGHT, deltaTime);
        if (state[SDL_SCANCODE_LALT])                      camera.ProcessKeyboard(CameraDirection::UP, deltaTime);
        if (state[SDL_SCANCODE_LSHIFT])                     camera.ProcessKeyboard(CameraDirection::DOWN, deltaTime);
    }

    return true;
}