#include <gtest/gtest.h>

#include "../mmap.h"

TEST(mmap_test, mmap_works)
{
  auto src = ghassanpl::make_mmap_source("LICENSE");
}