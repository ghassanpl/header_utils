/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#ifdef __cpp_consteval
#include "source_location.h"
#endif
#include "bytes.h"
#include "bits.h"
#include <array>
#include <bit>

namespace ghassanpl
{
	/// \defgroup Hashes Hashes
	/// Hashing functions.

	static constexpr inline uint32_t crc32_table[256] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	/// Calculates a CRC32 for a range of bytelikes. constexpr!
	/// \ingroup Hashes
	template <bytelike_range RANGE>
	[[nodiscard]] constexpr uint32_t crc32(RANGE&& bytes)
	{
		/// TODO: if (!is_constant_evaluated), we could use _mm_crc32_u8 & duff's - requires is_constant_evaluated and SSE4.2,
		/// and there are probably libraries for that already.
		uint32_t crc = 0xFFFFFFFFu;
		for (auto byte: bytes)
			crc = crc32_table[(crc ^ to_u8(byte)) & 0xFF] ^ (crc >> 8);
		return ~crc;
	}

	/// Calculates a CRC32 for a variadic number of bytelikes. constexpr!
	/// \ingroup Hashes
	template <bytelike... BYTES>
	[[nodiscard]] constexpr uint32_t crc32(BYTES... bytes)
	{
		uint32_t crc = 0xFFFFFFFFu;
		((crc = crc32_table[(crc ^ to_u8(bytes)) & 0xFF] ^ (crc >> 8)), ...);
		return ~crc;
	}

#ifdef __cpp_consteval
	/// Calculates a CRC32 of a source_location (constexpr, so can be used at compile time)
	/// \ingroup Hashes
	[[nodiscard]] constexpr auto crc32(const source_location& k)
	{
		return crc32(std::string_view{ k.file_name() }) ^ k.line() ^ k.column();
	}
#endif

