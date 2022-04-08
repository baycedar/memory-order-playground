/*
 * Copyright 20yy Database Group, Nagoya University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"

class HelloWorldFixture : public ::testing::Test
{
 protected:
  void
  SetUp() override
  {
  }

  void
  TearDown() override
  {
  }

  int64_t common_variable_ = 0;
};

TEST_F(HelloWorldFixture, TestTargetWithSituationGetDesiredResults)
{
  EXPECT_EQ(0, common_variable_);
  EXPECT_NE(1, common_variable_);
  EXPECT_LT(common_variable_, 1);
  EXPECT_LE(common_variable_, 0);
  EXPECT_GT(1, common_variable_);
  EXPECT_GE(0, common_variable_);
  auto is_equal = common_variable_ == 0;
  EXPECT_TRUE(is_equal);
  EXPECT_FALSE(!is_equal);
}
