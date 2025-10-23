#include "Renderer.h"
#include "FractalPyramid.h"
#include "stb_image.h"
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Stałe Shadery
const char* vertexShaderSource = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 460 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0f);
}
)";

Renderer::Renderer(int width, int height) : WIDTH(width), HEIGHT(height) {}

Renderer::~Renderer() {
    if (shader_program != 0) glDeleteProgram(shader_program);
    if (texture_id != 0) glDeleteTextures(1, &texture_id);
    spdlog::info("Renderer resources cleaned up.");
}

GLuint Renderer::compileShader(GLenum type, const char* source) const {
    // ... Implementacja z oryginalnego kodu ...
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        spdlog::error("Shader compilation error: {}", infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint Renderer::createShaderProgram(const char* vertexSource, const char* fragmentSource) const {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (vertexShader == 0 || fragmentShader == 0) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        spdlog::error("Shader program linking error: {}", infoLog);
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint Renderer::loadTexture(const std::string& path) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Ustawienia tekstury
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 4) format = GL_RGBA;
        else if (nrChannels == 1) format = GL_RED;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        spdlog::info("Texture loaded successfully: {}", path);
    }
    else
    {
        spdlog::error("Failed to load texture: {}", path);
        glDeleteTextures(1, &texture);
        texture = 0;
    }
    stbi_image_free(data);
    return texture;
}


bool Renderer::init() {
    // Głębia
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Kompilowanie Shaderów
    shader_program = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shader_program == 0) {
        spdlog::error("Failed to create shader program!");
        return false;
    }

    // Definiowanie domyślnego Texture Unit
    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "ourTexture"), 0);

    return true;
}

void Renderer::load_texture(const std::string& path) {
    if (texture_id != 0) {
        glDeleteTextures(1, &texture_id); // Załadowanie nowej tekstury i usunięcie starej
    }
    texture_id = loadTexture(path);
    if (texture_id == 0) {
        spdlog::warn("Could not load texture. Pyramid will render with solid color/default texture.");
    }
}

void Renderer::draw(const FractalPyramid& pyramid, float rotX, float rotY, float rotZ, const glm::vec4& clearColor) const {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (shader_program == 0) return;

    glUseProgram(shader_program);

    // Aktywacja i powiązanie tekstury
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Macierze transformacji
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(0.5f));

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));

    // Przeliczanie macierzy projekcji
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

    // Przekazanie macierzy do shaderów
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Rysowanie
    pyramid.draw();
}