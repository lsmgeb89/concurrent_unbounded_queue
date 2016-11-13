#ifndef CONCURRENT_LINKED_LIST_COARSE_GRAINED_LINKED_LIST_H_
#define CONCURRENT_LINKED_LIST_COARSE_GRAINED_LINKED_LIST_H_

#include <exception>
#include <mutex>
#include <sstream>
#include "queue_node.h"
#include "log_util.h"

namespace utils {

class LockedQueue {
 public:
  LockedQueue(void)
    : head_(new QueueNode(0, nullptr)),
      tail_(head_) {}

  ~LockedQueue() {
    QueueNode* curr(head_->next_);
    QueueNode* tmp(nullptr);
    while (curr) {
      tmp = curr;
      curr = curr->next_;
      debug_clog << "~LockedQueue free " << tmp->val_ << std::endl;
      delete tmp;
    }
    delete head_;
  }

  const bool IsEmpty(void) const {
    return !head_->next_;
  }

  void Enque(const int& value) {
    std::lock_guard<std::mutex> guard(enq_mutex_);
    QueueNode *e(new QueueNode(value, nullptr));
    tail_->next_ = e;
    tail_ = e;
  }

  const int Deque(void) {
    std::lock_guard<std::mutex> guard(deq_mutex_);
    if (!head_->next_) {
      throw std::underflow_error("");
    }
    int res(head_->next_->val_);
    head_ = head_->next_;
    return res;
  }

  const std::string ToString(void) const {
    std::stringstream ss;
    QueueNode *curr(head_->next_);
    while (curr) {
      ss << curr->val_;
      if (curr->next_) {
        ss << " ";
      }
      curr = curr->next_;
    }
    return ss.str();
  }

 private:
  std::mutex enq_mutex_;
  std::mutex deq_mutex_;
  QueueNode *head_;
  QueueNode *tail_;

 public:
  static constexpr auto name_ = "LockedQueue";
};

} // namespace utils

#endif // CONCURRENT_LINKED_LIST_COARSE_GRAINED_LINKED_LIST_H_
