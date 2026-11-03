#include "shader_program.hpp"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>

#include "helpers/logger.hpp"

namespace fs = std::filesystem;

using namespace std;

ShaderProgram::ShaderProgram() : m_shaderProgram(0) { }
ShaderProgram::~ShaderProgram() { 
    if (m_shaderProgram > 0 && glfwGetCurrentContext() != nullptr && glIsProgram(m_shaderProgram)){
        glDeleteProgram(m_shaderProgram);
        s_uniformCache.erase(m_shaderProgram);
    }
}
GLuint ShaderProgram::s_currentlyUsedProgram = 0;
unordered_map<GLuint, unordered_map<string, GLuint>> ShaderProgram::s_uniformCache = {};

void ShaderProgram::create(){
    m_shaderProgram = glCreateProgram();
    s_uniformCache[m_shaderProgram] = unordered_map<string, GLuint>();
}

// Extracts path from lines like : #pragma include "path"
fs::path ShaderProgram::extractPath(const string& line){
    int start = (int)line.find_first_of("\"") + 1;
    int end = (int)line.find_last_of("\"");
    return fs::path(line.substr(start, end - start));
}

string ShaderProgram::getShaderSource(const string& path){
    ifstream file(path);
    stringstream ss;
    string line; 

    if (!file.is_open()) {
        Logger::logError("Cannot open file at " + path, __LOG_DATA__);
        return "";
    }

    bool isForwardDeclaration = false;

    while (getline(file, line)) { 
        // Includes
        if (line.find("#pragma include") != std::string::npos) {
            fs::path includePath = fs::path(path).parent_path() / extractPath(line);
            ss << getShaderSource(includePath.string().c_str());
        // Forward declarations
        } else if (line == "#pragma FDECLARE") {
            isForwardDeclaration = true;
        } else if (line == "#pragma FEND") {
            isForwardDeclaration = false;
        // Normal lines
        } else {
            if (!isForwardDeclaration) ss << line << "\n";
        }
    }

    return ss.str();
}

void ShaderProgram::load(int type, const string& path){
    m_types.push_back(type);
    m_paths.push_back(path);

    filesystem::path p(path);
    m_name = p.stem().string();

    //Create shader
    unsigned int shader;
    shader = glCreateShader(type);
    m_shaders.push_back(shader);
    
    //Compile shader
    string shaderSourceString = getShaderSource(path); 
    const char *shaderSource = shaderSourceString.c_str();
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    //Check for errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        Logger::logError("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + string(infoLog) + "\nat " + m_name + ".glsl", __LOG_DATA__);
    }

    //Add to program
    glAttachShader(m_shaderProgram, shader);
    Logger::logInfo("Compiling " + m_name + ".glsl...", __LOG_DATA__);
}

void ShaderProgram::reload(){
    destroy();

    create();
    for (size_t i = 0; i < m_paths.size(); i++)
    {
        //Create shader
        unsigned int shader;
        shader = glCreateShader(m_types[i]);
        m_shaders.push_back(shader);
        
        //Compile shader
        string shaderSourceString = getShaderSource(m_paths[i]); 
        const char *shaderSource = shaderSourceString.c_str();
        glShaderSource(shader, 1, &shaderSource, NULL);
        glCompileShader(shader);

        //Check for errors
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            Logger::logError("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + string(infoLog) + "\nat " + m_name + ".glsl", __LOG_DATA__);
        }

        //Add to program
        glAttachShader(m_shaderProgram, shader);
    }
    link();
}

void ShaderProgram::link(){
    //Link
    glLinkProgram(m_shaderProgram);

    //Check for errors
    int success;
    char infoLog[512];
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
        Logger::logError("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + string(infoLog) + "\nat " + m_name + ".glsl", __LOG_DATA__);
    }
    
    //Delete shaders
    for(const int shader : m_shaders){
        glDeleteShader(shader);
    }
    m_shaders.clear();
}

void ShaderProgram::use() const {
    glUseProgram(m_shaderProgram);
    s_currentlyUsedProgram = m_shaderProgram;
}

void ShaderProgram::dispatch(GLuint x, GLuint y, GLuint z){
    glDispatchCompute(x, y, z);
}

void ShaderProgram::dispatch(glm::ivec3 xyz){
    glDispatchCompute(xyz.x, xyz.y, xyz.z);
}

void ShaderProgram::SSBOBarrier(){
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ShaderProgram::indirectBarrier(){
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void ShaderProgram::indirectDispatch(GLuint buffer, int offset){
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer);
    glDispatchComputeIndirect(offset);
}

void ShaderProgram::loadCompute(ShaderProgram& prog, const std::string& relativePath){
    prog.create();
    prog.load(GL_COMPUTE_SHADER, relativePath);
    prog.link();
}

GLuint ShaderProgram::getVarLoc(const string& name){
    auto& progUniformCache = s_uniformCache[s_currentlyUsedProgram];
    auto it = progUniformCache.find(name);
    if (it != progUniformCache.end()){
        return it->second;
    }
    GLuint loc = glGetUniformLocation(s_currentlyUsedProgram, name.c_str());
    progUniformCache[name] = loc;
    return loc;
}

void ShaderProgram::destroy() {
    if (m_shaderProgram == 0) return;
    glDeleteProgram(m_shaderProgram);
    s_uniformCache.erase(m_shaderProgram);
    m_shaderProgram = 0;
}

unsigned int ShaderProgram::id() const {
    return m_shaderProgram;
}