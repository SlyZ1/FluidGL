#ifndef METRICS_HPP
#define METRICS_HPP

#include <glad/glad.h>
#include <string>

using CounterValue = int;
using TimerValue = float;
using StorageValue = int;

namespace Metrics {
    inline std::string storageSuffix(StorageValue& v){
        static std::string m_storageSuffixes[5] = {"B", "KB", "MB", "GB", "TB"};
        std::string result = m_storageSuffixes[0];
        for (int i = 1; i < 5; i++)
        {
            if (v <= 1e3) break;
            v /= (StorageValue)1e3;
            result = m_storageSuffixes[i];
        }
        return result;
    }
};

class FPSCounter {
public:
    FPSCounter(){};
    FPSCounter(float updateInterval) : m_updateInterval(updateInterval) {};
    CounterValue get() const { return m_fps; }
    void update();
private:
    const float m_updateInterval = 0.25f;
    double m_lastTime = 0.0f;
    double m_updateTimer = 0.0f;
    int m_frameCount = 0;
    CounterValue m_fps = 0;
};

class CPUTimer {
public:
    CPUTimer(){};
    CPUTimer(float updateInterval) : m_updateInterval(updateInterval) {};
    TimerValue get() const { return m_time; }
    void begin();
    void end();
private:
    const float m_updateInterval = 0.25f;
    double m_lastTime = 0.0f;
    double m_updateTimer = 0.0f;
    int m_frameCount = 0;
    TimerValue m_time = 0;
};

class GPUTimer {
private:
    static const int NUM_QUERIES = 4;
    GLuint queries[NUM_QUERIES];
    bool inFlight[NUM_QUERIES] = {false, false, false, false};
    int writeIndex = 0;

    const float m_updateInterval = 0.1f;
    double m_updateTimer = 0.0f;
    int m_frameCount = 0;
    TimerValue m_time = 0;

public:
    GPUTimer(){};
    GPUTimer(float updateInterval) : m_updateInterval(updateInterval) {};
    void init(){ glGenQueries(NUM_QUERIES, queries); }
    void beginFrame();
    void endFrame();
    TimerValue getLastResultMs() const { return m_time; }
};

#endif