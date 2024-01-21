#include "../../include/ghassanpl/min-cpp-version/cpp14.h"

#include "../../include/ghassanpl/cpp98/enum_flags.h"
#include "../../include/ghassanpl/cpp11/named.h"
#include "../../include/ghassanpl/cpp11/string_view.h"

#include "../../include/ghassanpl/align.h"
//#include "../../include/ghassanpl/assuming.h" /// std::format
//#include "../../include/ghassanpl/bits.h" /// <bit>
//#include "../../include/ghassanpl/bit_view.h" /// <bit>
//#include "../../include/ghassanpl/buffers.h" /// span, string_view, <bit>
//#include "../../include/ghassanpl/bytes.h" /// span, string_view, <bit>
//#include "../../include/ghassanpl/colors.h" /// <=>
//#include "../../include/ghassanpl/constexpr_math.h" /// concepts, <bit>
//#include "../../include/ghassanpl/containers.h" /// ranges, span
//#include "../../include/ghassanpl/di.h" /// concepts
//	#include "../../include/ghassanpl/di_impl.h"
#include "../../include/ghassanpl/enums.h"
//#include "../../include/ghassanpl/enum_flags.h" /// concepts, nodiscard
//	#include "../../include/ghassanpl/flag_bits.h"
//	#include "../../include/ghassanpl/flag_bits_v.h"
//#include "../../include/ghassanpl/eval.h" /// charconv, optional, ranges, concepts
#include "../../include/ghassanpl/expected.h"
#include "../../include/ghassanpl/filesystem.h"
//#include "../../include/ghassanpl/formats.h" /// ranges
#include "../../include/ghassanpl/functional.h"
//#include "../../include/ghassanpl/hashes.h" /// span, string_view, <bit>
#include "../../include/ghassanpl/interpolation.h"
//#include "../../include/ghassanpl/json_helpers.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/mmap.h" /// span, filesystem
//	#include "../../include/ghassanpl/mmap_impl.h"
//#include "../../include/ghassanpl/named.h" /// concepts, <=>
#include "../../include/ghassanpl/noise.h"
//#include "../../include/ghassanpl/parsing.h" /// charconv, optional, ranges, concepts
#include "../../include/ghassanpl/platform.h"
#include "../../include/ghassanpl/random.h"
//	#include "../../include/ghassanpl/random_geom.h" /// is_constant_evaluated
	#include "../../include/ghassanpl/random_seq.h"
//#include "../../include/ghassanpl/ranges.h" /// ranges
#include "../../include/ghassanpl/rec2.h"
#include "../../include/ghassanpl/regex.h"
#include "../../include/ghassanpl/scope.h"
//#include "../../include/ghassanpl/sexps.h" /// charconv, optional, ranges, concepts
#include "../../include/ghassanpl/soptional.h"
//#include "../../include/ghassanpl/source_location.h" /// source_location obv
//#include "../../include/ghassanpl/stringification.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/string_interpolate.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/string_ops.h" /// concepts
//#include "../../include/ghassanpl/symbol.h" /// concepts, <=>, could work in C++17 but I'm lazy, according to Copilot
//#include "../../include/ghassanpl/templates.h" /// concepts
#include "../../include/ghassanpl/threading.h"
//#include "../../include/ghassanpl/unicode.h" /// stringops
//#include "../../include/ghassanpl/wilson.h" /// stringops
//#include "../../include/ghassanpl/with_sl.h" /// source_location, concepts
/* /// consteval
#include "../../include/ghassanpl/geometry/arcs.h"
#include "../../include/ghassanpl/geometry/direction.h"
#include "../../include/ghassanpl/geometry/ellipse.h"
#include "../../include/ghassanpl/geometry/geometry_common.h"
#include "../../include/ghassanpl/geometry/points.h"
#include "../../include/ghassanpl/geometry/polygon.h"
#include "../../include/ghassanpl/geometry/ray.h"
#include "../../include/ghassanpl/geometry/rectangles.h"
#include "../../include/ghassanpl/geometry/segment.h"
#include "../../include/ghassanpl/geometry/shape_concepts.h"
#include "../../include/ghassanpl/geometry/squares.h"
#include "../../include/ghassanpl/geometry/square_grid.h"
#include "../../include/ghassanpl/geometry/square_grid_algorithms.h"
#include "../../include/ghassanpl/geometry/triangles.h"
*/
#include "../../include/ghassanpl/windows/common.h"
#include "../../include/ghassanpl/windows/console.h"
#include "../../include/ghassanpl/windows/gfx.h"
#include "../../include/ghassanpl/windows/last_error.h"
#include "../../include/ghassanpl/windows/wm.h"