#include "FractalPyramid.h"
#include <spdlog/spdlog.h>

FractalPyramid::~FractalPyramid() {
    cleanup();
}

void FractalPyramid::cleanup() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    VAO = VBO = 0;
    vertex_count = 0;
    spdlog::info("FractalPyramid VAO/VBO cleaned up.");
}

void FractalPyramid::generateTetrahedron(
    const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
    int depth, int current_depth,
    const ImVec4& baseColor
) {
    if (current_depth == depth) {
        glm::vec3 color = glm::vec3(baseColor.x, baseColor.y, baseColor.z);

        // Współrzędne tekstur
        glm::vec2 uvA = glm::vec2(0.5f, 1.0f);
        glm::vec2 uvB = glm::vec2(0.0f, 0.0f);
        glm::vec2 uvC = glm::vec2(1.0f, 0.0f);

        // Ściana 1
        pyramid_vertices.push_back(p0); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvA);
        pyramid_vertices.push_back(p1); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvB);
        pyramid_vertices.push_back(p2); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvC);

        // Ściana 2
        pyramid_vertices.push_back(p0); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvA);
        pyramid_vertices.push_back(p1); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvB);
        pyramid_vertices.push_back(p3); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvC);

        // Ściana 3
        pyramid_vertices.push_back(p0); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvA);
        pyramid_vertices.push_back(p2); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvB);
        pyramid_vertices.push_back(p3); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvC);

        // Ściana 4
        pyramid_vertices.push_back(p1); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvA);
        pyramid_vertices.push_back(p2); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvB);
        pyramid_vertices.push_back(p3); pyramid_colors.push_back(color); pyramid_tex_coords.push_back(uvC);

    } else {
        // Rekurencyjne generowanie
        glm::vec3 m01 = (p0 + p1) / 2.0f;
        glm::vec3 m02 = (p0 + p2) / 2.0f;
        glm::vec3 m03 = (p0 + p3) / 2.0f;
        glm::vec3 m12 = (p1 + p2) / 2.0f;
        glm::vec3 m13 = (p1 + p3) / 2.0f;
        glm::vec3 m23 = (p2 + p3) / 2.0f;

        generateTetrahedron(p0, m01, m02, m03, depth, current_depth + 1, baseColor);
        generateTetrahedron(m01, p1, m12, m13, depth, current_depth + 1, baseColor);
        generateTetrahedron(m02, m12, p2, m23, depth, current_depth + 1, baseColor);
        generateTetrahedron(m03, m13, m23, p3, depth, current_depth + 1, baseColor);
    }
}

void FractalPyramid::generate_and_upload(int depth, const ImVec4& baseColor) {
    bool color_changed = (baseColor.x != (pyramid_colors.empty() ? -1.0f : pyramid_colors[0].x)) ||
                         (baseColor.y != (pyramid_colors.empty() ? -1.0f : pyramid_colors[0].y)) ||
                         (baseColor.z != (pyramid_colors.empty() ? -1.0f : pyramid_colors[0].z));

    if (depth == previous_depth && !color_changed) {
        return;
    }

    // Czyszczenie danych
    pyramid_vertices.clear();
    pyramid_colors.clear();
    pyramid_tex_coords.clear();

    // Definiowanie 4 bazowych wierzchołków tetrahedru
    glm::vec3 p0 = glm::vec3( 0.0f,  1.0f,  0.0f);
    glm::vec3 p1 = glm::vec3(-1.0f, -1.0f,  1.0f);
    glm::vec3 p2 = glm::vec3( 1.0f, -1.0f,  1.0f);
    glm::vec3 p3 = glm::vec3( 0.0f, -1.0f, -1.0f);

    // Generowanie piramidy
    generateTetrahedron(p0, p1, p2, p3, depth, 0, baseColor);
    
    vertex_count = pyramid_vertices.size();
    
    // Konfiguracja VAO i VBO
    if (VAO == 0) glGenVertexArrays(1, &VAO);
    if (VBO == 0) glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Obliczanie rozmiarów danych
    size_t sizeV = pyramid_vertices.size() * sizeof(glm::vec3);
    size_t sizeC = pyramid_colors.size() * sizeof(glm::vec3);
    size_t sizeT = pyramid_tex_coords.size() * sizeof(glm::vec2);
    size_t totalSize = sizeV + sizeC + sizeT;

    // Rezerwacja i wczytanie danych do VBO
    glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STATIC_DRAW);

    // Wierzchołki
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeV, pyramid_vertices.data());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Kolory
    glBufferSubData(GL_ARRAY_BUFFER, sizeV, sizeC, pyramid_colors.data());
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)sizeV);
    glEnableVertexAttribArray(1);

    // Współrzędne tekstur
    glBufferSubData(GL_ARRAY_BUFFER, sizeV + sizeC, sizeT, pyramid_tex_coords.data());
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeV + sizeC));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    previous_depth = depth;
    spdlog::info("Pyramid generated (Depth: {}) and uploaded to GPU. Vertices: {}", depth, vertex_count);
}

void FractalPyramid::draw() const {
    if (VAO != 0) {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);
        glBindVertexArray(0);
    }
}