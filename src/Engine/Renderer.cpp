#include "Renderer.h"
#include <algorithm>

namespace OpenXcom
{

std::vector<std::pair<std::string, std::string>> upscalers;

/// Register an upscaler
void registerUpscaler(std::string rendererName, std::string upscalerName)
{
    std::pair<std::string, std::string> upscalerPair = std::make_pair(rendererName, upscalerName);
    if (std::find(upscalers.begin(), upscalers.end(), upscalerPair) == upscalers.end())
    {
        upscalers.push_back(upscalerPair);
    }
}

/// Get all available upscalers
std::vector<std::pair<std::string, std::string>> getRegisteredUpscalers()
{
    return upscalers;
}

}