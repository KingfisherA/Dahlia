#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/3D/Texture.h"
#include "OpenGLContext.h"

namespace DH
{
	class DH_API OpenGLTexture2D : public Texture2D
	{
	public:
		using byte = unsigned char;

		explicit OpenGLTexture2D(const Dimension2D &dimensions, const byte *textureData,
								 PixelFormat texFormat);
		explicit OpenGLTexture2D(GLenum texID, const Dimension2D &dimensions,
								 PixelFormat texFormat);
#ifdef WIN32
		OpenGLTexture2D(const OpenGLTexture2D& tex);
#else 
		OpenGLTexture2D(const OpenGLTexture2D&) = delete;
#endif // WIN32

		OpenGLTexture2D(OpenGLTexture2D &&tex) noexcept;

	private:
		void CreateTexture(const Dimension2D &dimensions, const byte *textureData,
						   PixelFormat texFormat);

	public:
		virtual ~OpenGLTexture2D() override;

		virtual void Bind(unsigned short unit) override;
		virtual void Unbind() override;

		virtual void GenerateMipMaps(const MipmapFiltering filtering) override;
		virtual void DeleteMipMaps() override;

		virtual const Dimension2D &GetDimensions() const override { return mImageDimensions; }

		virtual void UpdateData(Dimension2D offset, Dimension2D size, const byte *data) override;

		virtual GLid GetRendererID() const noexcept override { return mTexID; }

		static GLenum FormatToGL(PixelFormat format) noexcept;

	private:
		Dimension2D mImageDimensions;
		GLid mTexID = 0;
		PixelFormat mPixelFormat;
		bool mMipMapsGenerated = false;
		uint16_t mTexUnit = 0;
	};
}