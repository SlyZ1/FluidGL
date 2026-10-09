#ifndef WIREFRAMES_HPP
#define WIREFRAMES_HPP

#include <glm/glm.hpp>

#include "core/shader_program.hpp"
#include "core/camera.hpp"

using WireframeIndex = int;

class Wireframes {
private:
    struct Line {
        glm::vec3 start;
        glm::vec3 end;
    };

    std::weak_ptr<Camera> m_camera;

    GLuint m_lineVertVBO = 0;
    GLuint m_linesVBO = 0;
    GLuint m_colorsVBO = 0;
    GLuint m_lineVAO = 0;

    ShaderProgram m_lineProgram = {};

    std::vector<Line> m_lines = {};
    std::vector<glm::vec4> m_colors = {};

    static constexpr float s_lineVerts[] = { 0.f, 1.f };

public:
    Wireframes(std::weak_ptr<Camera> camera);
    ~Wireframes();

    Wireframes(const Wireframes&) = delete;
    Wireframes& operator=(const Wireframes&) = delete;
    Wireframes(Wireframes&&) = default;
    Wireframes& operator=(Wireframes&&) = default;

    WireframeIndex addLine(glm::vec3 start, glm::vec3 end, glm::vec4 color);
    WireframeIndex addBox(glm::vec3 min, glm::vec3 max, glm::vec4 color);
    WireframeIndex addBox2D(glm::vec3 min, glm::vec3 max, glm::vec4 color);
    WireframeIndex addTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec4 color);
    void updateLine(WireframeIndex index, glm::vec3 start, glm::vec3 end, glm::vec4 color);
    void updateBox(WireframeIndex index, glm::vec3 min, glm::vec3 max, glm::vec4 color);
    void updateBox2D(WireframeIndex index, glm::vec3 min, glm::vec3 max, glm::vec4 color);
    void updateTriangle(WireframeIndex index, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec4 color);

    void setLineWidth(float lineWidth) { glLineWidth(lineWidth); }

    void uploadData();
    void reset();

    void render() const;
};

#endif