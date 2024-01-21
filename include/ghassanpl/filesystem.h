/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"
#include <filesystem>
#include "expected.h"

namespace ghassanpl::fs
{
	/// Github Copilot was worth it for this file alone

	namespace stdfs = std::filesystem;

	using stdfs::path;
	using stdfs::file_status;
	using stdfs::file_type;
	using stdfs::file_time_type;

	[[nodiscard]] inline auto absolute(const stdfs::path& path) -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::absolute(path, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto canonical(const stdfs::path& path) -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::canonical(path, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto weakly_canonical(const stdfs::path& path) -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::weakly_canonical(path, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto relative(const stdfs::path& p) -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::relative(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto relative(const stdfs::path& p, const stdfs::path& base) -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::relative(p, base, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto proximate(const stdfs::path& p) -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::proximate(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto proximate(const stdfs::path& p, const stdfs::path& base) -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::proximate(p, base, ec); if (ec) return unexpected(ec); return result; }
	              inline auto copy(const stdfs::path& from, const stdfs::path& to) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::copy(from, to, ec); if (ec) return unexpected(ec); return {}; }
	              inline auto copy(const stdfs::path& from, const stdfs::path& to, stdfs::copy_options options) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::copy(from, to, options, ec); if (ec) return unexpected(ec); return {}; }
	              inline auto copy_file(const stdfs::path& from, const stdfs::path& to) -> expected<uintmax_t, std::error_code> { std::error_code ec{}; auto result = stdfs::copy_file(from, to, ec); if (ec) return unexpected(ec); return result; }
	              inline auto copy_file(const stdfs::path& from, const stdfs::path& to, stdfs::copy_options options) -> expected<uintmax_t, std::error_code> { std::error_code ec{}; auto result = stdfs::copy_file(from, to, options, ec); if (ec) return unexpected(ec); return result; }
	              inline auto copy_symlink(const stdfs::path& from, const stdfs::path& to) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::copy_symlink(from, to, ec); if (ec) return unexpected(ec); return {}; }
	              inline auto create_directory(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::create_directory(p, ec); if (ec) return unexpected(ec); return result; }
	              inline auto create_directory(const stdfs::path& p, const stdfs::path& existing_p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::create_directory(p, existing_p, ec); if (ec) return unexpected(ec); return result; }
	              inline auto create_directories(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::create_directories(p, ec); if (ec) return unexpected(ec); return result; }
	              inline auto create_hard_link(const stdfs::path& target, const stdfs::path& link) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::create_hard_link(target, link, ec); if (ec) return unexpected(ec); return {}; }
	              inline auto create_symlink(const stdfs::path& target, const stdfs::path& link) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::create_symlink(target, link, ec); if (ec) return unexpected(ec); return {}; }
	              inline auto create_directory_symlink(const stdfs::path& target, const stdfs::path& link) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::create_directory_symlink(target, link, ec); if (ec) return unexpected(ec); return {}; }
	[[nodiscard]] inline auto current_path() -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::current_path(ec); if (ec) return unexpected(ec); return result; }
	              inline auto current_path(const stdfs::path& p) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::current_path(p, ec); if (ec) return unexpected(ec); return {}; }
	[[nodiscard]] inline auto exists(stdfs::file_status s) noexcept { return stdfs::exists(s); }
	[[nodiscard]] inline auto exists(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::exists(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto equivalent(const stdfs::path& p1, const stdfs::path& p2) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::equivalent(p1, p2, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto file_size(const stdfs::path& p) -> expected<uintmax_t, std::error_code> { std::error_code ec{}; auto result = stdfs::file_size(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto hard_link_count(const stdfs::path& p) -> expected<uintmax_t, std::error_code> { std::error_code ec{}; auto result = stdfs::hard_link_count(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto last_write_time(const stdfs::path& path) -> expected<stdfs::file_time_type, std::error_code> { std::error_code ec{}; auto result = stdfs::last_write_time(path, ec); if (ec) return unexpected(ec); return result; }
	              inline auto last_write_time(const stdfs::path& path, stdfs::file_time_type new_time) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::last_write_time(path, new_time, ec); if (ec) return unexpected(ec); return {}; }
	[[nodiscard]] inline auto permissions(const stdfs::path& p, stdfs::perms prms) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::permissions(p, prms, ec); if (ec) return unexpected(ec); return {}; }
	              inline auto permissions(const stdfs::path& p, stdfs::perms prms, stdfs::perm_options opts) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::permissions(p, prms, opts, ec); if (ec) return unexpected(ec); return {}; }
	[[nodiscard]] inline auto read_symlink(const stdfs::path& p) -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::read_symlink(p, ec); if (ec) return unexpected(ec); return result; }
	              inline auto remove(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::remove(p, ec); if (ec) return unexpected(ec); return result; }
	              inline auto remove_all(const stdfs::path& p) -> expected<uintmax_t, std::error_code> { std::error_code ec{}; auto result = stdfs::remove_all(p, ec); if (ec) return unexpected(ec); return result; }
	              inline auto rename(const stdfs::path& from, const stdfs::path& to) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::rename(from, to, ec); if (ec) return unexpected(ec); return {}; }
	              inline auto resize_file(const stdfs::path& p, uintmax_t size) -> expected<void, std::error_code> { std::error_code ec{}; stdfs::resize_file(p, size, ec); if (ec) return unexpected(ec); return {}; }
	[[nodiscard]] inline auto space(const stdfs::path& p) -> expected<stdfs::space_info, std::error_code> { std::error_code ec{}; auto result = stdfs::space(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto status(const stdfs::path& p) -> expected<stdfs::file_status, std::error_code> { std::error_code ec{}; auto result = stdfs::status(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto status_known(stdfs::file_status s) noexcept { return stdfs::status_known(s); }
	[[nodiscard]] inline auto symlink_status(const stdfs::path& p) -> expected<stdfs::file_status, std::error_code> { std::error_code ec{}; auto result = stdfs::symlink_status(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto temp_directory_path() -> expected<stdfs::path, std::error_code> { std::error_code ec{}; auto result = stdfs::temp_directory_path(ec); if (ec) return unexpected(ec); return result; }

	[[nodiscard]] inline auto is_block_file(stdfs::file_status s) noexcept { return stdfs::is_block_file(s); }
	[[nodiscard]] inline auto is_character_file(stdfs::file_status s) noexcept { return stdfs::is_character_file(s); }
	[[nodiscard]] inline auto is_directory(stdfs::file_status s) noexcept { return stdfs::is_directory(s); }
	[[nodiscard]] inline auto is_fifo(stdfs::file_status s) noexcept { return stdfs::is_fifo(s); }
	[[nodiscard]] inline auto is_other(stdfs::file_status s) noexcept { return stdfs::is_other(s); }
	[[nodiscard]] inline auto is_regular_file(stdfs::file_status s) noexcept { return stdfs::is_regular_file(s); }
	[[nodiscard]] inline auto is_socket(stdfs::file_status s) noexcept { return stdfs::is_socket(s); }
	[[nodiscard]] inline auto is_symlink(stdfs::file_status s) noexcept { return stdfs::is_symlink(s); }

	[[nodiscard]] inline auto is_block_file(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::is_block_file(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto is_character_file(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::is_character_file(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto is_directory(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::is_directory(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto is_empty(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::is_empty(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto is_fifo(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::is_fifo(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto is_other(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::is_other(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto is_regular_file(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::is_regular_file(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto is_socket(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::is_socket(p, ec); if (ec) return unexpected(ec); return result; }
	[[nodiscard]] inline auto is_symlink(const stdfs::path& p) -> expected<bool, std::error_code> { std::error_code ec{}; auto result = stdfs::is_symlink(p, ec); if (ec) return unexpected(ec); return result; }


	/// As if `to /= p`
	inline void path_append(std::string& to, path const& p)
	{
#if 1
		static_assert(stdfs::path::preferred_separator < 128);

		if (p.is_absolute() || to.empty())
		{
			to = p.string();
			return;
		}

		while (!to.empty() && to.back() == stdfs::path::preferred_separator)
			to.pop_back();

		auto str = p.string();
		auto sv = std::string_view{ str };
		while (!sv.empty() && sv[0] == stdfs::path::preferred_separator)
			sv.remove_prefix(1);

		to += (char)stdfs::path::preferred_separator;
		to += sv;
#else
		to = (to / p).string(); /// Possibly slower but provably correct
#endif
	}
}
