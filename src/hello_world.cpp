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

#include <gflags/gflags.h>

#include <iostream>

/*######################################################################################
 * CLI validators
 *####################################################################################*/

template <class Number>
static auto
ValidateNonZero(  //
    const char *flagname,
    const Number value)  //
    -> bool
{
  if (value != 0) {
    return true;
  }
  std::cout << "A value must be not zero for " << flagname << std::endl;
  return false;
}

/*######################################################################################
 * CLI arguments
 *####################################################################################*/

DEFINE_bool(show_value, true, "Show a value to stdout if true");       // NOLINT
DEFINE_uint64(nonzero_value, 1, "An option to set a positive value");  // NOLINT
DEFINE_validator(nonzero_value, &ValidateNonZero);                     // NOLINT

/*######################################################################################
 * Main function
 *####################################################################################*/

auto
main(  //
    int argc,
    char *argv[])  //
    -> int
{
  // parse and remove command line options
  gflags::SetUsageMessage("is a sample project for a C++ template repository.");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_show_value) {
    const auto option_value = FLAGS_nonzero_value;
    std::cout << option_value << std::endl;
  }

  return 0;
}
