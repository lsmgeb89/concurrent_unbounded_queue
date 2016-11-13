#ifndef CONCURRENT_LINKED_LIST_LOCK_FREE_LINKED_LIST_H_
#define CONCURRENT_LINKED_LIST_LOCK_FREE_LINKED_LIST_H_

#include <atomic>
#include <sstream>
#include "queue_node.h"
#include "log_util.h"

namespace utils {

class LockFreeQueue {
 public:
  LockFreeQueue(void)
    : head_(new AtomicQueueNode(0, nullptr)),
      tail_(head_.load()) {}

  ~LockFreeQueue(void) {
    AtomicQueueNode* curr(head_.load()->next_.load());
    AtomicQueueNode* tmp(nullptr);
    while (curr) {
      tmp = curr;
      curr = curr->next_.load();
      debug_clog << "~LockFreeQueue free " << tmp->val_ << std::endl;
      delete tmp;
    }
    delete head_.load();
  }

  const bool IsEmpty(void) const {
    return (!head_.load()->next_.load());
  }

  void Enque(const int& value) {
    AtomicQueueNode *new_node(new AtomicQueueNode(value, nullptr));
    while (true) {
      AtomicQueueNode *last(tail_.load());
      AtomicQueueNode *next(last->next_.load());
      if (last == tail_.load()) {
        if (!next) {
          if (std::atomic_compare_exchange_strong(&last->next_, &next, new_node)) {
            std::atomic_compare_exchange_strong(&tail_, &last, new_node);
            return;
          }
        } else {
          std::atomic_compare_exchange_strong(&tail_, &last, next);
        }
      }
    }
  }

  const int Deque(void) {
    while (true) {
      AtomicQueueNode *first(head_.load());
      AtomicQueueNode *last(tail_.load());
      AtomicQueueNode *next(first->next_.load());

      if (first == head_.load()) {
        if (first == last) {
          if (!next) {
            throw std::underflow_error("");
          } else {
            std::atomic_compare_exchange_strong(&tail_, &last, next);
          }
        } else {
          int res(next->val_);
          if (std::atomic_compare_exchange_strong(&head_, &first, next)) {
            return res;
          }
        }
      }
    }
  }

  const std::string ToString(void) const {
    std::stringstream ss;
    AtomicQueueNode *curr(head_.load()->next_.load());
    while (curr) {
      ss << curr->val_;
      if (curr->next_) {
        ss << " ";
      }
      curr = curr->next_.load();
    }
    return ss.str();
  }

 private:
  std::atomic<AtomicQueueNode*> head_;
  std::atomic<AtomicQueueNode*> tail_;

 public:
  static constexpr auto name_ = "LockFreeQueue";
};

} // namespace utils

#endif // CONCURRENT_LINKED_LIST_LOCK_FREE_LINKED_LIST_H_
