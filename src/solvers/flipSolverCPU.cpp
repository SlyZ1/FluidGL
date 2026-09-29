#include "flipSolverCPU.hpp"

#include <omp.h>
#include <iostream>

#include "helpers/utils.hpp"

using namespace std;
using namespace glm;

void FlipSolverCPU::genBuffers(){
    m_isWall = vector<bool>(m_config.getGridX() * m_config.getGridY(), false);
    m_velX = vector<float>((m_config.getGridX() + 1) * m_config.getGridY(), 0.f);
    m_velY = vector<float>(m_config.getGridX() * (m_config.getGridY() + 1), 0.f);
    m_rX = vector<float>((m_config.getGridX() + 1) * m_config.getGridY(), 0.f);
    m_rY = vector<float>(m_config.getGridX() * (m_config.getGridY() + 1), 0.f);
    m_partVel = vector<vec4>(m_config.getPartN(), vec4(0.f));
    m_partPos = vector<vec4>(m_config.getPartN(), vec4(0.f));
    int a = (int)glm::floor(sqrt(m_config.getPartN()));
    for (int i = 0; i < m_config.getPartN(); i++)
    {
        float x = (float)(i % a);
        int y = (int)((i - x) / a);
        x += (y % 2) * 0.5f;
        m_partPos[i] = (vec4(x-10, y, 0, 0) - vec4(a*0.5f, a*0.5f, 0, 0)) * 2.f * m_config.getPartRadius() * 1.f;
    }

    glDeleteBuffers(1, &m_posVBO); glDeleteBuffers(1, &m_velVBO);
    glGenBuffers(1, &m_posVBO); glGenBuffers(1, &m_velVBO);
}

FlipSolverCPU::FlipSolverCPU(FlipSolverCPUConfig config) 
: IParticleSolver(make_unique<FlipSolverCPUConfig>(move(config)), "FLIP CPU Solver"), m_config(static_cast<FlipSolverCPUConfig&>(*m_baseConfig)) {
    genBuffers();
}

ivec2 FlipSolverCPU::cellToCoord(int cell, int nx){
    int column = cell % nx;
    int row = (cell - column) / nx;
    return ivec2(column, row);
}

int FlipSolverCPU::coordToCell(ivec2 coord, int nx, int ny){
    return glm::clamp(coord.x % nx + nx * coord.y, 0, nx * ny - 1);
}

int FlipSolverCPU::posToCell(vec2 pos, int nx, int ny){
    int column = (int)glm::floor(pos.x / m_config.getH() + (nx % 2) * 0.5f);
    int row = (int)glm::floor(pos.y / m_config.getH() + (ny % 2) * 0.5f);
    column += (int)glm::floor(nx * 0.5f);
    row += (int)glm::floor(ny * 0.5f);
    return coordToCell(ivec2(column, row), nx, ny);
}

vec2 FlipSolverCPU::cellToPos(int cell, int nx, int ny){
    ivec2 coord = cellToCoord(cell, nx);
    vec2 pos = ((vec2)coord - vec2(nx - 1, ny - 1) * 0.5f) * m_config.getH();
    return pos;
}

vec2 FlipSolverCPU::coordToPos(ivec2 coord, int nx, int ny){
    vec2 pos = ((vec2)coord - vec2(nx - 1, ny - 1) * 0.5f) * m_config.getH();
    return pos;
}

void FlipSolverCPU::posToCoordAndDp(vec2 pos, int nx, int ny, ivec2& coord, vec2& dp){
    vec2 g = pos / m_config.getH() + vec2((nx % 2) * 0.5f, (ny % 2) * 0.5f);
    vec2 flooredG = glm::floor(g);
    coord = ivec2(flooredG) + ivec2((int)glm::floor(nx * 0.5f), (int)glm::floor(ny * 0.5f));
    dp = g - flooredG;
}

void FlipSolverCPU::integrateParticles(){
    #pragma omp parallel for
    for (int i = 0; i < m_config.getPartN(); i++)
    {
        m_partVel[i] += vec4(0.f, -9.81f, 0, 0) * m_config.getDt();
        m_partPos[i] += m_partVel[i] * m_config.getDt();
    }
}

