// UtilitySDL.h
#ifndef UTILITYSDL_H
#define UTILITYSDL_H

#include <iostream>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <SDL.h>

constexpr int FPS = 60;
constexpr int frameDelay = 1000 / FPS;

inline Uint32 frameStart;
inline int frameTime;


inline std::atomic<bool> quit(false);

// Define a command type
using Command = std::function<void()>;

inline std::queue<Command> command_queue;
inline std::mutex command_mutex;
inline std::condition_variable command_cv;

inline SDL_Window* window = nullptr;
inline SDL_Renderer* renderer = nullptr;
inline SDL_Texture* front_buffer = nullptr;
inline SDL_Texture* back_buffer = nullptr;

inline bool pending_buffer_swap = false;

inline void sdl_main_loop()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return;
    }

    window = SDL_CreateWindow("",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              800, 600,
                              SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    front_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 800, 600);
    back_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 800, 600);

    if (!front_buffer || !back_buffer)
    {
        std::cerr << "Could not create textures: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_Event e;
    while (!quit.load())
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit.store(true);
            }
        }

        frameStart = SDL_GetTicks();

        // Wait for commands or timeout every 16ms to maintain window responsiveness
        std::unique_lock<std::mutex> lock(command_mutex);
        command_cv.wait_for(lock, std::chrono::milliseconds(16), [] { return !command_queue.empty() || quit.load(); });

        // Process all commands
        while (!command_queue.empty())
        {
            Command cmd = std::move(command_queue.front());
            command_queue.pop();
            lock.unlock();

            cmd();
            lock.lock();
        }

        if (pending_buffer_swap)
        {
            // Render front buffer to screen
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_RenderCopy(renderer, front_buffer, nullptr, nullptr);
            SDL_RenderPresent(renderer);

            // Swap buffers
            std::swap(front_buffer, back_buffer);
            pending_buffer_swap = false;
        }


        // Render front buffer to screen
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, front_buffer, nullptr, nullptr);
        SDL_RenderPresent(renderer);


        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime)
        {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    SDL_DestroyTexture(front_buffer);
    SDL_DestroyTexture(back_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

inline void post_command(Command cmd)
{
    std::lock_guard<std::mutex> lock(command_mutex);
    command_queue.push(std::move(cmd));
    command_cv.notify_one();
}

inline void sdl_quit()
{
    post_command([] { quit.store(true); });
}

// Function to hide the SDL window using the command queue
inline void sdl_hide()
{
    post_command([]
    {
        if (window)
        {
            SDL_HideWindow(window);
        }
    });
}

// Function to show the SDL window using the command queue
inline void sdl_show()
{
    post_command([]
    {
        if (window)
        {
            SDL_ShowWindow(window);
        }
    });
}

// Function to set the SDL window title using the command queue
inline void sdl_set_window_title(const std::string& title)
{
    post_command([title]
    {
        if (window)
        {
            SDL_SetWindowTitle(window, title.c_str());
        }
    });
}

// Sample function to draw on the back buffer
inline void draw_on_back_buffer()
{
    post_command([]
    {
        if (back_buffer)
        {
            SDL_SetRenderTarget(renderer, back_buffer);
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green color
            SDL_RenderClear(renderer);
            SDL_Rect rect = {200, 150, 400, 300};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderTarget(renderer, nullptr);
        }
    });
}

inline void swap_buffers()
{
    post_command([]
    {
        pending_buffer_swap = true;
    });
}

// manage sdl threads

static std::unique_ptr<std::thread> sdlThread = nullptr;


// TODO: why can SDL not be restarted after it is stopped?
static void prim_start_sdl()
{
    if (sdlThread == nullptr)
        sdlThread = std::make_unique<std::thread>(sdl_main_loop);
}

static void prim_end_sdl()
{
    if (sdlThread && sdlThread->joinable())
    {
        sdlThread->join();
    }
    sdlThread.reset();
    sdlThread = nullptr;
}

#endif //UTILITYSDL_H
