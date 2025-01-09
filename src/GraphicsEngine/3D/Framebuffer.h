#pragma once

#include <vector>

#include "GraphicsEngine/GEng_API.h"
#include "GraphicsEngine/Utilities/Memory.h"
#include "GraphicsEngine/3D/Texture.h"


namespace GEng
{

	struct GENG_API FramebufferAttachment
	{
	public:
		FramebufferAttachment() = default;
		explicit FramebufferAttachment(PixelFormat format) : mPixelFormat(format) {}
		FramebufferAttachment(PixelFormat format, const bool isRenderbuffer) : mPixelFormat(format), Renderbuffer(isRenderbuffer) {}

		PixelFormat mPixelFormat = PixelFormat::None;
		bool Renderbuffer = false;
	};

	struct GENG_API  FramebufferSpecification
	{
	public:
		size_t Width = 0, Height = 0, Samples = 1;
		std::vector<FramebufferAttachment> Attachments;
	};

	class GENG_API  Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(size_t newWidth, size_t newHeight) = 0;

		virtual FramebufferAttachment *QueryAttachment(size_t index) = 0;
		virtual Texture2D *QueryTextureAttachment(size_t index = 0) = 0;

		[[nodiscard]] virtual const FramebufferSpecification &GetSpecification() const = 0;
		virtual FramebufferSpecification &GetSpecification() = 0;

		virtual void Clear(const glm::vec4& clearColour)=0;

		[[nodiscard]] size_t GetWidth() const { return GetSpecification().Width; }
		[[nodiscard]] size_t GetHeight() const { return GetSpecification().Height; }

		[[nodiscard]] virtual unsigned int GetRenderID() const = 0;

		static sRef<Framebuffer> Create(const FramebufferSpecification &spec);
	};
}