void FlipSolverCPU::pushAppartParticles(){
    int numCells = m_config.getGridX() * m_config.getGridY();

    vector<int> numCellParticles(numCells, 0);
    vector<int> cellOf(m_config.getPartN());

    for (int i = 0; i < m_config.getPartN(); i++)
    {
        int cell = posToCell(m_partPos[i], m_config.getGridX(), m_config.getGridY());
        cellOf[i] = cell;
        numCellParticles[cell]++;
    }

    vector<int> cellEnd(numCells, 0);
    vector<int> firstCellParticle(numCells + 1, 0);
    int first = 0;
    for (int i = 0; i < numCells; i++)
    {
        first += numCellParticles[i];
        firstCellParticle[i] = first;
        cellEnd[i] = first;
    }
    firstCellParticle[numCells] = first;

    vector<int> cellParticleIds(m_config.getPartN());
    for (int i = 0; i < m_config.getPartN(); i++)
    {
        int cell = cellOf[i];
        firstCellParticle[cell]--;
        cellParticleIds[firstCellParticle[cell]] = i;
    }

    vector<vec2> corrections(m_config.getPartN(), vec2(0.f));
    vector<int> numCorrections(m_config.getPartN(), 0);

    const int numIters = 2;
    const float minDist = 2.0f * m_config.getPartRadius();
    const float minDist2 = minDist * minDist;

    #pragma omp parallel
    {
        for (int iter = 0; iter < numIters; iter++)
        {
            #pragma omp for
            for (int i = 0; i < m_config.getPartN(); i++)
            {
                corrections[i] = vec2(0.f);
                numCorrections[i] = 0;
            }

            #pragma omp for
            for (int i = 0; i < m_config.getPartN(); i++)
            {
                int cell = cellOf[i];
                ivec2 coord = cellToCoord(cell, m_config.getGridX());

                for (int x = -1; x <= 1; x++)
                {
                    for (int y = -1; y <= 1; y++)
                    {
                        int cx = coord.x + x;
                        int cy = coord.y + y;
                        if (cx < 0 || cx >= m_config.getGridX() || cy < 0 || cy >= m_config.getGridY()) continue;

                        int newCell = coordToCell(ivec2(cx, cy), m_config.getGridX(), m_config.getGridY());
                        int rangeStart = firstCellParticle[newCell];
                        int rangeEnd = cellEnd[newCell];

                        for (int k = rangeStart; k < rangeEnd; k++)
                        {
                            int j = cellParticleIds[k];
                            if (j == i) continue;

                            vec2 diff = m_partPos[i] - m_partPos[j];
                            float dist2 = length2(diff);
                            if (dist2 < minDist2 && dist2 > 0.f)
                            {
                                float dist = sqrt(dist2);
                                vec2 push = 0.7f * 0.5f * diff * (minDist - dist) / dist;
                                corrections[i] += push;
                                numCorrections[i]++;
                            }
                        }
                    }
                }
            }

            #pragma omp for
            for (int i = 0; i < m_config.getPartN(); i++)
                if (numCorrections[i] > 0)
                    m_partPos[i] += vec4(corrections[i], 0, 0) / (float)numCorrections[i];
        }
    }
}

