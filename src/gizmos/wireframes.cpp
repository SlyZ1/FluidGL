#include "wireframes.hpp"

using namespace glm;

Wireframes::Wireframes(std::weak_ptr<Camera> camera) : m_camera(camera) {
    m_lineProgram.create();
    m_lineProgram.load(GL_VERTEX_SHADER, "src/shaders/gizmos/lineVert.glsl");
    m_lineProgram.load(GL_FRAGMENT_SHADER, "src/shaders/gizmos/lineFrag.glsl");
    m_lineProgram.link();

    glGenVertexArrays(1, &m_lineVAO);
    glBindVertexArray(m_lineVAO);

    glGenBuffers(1, &m_lineVertVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVertVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_lineVerts), &s_lineVerts[0], GL_STATIC_DRAW);

    glGenBuffers(1, &m_linesVBO);
    glGenBuffers(1, &m_colorsVBO);
}

Wireframes::~Wireframes() {
    glDeleteVertexArrays(1, &m_lineVAO);
    glDeleteBuffers(1, &m_lineVertVBO);
    glDeleteBuffers(1, &m_linesVBO);
    glDeleteBuffers(1, &m_colorsVBO);
}

WireframeIndex Wireframes::addLine(vec3 start, vec3 end, vec4 color) {
    m_lines.push_back({ start, end });
    m_colors.push_back(color);
    return (WireframeIndex)(m_lines.size() - 1);
}

WireframeIndex Wireframes::addBox(vec3 min, vec3 max, vec4 color) {
    WireframeIndex result = addLine(min, vec3(max.x, min.y, min.z), color);
    addLine(min, vec3(min.x, max.y, min.z), color);
    addLine(min, vec3(min.x, min.y, max.z), color);

    addLine(max, vec3(min.x, max.y, max.z), color);
    addLine(max, vec3(max.x, min.y, max.z), color);
    addLine(max, vec3(max.x, max.y, min.z), color);

    addLine(vec3(max.x, min.y, min.z), vec3(max.x, max.y, min.z), color);
    addLine(vec3(max.x, min.y, min.z), vec3(max.x, min.y, max.z), color);

    addLine(vec3(min.x, max.y, min.z), vec3(max.x, max.y, min.z), color);
    addLine(vec3(min.x, max.y, min.z), vec3(min.x, max.y, max.z), color);

    addLine(vec3(min.x, min.y, max.z), vec3(max.x, min.y, max.z), color);
    addLine(vec3(min.x, min.y, max.z), vec3(min.x, max.y, max.z), color);

    return result;
}

WireframeIndex Wireframes::addBox2D(vec3 min, vec3 max, vec4 color){
    WireframeIndex result = addLine(min, vec3(min.x, max.y, min.z), color);
    addLine(min, vec3(max.x, min.y, max.z), color);
    addLine(vec3(min.x, max.y, min.z), max, color);
    addLine(vec3(max.x, min.y, max.z), max, color);

    return result;
}

WireframeIndex Wireframes::addTriangle(vec3 v1, vec3 v2, vec3 v3, vec4 color){
    WireframeIndex result = addLine(v1, v2, color);
    addLine(v2, v3, color);
    addLine(v3, v1, color);

    return result;
}

void Wireframes::updateLine(WireframeIndex index, vec3 start, vec3 end, vec4 color) {
    m_lines[index] = { start, end };
    m_colors[index] = color;
}

void Wireframes::updateBox(WireframeIndex index, vec3 min, vec3 max, vec4 color) {
    updateLine(index + 0, min, vec3(max.x, min.y, min.z), color);
    updateLine(index + 1, min, vec3(min.x, max.y, min.z), color);
    updateLine(index + 2, min, vec3(min.x, min.y, max.z), color);

    updateLine(index + 3, max, vec3(min.x, max.y, max.z), color);
    updateLine(index + 4, max, vec3(max.x, min.y, max.z), color);
    updateLine(index + 5, max, vec3(max.x, max.y, min.z), color);

    updateLine(index + 6, vec3(max.x, min.y, min.z), vec3(max.x, max.y, min.z), color);
    updateLine(index + 7, vec3(max.x, min.y, min.z), vec3(max.x, min.y, max.z), color);

    updateLine(index + 8, vec3(min.x, max.y, min.z), vec3(max.x, max.y, min.z), color);
    updateLine(index + 9, vec3(min.x, max.y, min.z), vec3(min.x, max.y, max.z), color);

    updateLine(index + 10, vec3(min.x, min.y, max.z), vec3(max.x, min.y, max.z), color);
    updateLine(index + 11, vec3(min.x, min.y, max.z), vec3(min.x, max.y, max.z), color);
}

void Wireframes::updateBox2D(WireframeIndex index, vec3 min, vec3 max, vec4 color){
    updateLine(index + 0, min, vec3(min.x, max.y, min.z), color);
    updateLine(index + 1, min, vec3(max.x, min.y, max.z), color);
    updateLine(index + 2, vec3(min.x, max.y, min.z), max, color);
    updateLine(index + 3, vec3(max.x, min.y, max.z), max, color);
}

void Wireframes::updateTriangle(WireframeIndex index, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec4 color){
    updateLine(index + 0, v1, v2, color);
    updateLine(index + 1, v2, v3, color);
    updateLine(index + 2, v3, v1, color);
}

void Wireframes::uploadData(){
    glBindBuffer(GL_ARRAY_BUFFER, m_linesVBO);
    glBufferData(GL_ARRAY_BUFFER, m_lines.size() * sizeof(Line), m_lines.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
    glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(vec4), m_colors.data(), GL_STATIC_DRAW);
}

void Wireframes::reset(){
    m_lines.clear();
    m_colors.clear();
}

void Wireframes::render() const {
    m_lineProgram.use();

    if (auto camera = m_camera.lock()){
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uView"), 1, GL_FALSE, &camera->viewMatrix()[0][0]);
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &camera->projectionMatrix()[0][0]);
    }

    glBindVertexArray(m_lineVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_lineVertVBO);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_linesVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Line), 0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Line), (void*)sizeof(vec3));
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), 0);
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(3);

    glDrawArraysInstanced(GL_LINES, 0, 2, m_lines.size());
}