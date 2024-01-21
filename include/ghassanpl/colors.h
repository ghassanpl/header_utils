/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <charconv>
#include <string_view>
#include <glm/common.hpp>
#include <glm/ext/vector_float4.hpp>
#include "named.h"
#include "constexpr_math.h"

namespace ghassanpl
{
	/// \defgroup Colors Colors
	/// Colors and things

	/// Implementation note: These functions use the .xyzw members instead of the .rgba members because accessing the latter is not constexpr

	/// \ingroup Colors
	///@{

	enum class color_space
	{
		linear_rgba,
		srgb,
		hsva,

		rgbe, /// Radiant HDR
	};

	/// Represents a color in RGBA color space, with 0.0-1.0 float elements
	using color_rgba_t = glm::vec4;
	/// Represents a color in HSVA color space, with H=0.0-6.0, S=0.0-1.0, V=0.0-1.0, A=0.0-1.0 float elements
	using color_hsva_t = named<glm::vec4, "color_hsva">;
	/// Default `color_t` type is RGBA
	using color_t = color_rgba_t;

	/// Represent a packed RGBA color with 8 bits per pixel
	using color_rgba_u32_t = named<uint32_t, "color_rgba_u32">;
	/// Represent a packed ABGR color with 8 bits per pixel
	using color_abgr_u32_t = named<uint32_t, "color_abgr_u32">;

	/// Contains the basic colors as variables and functions. The functions take an `alpha` argument that determines the alpha of the returned color.
	/// \ingroup Colors
	namespace colors
	{
#define DEF_COLOR(name, r, g, b) \
	[[nodiscard]] constexpr color_t get_##name(float alpha) { return color_t{ float(r), float(g), float(b), alpha }; } \
	constexpr inline color_t name = get_##name(1.0f); 
#define DEF_COLORS(name, r, g, b) \
	DEF_COLOR(name, r, g, b) \
	[[nodiscard]] constexpr color_t get_dark_##name(float alpha) { return color_t{ float(r) * 0.5f, float(g) * 0.5f, float(b) * 0.5f, alpha }; } \
	constexpr inline color_t dark_##name = get_dark_##name(1.0f); \
	[[nodiscard]] constexpr color_t get_light_##name(float alpha) { return color_t{ r + float(1.0f-r) * 0.5f, g + float(1.0f-g) * 0.5f, b + float(1.0f-b) * 0.5f, alpha }; } \
	constexpr inline color_t light_##name = get_light_##name(1.0f);

		DEF_COLORS(red, 1, 0, 0)
		DEF_COLORS(green, 0, 1, 0)
		DEF_COLORS(blue, 0, 0, 1)
		DEF_COLORS(yellow, 1, 1, 0)
		DEF_COLORS(magenta, 1, 0, 1)
		DEF_COLORS(cyan, 0, 1, 1)
		DEF_COLORS(gray, 0.5f, 0.5f, 0.5f)
		DEF_COLORS(grey, 0.5f, 0.5f, 0.5f)

		DEF_COLORS(orange, 1, 0.65f, 0)
		DEF_COLORS(brown, 0.59f, 0.29f, 0)
		
		DEF_COLOR(black, 0, 0, 0)
		DEF_COLOR(white, 1, 1, 1)
		constexpr inline color_t transparent = get_black(0.0f);

		#undef DEF_COLOR
		#undef DEF_COLORS
	}

	/// Returns the color multiplied by its own alpha
	[[nodiscard]] constexpr color_t premultiplied(color_t const& color)
	{
		return { color.x * color.w, color.y * color.w, color.z * color.w, color.w };
	}

	/// Returns a color with all elements clamped between 0 and 1
	[[nodiscard]] constexpr color_t saturated(color_t const& color)
	{
		return glm::clamp(color_t(color.x, color.y, color.z, color.w), color_t{ 0,0,0,0 }, color_t{ 1,1,1,1 });
	}