void FlipSolverCPU::particleCollisions(){
    vec2 minPos = cellToPos(0, m_config.getGridX(), m_config.getGridY());
    vec2 maxPos = cellToPos(m_config.getGridX() * m_config.getGridY() - 1, m_config.getGridX(), m_config.getGridY());
    #pragma omp parallel for
    for (int i = 0; i < m_config.getPartN(); i++)
    {
        vec2 pos = m_partPos[i];

        vec2 diff = pos - m_obstaclePos;
        float dist2 = glm::length2(diff);
        if (dist2 < (m_config.getPartRadius() + m_obstacleRadius) * (m_config.getPartRadius() + m_obstacleRadius)){
            float d = sqrt(dist2);
            vec4 dir = vec4(diff / d, 0, 0);
            m_partPos[i] += (m_config.getPartRadius() + m_obstacleRadius - d) * dir;
            m_partVel[i] = m_partVel[i] - glm::max(0.f, dot(m_partVel[i], dir)) * dir + vec4(m_obstacleVel, 0, 0);
        }

        if (pos.x < minPos.x + 1e-2f){
            m_partPos[i].x = minPos.x + 1e-2f;
            m_partVel[i].x = - m_partVel[i].x * 0.2f;
        }
        else if (pos.x > maxPos.x - 1e-2f){
            m_partPos[i].x = maxPos.x - 1e-2f;
            m_partVel[i].x = 0.f;
        }
        if (pos.y < minPos.y + 1e-2f){
            m_partPos[i].y = minPos.y + 1e-2f;
            m_partVel[i].y = 0.f;
        }
        else if (pos.y > maxPos.y - 1e-2f){
            m_partPos[i].y = maxPos.y - 1e-2f;
            m_partVel[i].y = 0.f;
        }
    }
}

