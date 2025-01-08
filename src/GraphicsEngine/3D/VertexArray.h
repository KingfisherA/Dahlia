#pragma once

#include "GraphicsEngine/Maths/Geometry.h"
#include "GraphicsEngine/GEng_API.h"
#include "GraphicsEngine/3D/Buffers.h"
#include "GraphicsEngine/Utilities/Memory.h"

namespace GEng {
    GENG_API struct VertexSpecification {
        unsigned char vertexSize = 3;
        // In floats
        unsigned int vertexStride = 0;
        // In floats
        unsigned int vertexOffset = 0;
        // In instances. Specify the number of instances
        //  that will pass between updates of the generic attribute at slot index.
        unsigned int instanceDivisor = 0;
    };


    using Index = unsigned int;
    constexpr Index IndexUndefined = static_cast<Index>(-1);

    GENG_API enum class Primitive {
        Triangles = 0,
        Lines,
        Points,
        TriangleFan,
        Quads,
        QuadStrip
    };

    GENG_API class VertexArray {
    public:
        virtual ~VertexArray() = default;

        static sRef<VertexArray> Create();
        static sRef<VertexArray> Create(const Mesh_F32 &meshData);

        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void SubmitArrays(Primitive drawMode = Primitive::Triangles, size_t instances = 1) = 0;

        virtual Index AddVertexBuffer(const sRef<VertexBuffer> &buffer, VertexSpecification spec = {3, 0, 0}) = 0;
        virtual void AddIndexBuffer(const sRef<IndexBuffer> &buffer) = 0;

        virtual void StoreMesh(const Mesh_F32 &meshData);

        [[nodiscard]] virtual Index QueryBufferIndex(const sRef<VertexBuffer> &buffer) = 0;

        [[nodiscard]] virtual sRef<VertexBuffer> GetVBuffer(Index index) = 0;
        [[nodiscard]] virtual sRef<IndexBuffer> GetIBuffer() = 0;
        [[nodiscard]] virtual bool IsIndexed() const = 0;

        [[nodiscard]] virtual size_t GetNoOfVBuffers() const = 0;

        void UpdateElementLength(size_t newEleLength);
        [[nodiscard]] size_t GetArrayElementLength() const;

    protected:
        size_t mElementLength = 0;
    };
}
