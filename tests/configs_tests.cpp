#include "../include/ghassanpl/configs.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <gtest/gtest.h>
#undef isascii

using namespace ghassanpl::config;
using namespace glm;
using namespace nlohmann;

namespace glm
{
	template<typename T> void to_json(json& j, vec<2, T> const& v) { j = { v.x, v.y }; }
	template<typename T> void from_json(json const& j, vec<2, T>& v) { v.x = j[0]; v.y = j[1]; }
	template<typename T> void to_json(json& j, vec<3, T> const& v) { j = { v.x, v.y, v.z }; }
	template<typename T> void from_json(json const& j, vec<3, T>& v) { v.x = j[0]; v.y = j[1]; v.z = j[2]; }
	template<typename T> void to_json(json& j, vec<4, T> const& v) { j = { v.x, v.y, v.z, v.w }; }
	template<typename T> void from_json(json const& j, vec<4, T>& v) { v.x = j[0]; v.y = j[1]; v.z = j[2]; v.w = j[3]; }
}

namespace config
{
	cvar_group_t cvgRender{ "Render" };
	namespace Render
	{
		cvar_t<ivec2> cvWindowedResolution{ cvgRender, "WindowedResolution", {1280, 720} };
	}
}

TEST(configs_test, basics)
{

	config::Render::cvWindowedResolution.json();
	config::Render::cvWindowedResolution.json(json{});

}