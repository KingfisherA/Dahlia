#include <algorithm>
#include <stdexcept>

#include "Dahlia/Graphics/BindGroup.h"
#include "Dahlia/Graphics/ShaderModule.h"
#include "Dahlia/Utilities/Assert.h"

#include "Dahlia/Graphics/GraphicsPipeline.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLGraphicsPipeline.h"
#ifdef DH_VULKAN_ENABLED
#include "Dahlia/Platform/Vulkan/VulkanGraphicsPipeline.h"
#endif
#ifdef DH_METAL_ENABLED
#include "Dahlia/Platform/Metal/MetalGraphicsPipeline.h"
#endif

namespace DH
{
    GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineDesc& desc)
      : mDesc(desc)
    {
        if (!mDesc.vertexShader || !mDesc.fragmentShader)
            throw std::invalid_argument("A graphics pipeline needs a vertex and fragment shader");
        Validate();
    }

    void GraphicsPipeline::Validate() const
    {
        if (mDesc.pushConstantSize > MAX_PUSH_CONSTANT_SIZE)
            throw std::invalid_argument("pushConstantSize exceeds MAX_PUSH_CONSTANT_SIZE");
        if (mDesc.bindGroupLayouts.size() > MAX_BIND_GROUPS)
            throw std::invalid_argument("Too many bind group layouts");
        if (mDesc.blend.size() > 1 &&
            mDesc.blend.size() != mDesc.targetLayout.colourFormats.size())
            throw std::invalid_argument(
              "GraphicsPipelineDesc::blend must have 0, 1 or one entry per colour attachment in "
              "targetLayout");

        for (const sRef<ShaderModule>& module : { mDesc.vertexShader, mDesc.fragmentShader })
        {
            const ShaderReflection& reflection = module->GetReflection();

            if (reflection.pushConstantSize > mDesc.pushConstantSize)
            {
                DH_WARN(DH::GraphicsPipeline,
                        "Shader push constant block (" << reflection.pushConstantSize
                                                       << " bytes) is larger than the pipeline's "
                                                          "pushConstantSize ("
                                                       << mDesc.pushConstantSize << " bytes)");
            }

            for (const ShaderResource& resource : reflection.resources)
            {
                const BindGroupLayout* layout =
                  resource.set < mDesc.bindGroupLayouts.size()
                    ? mDesc.bindGroupLayouts[resource.set].get()
                    : nullptr;

                const bool declared =
                  layout != nullptr &&
                  std::any_of(layout->GetEntries().begin(),
                              layout->GetEntries().end(),
                              [&resource](const BindGroupLayoutEntry& entry)
                              { return entry.binding == resource.binding && entry.type == resource.type; });

                if (!declared)
                {
                    DH_WARN(DH::GraphicsPipeline,
                            "Shader resource '" << resource.name << "' (set " << resource.set
                                                << ", binding " << resource.binding
                                                << ") is not declared in the pipeline's bind group "
                                                   "layouts");
                }
            }
        }
    }

    sRef<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineDesc& desc)
    {
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
                return make<OpenGLGraphicsPipeline>(desc);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanGraphicsPipeline>(desc);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return make<MetalGraphicsPipeline>(desc);
#endif
            default:
                throw std::runtime_error(
                  "GraphicsPipeline is not implemented for this graphics API yet");
        }
    }
}
