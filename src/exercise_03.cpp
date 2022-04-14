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

#include <array>
#include <atomic>
#include <future>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

/*######################################################################################
 * Global constants
 *####################################################################################*/

constexpr size_t kRepeatNum = 3e6;
constexpr bool kTargetIsX = true;
constexpr bool kTargetIsY = false;

enum Exercise
{
  kWOFence,
  kWithFence,
  kWithAdditionalFence
};

/*######################################################################################
 * Global variables
 *####################################################################################*/

// target data
std::array<std::atomic_size_t, kRepeatNum> arr_x{};
std::array<std::atomic_size_t, kRepeatNum> arr_y{};

/*######################################################################################
 * Exercises
 *####################################################################################*/

void
AddWithoutFence(const bool read_x)
{
  if (read_x) {
    for (size_t i = 0; i < kRepeatNum; ++i) {
      arr_x.at(i).store(1, std::memory_order_release);
    }
  } else {
    for (size_t i = 0; i < kRepeatNum; ++i) {
      arr_y.at(i).store(1, std::memory_order_release);
    }
  }
}

void
ReadWithoutFence(  //
    const bool x_prev_y,
    std::promise<std::array<bool, kRepeatNum>> p)
{
  std::array<bool, kRepeatNum> one_zero_orders{};
  if (x_prev_y) {
    for (size_t i = 0; i < kRepeatNum; ++i) {
      auto x = arr_x.at(i).load(std::memory_order_acquire);
      while (x == 0) {
        x = arr_x.at(i).load(std::memory_order_acquire);
      }
      const auto y = arr_y.at(i).load(std::memory_order_acquire);
      one_zero_orders.at(i) = x > y;
    }
  } else {
    for (size_t i = 0; i < kRepeatNum; ++i) {
      auto y = arr_y.at(i).load(std::memory_order_acquire);
      while (y == 0) {
        y = arr_y.at(i).load(std::memory_order_acquire);
      }
      const auto x = arr_x.at(i).load(std::memory_order_acquire);
      one_zero_orders.at(i) = y > x;
    }
  }

  p.set_value(one_zero_orders);
}

/*######################################################################################
 * Main function
 *####################################################################################*/

auto
main(  //
    [[maybe_unused]] int argc,
    [[maybe_unused]] char *argv[])  //
    -> int
{
  // select a run mode
  std::cout << "0: w/o release/acquire fences" << std::endl << "Select one of the run mode: ";
  std::string in{};
  std::cin >> in;
  std::cout << std::endl;
  const auto exe = static_cast<Exercise>(std::stoi(in));

  // set up targets
  for (size_t i = 0; i < kRepeatNum; ++i) {
    arr_x.at(i) = 0;
    arr_y.at(i) = 0;
  }

  // create worker threads for multi-threading
  std::thread writer_x{};
  std::thread writer_y{};
  std::promise<std::array<bool, kRepeatNum>> p_x_y{};
  std::promise<std::array<bool, kRepeatNum>> p_y_x{};
  auto &&future_x_y = p_x_y.get_future();
  auto &&future_y_x = p_y_x.get_future();
  switch (exe) {
    case kWOFence:
      std::thread{ReadWithoutFence, kTargetIsX, std::move(p_x_y)}.detach();
      std::thread{ReadWithoutFence, kTargetIsY, std::move(p_y_x)}.detach();
      writer_x = std::thread{AddWithoutFence, kTargetIsX};
      writer_y = std::thread{AddWithoutFence, kTargetIsY};
      break;
    case kWithFence:
      break;
    case kWithAdditionalFence:
      break;
  }

  // wait for the worker threads to complete their jobs
  writer_x.join();
  writer_y.join();
  const auto &reads_x_y = future_x_y.get();
  const auto &reads_y_x = future_y_x.get();

  // check the results is sequentially consistent
  auto is_consistent = true;
  for (size_t i = 0; i < kRepeatNum; ++i) {
    if (reads_x_y.at(i) && reads_y_x.at(i)) {
      is_consistent = false;
    }
  }
  if (is_consistent) {
    std::cout << "The reader threads loaded only consistent data." << std::endl;
  } else {
    std::cout << "The reader threads loaded inconsistent data." << std::endl;
  }

  return 0;
}
