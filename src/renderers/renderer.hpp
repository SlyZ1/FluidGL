#ifndef RENDERER_TEMPLATE_HPP
#define RENDERER_TEMPLATE_HPP

#include "helpers/stats.hpp"

class IRenderer : public IStatsProvider {
public:
    IRenderer(std::string statsName) : IStatsProvider(statsName) {}
    virtual ~IRenderer() = default;
    virtual void render(bool clear = false) = 0;
    virtual void reload() = 0;
};

#endif