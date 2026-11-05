#pragma once

#include <cstdint>
#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsTypes.h"
#include "Dahlia/Graphics/ShaderModule.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    class Buffer;
    class Texture;
    class Sampler;

    // A bind group is a set of shader resources bound together, matching one
    // `layout(set = N, binding = ...)` set in the shader.
    //   OpenGL: a list of uniform block / texture unit bindings applied at draw time
    //   Vulkan: VkDescriptorSetLayout + VkDescriptorSet
    //   Metal:  buffer/texture/sampler argument indices set on the render encoder
    struct DH_API BindGroupLayoutEntry
    {
        uint32_t binding = 0;
        ShaderResourceType type = ShaderResourceType::UniformBuffer;
        ShaderStage stages = ShaderStage::AllGraphics;
    };

    class DH_API BindGroupLayout
    {
      public:
        virtual ~BindGroupLayout() = default;

        [[nodiscard]] const std::vector<BindGroupLayoutEntry>& GetEntries() const { return mEntries; }

        static sRef<BindGroupLayout> Create(const std::vector<BindGroupLayoutEntry>& entries);

      protected:
        explicit BindGroupLayout(const std::vector<BindGroupLayoutEntry>& entries)
          : mEntries(entries)
        {
        }

        std::vector<BindGroupLayoutEntry> mEntries;
    };

    struct DH_API BindGroupEntry
    {
        uint32_t binding = 0;

        // UniformBuffer
        sRef<Buffer> buffer;
        size_t offset = 0;
        size_t size = 0; // 0 means the whole buffer from offset

        // CombinedTextureSampler (a Texture2D) or CombinedTextureSampler3D (a Texture3D)
        sRef<Texture> texture;
        sRef<Sampler> sampler;

        static BindGroupEntry UniformBuffer(uint32_t binding,
                                            const sRef<Buffer>& buffer,
                                            size_t offset = 0,
                                            size_t size = 0)
        {
            BindGroupEntry entry;
            entry.binding = binding;
            entry.buffer = buffer;
            entry.offset = offset;
            entry.size = size;
            return entry;
        }

        static BindGroupEntry TextureSampler(uint32_t binding,
                                             const sRef<Texture>& texture,
                                             const sRef<Sampler>& sampler)
        {
            BindGroupEntry entry;
            entry.binding = binding;
            entry.texture = texture;
            entry.sampler = sampler;
            return entry;
        }
    };

    class DH_API BindGroup
    {
      public:
        virtual ~BindGroup() = default;

        [[nodiscard]] const sRef<BindGroupLayout>& GetLayout() const { return mLayout; }
        [[nodiscard]] const std::vector<BindGroupEntry>& GetEntries() const { return mEntries; }

        static sRef<BindGroup> Create(const sRef<BindGroupLayout>& layout,
                                      const std::vector<BindGroupEntry>& entries);

      protected:
        BindGroup(const sRef<BindGroupLayout>& layout, const std::vector<BindGroupEntry>& entries);

        sRef<BindGroupLayout> mLayout;
        std::vector<BindGroupEntry> mEntries;
    };
}