	static constexpr inline uint64_t crc64_table[256] = {
		0x0000000000000000, 0x42F0E1EBA9EA3693, 0x85E1C3D753D46D26, 0xC711223CFA3E5BB5, 0x493366450E42ECDF, 0x0BC387AEA7A8DA4C, 0xCCD2A5925D9681F9, 0x8E224479F47CB76A, 
		0x9266CC8A1C85D9BE, 0xD0962D61B56FEF2D, 0x17870F5D4F51B498, 0x5577EEB6E6BB820B, 0xDB55AACF12C73561, 0x99A54B24BB2D03F2, 0x5EB4691841135847, 0x1C4488F3E8F96ED4, 
		0x663D78FF90E185EF, 0x24CD9914390BB37C, 0xE3DCBB28C335E8C9, 0xA12C5AC36ADFDE5A, 0x2F0E1EBA9EA36930, 0x6DFEFF5137495FA3, 0xAAEFDD6DCD770416, 0xE81F3C86649D3285, 
		0xF45BB4758C645C51, 0xB6AB559E258E6AC2, 0x71BA77A2DFB03177, 0x334A9649765A07E4, 0xBD68D2308226B08E, 0xFF9833DB2BCC861D, 0x388911E7D1F2DDA8, 0x7A79F00C7818EB3B, 
		0xCC7AF1FF21C30BDE, 0x8E8A101488293D4D, 0x499B3228721766F8, 0x0B6BD3C3DBFD506B, 0x854997BA2F81E701, 0xC7B97651866BD192, 0x00A8546D7C558A27, 0x4258B586D5BFBCB4, 
		0x5E1C3D753D46D260, 0x1CECDC9E94ACE4F3, 0xDBFDFEA26E92BF46, 0x990D1F49C77889D5, 0x172F5B3033043EBF, 0x55DFBADB9AEE082C, 0x92CE98E760D05399, 0xD03E790CC93A650A,
		0xAA478900B1228E31, 0xE8B768EB18C8B8A2, 0x2FA64AD7E2F6E317, 0x6D56AB3C4B1CD584, 0xE374EF45BF6062EE, 0xA1840EAE168A547D, 0x66952C92ECB40FC8, 0x2465CD79455E395B, 
		0x3821458AADA7578F, 0x7AD1A461044D611C, 0xBDC0865DFE733AA9, 0xFF3067B657990C3A, 0x711223CFA3E5BB50, 0x33E2C2240A0F8DC3, 0xF4F3E018F031D676, 0xB60301F359DBE0E5, 
		0xDA050215EA6C212F, 0x98F5E3FE438617BC, 0x5FE4C1C2B9B84C09, 0x1D14202910527A9A, 0x93366450E42ECDF0, 0xD1C685BB4DC4FB63, 0x16D7A787B7FAA0D6, 0x5427466C1E109645,
		0x4863CE9FF6E9F891, 0x0A932F745F03CE02, 0xCD820D48A53D95B7, 0x8F72ECA30CD7A324, 0x0150A8DAF8AB144E, 0x43A04931514122DD, 0x84B16B0DAB7F7968, 0xC6418AE602954FFB,
		0xBC387AEA7A8DA4C0, 0xFEC89B01D3679253, 0x39D9B93D2959C9E6, 0x7B2958D680B3FF75, 0xF50B1CAF74CF481F, 0xB7FBFD44DD257E8C, 0x70EADF78271B2539, 0x321A3E938EF113AA,
		0x2E5EB66066087D7E, 0x6CAE578BCFE24BED, 0xABBF75B735DC1058, 0xE94F945C9C3626CB, 0x676DD025684A91A1, 0x259D31CEC1A0A732, 0xE28C13F23B9EFC87, 0xA07CF2199274CA14,
		0x167FF3EACBAF2AF1, 0x548F120162451C62, 0x939E303D987B47D7, 0xD16ED1D631917144, 0x5F4C95AFC5EDC62E, 0x1DBC74446C07F0BD, 0xDAAD56789639AB08, 0x985DB7933FD39D9B,
		0x84193F60D72AF34F, 0xC6E9DE8B7EC0C5DC, 0x01F8FCB784FE9E69, 0x43081D5C2D14A8FA, 0xCD2A5925D9681F90, 0x8FDAB8CE70822903, 0x48CB9AF28ABC72B6, 0x0A3B7B1923564425, 
		0x70428B155B4EAF1E, 0x32B26AFEF2A4998D, 0xF5A348C2089AC238, 0xB753A929A170F4AB, 0x3971ED50550C43C1, 0x7B810CBBFCE67552, 0xBC902E8706D82EE7, 0xFE60CF6CAF321874, 
		0xE224479F47CB76A0, 0xA0D4A674EE214033, 0x67C58448141F1B86, 0x253565A3BDF52D15, 0xAB1721DA49899A7F, 0xE9E7C031E063ACEC, 0x2EF6E20D1A5DF759, 0x6C0603E6B3B7C1CA, 
		0xF6FAE5C07D3274CD, 0xB40A042BD4D8425E, 0x731B26172EE619EB, 0x31EBC7FC870C2F78, 0xBFC9838573709812, 0xFD39626EDA9AAE81, 0x3A28405220A4F534, 0x78D8A1B9894EC3A7, 
		0x649C294A61B7AD73, 0x266CC8A1C85D9BE0, 0xE17DEA9D3263C055, 0xA38D0B769B89F6C6, 0x2DAF4F0F6FF541AC, 0x6F5FAEE4C61F773F, 0xA84E8CD83C212C8A, 0xEABE6D3395CB1A19, 
		0x90C79D3FEDD3F122, 0xD2377CD44439C7B1, 0x15265EE8BE079C04, 0x57D6BF0317EDAA97, 0xD9F4FB7AE3911DFD, 0x9B041A914A7B2B6E, 0x5C1538ADB04570DB, 0x1EE5D94619AF4648, 
		0x02A151B5F156289C, 0x4051B05E58BC1E0F, 0x87409262A28245BA, 0xC5B073890B687329, 0x4B9237F0FF14C443, 0x0962D61B56FEF2D0, 0xCE73F427ACC0A965, 0x8C8315CC052A9FF6, 
		0x3A80143F5CF17F13, 0x7870F5D4F51B4980, 0xBF61D7E80F251235, 0xFD913603A6CF24A6, 0x73B3727A52B393CC, 0x31439391FB59A55F, 0xF652B1AD0167FEEA, 0xB4A25046A88DC879,
		0xA8E6D8B54074A6AD, 0xEA16395EE99E903E, 0x2D071B6213A0CB8B, 0x6FF7FA89BA4AFD18, 0xE1D5BEF04E364A72, 0xA3255F1BE7DC7CE1, 0x64347D271DE22754, 0x26C49CCCB40811C7, 
		0x5CBD6CC0CC10FAFC, 0x1E4D8D2B65FACC6F, 0xD95CAF179FC497DA, 0x9BAC4EFC362EA149, 0x158E0A85C2521623, 0x577EEB6E6BB820B0, 0x906FC95291867B05, 0xD29F28B9386C4D96, 
		0xCEDBA04AD0952342, 0x8C2B41A1797F15D1, 0x4B3A639D83414E64, 0x09CA82762AAB78F7, 0x87E8C60FDED7CF9D, 0xC51827E4773DF90E, 0x020905D88D03A2BB, 0x40F9E43324E99428, 
		0x2CFFE7D5975E55E2, 0x6E0F063E3EB46371, 0xA91E2402C48A38C4, 0xEBEEC5E96D600E57, 0x65CC8190991CB93D, 0x273C607B30F68FAE, 0xE02D4247CAC8D41B, 0xA2DDA3AC6322E288, 
		0xBE992B5F8BDB8C5C, 0xFC69CAB42231BACF, 0x3B78E888D80FE17A, 0x7988096371E5D7E9, 0xF7AA4D1A85996083, 0xB55AACF12C735610, 0x724B8ECDD64D0DA5, 0x30BB6F267FA73B36, 
		0x4AC29F2A07BFD00D, 0x08327EC1AE55E69E, 0xCF235CFD546BBD2B, 0x8DD3BD16FD818BB8, 0x03F1F96F09FD3CD2, 0x41011884A0170A41, 0x86103AB85A2951F4, 0xC4E0DB53F3C36767, 
		0xD8A453A01B3A09B3, 0x9A54B24BB2D03F20, 0x5D45907748EE6495, 0x1FB5719CE1045206, 0x919735E51578E56C, 0xD367D40EBC92D3FF, 0x1476F63246AC884A, 0x568617D9EF46BED9, 
		0xE085162AB69D5E3C, 0xA275F7C11F7768AF, 0x6564D5FDE549331A, 0x279434164CA30589, 0xA9B6706FB8DFB2E3, 0xEB46918411358470, 0x2C57B3B8EB0BDFC5, 0x6EA7525342E1E956, 
		0x72E3DAA0AA188782, 0x30133B4B03F2B111, 0xF7021977F9CCEAA4, 0xB5F2F89C5026DC37, 0x3BD0BCE5A45A6B5D, 0x79205D0E0DB05DCE, 0xBE317F32F78E067B, 0xFCC19ED95E6430E8,
		0x86B86ED5267CDBD3, 0xC4488F3E8F96ED40, 0x0359AD0275A8B6F5, 0x41A94CE9DC428066, 0xCF8B0890283E370C, 0x8D7BE97B81D4019F, 0x4A6ACB477BEA5A2A, 0x089A2AACD2006CB9, 
		0x14DEA25F3AF9026D, 0x562E43B4931334FE, 0x913F6188692D6F4B, 0xD3CF8063C0C759D8, 0x5DEDC41A34BBEEB2, 0x1F1D25F19D51D821, 0xD80C07CD676F8394, 0x9AFCE626CE85B507,
	};

