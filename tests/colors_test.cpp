/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include "test_system.h"

#include "../include/ghassanpl/colors.h"
#include "../include/ghassanpl/geometry/geometry_common.h"

FunctionUnderTest(ghassanpl::color_cast)
{
	using namespace ghassanpl;
	using namespace glm;

	CheckingIfIt("converts colors between linear and u32 appropriately")
	{
		ItShould(ConvertBlack).WhenEqual(color_cast<color_rgba_u32_t>(colors::black).value, 0x000000FF);
		ItShould(ConvertRed).WhenEqual(color_cast<color_rgba_u32_t>(colors::red).value, 0xFF0000FF);
		ItShould(ConvertGreen).WhenEqual(color_cast<color_rgba_u32_t>(colors::green).value, 0x00FF00FF);
		ItShould(ConvertBlue).WhenEqual(color_cast<color_rgba_u32_t>(colors::blue).value, 0x0000FFFF);
		ItShould(ConvertTransparent).WhenEqual(color_cast<color_rgba_u32_t>(colors::transparent).value, 0);
	}
}

FunctionUnderTest(ghassanpl::to_hsv)
{
	using namespace ghassanpl;

	CheckingIfIt("returns sane values")
	{
		ItShould(ReturnDifferentValuesForGreenAndBlue).WhenNotEqual(to_hsv(colors::green), to_hsv(colors::blue));
	}

	CheckingIfIt("roundtrips correctly")
	{
		ItShouldForEachValue(RoundtripCorrectly, colors::white, colors::red, colors::blue, colors::green, colors::black, colors::transparent)
		{
			DoesRoundtripCorrectly.WhenEqual(to_rgb(to_hsv(Value)), Value);
		};
	}
}
