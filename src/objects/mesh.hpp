#ifndef FLUIDGL_MESH_HPP
#define FLUIDGL_MESH_HPP

#include "core/shader_program.hpp"
#include <glm/glm.hpp>

struct Triangle {
    glm::vec4 v1, n1;
    glm::vec4 v2, n2;
    glm::vec4 v3, n3;
};

struct Bounds {
    glm::vec4 min;
    glm::vec4 max;
};

class Mesh {
private:
    GLuint m_posBuffer = 0;
    GLuint m_normalBuffer = 0;
    GLuint m_triangleBuffer = 0;
    std::vector<Triangle> m_triangles = {};
    std::vector<unsigned int> m_indicies = {};

    GLuint m_sdfBuffer = 0;
    GLuint m_sdfGradBuffer = 0;

    ShaderProgram m_constructSdfShader = {};
    ShaderProgram m_computeGradShader = {};

    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_velocity = glm::vec3(0.0f);
    glm::vec3 m_scale = glm::vec3(1.0f);
    glm::vec3 m_minBounds = glm::vec3(0.0f);
    glm::vec3 m_maxBounds = glm::vec3(0.0f);
    glm::ivec3 m_N = glm::ivec3(0);

public:
    Mesh();
    ~Mesh();

    Mesh(const Mesh&) = delete; 
    Mesh& operator=(const Mesh&) = delete;

    GLuint getPosBuffer() const { return m_posBuffer; }
    GLuint getNormalBuffer() const { return m_normalBuffer; }
    std::vector<unsigned int> getIndiciesBuffer() const { return m_indicies; }

    void loadObj(std::string path);
    void constructSDF(float dx);
    GLuint getSDF() const { return m_sdfBuffer; }
    GLuint getSDFGrad() const { return m_sdfGradBuffer; }
    Bounds getSDFBounds() const;
    glm::ivec3 getN() const { return m_N; }
    void setPos(glm::vec3 pos) { m_position = pos; }
    void setVel(glm::vec3 velocity) { m_velocity = velocity; }
    void setScale(glm::vec3 scale) { m_scale = scale; }
    glm::vec3 getPos() const { return m_position; }
    glm::vec3 getVel() const { return m_velocity; }
    glm::vec3 getScale() const { return m_scale; }
    glm::mat4 getModelMatrix() const;
};

#endif