	/// Returns a color lightened by a coefficient
	[[nodiscard]] constexpr color_t lighten(color_t const& color, float coef)
	{
		const auto rgb_max = glm::max(color.x, glm::max(color.y, color.z));
		const auto lighter = color * (1.0f / rgb_max);
		const auto dif = rgb_max;
		return saturated(color_t(lighter.x + dif * coef, lighter.y + dif * coef, lighter.z + dif * coef, 1.0f) * rgb_max);
	}

	/// Returns a color with its contrast changed
	/// \note `contrast` here is between 0.0 and 1.0
	/// \sa contrast2
	[[nodiscard]] constexpr color_t contrast(color_t const& color, float contrast)
	{
		const auto t = (1.0f - contrast) * 0.5f;
		return color_t(color.x*contrast + t, color.y*contrast + t, color.z*contrast + t, color.w);
	}

	/// Returns a color with its contrast changed
	/// This uses a differen algorithm and `contrast` value than \ref contrast.
	/// \note `contrast` here is between -1.0 and 1.0
	/// \sa contrast
	[[nodiscard]] constexpr color_t contrast2(color_t const& color, float contrast)
	{
		constexpr double m = 1.0156862745098039215686274509804;
		const auto t = (m * (contrast + 1.0f)) / (m - contrast);
		return color_t(t * (color.x - 0.5f) + 0.5f, t * (color.y - 0.5f) + 0.5f, t * (color.z - 0.5f) + 0.5f, color.w);
	}

	/// Returns a gamma corrected color
	[[nodiscard]] constexpr color_t gamma_correct(color_t const& color, const float gamma)
	{
		const auto gamma_correct = 1.0f / gamma;
		return { cem::pow(color.x, gamma_correct), cem::pow(color.y, gamma_correct), cem::pow(color.z, gamma_correct), color.w };
	}

	/// Returns an inverted color, i.e. with (1.0-x) on all of its elements, excluding its alpha
	[[nodiscard]] constexpr color_t inverted(color_t const& color)
	{
		return color_t(1.0f - color.x, 1.0f - color.y, 1.0f - color.z, color.w);
	}

	/// Returns a color that's a good contrasting color for the original
	[[nodiscard]] constexpr color_t contrasting(color_t const& color)
	{
		return color_t(cem::fmod(color.x + 0.5f, 1.0f), cem::fmod(color.y + 0.5f, 1.0f), cem::fmod(color.z + 0.5f, 1.0f), color.w);
	}
	
	/// Get brightness of color
	[[nodiscard]] constexpr float luminance(color_t const& color)
	{		
		return color.x * 0.3f + color.y * 0.59f + color.z * 0.11f;
	}

	/// Returns a color sapped of `desaturation` percent (0-1) of its saturation.
	[[nodiscard]] constexpr color_t desaturated(color_t const& color, float desaturation)
	{
		const auto l = luminance(color);
		return glm::mix(color, color_t{l, l, l, color.w}, desaturation);
	}
	
	namespace detail 
	{
		constexpr float b2f(uint32_t byte) { return (byte & 0xFF) / 255.0f; } 
		constexpr uint32_t f2b(float f) { return uint8_t(0.5f + f * 255.0f); } 
		constexpr uint32_t f2u4(float b1, float b2, float b3, float b4) { return (f2b(b1) << 24) | (f2b(b2) << 16) | (f2b(b3) << 8) | f2b(b4); }
		constexpr uint32_t f2u4(float b1, float b2, float b3) { return (f2b(b1) << 16) | (f2b(b2) << 8) | f2b(b3); }

		/// RATIONALE: glm has two versions of 3-param min/max for some reason. NEITHER are constexpr :/
		template <typename T> constexpr auto min(const T& x, const T& y, const T& z) { return glm::min(glm::min(x, y), z); }
		template <typename T> constexpr auto max(const T& x, const T& y, const T& z) { return glm::max(glm::max(x, y), z); }
	}

