#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "../include/ghassanpl/uri.h"

using namespace nlohmann;

namespace ghassanpl
{
	void to_json(json& j, decomposed_uri const& dec)
	{
		j["scheme"] = dec.scheme;
		j["authority"] = dec.authority;
		j["user_info"] = dec.user_info;
		j["host"] = dec.host;
		j["port"] = dec.port;
		j["path"] = dec.path;
		if (dec.decompose_flags.contain(uri_decompose_flags::split_path_elements))
			j["path_elements"] = dec.path_elements;
		j["normalized_path"] = dec.normalized_path();
		j["query"] = dec.query;
		j["query_elements"] = dec.query_elements;
		j["fragment"] = dec.fragment;
	}
}

using namespace ghassanpl;

void uri_equal(uri_view view, decomposed_uri&& uri, json&& whats)
{
	auto requested = (json)uri;
	for (auto& kvp : requested.get<json::object_t>())
	{
		if (whats.contains(kvp.first))
			EXPECT_EQ(whats[kvp.first], kvp.second) << "when comparing field " << kvp.first << " of uri " << view;
		//else
			//EXPECT_EQ(std::string{ "" }, kvp.second) << "when comparing field " << kvp.first << " of uri " << view;
	}
}

TEST(uri, uri_decompose_works)
{
#define URI(s, ...) uri_equal(s, decompose_uri(s).value(), json::object_t(__VA_ARGS__));
#include "uri_tests.inc"
#undef URI


	/// Try with no flags
#define URI(s, ...) uri_equal(s, decompose_uri(s, {}).value(), json::object_t(__VA_ARGS__));
#include "uri_tests.inc"
#undef URI
	/*
#define URI(s, ...) std::cout << std::format("==> {}\n{}\n", s, json(decompose_uri(s).value()).dump(2));
#include "uri_tests.inc"
#undef URI
*/
}

TEST(uri, uri_decompose_properly_catches_degenerate_cases)
{
	auto uri = decompose_uri("FTP://cnn.example.com&story=breaking_news@10.0.0.1/top_story.htm").value();
	EXPECT_EQ(uri.host, "10.0.0.1");
	EXPECT_EQ(uri.path, "/top_story.htm");
	EXPECT_EQ(uri.user_info, "cnn.example.com&story=breaking_news");
	EXPECT_EQ(uri.scheme, "ftp");
}

TEST(uri, uri_decompose_handles_simplified_file_uris)
{
	auto uri = decompose_uri("file:/a/b/c").value();
	EXPECT_EQ(uri.scheme, "file");
	EXPECT_EQ(uri.path, "/a/b/c");
	EXPECT_EQ(uri.host, "");
	EXPECT_EQ(uri.user_info, "");
	EXPECT_EQ(uri.port, "");
	EXPECT_EQ(uri.authority, "");
	EXPECT_EQ(uri.query, "");
	EXPECT_EQ(uri.fragment, "");
	EXPECT_EQ(uri.path_elements, (std::vector<std::string>{"a", "b", "c"}));
	EXPECT_TRUE(uri.query_elements.empty());


	auto uri2 = decompose_uri("file:a/b/c").value();
	EXPECT_EQ(uri2.scheme, "file");
	EXPECT_EQ(uri2.path, "a/b/c");
	EXPECT_EQ(uri2.host, "");
	EXPECT_EQ(uri2.user_info, "");
	EXPECT_EQ(uri2.port, "");
	EXPECT_EQ(uri2.authority, "");
	EXPECT_EQ(uri2.query, "");
	EXPECT_EQ(uri2.fragment, "");
	EXPECT_EQ(uri2.path_elements, (std::vector<std::string>{"a", "b", "c"}));
	EXPECT_TRUE(uri2.query_elements.empty());
}
