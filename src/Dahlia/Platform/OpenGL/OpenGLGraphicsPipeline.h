#pragma once

#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsPipeline.h"
#include "Dahlia/Graphics/ShaderModule.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"

namespace DH
{
    // OpenGL has no pipeline object: this is a linked program, a VAO and the fixed function state
    // from the description. OpenGLCommandList::BindPipeline applies the state with
    // glEnable/glDisable/glBlendFunc/glPolygonMode... (skipping calls whose state is unchanged).
    class DH_API OpenGLGraphicsPipeline : public GraphicsPipeline
    {
      public:
        // Where a shader resource (set, binding) is bound in this program: a uniform buffer
        // binding index or a texture unit
        struct ResourceSlot
        {
            uint32_t set = 0;
            uint32_t binding = 0;
            ShaderResourceType type = ShaderResourceType::UniformBuffer;
            GLuint slot = 0;
        };

        static constexpr GLuint NO_SLOT = ~0u;

        explicit OpenGLGraphicsPipeline(const GraphicsPipelineDesc& desc);
        OpenGLGraphicsPipeline(const OpenGLGraphicsPipeline&) = delete;
        ~OpenGLGraphicsPipeline() override;

        [[nodiscard]] GLuint GetProgram() const { return mProgram; }
        [[nodiscard]] GLuint GetVAO() const { return mVAO; }
        [[nodiscard]] const std::vector<ResourceSlot>& GetResourceSlots() const { return mSlots; }
        // Uniform buffer binding of the push constant block, or NO_SLOT if no stage uses one
        [[nodiscard]] GLuint GetPushConstantSlot() const { return mPushConstantSlot; }

        [[nodiscard]] const ResourceSlot* FindSlot(uint32_t set, uint32_t binding) const;

      private:
        void LinkProgram();
        void AssignResourceSlots();

        GLuint mProgram = 0;
        GLuint mVAO = 0;
        std::vector<ResourceSlot> mSlots;
        GLuint mPushConstantSlot = NO_SLOT;
    };
}
