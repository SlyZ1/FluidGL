#include "mesh.hpp"

#include <filesystem>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>
#include "helpers/logger.hpp"
#include "helpers/utils.hpp"

using namespace glm;
using namespace std;
namespace fs = filesystem;

Mesh::Mesh() {
    ShaderProgram::loadCompute(m_constructSdfShader, "src/shaders/sdf/constructsdf.glsl");
    ShaderProgram::loadCompute(m_computeGradShader, "src/shaders/field/computegrad.glsl");
}

Mesh::~Mesh() {
    ShaderProgram::deleteBuffer(m_posBuffer);
    ShaderProgram::deleteBuffer(m_normalBuffer);
    ShaderProgram::deleteBuffer(m_sdfBuffer);
    ShaderProgram::deleteBuffer(m_sdfGradBuffer);
}

void Mesh::loadObj(string path) {
    if (fs::path(path).extension().string() != ".obj") return;

    Logger::logInfo("Loading OBJ file : " + path, __LOG_DATA__);
    ifstream ifs(path);
    if (!ifs) {
        Logger::logError("Cannot open file.", __LOG_DATA__);
        return;
    }

    vector<vec4> objPoses = {};
    vector<vec4> objNormals = {};

    tinyobj::attrib_t attrib;
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;
    string warn, err;

    tinyobj::LoadObj(
        &attrib,
        &shapes,
        &materials,
        &warn,
        &err,
        &ifs,
        nullptr
    );

    if (!warn.empty()) Logger::logWarning(warn, __LOG_DATA__);
    if (!err.empty()){
        Logger::logError(err, __LOG_DATA__);
        return;
    }
    
    auto getPos = [&](tinyobj::index_t i){
        return vec4(
            attrib.vertices[3 * i.vertex_index + 0],
            attrib.vertices[3 * i.vertex_index + 1],
            attrib.vertices[3 * i.vertex_index + 2],
            1.0f
        );
    };

    auto getNormal = [&](tinyobj::index_t i){
        if (i.normal_index >= 0) {
            return vec4(
                attrib.normals[3 * i.normal_index + 0],
                attrib.normals[3 * i.normal_index + 1],
                attrib.normals[3 * i.normal_index + 2],
                0.0f
            );
        }
        return vec4(0.0f); // pas de normale dans le fichier
    };

    for(const auto &shape : shapes){
        const auto &mesh = shape.mesh;
        int numVertex = 3;
        int size = (int)mesh.num_face_vertices.size();
        for (int i = 0; i < size; i++)
        {
            int fv = mesh.num_face_vertices[i];
            if (fv == 3){
                auto i1 = mesh.indices[numVertex * i + 0];
                auto i2 = mesh.indices[numVertex * i + 1];
                auto i3 = mesh.indices[numVertex * i + 2];

                Triangle tri = {
                    getPos(i1), getNormal(i1),
                    getPos(i2), getNormal(i2),
                    getPos(i3), getNormal(i3)
                };

                m_minBounds = min(m_minBounds, Utils::xyz(tri.v1));
                m_minBounds = min(m_minBounds, Utils::xyz(tri.v2));
                m_minBounds = min(m_minBounds, Utils::xyz(tri.v3));
                m_maxBounds = max(m_maxBounds, Utils::xyz(tri.v1));
                m_maxBounds = max(m_maxBounds, Utils::xyz(tri.v2));
                m_maxBounds = max(m_maxBounds, Utils::xyz(tri.v3));
    
                m_indicies.push_back((unsigned int)objPoses.size());
                objPoses.push_back(tri.v1);
                objNormals.push_back(tri.n1);
                
                m_indicies.push_back((unsigned int)objPoses.size());
                objPoses.push_back(tri.v2);
                objNormals.push_back(tri.n2);
                
                m_indicies.push_back((unsigned int)objPoses.size());
                objPoses.push_back(tri.v3);
                objNormals.push_back(tri.n3);

                m_triangles.push_back(tri);
            }
        }
    }
    vec3 maxAbsBounds = max(abs(m_minBounds), abs(m_maxBounds));
    m_minBounds = -maxAbsBounds;
    m_maxBounds = maxAbsBounds;

    ShaderProgram::genDataSSBO<vec4>(m_posBuffer, (int)objPoses.size(), objPoses.data(), GL_STATIC_DRAW);
    ShaderProgram::genDataSSBO<vec4>(m_normalBuffer, (int)objNormals.size(), objNormals.data(), GL_STATIC_DRAW);
    ShaderProgram::genDataSSBO<Triangle>(m_triangleBuffer, (int)m_triangles.size(), m_triangles.data(), GL_STATIC_DRAW);    

    Logger::logSuccess("OBJ file successfuly loaded.", __LOG_DATA__);
}

void Mesh::constructSDF(float dx) {
    float s = glm::max(m_scale.x, glm::max(m_scale.y, m_scale.z));
    dx /= glm::max(s, 1e-10f);

    vec3 volume = 2.0f * m_maxBounds;
    m_N = ivec3(floor(volume / dx)) + ivec3(20);
    volume = vec3(m_N) * dx;

    m_maxBounds = volume * 0.5f;
    m_minBounds = -volume * 0.5f;

    ShaderProgram::genSSBO<float>(m_sdfBuffer, m_N.x * m_N.y * m_N.z, 0.0f, GL_STATIC_DRAW);
    ShaderProgram::genSSBO<vec4>(m_sdfGradBuffer, m_N.x * m_N.y * m_N.z, vec4(0.0f), GL_STATIC_DRAW);

    ShaderProgram::SSBOBarrier();

    m_constructSdfShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_sdfBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_triangleBuffer);
    glUniform3i(ShaderProgram::getVarLoc("N"), m_N.x, m_N.y, m_N.z);
    glUniform1i(ShaderProgram::getVarLoc("numTriangles"), (int)m_triangles.size());
    glUniform1f(ShaderProgram::getVarLoc("dx"), dx);
    m_constructSdfShader.dispatch((m_N + 7) / 8);

    ShaderProgram::SSBOBarrier();

    m_computeGradShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_sdfBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_sdfGradBuffer);
    glUniform3i(ShaderProgram::getVarLoc("N"), m_N.x, m_N.y, m_N.z);
    glUniform1f(ShaderProgram::getVarLoc("h"), dx);
    m_computeGradShader.dispatch((m_N + 7) / 8);

    ShaderProgram::SSBOBarrier();
}

Bounds Mesh::getSDFBounds() const { 
    mat4 model = getModelMatrix();
    return (Bounds){ 
        model * vec4(m_minBounds, 1.0f), 
        model * vec4(m_maxBounds, 1.0f) 
    }; 
}

mat4 Mesh::getModelMatrix() const {
    mat4 result = mat4(1.0f);
    result = translate(result, m_position);
    result = scale(result, m_scale);
    return result;
}