void FlipSolverCPU::particlesToGrid(){
    m_isAir.clear();
    m_isAir = vector<bool>(m_config.getGridX() * m_config.getGridY(), true);
    m_rY.clear();
    m_rX.clear();
    m_velY.clear();
    m_velX.clear();
    m_rY = vector<float>(m_config.getGridX() * (m_config.getGridY() + 1), 0.f);
    m_rX = vector<float>((m_config.getGridX() + 1) * m_config.getGridY(), 0.f);
    m_velY = vector<float>(m_config.getGridX() * (m_config.getGridY() + 1), 0.f);
    m_velX = vector<float>((m_config.getGridX() + 1) * m_config.getGridY(), 0.f);
    for (int i = 0; i < m_config.getPartN(); i++)
    {
        vec2 currentPos = m_partPos[i];
        vec2 vel = m_partVel[i];
        m_isAir[posToCell(m_partPos[i], m_config.getGridX(), m_config.getGridY())] = false;

        {
            vec2 pos = currentPos;
            // ivec2 coord;
            // vec2 dp;
            // posToCoordAndDp(pos - vec2(h * 0.5f, 0.f), m_config.getGridX(), m_config.getGridY(), coord, dp);

            // int cell = posToCell(pos - vec2(h * 0.5f, 0.f), m_config.getGridX(), m_config.getGridY());
            // ivec2 coord = cellToCoord(cell, m_config.getGridX());
            // vec2 cellCenter = cellToPos(cell, m_config.getGridX(), m_config.getGridY()) - vec2(0.f, h * 0.5f);
            // vec2 dp = (pos - cellCenter) / h;
            int cell = posToCell(pos - vec2(m_config.getH() * 0.5f, 0.f), m_config.getGridX(), m_config.getGridY());
            ivec2 coord = cellToCoord(cell, m_config.getGridX());
            vec2 cellCenter = cellToPos(cell, m_config.getGridX(), m_config.getGridY()) - vec2(0.f, m_config.getH() * 0.5f);
            vec2 dp = (pos - cellCenter) / m_config.getH();

            float w1 = (1.f - dp.x) * (1.f - dp.y);
            float w2 = dp.x * (1.f - dp.y);
            float w3 = dp.x * dp.y;
            float w4 = (1.f - dp.x) * dp.y;
    
            int bottomLeft = coordToCell(coord + ivec2(0, 0), m_config.getGridX(), m_config.getGridY()+1);
            int bottomRight = coordToCell(coord + ivec2(1, 0), m_config.getGridX(), m_config.getGridY()+1);
            int topRight = coordToCell(coord + ivec2(1, 1), m_config.getGridX(), m_config.getGridY()+1);
            int topLeft = coordToCell(coord + ivec2(0, 1), m_config.getGridX(), m_config.getGridY()+1);
            
            m_velY[bottomLeft] += w1 * vel.y;
            m_velY[bottomRight] += w2 * vel.y;
            m_velY[topRight] += w3 * vel.y;
            m_velY[topLeft] += w4 * vel.y;
            m_rY[bottomLeft] += w1;
            m_rY[bottomRight] += w2;
            m_rY[topRight] += w3;
            m_rY[topLeft] += w4;
        }
        {
            vec2 pos = currentPos;
            // ivec2 coord;
            // vec2 dp;
            // posToCoordAndDp(pos, m_config.getGridX(), m_config.getGridY(), coord, dp);
            int cell = posToCell(pos - vec2(0.f, m_config.getH() * 0.5f), m_config.getGridX(), m_config.getGridY());
            ivec2 coord = cellToCoord(cell, m_config.getGridX());
            vec2 cellCenter = cellToPos(cell, m_config.getGridX(), m_config.getGridY()) - vec2(m_config.getH() * 0.5f, 0.f);
            vec2 dp = (pos - cellCenter) / m_config.getH();
            
            float w1 = (1.f - dp.x) * (1.f - dp.y);
            float w2 = dp.x * (1.f - dp.y);
            float w3 = dp.x * dp.y;
            float w4 = (1.f - dp.x) * dp.y;

            int bottomLeft = coordToCell(coord + ivec2(0, 0), m_config.getGridX()+1, m_config.getGridY());
            int bottomRight = coordToCell(coord + ivec2(1, 0), m_config.getGridX()+1, m_config.getGridY());
            int topRight = coordToCell(coord + ivec2(1, 1), m_config.getGridX()+1, m_config.getGridY());
            int topLeft = coordToCell(coord + ivec2(0, 1), m_config.getGridX()+1, m_config.getGridY());

            m_velX[bottomLeft] += w1 * vel.x;
            m_velX[bottomRight] += w2 * vel.x;
            m_velX[topRight] += w3 * vel.x;
            m_velX[topLeft] += w4 * vel.x;
            m_rX[bottomLeft] += w1;
            m_rX[bottomRight] += w2;
            m_rX[topRight] += w3;
            m_rX[topLeft] += w4;
        }
    }

    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < (m_config.getGridX() + 1) * m_config.getGridY(); i++)
        {
            float r = m_rX[i];
            if (r > 0) m_velX[i] /= r;
        }
        #pragma omp for
        for (int i = 0; i < m_config.getGridX() * (m_config.getGridY() + 1); i++)
        {
            float r = m_rY[i];
            if (r > 0) m_velY[i] /= r;
        }
    
        // #pragma omp for
        // for (int i = 0; i < m_config.getGridX(); i++)
        // {
        //     velY[i] = velY[i + m_config.getGridX()];
        //     velY[i + m_config.getGridX() * m_config.getGridY()] = velY[i + m_config.getGridX() * (m_config.getGridY() - 1)];
        // }
        // #pragma omp for
        // for (int j = 0; j < m_config.getGridY(); j++)
        // {
        //     velX[j * (m_config.getGridX() + 1)] = velX[1 + j * (m_config.getGridX() + 1)];
        //     velX[m_config.getGridX() + j * (m_config.getGridX() + 1)] = velX[m_config.getGridX() - 1 + j * (m_config.getGridX() + 1)];
        // }

        #pragma omp for
        for (int i = 0; i < m_config.getGridX(); i++)
        {
            m_velY[i] = 0.3f;
            m_velY[i + m_config.getGridX() * m_config.getGridY()] = -0.3f;
        }
        #pragma omp for
        for (int j = 0; j < m_config.getGridY(); j++)
        {
            m_velX[j * (m_config.getGridX() + 1)] = 0.3f;
            m_velX[m_config.getGridX() + j * (m_config.getGridX() + 1)] = -0.3f;
        }
    }
}

