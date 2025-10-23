#pragma once

#include "imgui.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class FractalPyramid {
public:
    FractalPyramid() = default;
    ~FractalPyramid();

    void generate_and_upload(int depth, const ImVec4& baseColor);
    void draw() const;

    size_t get_vertex_count() const { return vertex_count; }

private:
    GLuint VAO = 0;
    GLuint VBO = 0;
    size_t vertex_count = 0;
    int previous_depth = -1;

    std::vector<glm::vec3> pyramid_vertices;
    std::vector<glm::vec3> pyramid_colors;
    std::vector<glm::vec2> pyramid_tex_coords;

    void cleanup();

    void generateTetrahedron(
        const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
        int depth, int current_depth,
        const ImVec4& baseColor
    );
};