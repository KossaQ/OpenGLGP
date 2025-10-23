#include "Window.h"
#include <spdlog/spdlog.h>

int main(int, char**) {
    try {
        Window app(1920, 1080, "Fractal Pyramid (Class-Based)");

        // Inicjalizacja komponentów
        if (!app.init()) {
            spdlog::error("Application initialization failed!");
            return EXIT_FAILURE;
        }

        spdlog::info("Application initialized. Running main loop.");

        app.run();

    } catch (const std::exception& e) {
        spdlog::critical("Fatal error: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        spdlog::critical("An unknown fatal error occurred.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}