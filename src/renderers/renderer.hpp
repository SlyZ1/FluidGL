#ifndef RENDERER_TEMPLATE_HPP
#define RENDERER_TEMPLATE_HPP

#include "helpers/stats.hpp"

class IRenderer : public IStatsProvider {
protected:
    static constexpr std::array<float, 12> s_quadVerts = {
        1.f,  1.f, 0.f,
        1.f, -1.f, 0.f,
        -1.f, -1.f, 0.f,
        -1.f,  1.f, 0.f,
    };
    static constexpr std::array<unsigned int, 6> s_quadIndices = {
        0, 1, 2,
        0, 2, 3
    };

    static constexpr std::array<float, 24> s_cubeVerts = {
        -1.f, -1.f, -1.f,
        1.f, -1.f, -1.f,
        1.f,  1.f, -1.f,
        -1.f,  1.f, -1.f,
        -1.f, -1.f,  1.f,
        1.f, -1.f,  1.f,
        1.f,  1.f,  1.f,
        -1.f,  1.f,  1.f,
    };

    static constexpr std::array<unsigned int, 36> s_cubeIndices = {
        4, 5, 6,  4, 6, 7,
        1, 0, 3,  1, 3, 2,
        5, 1, 2,  5, 2, 6,
        0, 4, 7,  0, 7, 3,
        3, 7, 6,  3, 6, 2,
        0, 1, 5,  0, 5, 4,
    };
public:
    IRenderer(std::string statsName) : IStatsProvider(statsName) {}
    virtual ~IRenderer() = default;
    virtual void render(bool clear = false) = 0;
    virtual void reload() = 0;
};

#endif