#include "Window.h"
#include "SceneManager.h"
#include <SDL3/SDL_mixer.h>
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
    MIX_Quit();
    SDL_Quit();
}

bool Window::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
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










    // AUDIO


    char* path = NULL;
    MIX_Audio* audio = NULL;

    SDL_SetAppMetadata("Example Load And Play", "1.0", "com.example.load-and-play");

    /* this doesn't have to run very much, so give up tons of CPU time between iterations. Optional! */
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "5");

    /* we don't need video, but we'll make a window for smooth operation. */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return false;
    }

    if (!MIX_Init()) {
        SDL_Log("Couldn't init SDL_mixer library: %s", SDL_GetError());
        return false;
    }

    /* Create a mixer on the default audio device. Don't care about the specific audio format. */
    m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!m_mixer) {
        SDL_Log("Couldn't create mixer on default device: %s", SDL_GetError());
        return false;
    }

    /* load a sound file */
    SDL_asprintf(&path, "external/Assets/Music/Medieval Music – Wild Boars Inn.mp3", SDL_GetBasePath());  /* allocate a string of the full file path */
    audio = MIX_LoadAudio(m_mixer, path, false);
    if (!audio) {
        SDL_Log("Couldn't load %s: %s", path, SDL_GetError());
        SDL_free(path);
        return false;
    }

    SDL_free(path);  /* done with this, the file is loaded. */

    /* we need a track on the mixer to play the audio. Each track has audio assigned to it, and
       all playing tracks are mixed together for the final output. */

    m_track = MIX_CreateTrack(m_mixer);
    if (!m_track) {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        return false;
    }
    MIX_SetTrackAudio(m_track, audio);

    /* start the audio playing! */
    MIX_PlayTrack(m_track, 0);  /* no extra options this time, so a zero for the second argument. */

    /* we don't save `audio`; SDL_mixer will clean it up for us during MIX_Quit(). */

    return true;  /* carry on with the program! */
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
        if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_Z]) camera.ProcessKeyboard(CameraDirection::FORWARD, deltaTime);
        if (state[SDL_SCANCODE_S])                          camera.ProcessKeyboard(CameraDirection::BACKWARD, deltaTime);
        if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_Q]) camera.ProcessKeyboard(CameraDirection::LEFT, deltaTime);
        if (state[SDL_SCANCODE_D])                          camera.ProcessKeyboard(CameraDirection::RIGHT, deltaTime);
        if (state[SDL_SCANCODE_LALT])                      camera.ProcessKeyboard(CameraDirection::UP, deltaTime);
        if (state[SDL_SCANCODE_LSHIFT])                     camera.ProcessKeyboard(CameraDirection::DOWN, deltaTime);
    }

    return true;
}