#include <stdexcept>

#include "Dahlia/Graphics/BindGroup.h"
#include "Dahlia/Platform/GPUContext.h"
#ifdef DH_VULKAN_ENABLED
#include "Dahlia/Platform/Vulkan/VulkanBindGroup.h"
#endif

namespace DH
{
    namespace
    {
        // OpenGL and Metal need no API objects for bind groups: the command list reads the
        // entries at draw time and binds each resource to the slot/argument index the pipeline
        // or shader module assigned to its (set, binding).
        class GenericBindGroupLayout final : public BindGroupLayout
        {
          public:
            explicit GenericBindGroupLayout(const std::vector<BindGroupLayoutEntry>& entries)
              : BindGroupLayout(entries)
            {
            }
        };

        class GenericBindGroup final : public BindGroup
        {
          public:
            GenericBindGroup(const sRef<BindGroupLayout>& layout,
                            const std::vector<BindGroupEntry>& entries)
              : BindGroup(layout, entries)
            {
            }
        };
    }

    BindGroup::BindGroup(const sRef<BindGroupLayout>& layout,
                         const std::vector<BindGroupEntry>& entries)
      : mLayout(layout)
      , mEntries(entries)
    {
        if (!mLayout)
            throw std::invalid_argument("A BindGroup needs a layout");

        for (const BindGroupEntry& entry : mEntries)
        {
            const BindGroupLayoutEntry* layoutEntry = nullptr;
            for (const BindGroupLayoutEntry& candidate : mLayout->GetEntries())
            {
                if (candidate.binding == entry.binding)
                    layoutEntry = &candidate;
            }

            if (layoutEntry == nullptr)
                throw std::invalid_argument("BindGroup entry binding " +
                                            std::to_string(entry.binding) +
                                            " is not in the layout");

            const bool isBuffer = layoutEntry->type == ShaderResourceType::UniformBuffer;
            if (isBuffer ? entry.buffer == nullptr : (entry.texture == nullptr || entry.sampler == nullptr))
                throw std::invalid_argument("BindGroup entry binding " +
                                            std::to_string(entry.binding) +
                                            " does not provide the resource its layout requires");
        }
    }

    sRef<BindGroupLayout> BindGroupLayout::Create(const std::vector<BindGroupLayoutEntry>& entries)
    {
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
            case GraphicsAPI::Metal:
                return make<GenericBindGroupLayout>(entries);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanBindGroupLayout>(entries);
#endif
            default:
                throw std::runtime_error(
                  "BindGroupLayout is not implemented for this graphics API yet");
        }
    }

    sRef<BindGroup> BindGroup::Create(const sRef<BindGroupLayout>& layout,
                                      const std::vector<BindGroupEntry>& entries)
    {
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
            case GraphicsAPI::Metal:
                return make<GenericBindGroup>(layout, entries);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanBindGroup>(layout, entries);
#endif
            default:
                throw std::runtime_error("BindGroup is not implemented for this graphics API yet");
        }
    }
}
