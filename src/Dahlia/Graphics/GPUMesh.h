#pragma once

#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsPipeline.h"
#include "Dahlia/Maths/Geometry.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    class Buffer;
    class CommandList;

    // GPU copy of a Mesh_F32. Each attribute lives in its own vertex buffer at a fixed binding and
    // shader location:
    //   binding/location 0: positions (vec2 or vec3)
    //   binding/location 1: normals   (vec3), if present
    //   binding/location 2: texcoords (vec2), if present
    class DH_API GPUMesh
    {
      public:
        static constexpr uint32_t POSITION_LOCATION = 0;
        static constexpr uint32_t NORMAL_LOCATION = 1;
        static constexpr uint32_t TEXCOORD_LOCATION = 2;

        explicit GPUMesh(const Mesh_F32& meshData);

        static sRef<GPUMesh> Create(const Mesh_F32& meshData) { return make<GPUMesh>(meshData); }

        // Vertex layout to put in a GraphicsPipelineDesc that draws this mesh
        [[nodiscard]] const std::vector<VertexBinding>& GetVertexBindings() const
        {
            return mBindings;
        }
        [[nodiscard]] const std::vector<VertexAttribute>& GetVertexAttributes() const
        {
            return mAttributes;
        }

        // Binds the vertex/index buffers and issues the draw. A pipeline must already be bound.
        void Draw(CommandList& cmd, uint32_t instanceCount = 1) const;

        [[nodiscard]] const sRef<Buffer>& GetPositionBuffer() const { return mPositions; }
        [[nodiscard]] uint32_t GetVertexCount() const { return mVertexCount; }
        [[nodiscard]] uint32_t GetIndexCount() const { return mIndexCount; }
        [[nodiscard]] bool IsIndexed() const { return mIndices != nullptr; }

      private:
        sRef<Buffer> mPositions, mNormals, mTexCoords, mIndices;
        uint32_t mVertexCount = 0, mIndexCount = 0;

        std::vector<VertexBinding> mBindings;
        std::vector<VertexAttribute> mAttributes;
    };
}
