#include "Window.h"
#include "SceneManager.h"
#include <iostream>

Window::Window(int renderWidth, int renderHeight, int windowWidth, int windowHeight, const std::string& title)
    : m_renderWidth(renderWidth), m_renderHeight(renderHeight),
    m_windowWidth(windowWidth), m_windowHeight(windowHeight),
    m_title(title)
{
    m_editMode = false;
}

Window::~Window()
{
    if (m_track) {
        MIX_DestroyTrack(m_track);
    }
    if (m_mixer) {
        MIX_DestroyMixer(m_mixer);
    }

    MIX_Quit();

    if (m_framebufferTexture) {
        SDL_DestroyTexture(m_framebufferTexture);
    }
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }

    SDL_Quit();
}

bool Window::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return false;
    } 

    m_window = SDL_CreateWindow(m_title.c_str(), m_windowWidth, m_windowHeight, SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        std::cerr << "Erreur SDL_CreateWindow : " << SDL_GetError() << std::endl;
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "Erreur SDL_CreateRenderer : " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetRenderLogicalPresentation(m_renderer, m_renderWidth, m_renderHeight, SDL_LOGICAL_PRESENTATION_STRETCH);

    m_framebufferTexture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        m_renderWidth,
        m_renderHeight
    );

    // AUDIO

    MIX_Audio* audio = NULL;

    if (!MIX_Init()) {
        SDL_Log("Couldn't init SDL_mixer library: %s", SDL_GetError());
        return false;
    }

    m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!m_mixer) {
        SDL_Log("Couldn't create mixer on default device: %s", SDL_GetError());
        return false;
    }

    audio = MIX_LoadAudio(
        m_mixer,
        "external/Assets/Music/Medieval Music – Wild Boars Inn.mp3",
        false
    );

    m_track = MIX_CreateTrack(m_mixer);
    if (!m_track) {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        return false;
    }
    MIX_SetTrackAudio(m_track, audio);
    MIX_PlayTrack(m_track, 0);  

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
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                return false;
            }
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
                sceneManager.NextScene(true);
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
        if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_Z]) {
            camera.ProcessKeyboard(CameraDirection::FORWARD, deltaTime);
        }
        if (state[SDL_SCANCODE_S]) {
            camera.ProcessKeyboard(CameraDirection::BACKWARD, deltaTime);
        }
        if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_Q]) {
            camera.ProcessKeyboard(CameraDirection::LEFT, deltaTime);
        }
        if (state[SDL_SCANCODE_D]) {
            camera.ProcessKeyboard(CameraDirection::RIGHT, deltaTime);
        }
        if (state[SDL_SCANCODE_LALT]) {
            camera.ProcessKeyboard(CameraDirection::UP, deltaTime);
        }
        if (state[SDL_SCANCODE_LSHIFT]) {
            camera.ProcessKeyboard(CameraDirection::DOWN, deltaTime);
        }
    }

    return true;
}