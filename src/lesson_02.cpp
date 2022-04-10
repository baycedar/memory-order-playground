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
  kWithFence
};

/*######################################################################################
 * Global variables
 *####################################################################################*/

struct Container {
  size_t sum{0};
};

/// a target memory address with std::atomic
std::atomic<Container *> ptr{nullptr};

/*######################################################################################
 * Exercises
 *####################################################################################*/

void
AddWithFence()
{
  for (size_t i = 0; i < kRepeatNum; ++i) {
    auto *target = ptr.load(std::memory_order_acquire);
    target->sum += 1;
    ptr.store(target, std::memory_order_release);
  }
}

void
AddWithoutFence()
{
  for (size_t i = 0; i < kRepeatNum; ++i) {
    auto *target = ptr.load(std::memory_order_relaxed);
    target->sum += 1;
    ptr.store(target, std::memory_order_relaxed);
  }
}

void
ReadWithFence()
{
  while (true) {
    const auto *target = ptr.load(std::memory_order_acquire);
    const auto cur_sum = target->sum;

    std::cout << "sum: " << cur_sum << std::endl;
    if (cur_sum >= kRepeatNum) break;
  }
}

void
ReadWithoutFence()
{
  while (true) {
    const auto *target = ptr.load(std::memory_order_relaxed);
    const auto cur_sum = target->sum;

    std::cout << "sum: " << cur_sum << std::endl;
    if (cur_sum >= kRepeatNum) break;
  }
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
  std::cout << "0: w/o std::atomic" << std::endl
            << "1: with std::atomic" << std::endl
            << "2: with compare-and-swap" << std::endl
            << "3: with fetch-add" << std::endl
            << "Select one of the run mode: ";
  std::string in{};
  std::cin >> in;
  std::cout << std::endl;
  const auto exe = static_cast<Exercise>(std::stoi(in));

  // set up targets
  ptr.store(new Container{0}, std::memory_order_release);

  // create worker threads for multi-threading
  std::thread reader{};
  std::thread writer{};
  switch (exe) {
    case kWOFence:
      reader = std::thread{ReadWithoutFence};
      writer = std::thread{AddWithoutFence};
      break;
    case kWithFence:
      reader = std::thread{ReadWithFence};
      writer = std::thread{AddWithFence};
      break;
  }

  // wait for the worker threads to complete their jobs
  writer.join();
  reader.join();

  // tear down  targets
  delete ptr.load(std::memory_order_relaxed);

  return 0;
}
