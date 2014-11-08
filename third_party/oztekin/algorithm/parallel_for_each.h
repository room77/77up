// Copyright 2014 B. Uygar Oztekin

#ifndef _PUBLIC_THIRD_PARTY_OZTEKIN_ALGORITHM_FOR_EACH_H_
#define _PUBLIC_THIRD_PARTY_OZTEKIN_ALGORITHM_FOR_EACH_H_

#include <algorithm>
#include <future>

// Similar to std::for_each but can run the loop in parallel in roughly equal
// chunks. Unlike std::for_each, iterators need to be random access iterators.
template<class Iterator, class Function>
Function parallel_for_each(Iterator begin, Iterator end, Function f, int parallelism = 4) {
  // If there is no parallelism, fall back to std::for_each.
  if (parallelism <= 1) return std::for_each(begin, end, f);
  int range_size = end - begin;
  int chunk_size = (range_size + parallelism - 1) / parallelism;
  if (chunk_size < 1) return f;
  int num_chunks = (range_size + chunk_size - 1) / chunk_size;

  // Launch all but the last chunk in async threads.
  std::future<void> futures[num_chunks - 1];
  for (int i = 0; i < num_chunks - 1; ++i)
    futures[i] = std::async(std::launch::async,
        [=]{ std::for_each(begin + i * chunk_size, begin + (i+1) * chunk_size, f); } );

  // Launch the last chunk within this thread.
  std::for_each(begin + (num_chunks - 1) * chunk_size, end, f);

  // Wait for all async tasks to finish.
  for (auto& future: futures) future.get();
  return f;
}

#endif  // _PUBLIC_THIRD_PARTY_OZTEKIN_ALGORITHM_FOR_EACH_H_
