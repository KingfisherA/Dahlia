// MetalContext.cpp

#include "MetalContext.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace DH
{
    //
    MetalGPUContext::MetalGPUContext()
    {
        mtlDevice = MTL::CreateSystemDefaultDevice();
    }
    //
    void MetalGPUContext::Clear(const glm::vec4& clearColour) {}
    //
    void MetalGPUContext::DestroyContext()
    {
        mtlDevice->release();
    }
    //
} // namespace DH