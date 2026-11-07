#ifndef SHADER_PROG_HPP
#define SHADER_PROG_HPP

#include <iostream>
#include <vector>
#include <filesystem>
#include <glad/glad.h>
#include <unordered_map>
#include <glm/glm.hpp>

class ShaderProgram {
    private:
        static GLuint s_currentlyUsedProgram; 
        GLuint m_shaderProgram = 0;
        std::vector<GLuint> m_shaders = {};
        std::vector<std::string> m_paths = {};
        std::vector<int> m_types = {};
        std::string m_name = "";
        
        static std::unordered_map<GLuint, std::unordered_map<std::string, GLuint>> s_uniformCache;
        static std::filesystem::path extractPath(const std::string& line);
        static std::string getShaderSource(const std::string& path);

    public:
        ShaderProgram();
        ~ShaderProgram();
        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;

        GLuint id() const;
        void create();
        void load(int type, const std::string& path);
        void reload();
        void link();
        void use() const;
        static void dispatch(GLuint x = 1, GLuint y = 1, GLuint z = 1);
        static void dispatch(glm::ivec3 xyz);
        static void indirectBarrier();
        static void indirectDispatch(GLuint buffer, int offset = 0);
        void destroy();
        static void loadCompute(ShaderProgram& prog, const std::string& relativePath);
        static GLuint getVarLoc(const std::string& name);
        static void SSBOBarrier();

        template<typename T>
        static void genSSBO(GLuint& buffer, int size, T value, GLenum usage = GL_DYNAMIC_DRAW){
            genDataSSBO(buffer, size, std::vector<T>(size, value).data(), usage);
        }

        template<typename T>
        static void genDataSSBO(GLuint& buffer, int size, const T* data, GLenum usage = GL_DYNAMIC_DRAW){
            deleteBuffer(buffer);
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(T), data, usage);
        }

        static void deleteBuffer(GLuint& buffer) {
            glDeleteBuffers(1, &buffer);
            buffer = 0;
        }

        template<typename T>
        static std::tuple<GLuint, GLuint, GLuint> 
        addData(const std::vector<T>& data, const std::vector<GLuint>& indices){
            GLuint VBObj, VAObj, EBObj;
            
            glGenVertexArrays(1, &VAObj);
            glBindVertexArray(VAObj);
            
            glGenBuffers(1, &VBObj);
            glBindBuffer(GL_ARRAY_BUFFER, VBObj);
            glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
            
            glGenBuffers(1, &EBObj);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBObj);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

            return {VBObj, VAObj, EBObj};
        }

        static void linkData(int numCoords, int typesize, int layout, int glType = GL_FLOAT, int normalize = GL_FALSE){
            glVertexAttribPointer(layout, numCoords, glType, (GLboolean)normalize, numCoords * typesize, (void*)0);
            glEnableVertexAttribArray(layout);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
};

#endif