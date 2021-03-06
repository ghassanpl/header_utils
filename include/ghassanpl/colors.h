#include <glm/vec4.hpp>
#include "named.h"

namespace ghassanpl
{
	//using color_rgba_t = named_t<glm::vec4, "color_rgba">;
	using color_rgba_t = glm::vec4;
	using color_hsva_t = named<glm::vec4, "color_hsva">;
	using color_t = color_rgba_t;

	namespace colors
	{
#define DEF_COLOR(name, r, g, b) \
	inline constexpr color_t get_##name(float alpha) { return color_t{ float(r), float(g), float(b), alpha }; } \
	inline constexpr color_t name = get_##name(1.0f); 
#define DEF_COLORS(name, r, g, b) \
	DEF_COLOR(name, r, g, b) \
	inline constexpr color_t get_dark_##name(float alpha) { return color_t{ float(r) * 0.5f, float(g) * 0.5f, float(b) * 0.5f, alpha }; } \
	inline constexpr color_t dark_##name = get_dark_##name(1.0f); \
	inline constexpr color_t get_light_##name(float alpha) { return color_t{ r + float(1.0f-r) * 0.5f, g + float(1.0f-g) * 0.5f, b + float(1.0f-b) * 0.5f, alpha }; } \
	inline constexpr color_t light_##name = get_light_##name(1.0f);

		DEF_COLORS(red, 1, 0, 0)
		DEF_COLORS(green, 0, 1, 0)
		DEF_COLORS(blue, 0, 0, 1)
		DEF_COLORS(yellow, 1, 1, 0)
		DEF_COLORS(magenta, 1, 0, 1)
		DEF_COLORS(cyan, 0, 1, 1)
		DEF_COLORS(gray, 0.5f, 0.5f, 0.5f)
		DEF_COLORS(grey, 0.5f, 0.5f, 0.5f)
		
		DEF_COLOR(black, 0, 0, 0)
		DEF_COLOR(white, 1, 1, 1)
		inline constexpr color_t transparent = get_black(0.0f);

		#undef DEF_COLOR
		#undef DEF_COLORS
	}

	constexpr inline color_t saturated(color_t const& color)
	{
		return glm::clamp(color_t(color.r, color.g, color.b, color.a), color_t{ 0,0,0,0 }, color_t{ 1,1,1,1 });
	}

	constexpr inline color_t lighten(color_t const& color, float coef)
	{
		const auto rgb_max = glm::max(color.r, glm::max(color.g, color.b));
		const auto lighter = color * (1.0f / rgb_max);
		const auto dif = rgb_max;
		return saturated(color_t(lighter.r + dif * coef, lighter.g + dif * coef, lighter.b + dif * coef, 1.0f) * rgb_max);
	}

	/// NOTE: `contrast` here is between 0.0 and 1.0
	constexpr inline color_t contrast(color_t const& color, float contrast)
	{
		const auto t = (1.0f - contrast) * 0.5f;
		return color_t(color.r*contrast + t, color.g*contrast + t, color.b*contrast + t, color.a);
	}

	/// NOTE: `contrast` here is between -1.0 and 1.0
	constexpr inline color_t contrast2(color_t const& color, float contrast)
	{
		constexpr double m = 1.0156862745098039215686274509804;
		const auto t = (m * (contrast + 1.0f)) / (m - contrast);
		return color_t(t * (color.r - 0.5f) + 0.5f, t * (color.g - 0.5f) + 0.5f, t * (color.b - 0.5f) + 0.5f, color.a);
	}

	constexpr inline color_t gamma_correct(color_t const& color, const float gamma)
	{
		const auto gamma_correct = 1.0f / gamma;
		return { std::pow(color.r, gamma_correct), std::pow(color.g, gamma_correct),std::pow(color.b, gamma_correct), color.a };
	}

	constexpr inline color_t inverted(color_t const& color)
	{
		return color_t(1.0f - color.r, 1.0f - color.g, 1.0f - color.b, color.a);
	}

	constexpr inline color_t contrasting(color_t const& color)
	{
		return color_t(fmod(color.r + 0.5f, 1.0f), fmod(color.g + 0.5f, 1.0f), fmod(color.b + 0.5f, 1.0f), color.a);
	}

	namespace detail 
	{
		constexpr inline float byte_to_float(uint32_t byte) { return (byte & 0xFF) / 255.0f; } 

		template <typename T> constexpr inline const T& min3(const T& x, const T& y, const T& z) { return ((y) <= (z) ? ((x) <= (y) ? (x) : (y)) : ((x) <= (z) ? (x) : (z))); }
		template <typename T> constexpr inline const T& max3(const T& x, const T& y, const T& z) { return ((y) >= (z) ? ((x) >= (y) ? (x) : (y)) : ((x) >= (z) ? (x) : (z))); }
	}

	constexpr inline color_t from_u32_rgb(uint32_t rgb) { return color_t(detail::byte_to_float(rgb >> 16), detail::byte_to_float(rgb >> 8), detail::byte_to_float(rgb), 1.0f); }
	constexpr inline color_t from_u32_bgr(uint32_t rgb) { return color_t(detail::byte_to_float(rgb), detail::byte_to_float(rgb >> 8), detail::byte_to_float(rgb >> 16), 1.0f); }
	constexpr inline color_t from_u32_rgba(uint32_t rgb) { return color_t(detail::byte_to_float(rgb >> 24), detail::byte_to_float(rgb >> 16), detail::byte_to_float(rgb >> 8), detail::byte_to_float(rgb)); }
	constexpr inline color_t from_u32_bgra(uint32_t rgb) { return color_t(detail::byte_to_float(rgb >> 8), detail::byte_to_float(rgb >> 16), detail::byte_to_float(rgb >> 24), detail::byte_to_float(rgb)); }
	constexpr inline color_t from_u32_argb(uint32_t rgb) { return color_t(detail::byte_to_float(rgb >> 16), detail::byte_to_float(rgb >> 8), detail::byte_to_float(rgb), detail::byte_to_float(rgb >> 24)); }
	constexpr inline color_t from_u32_abgr(uint32_t rgb) { return color_t(detail::byte_to_float(rgb), detail::byte_to_float(rgb >> 8), detail::byte_to_float(rgb >> 16), detail::byte_to_float(rgb >> 24)); }

