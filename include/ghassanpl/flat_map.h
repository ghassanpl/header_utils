#pragma once

#include <version>

#if defined(__cpp_lib_flat_map)
#include <flat_map>
namespace ghassanpl
{
	template <typename K, typename V, typename C = std::less<K>>
	using flat_map = std::flat_map<K, V, C>;
	template <typename K, typename V, typename C = std::less<K>>
	using flat_map_preferred = std::flat_map<K, V, C>;
}
#else

#include <map>

namespace ghassanpl
{
	template <typename K, typename V, typename C = std::less<K>>
	using flat_map_preferred = std::map<K, V, C>;

#define GHPL_FLAT_MAP_IS_MAP 1
}

#if __has_include(<boost/container/flat_map.hpp>)

#include <boost/container/flat_map.hpp>
namespace ghassanpl
{
	using boost::flat_map;
}
#endif


#endif


#if defined(__cpp_lib_flat_set)
#include <flat_set>
namespace ghassanpl
{
	template <typename K, typename C = std::less<K>>
	using flat_set = std::flat_set<K, C>;
	template <typename K, typename C = std::less<K>>
	using flat_set_preferred = std::flat_set<K, C>;
}
#else

#include <set>

namespace ghassanpl
{
	template <typename K, typename C = std::less<K>>
	using flat_set_preferred = std::set<K, C>;

#define GHPL_FLAT_SET_IS_SET 1
}

#if __has_include(<boost/container/flat_set.hpp>)

#include <boost/container/flat_set.hpp>
namespace ghassanpl
{
	using boost::flat_set;
}
#endif


#endif

