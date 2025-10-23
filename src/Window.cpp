#include "Window.h"
#include "Renderer.h"
#include "FractalPyramid.h"

#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"

#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const char* glsl_version = "#version 460 core";
constexpr int32_t GL_VERSION_MAJOR = 4;
constexpr int32_t GL_VERSION_MINOR = 6;
const std::string TEXTURE_PATH = "../../res/textures/texture.jpg";

Window::Window(int width, int height, const std::string& title)
    : WIDTH(width), HEIGHT(height), TITLE(title) {
    // inicjalizacja komponentów
    renderer = std::make_unique<Renderer>(WIDTH, HEIGHT);
    pyramid = std::make_unique<FractalPyramid>();
}

Window::~Window() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (glfw_window) {
        glfwDestroyWindow(glfw_window);
    }
    glfwTerminate();
}

void Window::glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool Window::init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        spdlog::error("Failed to initialize GLFW!");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfw_window = glfwCreateWindow(WIDTH, HEIGHT, TITLE.c_str(), NULL, NULL);
    if (!glfw_window) {
        spdlog::error("Failed to create GLFW Window!");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(glfw_window);
    glfwSwapInterval(1); // Enable VSync

    // Inicjalizacja GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        spdlog::error("Failed to initialize OpenGL loader!");
        return false;
    }

    // Inicjalizacja Renderera i Modelu
    if (!renderer->init()) return false;
    renderer->load_texture(TEXTURE_PATH);
    pyramid->generate_and_upload(recursion_depth, pyramid_base_color);

    init_imgui();
    return true;
}

void Window::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Włącz klawiaturowe sterowanie

    ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
    spdlog::info("Initialized ImGui.");
}

void Window::input() {

}

void Window::update() {

}

void Window::render_scene() {
    glm::vec4 clear_color_glm = glm::vec4(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    renderer->draw(*pyramid, rotation_x, rotation_y, rotation_z, clear_color_glm);
}

void Window::render_gui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Sprawdzanie i ewentualne ponowne generowanie piramidy
    static int prev_recursion_depth = -1;
    static ImVec4 prev_base_color = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);

    // Renderowanie okna Im_Gui
    {
        ImGui::Begin("Fractal Pyramid Controls");

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::ColorEdit3("Clear color", (float*)&clear_color);

        ImGui::Separator();
        ImGui::Text("Fractal Pyramid Parameters");

        ImGui::SliderInt("Recursion Depth", &recursion_depth, 0, 10);
        ImGui::ColorEdit3("Pyramid Base Color", (float*)&pyramid_base_color);

        ImGui::SliderFloat("Rotation X", &rotation_x, 0.0f, 360.0f);
        ImGui::SliderFloat("Rotation Y", &rotation_y, 0.0f, 360.0f);
        ImGui::SliderFloat("Rotation Z", &rotation_z, 0.0f, 360.0f);

        // Zresetuj obrót
        if (rotation_x >= 360.0f) rotation_x -= 360.0f;
        if (rotation_y >= 360.0f) rotation_y -= 360.0f;
        if (rotation_z >= 360.0f) rotation_z -= 360.0f;

        ImGui::End();
    }

    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    // Ponowne generowanie/ładowanie, jeśli parametry uległy zmianie
    if (recursion_depth != prev_recursion_depth ||
        memcmp(&pyramid_base_color, &prev_base_color, sizeof(ImVec4)) != 0)
    {
        pyramid->generate_and_upload(recursion_depth, pyramid_base_color);
        prev_recursion_depth = recursion_depth;
        prev_base_color = pyramid_base_color;
    }

    ImGui::Render();
}

void Window::end_frame() {
    int display_w, display_h;
    glfwMakeContextCurrent(glfw_window);
    glfwGetFramebufferSize(glfw_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwPollEvents();
    glfwSwapBuffers(glfw_window);
}

void Window::run() {
    while (!glfwWindowShouldClose(glfw_window)) {
        input();
        update();
        render_scene();
        render_gui();
        end_frame();
    }
}