	/// Gets a color from an RGB 8bpp integer, with R being most significant
	[[nodiscard]] constexpr color_t from_u32_rgb(uint32_t rgb) { return color_t(detail::b2f(rgb >> 16), detail::b2f(rgb >> 8), detail::b2f(rgb), 1.0f); }
	/// Gets a color from an BGR 8bpp integer, with R being least significant
	[[nodiscard]] constexpr color_t from_u32_bgr(uint32_t rgb) { return color_t(detail::b2f(rgb), detail::b2f(rgb >> 8), detail::b2f(rgb >> 16), 1.0f); }
	/// Gets a color from an RGBA 8bpp integer, with R being most significant
	[[nodiscard]] constexpr color_t from_u32_rgba(uint32_t rgb) { return color_t(detail::b2f(rgb >> 24), detail::b2f(rgb >> 16), detail::b2f(rgb >> 8), detail::b2f(rgb)); }
	/// Gets a color from an BGRA 8bpp integer, with A being least significant, and B being most significant
	[[nodiscard]] constexpr color_t from_u32_bgra(uint32_t rgb) { return color_t(detail::b2f(rgb >> 8), detail::b2f(rgb >> 16), detail::b2f(rgb >> 24), detail::b2f(rgb)); }
	/// Gets a color from an ARGB 8bpp integer, with A being most significant, and B being least significant
	[[nodiscard]] constexpr color_t from_u32_argb(uint32_t rgb) { return color_t(detail::b2f(rgb >> 16), detail::b2f(rgb >> 8), detail::b2f(rgb), detail::b2f(rgb >> 24)); }
	/// Gets a color from an ABGR 8bpp integer, with A being most significant, and R being least significant
	[[nodiscard]] constexpr color_t from_u32_abgr(uint32_t rgb) { return color_t(detail::b2f(rgb), detail::b2f(rgb >> 8), detail::b2f(rgb >> 16), detail::b2f(rgb >> 24)); }

	/// Creates an 8bpp ARGB integer from a color
	[[nodiscard]] constexpr uint32_t to_u32_argb(color_t const& rgba) { return detail::f2u4(rgba.w, rgba.x, rgba.y, rgba.z); }
	/// Creates an 8bpp ABGR integer from a color
	[[nodiscard]] constexpr uint32_t to_u32_abgr(color_t const& rgba) { return detail::f2u4(rgba.w, rgba.z, rgba.y, rgba.x); }
	/// Creates an 8bpp RGBA integer from a color
	[[nodiscard]] constexpr uint32_t to_u32_rgba(color_t const& rgba) { return detail::f2u4(rgba.x, rgba.y, rgba.z, rgba.w); }
	/// Creates an 8bpp BGRA integer from a color
	[[nodiscard]] constexpr uint32_t to_u32_bgra(color_t const& rgba) { return detail::f2u4(rgba.z, rgba.y, rgba.x, rgba.w); }
	/// Creates a 32 bitbpp RGB integer from a color, with the most significant 8 bits set to 0
	[[nodiscard]] constexpr uint32_t to_u32_rgb(color_t const& rgba) { return detail::f2u4(rgba.x, rgba.y, rgba.z); }
	/// Creates a 32 bitbpp BGR integer from a color, with the most significant 8 bits set to 0
	[[nodiscard]] constexpr uint32_t to_u32_bgr(color_t const& rgba) { return detail::f2u4(rgba.z, rgba.y, rgba.x); }

	template <std::same_as<color_rgba_u32_t> TO>
	[[nodiscard]] constexpr TO named_cast(color_rgba_t const& from)
	{
		return TO{ to_u32_rgba(from) };
	}

	template <std::same_as<color_abgr_u32_t> TO>
	[[nodiscard]] constexpr TO named_cast(color_rgba_t const& from)
	{
		return TO{ to_u32_abgr(from) };
	}

	template <typename TO, typename FROM>
	[[nodiscard]] TO color_cast(FROM const& from)
	{
		return named_cast<TO>(from);
	}

