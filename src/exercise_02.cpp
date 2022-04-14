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

constexpr size_t kRepeatNum = 1e6;

enum Exercise
{
  kWOFence,
  kWithFence,
  kWithAdditionalFence
};

/*######################################################################################
 * Global variables
 *####################################################################################*/

/// target data
std::array<size_t, kRepeatNum> arr{};

/// a memory address for inserting fences
std::atomic_size_t pos{0};

/*######################################################################################
 * Exercises
 *####################################################################################*/

void
AddWithFence()
{
  for (size_t i = 0; i < kRepeatNum; ++i) {
    arr.at(i) = 1;
    pos.store(i, std::memory_order_release);
  }
}

void
AddWithoutFence()
{
  for (size_t i = 0; i < kRepeatNum; ++i) {
    arr.at(i) = 1;
    pos.store(i, std::memory_order_relaxed);
  }
}

void
AddWithAdditionalFence()
{
  for (size_t i = 0; i < kRepeatNum; ++i) {
    arr.at(i) = 1;
    pos.store(i, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_release);
  }
}

void
ReadWithFence(std::promise<bool> p)
{
  auto read_zero = false;
  while (true) {
    const auto cur_pos = pos.load(std::memory_order_acquire);
    if (cur_pos == 0) continue;

    const auto val = arr.at(cur_pos);
    std::cout << cur_pos << ": " << val << std::endl;
    if (val == 0) {
      read_zero = true;
    }

    if (cur_pos >= kRepeatNum - 1) break;
  }

  p.set_value(read_zero);
}

void
ReadWithoutFence(std::promise<bool> p)
{
  auto read_zero = false;
  while (true) {
    const auto cur_pos = pos.load(std::memory_order_relaxed);
    if (cur_pos == 0) continue;

    const auto val = arr.at(cur_pos);
    std::cout << cur_pos << ": " << val << std::endl;
    if (val == 0) {
      read_zero = true;
    }

    if (cur_pos >= kRepeatNum - 1) break;
  }

  p.set_value(read_zero);
}

void
ReadWithAdditionalFence(std::promise<bool> p)
{
  auto read_zero = false;
  while (true) {
    const auto cur_pos = pos.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (cur_pos == 0) continue;

    const auto val = arr.at(cur_pos);
    std::cout << cur_pos << ": " << val << std::endl;
    if (val == 0) {
      read_zero = true;
    }

    if (cur_pos >= kRepeatNum - 1) break;
  }

  p.set_value(read_zero);
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
  std::cout << "0: w/o release/acquire fences" << std::endl
            << "1: with release/acquire fences" << std::endl
            << "2: with relaxed and additional release/acquire fences" << std::endl
            << "Select one of the run mode: ";
  std::string in{};
  std::cin >> in;
  std::cout << std::endl;
  const auto exe = static_cast<Exercise>(std::stoi(in));

  // set up targets
  for (size_t i = 0; i < kRepeatNum; ++i) {
    arr.at(i) = 0;
  }

  // create worker threads for multi-threading
  std::thread writer{};
  std::promise<bool> p{};
  auto &&future = p.get_future();
  switch (exe) {
    case kWOFence:
      std::thread{ReadWithoutFence, std::move(p)}.detach();
      writer = std::thread{AddWithoutFence};
      break;
    case kWithFence:
      std::thread{ReadWithFence, std::move(p)}.detach();
      writer = std::thread{AddWithFence};
      break;
    case kWithAdditionalFence:
      std::thread{ReadWithAdditionalFence, std::move(p)}.detach();
      writer = std::thread{AddWithAdditionalFence};
      break;
  }

  // wait for the worker threads to complete their jobs
  writer.join();
  const auto read_zero = future.get();
  if (read_zero) {
    std::cout << "The reader thread loaded zero." << std::endl;
  } else {
    std::cout << "The reader thread loaded only one." << std::endl;
  }

  return 0;
}