void FlipSolverCPU::solveIncompressibility(int iterations){
    m_oldVelX = m_velX;
    m_oldVelY = m_velY;
    vector<ivec2> redCoords = {};
    vector<ivec2> blackCoords = {};
    for (int i = 0; i < m_config.getGridX(); i++)
    {
        for (int j = 0; j < m_config.getGridY(); j++)
        {
            if (m_isAir[coordToCell(ivec2(i, j), m_config.getGridX(), m_config.getGridY())]) continue;
            if ((i + j) % 2 == 0) redCoords.push_back(ivec2(i, j));
            else blackCoords.push_back(ivec2(i, j));
        }
    }
    auto solveCell = [&](ivec2 coord){
        int i = coord.x; int j = coord.y;

        float s0 = (float)(j-1 >= 0);
        float s1 = (float)(j+1 < m_config.getGridY());
        float s2 = (float)(i-1 >= 0);
        float s3 = (float)(i+1 < m_config.getGridX());
        float s = s0 + s1 + s2 + s3;
        if (s <= 0) return;

        int bottom = coordToCell(ivec2(i, j), m_config.getGridX(), m_config.getGridY()+1);
        int top = coordToCell(ivec2(i, j+1), m_config.getGridX(), m_config.getGridY()+1);
        int left = coordToCell(ivec2(i, j), m_config.getGridX()+1, m_config.getGridY());
        int right = coordToCell(ivec2(i+1, j), m_config.getGridX()+1, m_config.getGridY());

        float d = s1 * m_velY[top] - s0 * m_velY[bottom];
        d += s3 * m_velX[right] - s2 * m_velX[left];
        d *= m_config.getOverrelaxation(); // overrelaxation
        
        float rho = s2 * m_rX[left] + s3 * m_rX[right];
        rho += s0 * m_rY[bottom] + s1 * m_rY[top];
        rho /= s;
        d -= glm::max(rho - m_config.getDensity(), 0.f); // drift compensation
        
        m_velY[bottom] += d * s0 / s;
        m_velY[top] -= d * s1 / s;
        m_velX[left] += d * s2 / s;
        m_velX[right] -= d * s3 / s;
    };
    #pragma omp parallel
    {
        for (int k = 0; k < iterations; k++)
        {
            #pragma omp for
            for (int idx = 0; idx < (int)redCoords.size(); idx++)
                solveCell(redCoords[idx]);

            #pragma omp for
            for (int idx = 0; idx < (int)blackCoords.size(); idx++)
                solveCell(blackCoords[idx]);
        }
    }
}

