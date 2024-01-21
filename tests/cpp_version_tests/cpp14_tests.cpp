#include "../../include/ghassanpl/min-cpp-version/cpp14.h"

#include "../../include/ghassanpl/cpp98/enum_flags.h"
#include "../../include/ghassanpl/cpp11/named.h"
#include "../../include/ghassanpl/cpp11/string_view.h"

//#include "../../include/ghassanpl/align.h" /// Inline variables
//#include "../../include/ghassanpl/assuming.h" /// std::format
//#include "../../include/ghassanpl/bits.h" /// <bit>
//#include "../../include/ghassanpl/bit_view.h" /// bit
//#include "../../include/ghassanpl/buffers.h" /// span, string_view, <bit>
//#include "../../include/ghassanpl/bytes.h" /// span, string_view, <bit>
//#include "../../include/ghassanpl/colors.h" /// charconv, string_view
//#include "../../include/ghassanpl/constexpr_math.h" /// concepts, <bit>
//#include "../../include/ghassanpl/containers.h" /// ranges, span
//#include "../../include/ghassanpl/di.h" /// concepts
//	#include "../../include/ghassanpl/di_impl.h"
//#include "../../include/ghassanpl/enums.h" /// optional, string_view
//#include "../../include/ghassanpl/enum_flags.h" /// concepts, nodiscard
//	#include "../../include/ghassanpl/flag_bits.h"
//	#include "../../include/ghassanpl/flag_bits_v.h"
//#include "../../include/ghassanpl/eval.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/expected.h" /// invoke_result_t and whatever tl::expected needs
//#include "../../include/ghassanpl/filesystem.h" /// filesystem
//#include "../../include/ghassanpl/formats.h" /// charconv, optional, ranges
//#include "../../include/ghassanpl/functional.h" /// optional, ranges
//#include "../../include/ghassanpl/hashes.h" /// span, string_view, <bit>
#include "../../include/ghassanpl/interpolation.h"
//#include "../../include/ghassanpl/json_helpers.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/mmap.h" /// span, filesystem
//	#include "../../include/ghassanpl/mmap_impl.h"
//#include "../../include/ghassanpl/named.h" /// concepts, <=>
//#include "../../include/ghassanpl/noise.h" /// concepts
//#include "../../include/ghassanpl/parsing.h" /// charconv, optional, ranges, concepts
#include "../../include/ghassanpl/platform.h"
//#include "../../include/ghassanpl/random.h" /// concepts, span
//	#include "../../include/ghassanpl/random_geom.h"
//	#include "../../include/ghassanpl/random_seq.h"
//#include "../../include/ghassanpl/ranges.h" /// ranges
//#include "../../include/ghassanpl/rec2.h" /// span, concepts
//#include "../../include/ghassanpl/regex.h" /// string_view
//#include "../../include/ghassanpl/scope.h" /// concepts
//#include "../../include/ghassanpl/sexps.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/soptional.h" /// optional
//#include "../../include/ghassanpl/source_location.h" /// source_location obv
//#include "../../include/ghassanpl/stringification.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/string_interpolate.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/string_ops.h" /// concepts
//#include "../../include/ghassanpl/symbol.h" /// string_view
//#include "../../include/ghassanpl/templates.h" /// variant, optional, any
#include "../../include/ghassanpl/threading.h"
//#include "../../include/ghassanpl/unicode.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/wilson.h" /// charconv, optional, ranges, concepts
//#include "../../include/ghassanpl/with_sl.h" /// source_location
/* /// inline variables
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