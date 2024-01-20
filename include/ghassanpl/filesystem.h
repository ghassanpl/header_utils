/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include "expected.h"

namespace ghassanpl::fs
{
    namespace stdfs = std::filesystem;

    using stdfs::path;
    using stdfs::file_status;
    using stdfs::file_type;
    using stdfs::file_time_type;

#define GHPL_CALL_WEC(fname) []<typename... ARGS>(ARGS&&... args) { return fname(std::forward<ARGS>(args)...); }

    inline auto absolute(const stdfs::path& path) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::absolute), path); }
    inline auto canonical(const stdfs::path& path) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::canonical), path); }
    inline auto weakly_canonical(const stdfs::path& path) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::weakly_canonical), path); }
    inline auto relative(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::relative), p); }
    inline auto relative(const stdfs::path& p, const stdfs::path& base = stdfs::current_path()) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::relative), p, base); }
    inline auto proximate(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::proximate), p); }
    inline auto proximate(const stdfs::path& p, const stdfs::path& base = stdfs::current_path()) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::proximate), p, base); }
    inline auto copy(const stdfs::path& from, const stdfs::path& to) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::copy), from, to); }
    inline auto copy(const stdfs::path& from, const stdfs::path& to, stdfs::copy_options options) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::copy), from, to, options); }
    inline auto copy_file(const stdfs::path& from, const stdfs::path& to) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::copy_file), from, to); }
    inline auto copy_file(const stdfs::path& from, const stdfs::path& to, stdfs::copy_options options) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::copy_file), from, to, options); }
    /// This is supposed to be noexcept but msvc's impl isn't
    inline auto copy_symlink(const stdfs::path& from, const stdfs::path& to) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::copy_symlink), from, to); }
    inline auto create_directory(const stdfs::path& p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::create_directory), p); }
    inline auto create_directory(const stdfs::path& p, const stdfs::path& existing_p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::create_directory), p, existing_p); }
    inline auto create_directories(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::create_directories), p); }
    inline auto create_hard_link(const stdfs::path& target, const stdfs::path& link) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::create_hard_link), target, link); }
    inline auto create_symlink(const stdfs::path& target, const stdfs::path& link) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::create_symlink), target, link); }
    inline auto create_directory_symlink(const stdfs::path& target, const stdfs::path& link) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::create_directory_symlink), target, link); }
    inline auto current_path() { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::current_path)); }
    inline auto current_path(const stdfs::path& p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::current_path), p); }
    inline bool exists(stdfs::file_status s) noexcept { return stdfs::exists(s); }
    inline auto exists(const stdfs::path& p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::exists), p); }
    inline auto equivalent(const stdfs::path& p1, const stdfs::path& p2) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::equivalent), p1, p2); }
    inline auto file_size(const stdfs::path& p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::file_size), p); }
    inline auto hard_link_count(const stdfs::path& p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::hard_link_count), p); }
    inline auto last_write_time(const stdfs::path& path) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::last_write_time), path); }
    inline auto last_write_time(const stdfs::path& path, stdfs::file_time_type new_time) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::last_write_time), path, new_time); }
    inline auto permissions(const stdfs::path& p, stdfs::perms prms) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::permissions), p, prms); }
    inline auto permissions(const stdfs::path& p, stdfs::perms prms, stdfs::perm_options opts = stdfs::perm_options::replace) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::permissions), p, prms, opts); }
    inline auto read_symlink(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::read_symlink), p); }
    inline auto remove(const stdfs::path& p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::remove), p); }
    inline auto remove_all(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::remove_all), p); }
    inline auto rename(const stdfs::path& from, const stdfs::path& to) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::rename), from, to); }
    inline auto resize_file(const stdfs::path& p, uintmax_t size) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::resize_file), p, size); }
    inline auto space(const stdfs::path& p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::space), p); }
    inline auto status(const stdfs::path& p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::status), p); }
    inline auto status_known(stdfs::file_status s) noexcept { return stdfs::status_known(s); }
    inline auto symlink_status(const stdfs::path& p) noexcept { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::symlink_status), p); }
    inline auto temp_directory_path() { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::temp_directory_path)); }

    inline auto is_block_file(stdfs::file_status s) noexcept { return stdfs::is_block_file(s); }
    inline auto is_block_file(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::is_block_file), p); }
    inline auto is_character_file(stdfs::file_status s) noexcept { return stdfs::is_character_file(s); }
    inline auto is_character_file(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::is_character_file), p); }
    inline auto is_directory(stdfs::file_status s) noexcept { return stdfs::is_directory(s); }
    inline auto is_directory(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::is_directory), p); }
    inline auto is_empty(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::is_empty), p); }
    inline auto is_fifo(stdfs::file_status s) noexcept { return stdfs::is_fifo(s); }
    inline auto is_fifo(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::is_fifo), p); }
    inline auto is_other(stdfs::file_status s) noexcept { return stdfs::is_other(s); }
    inline auto is_other(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::is_other), p); }
    inline auto is_regular_file(stdfs::file_status s) noexcept { return stdfs::is_regular_file(s); }
    inline auto is_regular_file(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::is_regular_file), p); }
    inline auto is_socket(stdfs::file_status s) noexcept { return stdfs::is_socket(s); }
    inline auto is_socket(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::is_socket), p); }
    inline auto is_symlink(stdfs::file_status s) noexcept { return stdfs::is_symlink(s); }
    inline auto is_symlink(const stdfs::path& p) { return call_with_expected_ec(GHPL_CALL_WEC(stdfs::is_symlink), p); }

#undef GHPL_CALL_WEC
}
