#ifndef CONCURRENT_LINKED_LIST_TESTER_H_
#define CONCURRENT_LINKED_LIST_TESTER_H_

#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include <vector>
#include <condition_variable>
#include "lock_based_queue.h"
#include "lock_free_queue.h"
#include "log_util.h"

namespace utils {

enum OperationType {
  Enque = 0,
  Deque = 1,
  IsEmpty = 2
};

struct TestOperation {
  OperationType type_;
  int parameter_;
};

typedef std::pair<int, int> KeySpace;
typedef std::pair<OperationType, float> OpThroughput;
typedef std::chrono::duration<double, std::nano> TestResult;

class TestThroughput {
 public:
  TestThroughput(const std::string name,
                 const std::array<OpThroughput, 3>& profile)
    : name_(name),
      profile_(profile) {}

  std::string ToString(void) {
    std::stringstream out_stream;
    out_stream << name_ << ": ";

    for (auto p : profile_) {
      out_stream << p.second;
      if (IsEmpty == p.first) {
        out_stream << " IsEmpty ";
      } else if (Enque == p.first) {
        out_stream << " Enque ";
      } else {
        out_stream << " Deque ";
      }
    }

    return out_stream.str();
  }

 public:
  std::string name_;
  std::array<OpThroughput, 3> profile_;
};

#ifndef NDEBUG
#define CALL_FUNC_IF_MATCH                                                                \
  if (operation.type_ == Enque) {                                                         \
    sstr.str("");                                                                         \
    sstr << "[thread " << id << "] " << queue_.name_                                      \
         << "::Enque(" << operation.parameter_ << ")\n";                                  \
    std::clog << sstr.str();                                                              \
    queue_.Enque(operation.parameter_);                                                   \
    sstr.str("");                                                                         \
    sstr << "[thread " << id << "] " << queue_.name_                                      \
         << "::Enque(" << operation.parameter_ << ") = void\n";                           \
    std::clog << sstr.str();                                                              \
  } else if (operation.type_ == Deque) {                                                  \
    sstr.str("");                                                                         \
    sstr << "[thread " << id << "] " << queue_.name_ << "::Deque()\n";                    \
    std::clog << sstr.str();                                                              \
    try {                                                                                 \
      int ret = queue_.Deque();                                                           \
      sstr.str("");                                                                       \
      sstr << "[thread " << id << "] " << queue_.name_ << "::Deque() = " << ret << "\n";  \
      std::clog << sstr.str();                                                            \
    } catch (const std::underflow_error& e) {                                             \
      sstr.str("");                                                                       \
      sstr << "[thread " << id << "] " << queue_.name_ << "::Deque() = empty\n";          \
      std::clog << sstr.str();                                                            \
    }                                                                                     \
  } else {                                                                                \
    sstr.str("");                                                                         \
    sstr << "[thread " << id << "] " << queue_.name_ << "::IsEmpty()\n";                  \
    std::clog << sstr.str();                                                              \
    bool ret = queue_.IsEmpty();                                                          \
    sstr.str("");                                                                         \
    sstr << "[thread " << id << "] " << queue_.name_ << "::IsEmpty() = "                  \
         << std::boolalpha << ret << "\n";                                                \
    std::clog << sstr.str();                                                              \
  }
#else
#define CALL_FUNC_IF_MATCH                  \
  try {                                     \
    if (operation.type_ == Enque) {         \
      queue_.Enque(operation.parameter_);   \
    } else if (operation.type_ == Deque) {  \
      queue_.Deque();                       \
    } else {                                \
      queue_.IsEmpty();                     \
    }                                       \
  } catch (...) {}
#endif

template <typename QueueType> class UnitTester {
 public:
  UnitTester(void) {}
  ~UnitTester(void) {}

  static std::string GetName(void) { return QueueType::name_; }

  void ThreadFunc(const std::size_t& id, const std::vector<TestOperation>& operation_list) {
    // notify master I'm ready
    {
      std::unique_lock<std::mutex> lock(thread_ready_mutex_);
      thread_ready_.at(id) = 1;
      thread_ready_cv_.notify_one();
    }
    // wait master to signal
    {
      std::unique_lock<std::mutex> lock(thread_start_mutex_);
      thread_start_cv_.wait(lock, [this] { return thread_start_flag_; });
    }

#ifndef NDEBUG
    std::stringstream sstr;
#endif
    for (const auto& operation : operation_list) {
      CALL_FUNC_IF_MATCH
    }
  }

  TestResult UnitTest(const std::vector<std::vector<TestOperation>>& operation_list_group) {
    debug_clog << "--- [" << queue_.name_ << "] Thread = "
               << operation_list_group.size() << " Concurrent History ---\n";

    thread_ready_.resize(operation_list_group.size());
    for (auto &ready : thread_ready_) { ready = 0; }
    thread_start_flag_ = false;

    for (std::size_t i(0); i < operation_list_group.size(); i++) {
      this->thread_pool_.emplace_back([this, operation_list_group, i] { this->ThreadFunc(i, operation_list_group.at(i)); });
    }

    // wait all threads ready
    {
      std::unique_lock<std::mutex> lock(thread_ready_mutex_);
      thread_ready_cv_.wait(lock,
                            [this] { return std::all_of(thread_ready_.cbegin(), thread_ready_.cend(),
                                                        [](const uint8_t & ready) { return ready; }); });
    }

    decltype(std::chrono::steady_clock::now()) begin;
    // start testing
    {
      std::unique_lock<std::mutex> lock(thread_start_mutex_);
      thread_start_flag_ = true;
      begin = std::chrono::steady_clock::now();
      thread_start_cv_.notify_all();
    }

    for (auto& thread : thread_pool_) {
      thread.join();
    }

    auto end = std::chrono::steady_clock::now();

    debug_clog << "--- [" << queue_.name_ << "] Thread = " << operation_list_group.size() << ", Total Time = "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()
              << " (ns) ---" << std::endl;

    thread_pool_.clear();
    return (end - begin);
  }

 private:
  QueueType queue_;
  std::vector<std::thread> thread_pool_;

  std::vector<uint8_t> thread_ready_;
  std::mutex thread_ready_mutex_;
  std::condition_variable thread_ready_cv_;

  bool thread_start_flag_;
  std::mutex thread_start_mutex_;
  std::condition_variable thread_start_cv_;
};

class Tester {
 public:
  Tester(const std::size_t& max_thread_num,
         const std::size_t& operation_num,
         const std::size_t& repeat_times,
         const std::vector<TestThroughput>& throughput_list,
         const KeySpace& key_space)
    : max_thread_num_(max_thread_num),
      operation_num_(operation_num),
      repeat_times_(repeat_times),
      throughput_list_(throughput_list),
      profile_dist_(0, throughput_list.front().profile_.size() - 1),
      key_dist_(key_space.first, key_space.second),
      dist_(0, std::numeric_limits<std::size_t>::max()) {
    random_engine_.seed(std::random_device()());
    test_results_.resize(throughput_list.size());
    for (auto& test_result : test_results_) {
      std::get<0>(test_result).resize(max_thread_num_);
      std::get<1>(test_result).resize(max_thread_num_);
    }
  }

