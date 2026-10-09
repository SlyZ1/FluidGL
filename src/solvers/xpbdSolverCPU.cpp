#include "xpdbSolverCPU.hpp"

using namespace std;
using namespace glm;

void XpbdSolverCPU::genBuffers(){
    glDeleteBuffers(1, &m_posBuffer); glDeleteBuffers(1, &m_velBuffer);
    glGenBuffers(1, &m_posBuffer); glGenBuffers(1, &m_velBuffer);
}

XpbdSolverCPU::XpbdSolverCPU(XpbdSolverCPUConfig config) 
: ISolver(make_unique<XpbdSolverCPUConfig>(move(config)), "FLIP GPU Solver"), m_config(static_cast<XpbdSolverCPUConfig&>(*m_baseConfig)) {
    genBuffers();
}

XpbdSolverCPU::~XpbdSolverCPU() {
    glDeleteBuffers(1, &m_posBuffer); glDeleteBuffers(1, &m_velBuffer);
}

void XpbdSolverCPU::integrate() {
    for (int i = 0; i < m_config.getNumVertices(); i++)
    {
        m_vertVel[i] += m_config.getGravity() * m_config.getDt();
        m_vertPos[i] += m_vertVel[i] * m_config.getDt();
    }
}

shared_ptr<GLuint> XpbdSolverCPU::getPosBuffer() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertPos.size() * sizeof(vec4), m_vertPos.data(), GL_STREAM_DRAW);
    return make_shared<GLuint>(m_posBuffer);
}

shared_ptr<GLuint> XpbdSolverCPU::getVelBuffer() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_velBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertVel.size() * sizeof(vec4), m_vertVel.data(), GL_STREAM_DRAW);
    return make_shared<GLuint>(m_velBuffer);
}

void XpbdSolverCPU::update() {
    genBuffers();
}

void XpbdSolverCPU::reload() {


    ISolver::reload();
}