	/// Returns the color as a `glm::vec4` of `uint8_t`s
	[[nodiscard]] constexpr glm::tvec4<uint8_t> to_u8(color_t const& rgba) { return { detail::f2b(rgba.x), detail::f2b(rgba.y), detail::f2b(rgba.z), detail::f2b(rgba.w) }; }

	/// Converts a HSVA color to RGBA space
	[[nodiscard]] constexpr color_t to_rgb(color_hsva_t const& hsva)
	{
		const auto hue = hsva->r;
		const auto saturation = hsva->g;
		const auto value = hsva->b;
		const auto alpha = hsva->a;
		const auto i = (int)hue;
		float fraction = hue - (float)i;
		if (!(i & 1)) fraction = 1.0f - fraction;
		const auto m = value * (1.0f - saturation);
		const auto n = value * (1.0f - saturation * fraction);
		switch (i)
		{
		case 6:
		case 0: return color_t(value, n, m, alpha);
		case 1: return color_t(n, value, m, alpha);
		case 2: return color_t(m, value, n, alpha);
		case 3: return color_t(m, n, value, alpha);
		case 4: return color_t(n, m, value, alpha);
		case 5: return color_t(value, m, n, alpha);
		default: return color_t(value, value, value, alpha);
		}
	}

	/// Converts an RGBA color to HSVA space
	[[nodiscard]] constexpr color_hsva_t to_hsv(color_t const& rgba)
	{
		const auto min = detail::min(rgba.x, rgba.y, rgba.z);
		const auto max = detail::max(rgba.x, rgba.y, rgba.z);
		const auto delta = max - min;

		float h = 0;

		if (delta != 0)
		{
			if (rgba.x == max)
				h = fmod((rgba.y - rgba.z) / delta, 6.0f);
			else if (rgba.y == max)
				h = 2 + (rgba.z - rgba.x) / delta;
			else
				h = 4 + (rgba.x - rgba.y) / delta;
		}

		return color_hsva_t{
			h,
			(max != 0) ? (delta / max) : 0,
			max,
			rgba.w
		};
	}