  void GenerateOperations(const TestThroughput& throughput,
                          const std::size_t& thread_num,
                          const std::size_t& operation_num,
                          std::vector<std::vector<TestOperation>>& operation_list_group) {
    auto operation_per_thread(operation_num / thread_num);
    std::vector<TestOperation> operations;
    operations.resize(operation_num);
    operation_list_group.resize(thread_num);
    for (auto& op_list : operation_list_group) {
      op_list.resize(operation_per_thread);
    }

    // generate operations
    auto op_iter(operations.begin());
    for (auto p : throughput.profile_) {
      auto limit(std::floor(p.second * operation_num));
      for (auto i(0); i < limit; ++i) {
        *op_iter++ = {p.first, key_dist_(random_engine_)};
      }
    }
    for (; op_iter != operations.end(); ++op_iter) {
      *op_iter = {throughput.profile_.at(profile_dist_(random_engine_)).first,
                  key_dist_(random_engine_)};
    }

    // randomly sort all operations
    std::random_shuffle(operations.begin(), operations.end());

    // distribute operations to each thread
    auto j(operations.begin());
    for (auto i(operation_list_group.begin()); i != operation_list_group.end(); ++i, j += operation_per_thread) {
      std::copy(j, j + operation_per_thread, (*i).begin());
    }

    // distribute remaining to threads randomly
    for (; j != operations.end(); ++j) {
      operation_list_group.at(dist_(random_engine_) % thread_num).push_back(*j);
    }
  }

  void Test(void) {
    for (std::size_t i = 0; i < throughput_list_.size(); i++) {
      for (std::size_t t_num = 1; t_num <= max_thread_num_; t_num++) {
        // For each thread number, we test multiple times
        debug_clog << "*** Profile: " << throughput_list_.at(i).name_ << " "
                   << t_num << " thread(s) test begins ***\n";
        for (std::size_t r = 0; r < repeat_times_; r++) {
          std::vector<std::vector<TestOperation>> operation_list_group;
          GenerateOperations(throughput_list_.at(i), t_num, operation_num_, operation_list_group);

          utils::UnitTester<utils::LockedQueue> lock_tester;
          utils::UnitTester<utils::LockFreeQueue> lock_free_tester;
          std::get<0>(test_results_.at(i)).at(t_num - 1) += lock_tester.UnitTest(operation_list_group);
          std::get<1>(test_results_.at(i)).at(t_num - 1) += lock_free_tester.UnitTest(operation_list_group);
        }

        // average
        std::get<0>(test_results_.at(i)).at(t_num - 1) /= repeat_times_;
        std::get<1>(test_results_.at(i)).at(t_num - 1) /= repeat_times_;
      }
    }
  }

  std::string ResultToString(void) {
    std::stringstream out_stream;

    for (std::size_t i = 0; i < throughput_list_.size(); i++) {
      // parameter
      out_stream << throughput_list_.at(i).ToString()
                 << "Thread Number: 1 ~ " << max_thread_num_
                 << ", Operation Number: " << operation_num_
                 << ", test times: " << repeat_times_
                 << ", Time Unit: Nanosecond"
                 << std::endl;

      // header
      out_stream << "ThreadNumber, "
                 << utils::UnitTester<utils::LockedQueue>::GetName() << ", "
                 << utils::UnitTester<utils::LockFreeQueue>::GetName() << std::endl;

      // line
      for (std::size_t j = 0; j < max_thread_num_; j++) {
        out_stream << j + 1 << ", "
                   << std::get<0>(test_results_.at(i)).at(j).count() << ", "
                   << std::get<1>(test_results_.at(i)).at(j).count() << std::endl;
      }

      out_stream << std::endl;
    }

    return out_stream.str();
  }

 private:
  std::size_t max_thread_num_;
  std::size_t operation_num_;
  std::size_t repeat_times_;
  std::vector<TestThroughput> throughput_list_;

  std::mt19937 random_engine_;
  std::uniform_int_distribution<std::size_t> profile_dist_;
  std::uniform_int_distribution<int> key_dist_;
  std::uniform_int_distribution<std::size_t> dist_;

  std::vector<std::tuple<std::vector<TestResult>, std::vector<TestResult>>> test_results_;
};

} // namespace utils

#endif // CONCURRENT_LINKED_LIST_TESTER_H_