	/// Calculates a CRC4 for a range of bytelikes
	/// \ingroup Hashes
	template <bytelike_range RANGE>
	[[nodiscard]] constexpr uint64_t crc64(RANGE&& bytes)
	{
		uint64_t crc = 0;
		for (auto byte : bytes)
			crc = crc64_table[crc >> 56] ^ ((crc << 8U) ^ to_u8(byte));
		return ~crc;
	}

	/// Calculates a CRC4 for a variadic number of bytelikes
	/// \ingroup Hashes
	template <bytelike... BYTES>
	[[nodiscard]] constexpr uint64_t crc64(BYTES... bytes)
	{
		uint64_t crc = 0;
		((crc = crc64_table[crc >> 56] ^ ((crc << 8U) ^ to_u8(bytes))), ...);
		return ~crc;
	}

#ifdef __cpp_consteval
	/// Calculates a CRC64 of a source_location (constexpr, so can be used at compile time)
	/// \ingroup Hashes
	[[nodiscard]] constexpr auto crc64(const source_location& k)
	{
		return crc64(std::string_view{ k.file_name() }) ^ k.line() ^ k.column();
	}

	struct crc64_hasher
	{
		constexpr uint64_t operator()(source_location const& l) const { return crc64(l); }
	};
#endif

	/// TODO: Add support for non-64bit hashes to all the functions below, especially since
	/// std::hash operates on size_t