void FlipSolverCPU::gridToParticles(){
    #pragma omp parallel for
    for (int i = 0; i < m_config.getPartN(); i++)
    {
        vec2 currentPos = m_partPos[i];
        {
            vec2 pos = currentPos;
            // ivec2 coord;
            // vec2 dp;
            // posToCoordAndDp(pos, m_config.getGridX(), m_config.getGridY(), coord, dp);
            int cell = posToCell(pos - vec2(m_config.getH() * 0.5f, 0.f), m_config.getGridX(), m_config.getGridY());
            ivec2 coord = cellToCoord(cell, m_config.getGridX());
            vec2 cellCenter = cellToPos(cell, m_config.getGridX(), m_config.getGridY()) - vec2(0.f, m_config.getH() * 0.5f);
            vec2 dp = (pos - cellCenter) / m_config.getH();
            
            float s1 = !m_isAir[coordToCell(coord + ivec2(0,0), m_config.getGridX(), m_config.getGridY())] || (coord.y + 1 <= m_config.getGridY() && !m_isAir[coordToCell(coord + ivec2(0,1), m_config.getGridX(), m_config.getGridY())]);
            float s2 = !m_isAir[coordToCell(coord + ivec2(1,0), m_config.getGridX(), m_config.getGridY())] || (coord.y + 1 <= m_config.getGridY() && !m_isAir[coordToCell(coord + ivec2(1,1), m_config.getGridX(), m_config.getGridY())]);
            float s3 = !m_isAir[coordToCell(coord + ivec2(1,1), m_config.getGridX(), m_config.getGridY())] || (coord.y + 2 <= m_config.getGridY() && !m_isAir[coordToCell(coord + ivec2(1,2), m_config.getGridX(), m_config.getGridY())]);
            float s4 = !m_isAir[coordToCell(coord + ivec2(0,1), m_config.getGridX(), m_config.getGridY())] || (coord.y + 2 <= m_config.getGridY() && !m_isAir[coordToCell(coord + ivec2(0,2), m_config.getGridX(), m_config.getGridY())]);

            float w1 = s1 * (1.f - dp.x) * (1.f - dp.y);
            float w2 = s2 * dp.x * (1.f - dp.y);
            float w3 = s3 * dp.x * dp.y;
            float w4 = s4 * (1.f - dp.x) * dp.y;
            float sumW = w1 + w2 + w3 + w4;
            if (sumW <= 0) continue;
    
            int bottomLeft = coordToCell(coord + ivec2(0, 0), m_config.getGridX(), m_config.getGridY()+1);
            int bottomRight = coordToCell(coord + ivec2(1, 0), m_config.getGridX(), m_config.getGridY()+1);
            int topRight = coordToCell(coord + ivec2(1, 1), m_config.getGridX(), m_config.getGridY()+1);
            int topLeft = coordToCell(coord + ivec2(0, 1), m_config.getGridX(), m_config.getGridY()+1);
    
            float pic = m_velY[bottomLeft] * w1 + m_velY[bottomRight] * w2 + m_velY[topRight] * w3 + m_velY[topLeft] * w4;
            float flip = pic - (m_oldVelY[bottomLeft] * w1 + m_oldVelY[bottomRight] * w2 + m_oldVelY[topRight] * w3 + m_oldVelY[topLeft] * w4);
            pic /= sumW;
            flip /= sumW;
            m_partVel[i].y += flip;
            m_partVel[i].y = glm::mix(pic, m_partVel[i].y, 0.9f);
        }
        {
            vec2 pos = currentPos;
            // ivec2 coord;
            // vec2 dp;
            // posToCoordAndDp(pos, m_config.getGridX(), m_config.getGridY(), coord, dp);
            int cell = posToCell(pos - vec2(0.f, m_config.getH() * 0.5f), m_config.getGridX(), m_config.getGridY());
            ivec2 coord = cellToCoord(cell, m_config.getGridX());
            vec2 cellCenter = cellToPos(cell, m_config.getGridX(), m_config.getGridY()) - vec2(m_config.getH() * 0.5f, 0.f);
            vec2 dp = (pos - cellCenter) / m_config.getH();

            float s1 = !m_isAir[coordToCell(coord + ivec2(0,0), m_config.getGridX(), m_config.getGridY())] || (coord.x + 1 <= m_config.getGridX() && !m_isAir[coordToCell(coord + ivec2(1,0), m_config.getGridX(), m_config.getGridY())]);
            float s2 = !m_isAir[coordToCell(coord + ivec2(1,0), m_config.getGridX(), m_config.getGridY())] || (coord.x + 1 <= m_config.getGridX() && !m_isAir[coordToCell(coord + ivec2(2,0), m_config.getGridX(), m_config.getGridY())]);
            float s3 = !m_isAir[coordToCell(coord + ivec2(1,1), m_config.getGridX(), m_config.getGridY())] || (coord.x + 2 <= m_config.getGridX() && !m_isAir[coordToCell(coord + ivec2(2,1), m_config.getGridX(), m_config.getGridY())]);
            float s4 = !m_isAir[coordToCell(coord + ivec2(0,1), m_config.getGridX(), m_config.getGridY())] || (coord.x + 2 <= m_config.getGridX() && !m_isAir[coordToCell(coord + ivec2(1,1), m_config.getGridX(), m_config.getGridY())]);
            
            float w1 = s1 * (1.f - dp.x) * (1.f - dp.y);
            float w2 = s2 * dp.x * (1.f - dp.y);
            float w3 = s3 * dp.x * dp.y;
            float w4 = s4 * (1.f - dp.x) * dp.y;
            float sumW = w1 + w2 + w3 + w4;
            if (sumW <= 0) continue;
        
            int bottomLeft = coordToCell(coord + ivec2(0, 0), m_config.getGridX()+1, m_config.getGridY());
            int bottomRight = coordToCell(coord + ivec2(1, 0), m_config.getGridX()+1, m_config.getGridY());
            int topRight = coordToCell(coord + ivec2(1, 1), m_config.getGridX()+1, m_config.getGridY());
            int topLeft = coordToCell(coord + ivec2(0, 1), m_config.getGridX()+1, m_config.getGridY());
        
            float pic = m_velX[bottomLeft] * w1 + m_velX[bottomRight] * w2 + m_velX[topRight] * w3 + m_velX[topLeft] * w4;
            float flip = pic - (m_oldVelX[bottomLeft] * w1 + m_oldVelX[bottomRight] * w2 + m_oldVelX[topRight] * w3 + m_oldVelX[topLeft] * w4);
            pic /= sumW;
            flip /= sumW;
            m_partVel[i].x += flip;
            m_partVel[i].x = glm::mix(pic, m_partVel[i].x, 0.9f);
        }
    }
}


