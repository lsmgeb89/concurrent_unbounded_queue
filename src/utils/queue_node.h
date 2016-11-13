#ifndef CONCURRENT_UNBOUNDED_QUEUE_QUEUE_NODE_H_
#define CONCURRENT_UNBOUNDED_QUEUE_QUEUE_NODE_H_

#include <atomic>

namespace utils {

class QueueNode {
 public:
  QueueNode(const int& val, QueueNode* const next)
    : val_(val),
      next_(next) {}

  int val_;
  QueueNode* next_;
};

class AtomicQueueNode {
 public:
  AtomicQueueNode(const int& val, AtomicQueueNode* const next)
    : val_(val),
      next_(next) {}

  int val_;
  std::atomic<AtomicQueueNode*> next_;
};

} // namespace utils

#endif // CONCURRENT_UNBOUNDED_QUEUE_QUEUE_NODE_H_
