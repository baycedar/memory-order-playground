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
 * Global variables
 *####################################################################################*/

/// a target memory address without std::atomic
size_t sum{0};

/// a target memory address with std::atomic
std::atomic_size_t atom_sum{0};

/*######################################################################################
 * Exercises
 *####################################################################################*/

void
AddWithoutAtomic(std::promise<std::pair<size_t, size_t>> p)
{
  const auto init_val = sum;

  for (size_t i = 0; i < kRepeatNum; ++i) {
    auto cur_val = sum;
    if (cur_val < kRepeatNum) {  // dummy if-statement to prevent optimization
      ++cur_val;
    }

    sum = cur_val;
  }

  const auto end_val = sum;

  p.set_value(std::make_pair(init_val, end_val));
}

void
AddWithAtomic(std::promise<std::pair<size_t, size_t>> p)
{
  const auto init_val = atom_sum.load(std::memory_order_relaxed);

  for (size_t i = 0; i < kRepeatNum; ++i) {
    auto cur_val = atom_sum.load(std::memory_order_relaxed);
    ++cur_val;
    atom_sum.store(cur_val, std::memory_order_relaxed);
  }

  const auto end_val = atom_sum.load(std::memory_order_relaxed);

  p.set_value(std::make_pair(init_val, end_val));
}

void
AddWithCAS(std::promise<std::pair<size_t, size_t>> p)
{
  const auto init_val = atom_sum.load(std::memory_order_relaxed);

  for (size_t i = 0; i < kRepeatNum; ++i) {
    auto cur_val = atom_sum.load(std::memory_order_relaxed);
    while (!atom_sum.compare_exchange_weak(cur_val, cur_val + 1, std::memory_order_relaxed)) {
      // continue until CAS succeeds
    }
  }

  const auto end_val = atom_sum.load(std::memory_order_relaxed);

  p.set_value(std::make_pair(init_val, end_val));
}

void
AddWithFetchAdd(std::promise<std::pair<size_t, size_t>> p)
{
  const auto init_val = atom_sum.load(std::memory_order_relaxed);

  for (size_t i = 0; i < kRepeatNum; ++i) {
    atom_sum.fetch_add(1, std::memory_order_relaxed);
  }

  const auto end_val = atom_sum.load(std::memory_order_relaxed);

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

  // create worker threads for multi-threading
  std::vector<std::future<std::pair<size_t, size_t>>> futures{};
  for (size_t i = 0; i < kThreadNum; ++i) {
    std::promise<std::pair<size_t, size_t>> p{};
    futures.emplace_back(p.get_future());

    switch (exe) {
      case kWOAtomic:
        std::thread{AddWithoutAtomic, std::move(p)}.detach();
        break;
      case kWithAtomic:
        std::thread{AddWithAtomic, std::move(p)}.detach();
        break;
      case kWithCAS:
        std::thread{AddWithCAS, std::move(p)}.detach();
        break;
      case kWithFetchAdd:
        std::thread{AddWithFetchAdd, std::move(p)}.detach();
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
  const auto cur_sum = (exe == kWOAtomic) ? sum : atom_sum.load(std::memory_order_relaxed);
  std::cout << std::endl << "Total: " << cur_sum << std::endl;

  return 0;
}
