
#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLVertexArray.h"
#include "GraphicsEngine/Platform/GPUContext.h"
//
//

namespace GEng
{
	sRef<VertexArray> VertexArray::Create()
	{
		switch (GPUContext::Get()->GetContextAPI())
		{
		case API::OpenGL:
			return make<OpenGLVertexArray>();
		}
	}

	sRef<VertexArray> VertexArray::Create(const sRef<Mesh> &meshData)
	{
		switch (GPUContext::Get()->GetContextAPI())
		{
		case API::OpenGL:
		{
			sRef<VertexArray> vArray = make<OpenGLVertexArray>();
			vArray->StoreMesh(meshData);
			return vArray;
		}
		}
	}

	void VertexArray::StoreMesh(const sRef<Mesh> &meshData)
	{
		if (meshData->GetVertexArray() != nullptr)
			this->AddVertexBuffer(VertexBuffer::Create(meshData->GetVertexArray(), meshData->GetVertexFloatCount()),
								  {meshData->vertexElementSize, 0, 0});
		if (meshData->GetNormalsArray() != nullptr)
			this->AddVertexBuffer(VertexBuffer::Create(meshData->GetNormalsArray(), meshData->GetNormalsFloatCount()));
		if (meshData->GetTexCoordsArray() != nullptr)
			this->AddVertexBuffer(VertexBuffer::Create(meshData->GetTexCoordsArray(), meshData->GetTexCoordFloatCount()),
								  {2, 0, 0});

		if (meshData->IsIndexed())
			this->AddIndexBuffer(IndexBuffer::Create(meshData->GetIndiciesArray(), meshData->GetIndexIntCount()));
	}
}
