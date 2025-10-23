#pragma once

#include "imgui.h"
#include <GLFW/glfw3.h>
#include <string>
#include <memory>

class Renderer;
class FractalPyramid;

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool init();
    void run();

private:
    const int32_t WIDTH;
    const int32_t HEIGHT;
    const std::string TITLE;

    GLFWwindow* glfw_window = nullptr;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool show_demo_window = true;
    float rotation_x = 0.0f;
    float rotation_y = 0.0f;
    float rotation_z = 0.0f;
    int recursion_depth = 0;
    ImVec4 pyramid_base_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<FractalPyramid> pyramid;

    void init_imgui();
    void input();
    void update();
    void render_scene();
    void render_gui();
    void end_frame();

    static void glfw_error_callback(int error, const char* description);
};