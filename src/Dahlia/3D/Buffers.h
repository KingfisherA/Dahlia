#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
	struct DH_API VertexLayout
	{
		unsigned char eleSize = 0;
		size_t offset = 0;
		size_t stride = 0;
	};

	DH_API enum class BufferUsage : unsigned char
	{
		StaticUpdate,
		DynamicUpdate
	};

	class DH_API  VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;
		//
		virtual void StoreData(const float *data, const size_t floatCount) = 0;
		virtual void UpdateData(const float *data, const size_t offset, const size_t floatCount) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		[[nodiscard]] virtual size_t GetBufferSize() const = 0;
		[[nodiscard]] virtual size_t GetBufferByteSize() const = 0;
		//
		static sRef<VertexBuffer> Create(BufferUsage usage = BufferUsage::StaticUpdate);
		static sRef<VertexBuffer> Create(const float *data, size_t floatCount, BufferUsage usage = BufferUsage::StaticUpdate);
	};

	class DH_API  IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;
		//
		virtual void StoreData(const unsigned int *data, size_t intCount) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		[[nodiscard]] virtual size_t GetBufferSize() const = 0;
		[[nodiscard]] virtual size_t GetBufferByteSize() const = 0;
		//
		static sRef<IndexBuffer> Create(BufferUsage usage = BufferUsage::StaticUpdate);
		static sRef<IndexBuffer> Create(const unsigned int *data, size_t intCount, BufferUsage usage = BufferUsage::StaticUpdate);
	};
}