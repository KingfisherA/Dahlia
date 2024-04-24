
#include "GraphicsEngine/3D/Buffers.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLBuffers.h"
#include "GraphicsEngine/Platform/API Core.h"
#include "GraphicsEngine/Utilities/Memory.h"
//
//

namespace GEng
{
	sRef<VertexBuffer> VertexBuffer::Create(BufferUsage usage)
	{
		switch (PROGRAM_API)
		{
		case API::OpenGL:
			return (sRef<VertexBuffer>)make<OpenGLVertexBuffer>(usage);
		}
	}

	sRef<VertexBuffer> VertexBuffer::Create(const float *data, size_t floatCount, BufferUsage usage)
	{
		switch (PROGRAM_API)
		{
		case API::OpenGL:
			return (sRef<VertexBuffer>)make<OpenGLVertexBuffer>(usage, data, floatCount);
		}
	}
	//
	//
	sRef<IndexBuffer> IndexBuffer::Create(BufferUsage usage)
	{
		switch (PROGRAM_API)
		{
		case API::OpenGL:
			return (sRef<IndexBuffer>)make<OpenGLIndexBuffer>(usage);
		}
	}

	sRef<IndexBuffer> IndexBuffer::Create(const unsigned int *data, size_t intCount, BufferUsage usage)
	{
		switch (PROGRAM_API)
		{
		case API::OpenGL:
			return make<OpenGLIndexBuffer>(usage, data, intCount);
		}
	}
}