	constexpr inline uint32_t to_u32_argb(color_t const& rgba) { return uint32_t(rgba.a * 255.0f) << 24 | uint32_t(rgba.r * 255.0f) << 16 | uint32_t(rgba.g * 255.0f) << 8 | uint32_t(rgba.b * 255.0f); }
	constexpr inline uint32_t to_u32_abgr(color_t const& rgba) { return uint32_t(rgba.a * 255.0f) << 24 | uint32_t(rgba.b * 255.0f) << 16 | uint32_t(rgba.g * 255.0f) << 8 | uint32_t(rgba.r * 255.0f); }
	constexpr inline uint32_t to_u32_rgba(color_t const& rgba) { return uint32_t(rgba.r * 255.0f) << 24 | uint32_t(rgba.g * 255.0f) << 16 | uint32_t(rgba.b * 255.0f) << 8 | uint32_t(rgba.a * 255.0f); }
	constexpr inline uint32_t to_u32_bgra(color_t const& rgba) { return uint32_t(rgba.b * 255.0f) << 24 | uint32_t(rgba.g * 255.0f) << 16 | uint32_t(rgba.r * 255.0f) << 8 | uint32_t(rgba.a * 255.0f); }
	constexpr inline uint32_t to_u32_rgb(color_t const& rgba) { return uint32_t(rgba.r * 255.0f) << 16 | uint32_t(rgba.g * 255.0f) << 8 | uint32_t(rgba.b * 255.0f); }
	constexpr inline uint32_t to_u32_bgr(color_t const& rgba) { return uint32_t(rgba.b * 255.0f) << 16 | uint32_t(rgba.g * 255.0f) << 8 | uint32_t(rgba.r * 255.0f); }

	constexpr inline color_t to_rgb(color_hsva_t const& hsva)
	{
		auto hue = hsva->r, saturation = hsva->g, value = hsva->b, alpha = hsva->a;
		const int i = (int)hue;
		float fraction = hue - (float)i;
		if (!(i & 1)) fraction = 1.0f - fraction;
		float m = value * (1.0f - saturation);
		float n = value * (1.0f - saturation * fraction);
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

	constexpr inline color_hsva_t to_hsv(color_t const& rgba)
	{
		auto min = detail::min3(rgba.r, rgba.g, rgba.b);
		auto max = detail::max3(rgba.r, rgba.g, rgba.b);
		auto delta = max - min;

		float h = 0;

		if (delta != 0)
		{
			if (rgba.r == max)
				h = (rgba.g - rgba.b) / delta;
			else if (rgba.g == max)
				h = 2 + (rgba.b - rgba.r) / delta;
			else
				h = 4 + (rgba.r - rgba.g) / delta;
			h /= 6.0;
			if (h < 0)
				h += 1.0;
		}

		return color_hsva_t{
			h,
			(max != 0) ? (delta / max) : 0,
			max,
			rgba.a
		};
	}

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
		static Color Multiply(const Color& c1, const Color& c2 ) { return Color(c1.R*c2.R, c1.G*c2.G, c1.B*c2.B); }
		static Color Screen(const Color& c1, const Color& c2) { return Multiply(c1.Inverted(), c2.Inverted()).Inverted(); }
		static Color Overlay(const Color& c1, const Color& c2) {
			return Color(
				c2.R < 0.5f ? c1.R*c2.R * 2 : (1 - 2 * (1 - c2.R)*(1 - c1.R)),
				c2.G < 0.5f ? c1.G*c2.G * 2 : (1 - 2 * (1 - c2.G)*(1 - c1.G)),
				c2.B < 0.5f ? c1.B*c2.B * 2 : (1 - 2 * (1 - c2.B)*(1 - c1.B)));
		}
		static Color HardLight(const Color& c2, const Color& c1) {
			return Color(
				c2.R < 0.5f ? c1.R*c2.R * 2 : (1 - 2 * (1 - c2.R)*(1 - c1.R)),
				c2.G < 0.5f ? c1.G*c2.G * 2 : (1 - 2 * (1 - c2.G)*(1 - c1.G)),
				c2.B < 0.5f ? c1.B*c2.B * 2 : (1 - 2 * (1 - c2.B)*(1 - c1.B)));
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
		//Color BlendMultiply(const Color& other) const { return BlendWithAlpha(other, Color::Multiply); }

		static Color FromHTML(StringView str)
		{
			str.Consume('#');
			if (str.size() == 6)
			{
				uint32_t hex = StringToUnsignedLong(str, nullptr, 16);
				return Color::FromRGBInt(hex);
			}
			else if (str.size() == 8)
			{
				uint32_t hex = StringToUnsignedLong(str, nullptr, 16);
				return Color::FromRGBAInt(hex);
			}
			else if (str.size() == 3)
			{
				uint32_t hex = StringToUnsignedLong(str, nullptr, 16);
				hex = ((hex << 12) & 0xF00000) | ((hex << 8) & 0xF000) | ((hex << 4) & 0xF0);
				hex = hex | (hex >> 4);
				return Color::FromRGBInt(hex);
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