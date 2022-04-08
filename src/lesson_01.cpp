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

constexpr size_t kThreadNum = 4;
constexpr size_t kRepeatNum = 2.5e7;

enum Exercise
{
  kWOAtomic,
  kWithAtomic,
  kWithCAS,
  kWithFetchAdd
};

/*######################################################################################
 * Exercises
 *####################################################################################*/

void
AddWithoutAtomic(  //
    uintptr_t sum_p,
    std::promise<std::pair<int64_t, int64_t>> p)
{
  auto *sum = reinterpret_cast<int64_t *>(sum_p);
  const auto init_val = *sum;

  for (size_t i = 0; i < kRepeatNum; ++i) {
    auto cur_val = *sum;
    if (cur_val >= 0) {  // dummy if-statement to prevent optimization
      ++cur_val;
    }
    *sum = cur_val;
  }

  const auto end_val = *sum;

  p.set_value(std::make_pair(init_val, end_val));
}

void
AddWithAtomic(  //
    uintptr_t sum_p,
    std::promise<std::pair<int64_t, int64_t>> p)
{
  auto *sum = reinterpret_cast<std::atomic_int64_t *>(sum_p);
  const auto init_val = sum->load(std::memory_order_relaxed);

  for (size_t i = 0; i < kRepeatNum; ++i) {
    auto cur_val = sum->load(std::memory_order_relaxed);
    ++cur_val;
    sum->store(cur_val, std::memory_order_relaxed);
  }

  const auto end_val = sum->load(std::memory_order_relaxed);

  p.set_value(std::make_pair(init_val, end_val));
}

void
AddWithCAS(  //
    uintptr_t sum_p,
    std::promise<std::pair<int64_t, int64_t>> p)
{
  auto *sum = reinterpret_cast<std::atomic_int64_t *>(sum_p);
  const auto init_val = sum->load(std::memory_order_relaxed);

  for (size_t i = 0; i < kRepeatNum; ++i) {
    auto cur_val = sum->load(std::memory_order_relaxed);
    while (!sum->compare_exchange_weak(cur_val, cur_val + 1, std::memory_order_relaxed)) {
      // continue until CAS succeeds
    }
  }

  const auto end_val = sum->load(std::memory_order_relaxed);

  p.set_value(std::make_pair(init_val, end_val));
}

void
AddWithFetchAdd(  //
    uintptr_t sum_p,
    std::promise<std::pair<int64_t, int64_t>> p)
{
  auto *sum = reinterpret_cast<std::atomic_int64_t *>(sum_p);
  const auto init_val = sum->load(std::memory_order_relaxed);

  for (size_t i = 0; i < kRepeatNum; ++i) {
    sum->fetch_add(1, std::memory_order_relaxed);
  }

  const auto end_val = sum->load(std::memory_order_relaxed);

  p.set_value(std::make_pair(init_val, end_val));
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

  // initialize a target memory address
  int64_t sum = 0;
  auto sum_p = reinterpret_cast<uintptr_t>(&sum);

  // create worker threads for multi-threading
  std::vector<std::future<std::pair<int64_t, int64_t>>> futures{};
  for (size_t i = 0; i < kThreadNum; ++i) {
    std::promise<std::pair<int64_t, int64_t>> p{};
    futures.emplace_back(p.get_future());

    switch (exe) {
      case kWOAtomic:
        std::thread{AddWithoutAtomic, sum_p, std::move(p)}.detach();
        break;
      case kWithAtomic:
        std::thread{AddWithAtomic, sum_p, std::move(p)}.detach();
        break;
      case kWithCAS:
        std::thread{AddWithCAS, sum_p, std::move(p)}.detach();
        break;
      case kWithFetchAdd:
        std::thread{AddWithFetchAdd, sum_p, std::move(p)}.detach();
        break;
    }
  }

  // collect and output workers' results
  for (size_t i = 0; i < kThreadNum; ++i) {
    const auto [init_val, end_val] = futures.at(i).get();
    std::cout << "Thread " << i << ":" << std::endl
              << "  initial val: " << init_val << std::endl
              << "  end val: " << end_val << std::endl;
  }
  std::cout << std::endl << "Total: " << sum << std::endl;

  return 0;
}