	/// Calculates a FNV Hash for a range of bytes
	/// \ingroup Hashes
	template <bytelike_range RANGE>
	[[nodiscard]] constexpr uint64_t fnv64(RANGE&& bytes)
	{
		uint64_t result = 0xcbf29ce484222325;
		for (auto byte : bytes)
			result = (result ^ to_u8(byte)) * 0x00000100000001b3U;
		return result;
	}

	/// Calculates a FNV Hash for a variadic number of bytes
	/// \ingroup Hashes
	template <bytelike... BYTES>
	[[nodiscard]] constexpr uint64_t fnv64(BYTES... bytes)
	{
		uint64_t result = 0xcbf29ce484222325;
		((result = (result ^ to_u8(bytes)) * 0x00000100000001b3U), ...);
		return result;
	}

	namespace integer
	{
		struct splitmix64_state { uint64_t state{}; };
		/// 
		[[nodiscard]] constexpr uint64_t splitmix64(splitmix64_state& state, uint64_t stream_id = 0x9e3779b97f4a7c15)
		{
			state.state = state.state + stream_id;
			uint64_t z = state.state;
			z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
			z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
			z = z ^ (z >> 31);
			return z;
		}

		///
		[[nodiscard]] constexpr uint64_t splitmix64(uint64_t seed, uint64_t index, uint64_t stream_id = 0x9e3779b97f4a7c15)
		{
			uint64_t z = seed + index * stream_id;
			z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
			z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
			z = z ^ (z >> 31);
			return z;
		}

		///
		[[nodiscard]] constexpr uint64_t murmurhash64(uint64_t k)
		{
			k ^= k >> 33;
			k *= 0xff51afd7ed558ccdULL;
			k ^= k >> 33;
			k *= 0xc4ceb9fe1a85ec53ULL;
			k ^= k >> 33;
			return k;
		}

		///
		[[nodiscard]] constexpr uint64_t murmurhash64_2(uint64_t x, uint64_t y)
		{
			constexpr auto kMul = 0x9ddfea08eb382d69ULL;
			auto a = (x ^ y) * kMul;
			a ^= (a >> 47);
			auto b = (y ^ a) * kMul;
			b ^= (b >> 47);
			b *= kMul;
			return b;
		}

		[[nodiscard]] constexpr uint32_t triple32(uint32_t x)
		{
			x ^= x >> 17;
			x *= 0xed5ad4bbU;
			x ^= x >> 11;
			x *= 0xac4c1b51U;
			x ^= x >> 15;
			x *= 0x31848babU;
			x ^= x >> 14;
			return x;
		}
	}

	constexpr void fold_in_hash64(uint64_t& hash1, uint64_t hash2) noexcept
	{
		constexpr uint64_t kMul = 0x9ddfea08eb382d69ULL;
		const auto seed = hash1;
		uint64_t a = (hash2 ^ seed) * kMul;
		a ^= (a >> 47);
		uint64_t b = (seed ^ a) * kMul;
		b ^= (b >> 47);
		b *= kMul;
		hash1 = b;
	}

	template <typename T>
	requires 
		(sizeof(T) <= sizeof(uint64_t)) && 
		((sizeof(T) & (sizeof(T) - 1)) == 0) && /// Power of 2
		std::is_trivially_copyable_v<T>
	[[nodiscard]] consteval uint64_t ce_hash64(T val) noexcept
	{
		static_assert(!std::is_pointer_v<T>, "cannot hash pointers at compile-time");
		if constexpr (std::floating_point<T>)
		{
			if (val == T{ 0 }) val = T{ 0 }; /// -0 == 0
		}
		const auto as_int = std::bit_cast<uintN_t<bit_count<T>>>(val);
		const auto as_array = to_u8_array(as_int);
		return fnv64(as_array);
	}

