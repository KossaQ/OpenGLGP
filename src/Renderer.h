#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>


class FractalPyramid;

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    bool init();
    void load_texture(const std::string& path);
    void draw(const FractalPyramid& pyramid, float rotX, float rotY, float rotZ, const glm::vec4& clearColor) const;

private:
    const int WIDTH;
    const int HEIGHT;
    GLuint shader_program = 0;
    GLuint texture_id = 0;

    GLuint compileShader(GLenum type, const char* source) const;
    GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) const;
    GLuint loadTexture(const std::string& path);
};