void FlipSolverCPU::update(){
    if (m_isPaused) return;
    
    integrateParticles();
    particleCollisions();
    pushAppartParticles();
    particleCollisions();
    
    particlesToGrid();
    solveIncompressibility(80);
    gridToParticles();
}

void FlipSolverCPU::reload(){
    m_config = static_cast<FlipSolverCPUConfig&>(*m_baseConfig);
    genBuffers();
}

GLuint FlipSolverCPU::getPosBuffer() const {
    if (m_config.getPartN() > 0) cout << "caccaca" << endl;
    glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
    glBufferData(GL_ARRAY_BUFFER, m_partPos.size() * sizeof(vec4), m_partPos.data(), GL_STREAM_DRAW);
    return m_posVBO;
}

GLuint FlipSolverCPU::getVelBuffer() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_velVBO);
    glBufferData(GL_ARRAY_BUFFER, m_partVel.size() * sizeof(vec4), m_partVel.data(), GL_STREAM_DRAW);
    return m_velVBO;
}

vector<vec4> FlipSolverCPU::getGrid(float width){
    vector<vec4> grid = {};
    for (int i = 0; i < m_config.getGridX(); i++)
    {
        vec2 pos = cellToPos(i, m_config.getGridX(), m_config.getGridY());
        grid.push_back(vec4(pos.x - m_config.getH() * 0.5f, 0.f, width, m_config.getGridY() * 0.5f * m_config.getH()));
    }
    for (int j = 0; j < m_config.getGridY(); j++)
    {
        vec2 pos = cellToPos(j * m_config.getGridX(), m_config.getGridX(), m_config.getGridY());
        grid.push_back(vec4(0.f, pos.y - m_config.getH() * 0.5f, m_config.getGridX() * 0.5f * m_config.getH(), width));
    }
    return grid;
}

vector<vec4> FlipSolverCPU::getCells(){
    vector<vec4> cells = vector<vec4>(m_config.getGridX() * m_config.getGridY(), vec4(0.f));
    #pragma omp parallel for
    for (int cell = 0; cell < m_config.getGridX() * m_config.getGridY(); cell++)
    {
        cells[cell] = vec4(cellToPos(cell, m_config.getGridX(), m_config.getGridY()), m_config.getH() * 0.5f, m_config.getH() * 0.5f);
    }
    return cells;
}

vector<vec4> FlipSolverCPU::getCellColors(){
    vector<vec4> colors = vector<vec4>(m_config.getGridX() * m_config.getGridY(), vec4(0,0,0,1));
    #pragma omp parallel for
    for (int x = 0; x < m_config.getGridX(); x++)
    {
        for (int y = 0; y < m_config.getGridY(); y++)
        {
            // int cellX = coordToCell(ivec2(x,y), m_config.getGridX()+1, m_config.getGridY());
            // float vx = velX[cellX];
            int cellY = coordToCell(ivec2(x,y), m_config.getGridX(), m_config.getGridY()+1);
            float vy = m_velY[cellY];
            colors[coordToCell(ivec2(x,y), m_config.getGridX(), m_config.getGridY())] = vec4(glm::max(0.f,vy) / 50, -glm::min(0.f,vy) / 50, 0.f, 1.f);
        }
    }
    // for (int i = 0; i < m_config.getPartN(); i++)
    // {
    //     int cell = posToCell(partPos[i], m_config.getGridX(), m_config.getGridY());
    //     colors[cell] += vec4(0.2f, 0.2f, 0.f, 0.f);
    // }
    
    return colors;
}

void FlipSolverCPU::updateObstacle(vec2 pos, vec2 vel, float rad){
    m_obstaclePos = pos;
    m_obstacleVel = vel;
    m_obstacleRadius = rad;
}