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

inline std::atomic<bool> quit(false);

// Define a command type
using Command = std::function<void()>;

inline std::queue<Command> command_queue;
inline std::mutex command_mutex;
inline std::condition_variable command_cv;

inline void sdl_main_loop() {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window* window = SDL_CreateWindow("My SDL Window",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    SDL_Event e;
    while (!quit.load()) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit.store(true);
            }
        }

        // Wait for commands or timeout every 16ms to maintain window responsiveness
        std::unique_lock<std::mutex> lock(command_mutex);
        command_cv.wait_for(lock, std::chrono::milliseconds(16), [] { return !command_queue.empty() || quit.load(); });

        while (!command_queue.empty()) {
            Command cmd = std::move(command_queue.front());
            command_queue.pop();
            lock.unlock();

            cmd();
            lock.lock();
        }

        // Your rendering code here...

        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

inline void post_command(Command cmd) {
    std::lock_guard<std::mutex> lock(command_mutex);
    command_queue.push(std::move(cmd));
    command_cv.notify_one();
}

#endif //UTILITYSDL_H