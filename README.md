# Concurrent Unbounded Queue

## Summary
  * Implemented a concurrent unbounded queue that supports three operations, `enque`, `deque` and `is-empty` by using the following two approaches:
    1. lock-based synchronization
    2. lock-free synchronization
  * Implemented a common test facility that could generate operations with different distribution and test unbounded queues by different degree of concurrency (number of threads concurrently operating on a unbounded queue) according to the requirements of testing throughput
  * Compared the averaged performance of the two implementations as a function of number of threads (varied from one to the number of logical cores in the machine) under different distribution of operations

## Project Information
  * Course: Introduction to Multicore Programming (CS 6301)
  * Professor: [Neeraj Mittal][mittal]
  * Semester: Fall 2016
  * Programming Language: C++
  * Build Tool: [CMake][cmake]

[mittal]: http://cs.utdallas.edu/people/faculty/mittal-neeraj
[cmake]: https://cmake.org