	/// Converts a HTML color string (like \#FBA or fafafa) to an RGBA color
	[[nodiscard]] constexpr color_rgba_t from_html(const char* str, size_t n)
	{
		if (n == 0)
			throw n;
		if (str[0] == '#')
		{
			str++;
			n--;
		}
		uint8_t r{};
		uint8_t g{};
		uint8_t b{};
		uint8_t a = 255;
		switch (n)
		{
		case 4:
			std::from_chars(str + 3, str + 4, a, 16); a = a << 4 | a;
			[[fallthrough]];
		case 3:
			std::from_chars(str + 0, str + 1, r, 16); r = r << 4 | r;
			std::from_chars(str + 1, str + 2, g, 16); g = g << 4 | g;
			std::from_chars(str + 2, str + 3, b, 16); b = b << 4 | b;
			break;
		case 8:
			std::from_chars(str + 6, str + 8, a, 16);
			[[fallthrough]];
		case 6:
			std::from_chars(str + 0, str + 2, r, 16);
			std::from_chars(str + 2, str + 4, g, 16);
			std::from_chars(str + 4, str + 6, b, 16);
			break;
		default: throw "invalid number of characters";
		}
		return { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
	}

	/// Converts a HTML color string (like \#FBA or fafafa) to an RGBA color
	[[nodiscard]] consteval color_rgba_t operator ""_rgb(const char* str, size_t n)
	{
		return from_html(str, n);
	}

	/// Converts a HTML color string (like \#FBA or fafafa) to an RGBA color
	[[nodiscard]] constexpr color_rgba_t from_html(std::string_view html)
	{
		return from_html(html.data(), html.size());
	}

	///@}

#if 0

	float GetDotProduct() const {
		return R*R + G*G + B*B;
	}
	float GetFullDotProduct() const {
		return R*R + G*G + B*B + A*A;
	}

	const Color* FindClosestColor(ArrayView<Color> palette) const {
		return std::min_element(palette.begin(), palette.end(), [this](const Color& a, const Color& b) {
			const auto diff_a = a - *this;
			const auto diff_b = b - *this;
			return diff_a.GetDotProduct() < diff_b.GetDotProduct();
		});
	}

	const Color* FindClosestFullColor(ArrayView<Color> palette) const {
		return std::min_element(palette.begin(), palette.end(), [this](const Color& a, const Color& b) {
			const auto diff_a = a.FullDifference(*this);
			const auto diff_b = b.FullDifference(*this);
			return diff_a.GetFullDotProduct() < diff_b.GetFullDotProduct();
		});
	}
	
	
		enum class GrayscaleType
		{
			Lightness,
			Average,
			Luminosity
		};

		template <GrayscaleType TYPE = GrayscaleType::Luminosity>
		Color Grayscale() const {
			switch (TYPE)
			{
			case GrayscaleType::Lightness: return Color((Max3(R, G, B) + Min3(R, G, B))*0.5, A);
			case GrayscaleType::Average: return Color((R+G+B)/3.0, A);
			default:
			case GrayscaleType::Luminosity: return Color(R*0.21 + G*0.72 + B*0.07, A);
			}
		}

		Color BlendedOver(const Color& other) const {
			const auto oma = 1.0f - A;
			return *this * A + other * oma;
		}

		template <typename FUNC>
		Color BlendWithAlpha(const Color& other, FUNC func) const {
			return Lerp(func(*this, other), other, A);
		}
		/*
		static Color Multiply(const Color& c1, const Color& c2 ) { return Color(c1.x*c2.x, c1.y*c2.y, c1.z*c2.z); }
		static Color Screen(const Color& c1, const Color& c2) { return Multiply(c1.Inverted(), c2.Inverted()).Inverted(); }
		static Color Overlay(const Color& c1, const Color& c2) {
			return Color(
				c2.x < 0.5f ? c1.x*c2.x * 2 : (1 - 2 * (1 - c2.x)*(1 - c1.x)),
				c2.y < 0.5f ? c1.y*c2.y * 2 : (1 - 2 * (1 - c2.y)*(1 - c1.y)),
				c2.z < 0.5f ? c1.z*c2.z * 2 : (1 - 2 * (1 - c2.z)*(1 - c1.z)));
		}
		static Color HardLight(const Color& c2, const Color& c1) {
			return Color(
				c2.x < 0.5f ? c1.x*c2.x * 2 : (1 - 2 * (1 - c2.x)*(1 - c1.x)),
				c2.y < 0.5f ? c1.y*c2.y * 2 : (1 - 2 * (1 - c2.y)*(1 - c1.y)),
				c2.z < 0.5f ? c1.z*c2.z * 2 : (1 - 2 * (1 - c2.z)*(1 - c1.z)));
		}
		static Color Overlay(const Color& c1, const Color& c2) {
			return Multiply((2 * c2).Inverted(), Multiply(c1, c1)) + 2 * Multiply(c1, c2); /// pegtop!
		}
		*/
		/*
		#define ChannelBlend_Normal(A,B)     ((uint8)(A))
		#define ChannelBlend_Lighten(A,B)    ((uint8)((B > A) ? B:A))
		#define ChannelBlend_Darken(A,B)     ((uint8)((B > A) ? A:B))
		#define ChannelBlend_Multiply(A,B)   ((uint8)((A * B) / 255))
		#define ChannelBlend_Average(A,B)    ((uint8)((A + B) / 2))
		#define ChannelBlend_Add(A,B)        ((uint8)(min(255, (A + B))))
		#define ChannelBlend_Subtract(A,B)   ((uint8)((A + B < 255) ? 0:(A + B - 255)))
		#define ChannelBlend_Difference(A,B) ((uint8)(abs(A - B)))
		#define ChannelBlend_Negation(A,B)   ((uint8)(255 - abs(255 - A - B)))
		#define ChannelBlend_Screen(A,B)     ((uint8)(255 - (((255 - A) * (255 - B)) >> 8)))
		#define ChannelBlend_Exclusion(A,B)  ((uint8)(A + B - 2 * A * B / 255))
		#define ChannelBlend_Overlay(A,B)    ((uint8)((B < 128) ? (2 * A * B / 255):(255 - 2 * (255 - A) * (255 - B) / 255)))
		#define ChannelBlend_SoftLight(A,B)  ((uint8)((B < 128)?(2*((A>>1)+64))*((float)B/255):(255-(2*(255-((A>>1)+64))*(float)(255-B)/255))))
		#define ChannelBlend_HardLight(A,B)  (ChannelBlend_Overlay(B,A))
		#define ChannelBlend_ColorDodge(A,B) ((uint8)((B == 255) ? B:min(255, ((A << 8 ) / (255 - B)))))
		#define ChannelBlend_ColorBurn(A,B)  ((uint8)((B == 0) ? B:max(0, (255 - ((255 - A) << 8 ) / B))))
		#define ChannelBlend_LinearDodge(A,B)(ChannelBlend_Add(A,B))
		#define ChannelBlend_LinearBurn(A,B) (ChannelBlend_Subtract(A,B))
		#define ChannelBlend_LinearLight(A,B)((uint8)(B < 128)?ChannelBlend_LinearBurn(A,(2 * B)):ChannelBlend_LinearDodge(A,(2 * (B - 128))))
		#define ChannelBlend_VividLight(A,B) ((uint8)(B < 128)?ChannelBlend_ColorBurn(A,(2 * B)):ChannelBlend_ColorDodge(A,(2 * (B - 128))))
		#define ChannelBlend_PinLight(A,B)   ((uint8)(B < 128)?ChannelBlend_Darken(A,(2 * B)):ChannelBlend_Lighten(A,(2 * (B - 128))))
		#define ChannelBlend_HardMix(A,B)    ((uint8)((ChannelBlend_VividLight(A,B) < 128) ? 0:255))
		#define ChannelBlend_Reflect(A,B)    ((uint8)((B == 255) ? B:min(255, (A * A / (255 - B)))))
		#define ChannelBlend_Glow(A,B)       (ChannelBlend_Reflect(B,A))
		#define ChannelBlend_Phoenix(A,B)    ((uint8)(min(A,B) - max(A,B) + 255))
		#define ChannelBlend_Alpha(A,B,O)    ((uint8)(O * A + (1 - O) * B))
		#define ChannelBlend_AlphaF(A,B,F,O) (ChannelBlend_Alpha(F(A,B),A,O))
		*/
		//Color BlendMultiply(const Color& other) const { return BlendWithAlpha(other, colors::Multiply); }

		static Color FromHTML(StringView str)
		{
			str.Consume('#');
			if (str.size() == 6)
			{
				uint32_t hex = StringToUnsignedLong(str, nullptr, 16);
				return colors::FromRGBInt(hex);
			}
			else if (str.size() == 8)
			{
				uint32_t hex = StringToUnsignedLong(str, nullptr, 16);
				return colors::FromRGBAInt(hex);
			}
			else if (str.size() == 3)
			{
				uint32_t hex = StringToUnsignedLong(str, nullptr, 16);
				hex = ((hex << 12) & 0xF00000) | ((hex << 8) & 0xF000) | ((hex << 4) & 0xF0);
				hex = hex | (hex >> 4);
				return colors::FromRGBInt(hex);
			}
			else return Color();
		}

		/// Getters and converters

		uint8_t GetR8() const { return uint8_t(R*255.0f); }
		uint8_t GetG8() const { return uint8_t(G*255.0f); }
		uint8_t GetB8() const { return uint8_t(B*255.0f); }
		uint8_t GetA8() const { return uint8_t(A*255.0f); }

#endif

/// TODO: Formatters and operator<<
}