#pragma once

#include <string>
#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/ShaderModule.h"

namespace MTL
{
    class Function;
    class Library;
}

namespace DH
{
    // Translates SPIR-V to MSL with SPIRV-Cross. `fixup_clipspace` maps OpenGL's [-1, 1] clip
    // depth to Metal's [0, 1]; Metal's NDC already has +y up, so nothing else changes.
    //
    // Argument table layout for each stage:
    //   buffer  0                  push constants (setVertexBytes / setFragmentBytes)
    //   buffer  1..                uniform buffers, in (set, binding) order
    //   buffer  VERTEX_BUFFER_BASE+n  vertex buffer binding n (vertex stage)
    //   texture/sampler 0..        combined image samplers, in (set, binding) order
    class DH_API MetalShaderModule : public ShaderModule
    {
      public:
        static constexpr uint32_t PUSH_CONSTANT_BUFFER_INDEX = 0;
        static constexpr uint32_t VERTEX_BUFFER_BASE = 16;

        struct ArgumentSlot
        {
            uint32_t set = 0;
            uint32_t binding = 0;
            ShaderResourceType type = ShaderResourceType::UniformBuffer;
            uint32_t index = 0; // Buffer index, or texture + sampler index
        };

        MetalShaderModule(ShaderStage stage, std::span<const uint32_t> spirv);
        MetalShaderModule(const MetalShaderModule&) = delete;
        ~MetalShaderModule() override;

        [[nodiscard]] MTL::Function* GetFunction() const { return mFunction; }
        [[nodiscard]] const std::string& GetMSLSource() const { return mMSLSource; }
        [[nodiscard]] const ArgumentSlot* FindSlot(uint32_t set, uint32_t binding) const;
        [[nodiscard]] bool UsesPushConstants() const { return mReflection.pushConstantSize > 0; }

      private:
        std::string mMSLSource;
        std::vector<ArgumentSlot> mSlots;
        MTL::Library* mLibrary = nullptr;
        MTL::Function* mFunction = nullptr;
    };
}
