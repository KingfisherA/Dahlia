#include <stdexcept>

#include "Dahlia/Graphics/Buffer.h"
#include "Dahlia/Graphics/CommandList.h"
#include "Dahlia/Graphics/GPUMesh.h"

namespace DH
{
    static sRef<Buffer> CreateStaticBuffer(BufferUsage usage, const void* data, size_t size)
    {
        return Buffer::Create({ size, usage, BufferMemory::Static }, data);
    }

    GPUMesh::GPUMesh(const Mesh_F32& meshData)
    {
        if (!meshData.HasVertex())
            throw std::invalid_argument("Cannot create a Mesh without vertex positions");

        const uint32_t vertexSize = meshData.GetVertexSize();
        const Format positionFormat = [vertexSize]()
        {
            switch (vertexSize)
            {
                case 2:
                    return Format::RG32F;
                case 3:
                    return Format::RGB32F;
                case 4:
                    return Format::RGBA32F;
                default:
                    throw std::invalid_argument("Mesh vertex size must be 2, 3 or 4");
            }
        }();

        mVertexCount = (uint32_t)(meshData.GetVertexFloatCount() / vertexSize);

        mPositions = CreateStaticBuffer(BufferUsage::Vertex,
                                        meshData.GetVertexArray().data(),
                                        meshData.GetVertexFloatCount() * sizeof(float));
        mBindings.push_back({ POSITION_LOCATION, vertexSize * (uint32_t)sizeof(float), false });
        mAttributes.push_back({ POSITION_LOCATION, POSITION_LOCATION, positionFormat, 0 });

        if (meshData.HasNormals())
        {
            mNormals = CreateStaticBuffer(BufferUsage::Vertex,
                                          meshData.GetNormalsArray().data(),
                                          meshData.GetNormalsFloatCount() * sizeof(float));
            mBindings.push_back({ NORMAL_LOCATION, 3 * sizeof(float), false });
            mAttributes.push_back({ NORMAL_LOCATION, NORMAL_LOCATION, Format::RGB32F, 0 });
        }

        if (meshData.HasTexCoords())
        {
            mTexCoords = CreateStaticBuffer(BufferUsage::Vertex,
                                            meshData.GetTexCoordsArray().data(),
                                            meshData.GetTexCoordFloatCount() * sizeof(float));
            mBindings.push_back({ TEXCOORD_LOCATION, 2 * sizeof(float), false });
            mAttributes.push_back({ TEXCOORD_LOCATION, TEXCOORD_LOCATION, Format::RG32F, 0 });
        }

        if (meshData.HasIndices())
        {
            mIndexCount = (uint32_t)meshData.GetIndexIntCount();
            mIndices = CreateStaticBuffer(BufferUsage::Index,
                                          meshData.GetIndicesArray().data(),
                                          meshData.GetIndexIntCount() * sizeof(uint32_t));
        }
    }

    void GPUMesh::Draw(CommandList& cmd, uint32_t instanceCount) const
    {
        cmd.BindVertexBuffer(POSITION_LOCATION, *mPositions);
        if (mNormals)
            cmd.BindVertexBuffer(NORMAL_LOCATION, *mNormals);
        if (mTexCoords)
            cmd.BindVertexBuffer(TEXCOORD_LOCATION, *mTexCoords);

        if (mIndices)
        {
            cmd.BindIndexBuffer(*mIndices, IndexType::UInt32);
            cmd.DrawIndexed(mIndexCount, instanceCount);
        }
        else
        {
            cmd.Draw(mVertexCount, instanceCount);
        }
    }
}
