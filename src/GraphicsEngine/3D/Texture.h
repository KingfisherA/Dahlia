#pragma once

#include<string_view>
#include<glm/glm.hpp>

#include "GraphicsEngine/GEng_API.h"
#include "GraphicsEngine/Utilities/Memory.h"

namespace GEng {
	using Dimension2D = glm::vec<2, size_t>;

	struct GENG_API PixelFormat {
	public:
		enum PixelEnum : unsigned char {
			None = 0,

			// Colour
			R,
			RGB,
			RGBA8,
			RG,
			RED_INTEGER,

			// Depth/stencil
			DEPTH24_STENCIL8
		} mValue = None;

		PixelFormat() noexcept = default;

		PixelFormat(PixelEnum pixEnum) noexcept : mValue(pixEnum) {
		}

		PixelFormat(const PixelFormat& format) noexcept = default;

		PixelFormat(PixelFormat&& format) noexcept : mValue(format.mValue) {
		}

		operator PixelEnum() const noexcept { return mValue; }

		PixelFormat& operator=(const PixelFormat& format) noexcept = default;

		PixelFormat& operator=(PixelFormat&& format) noexcept {
			mValue = format.mValue;
			return *this;
		}

		static bool IsDepth(PixelFormat format) noexcept {
			if (format.mValue == DEPTH24_STENCIL8)
				return true;

			return false;
		}

		bool IsDepth() { return IsDepth(*this); }
	};

	class GENG_API TextureLoader {
	public:
		using byte = unsigned char;

		explicit TextureLoader(std::string_view filePath);

		~TextureLoader();

		[[nodiscard]] byte* GetData() { return m_Data; }
		[[nodiscard]] const byte* GetData() const { return m_Data; }
		[[nodiscard]] const Dimension2D& GetDimensions() const { return m_imageDimensions; }
		[[nodiscard]] const PixelFormat GetFormat() const { return m_imageFormat; }

	private:
		byte* m_Data = nullptr;
		Dimension2D m_imageDimensions{};
		PixelFormat m_imageFormat{};
	};

	enum class MipmapFiltering : unsigned char {
		Bilinear,
		Trilinear
	};

	class GENG_API Texture {
	public:
		virtual ~Texture() = default;

		virtual void Bind(unsigned short unit) = 0;

		virtual void Unbind() = 0;

		virtual unsigned int GetRendererID() const noexcept = 0;
	};

	class GENG_API  Texture2D : public Texture {
	public:
		using byte = unsigned char;

		virtual void GenerateMipMaps(MipmapFiltering filtering) = 0;

		virtual void DeleteMipMaps() = 0;

		[[nodiscard]] virtual const Dimension2D& GetDimensions() const = 0;

		[[nodiscard]] const size_t GetWidth() const { return this->GetDimensions().x; }
		[[nodiscard]] const size_t GetHeight() const { return this->GetDimensions().y; }
		[[nodiscard]] const double GetAspect() const {
			return (double)this->GetDimensions().x / (double)this->GetDimensions().y;
		}
		[[nodiscard]] double GetInvAspect() const {
			return (double)this->GetDimensions().y / (double)this->GetDimensions().x;
		}

		virtual void UpdateData(Dimension2D offset, Dimension2D size, const byte* data) = 0;

		static sRef<Texture2D> Create(std::string_view filePath);
		static sRef<Texture2D> Create(const TextureLoader& imageData);
		static sRef<Texture2D> Create(const Dimension2D& dimensions, const byte* data, PixelFormat format);
	};
}
