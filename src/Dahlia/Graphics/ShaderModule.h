#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsTypes.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    // Shaders are authored once as Vulkan flavoured GLSL (#version 450, explicit set/binding and
    // push_constant blocks) and compiled offline to SPIR-V with glslc. Each backend translates the
    // SPIR-V at load time:
    //   OpenGL -> GLSL 410 via SPIRV-Cross     Vulkan -> used directly     Metal -> MSL via SPIRV-Cross
    //
    // Authoring rules imposed by OpenGL 4.1:
    //   * Uniform and push_constant blocks must be std140 compatible (no scalar/vec2 arrays).
    //   * Use combined image samplers (sampler2D/sampler3D), not separate texture/sampler objects.
    //   * No storage buffers, images or compute.
    enum class ShaderResourceType : uint8_t
    {
        UniformBuffer = 0,
        CombinedTextureSampler,  // sampler2D, bound to a Texture2D
        CombinedTextureSampler3D // sampler3D, bound to a Texture3D
    };

    [[nodiscard]] constexpr bool IsTextureResource(ShaderResourceType type) noexcept
    {
        return type == ShaderResourceType::CombinedTextureSampler ||
               type == ShaderResourceType::CombinedTextureSampler3D;
    }

    struct DH_API ShaderResource
    {
        std::string name;
        uint32_t set = 0;
        uint32_t binding = 0;
        ShaderResourceType type = ShaderResourceType::UniformBuffer;
    };

    struct DH_API ShaderReflection
    {
        ShaderStage stage = ShaderStage::None;
        std::vector<ShaderResource> resources;
        uint32_t pushConstantSize = 0; // In bytes, 0 if the stage has no push_constant block
    };

    class DH_API ShaderModule
    {
      public:
        virtual ~ShaderModule() = default;

        [[nodiscard]] ShaderStage GetStage() const { return mReflection.stage; }
        [[nodiscard]] const ShaderReflection& GetReflection() const { return mReflection; }

        static sRef<ShaderModule> Create(ShaderStage stage, std::span<const uint32_t> spirv);

        // Extract resource bindings and push constant size from SPIR-V
        static ShaderReflection Reflect(ShaderStage stage, std::span<const uint32_t> spirv);

      protected:
        ShaderReflection mReflection;
    };
}
