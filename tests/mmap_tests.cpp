#include <gtest/gtest.h>

#include "../include/ghassanpl/mmap.h"

TEST(mmap_test, mmap_works)
{
  auto src = ghassanpl::make_mmap_source<char>("LICENSE");
}