	template <bytelike T>
	[[nodiscard]] consteval uint64_t ce_hash64(std::basic_string_view<T> const& val) noexcept { return fnv64(val); }

	template <typename FIRST, typename... T>
	requires (sizeof...(T) > 0)
	[[nodiscard]] consteval uint64_t ce_hash64(FIRST const& first, T const&... values)
	{
		uint64_t result = ce_hash64(first);
		(fold_in_hash64(result, ce_hash64(values)), ...);
		return result;
	}

	template <typename TUPLE_TYPE, uint64_t... Is>
	consteval void ce_hash64_tuple_elements(uint64_t& hash, TUPLE_TYPE const& t, std::index_sequence<Is...>)
	{
		(fold_in_hash64(hash, ce_hash64(std::get<Is>(t))), ...);
	}

	template <typename... T>
	[[nodiscard]] consteval uint64_t ce_hash64(std::tuple<T...> const& tupl) noexcept
	{
		uint64_t result = 0;
		ce_hash64_tuple_elements(result, tupl, std::index_sequence_for<T...>{});
		return result;
	}

	/// TODO: ce_hash64(array/span)
	/// TODO: ce_hash64(thread::id) ?
	/// TODO: ce_hash64(optional/variant) ?

	/// Combines an existing hash value (`seed`) with the hash of value `v`
	/// \ingroup Hashes
	template <typename T, typename HASHER = std::hash<std::remove_cvref_t<T>>>
	constexpr void hash64_combine_to(uint64_t& seed, T&& v, HASHER&& hasher = HASHER{})
	{
		auto result = hasher(v);
		static_assert(std::is_same_v<decltype(result), uint64_t>, "hasher() must return a uint64_t");
		fold_in_hash64(seed, result);
	}

	template <template<typename> typename HASHER = std::hash, typename FIRST, typename... T>
	[[nodiscard]] constexpr uint64_t hash64(FIRST const& first, T&&... values)
	{
		auto hasher = HASHER<FIRST>{};
		
		static_assert(
			std::same_as<decltype(std::declval<HASHER<std::remove_cvref_t<FIRST>>>()(std::declval<FIRST>())), uint64_t> &&
			(std::same_as<decltype(std::declval<HASHER<std::remove_cvref_t<T>>>()(std::declval<T>())), uint64_t> && ... && true),
			"hasher() must return a uint64_t for each type");

		uint64_t result = hasher(first);
		(ghassanpl::hash64_combine_to(result, std::forward<T>(values), HASHER<std::remove_cvref_t<T>>{}), ...);
		return result;
	}

	/// Combines an existing hash value (`seed`) with the hash of a range of values
	/// \ingroup Hashes
	template<typename It, typename HASHER = std::hash<std::iter_value_t<It>>>
	constexpr void hash64_range(uint64_t& seed, It first, It last, HASHER const& hasher = {})
	{
		static_assert(std::same_as<decltype(hasher(std::declval<std::iter_value_t<It>>())), uint64_t>, "hasher() must return a uint64_t");
		for (; first != last; ++first)
			hash64_combine_to(seed, *first, hasher);
	}

	/// Hashes a range of values
	/// \ingroup Hashes
	template<typename It, typename HASHER = std::hash<std::iter_value_t<It>>>
	[[nodiscard]] constexpr uint64_t hash64_range(It first, It last, HASHER&& hasher = {})
	{
		static_assert(std::same_as<decltype(hasher(std::declval<std::iter_value_t<It>>())), uint64_t>, "hasher() must return a uint64_t");
		uint64_t seed = 0;
		hash64_range(seed, first, last, std::forward<HASHER>(hasher));
		return seed;
	}

	/// Hashes a range of values
	/// \ingroup Hashes
	template <std::ranges::range T, typename HASHER = std::hash<std::ranges::range_value_t<T>>>
	[[nodiscard]] constexpr uint64_t hash64_range(T range, HASHER&& hasher = {})
	{
		static_assert(std::same_as<decltype(hasher(std::declval<std::ranges::range_value_t<T>>())), uint64_t>, "hasher() must return a uint64_t");
		uint64_t seed = 0;
		hash64_range(seed, std::ranges::begin(range), std::ranges::end(range), std::forward<HASHER>(hasher));
		return seed;
	}
}