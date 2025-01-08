#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLVertexArray.h"
#include "GraphicsEngine/Platform/GPUContext.h"
//
//

namespace GEng {
    sRef<VertexArray> VertexArray::Create() {
        switch (GPUContext::Get()->GetContextAPI()) {
            case GraphicsAPI::OpenGL:
                return make<OpenGLVertexArray>();
        }
    }

    sRef<VertexArray> VertexArray::Create(const Mesh_F32 &meshData) {
        switch (GPUContext::Get()->GetContextAPI()) {
            case GraphicsAPI::OpenGL: {
                sRef<VertexArray> vArray = make<OpenGLVertexArray>();
                vArray->StoreMesh(meshData);
                return vArray;
            }
        }
    }

    void VertexArray::StoreMesh(const Mesh_F32 &meshData) {
        if (meshData.HasVertex())
            this->AddVertexBuffer(
                VertexBuffer::Create(meshData.GetVertexArray().data(), meshData.GetVertexFloatCount()),
                {meshData.GetVertexSize(), 0, 0});
        if (meshData.HasNormals())
            this->AddVertexBuffer(VertexBuffer::Create(meshData.GetNormalsArray().data(),
                                                       meshData.GetNormalsFloatCount()));
        if (meshData.HasTexCoords())
            this->AddVertexBuffer(
                VertexBuffer::Create(meshData.GetTexCoordsArray().data(), meshData.GetTexCoordFloatCount()),
                {2, 0, 0});

        if (meshData.HasIndices())
            this->AddIndexBuffer(IndexBuffer::Create(meshData.GetIndicesArray().data(), meshData.GetIndexIntCount()));
    }
    //
    void VertexArray::UpdateElementLength(const size_t newEleLength) {
        mElementLength = newEleLength;
    }
    //
    size_t VertexArray::GetArrayElementLength() const { return mElementLength; }
}
