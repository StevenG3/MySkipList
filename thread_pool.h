#ifndef __THREAD_POLL_H__
#define __THREAD_POLL_H__

#include "non_copyable.h"
#include "work_queue.h"

#include <future>
#include <functional>

class ThreadPool: NonCopyable {
public:
	explicit ThreadPool(int thread_number = 5);

	~ThreadPool() = default;

	int Size() const;

	template<typename F, typename... Args>
	auto SubmitWork(F &&f, Args &&...args) -> std::future<decltype(f(args...))>;

	void Shutdown();

	bool IsShutdown() const;

private:
	int thread_number_;
	bool shutdown_;
	WorkQueue<std::function<void()>> work_queue_; // 存储读写请求的任务队列
	std::vector<std::thread> work_thread_queue_;  // 工作线程队列
	std::mutex conditional_mutex_;                // 线程休眠锁
	std::condition_variable conditional_lock_;    // 线程环境锁
	class WorkThread_ {                           // 工作线程
	public:
		explicit WorkThread_(int id, ThreadPool* pool_ptr): thread_id_(id), pool_ptr_(pool_ptr) {}

		void operator()();

	private:
		int thread_id_;
		ThreadPool* pool_ptr_;
	};
};

template<typename F, typename... Args>
auto ThreadPool::SubmitWork(F&& f, Args &&...args) -> std::future<decltype(f(args...))> {
	// 将传入的函数指针和参数集封装为 std::function
	std::function<decltype(f(args...))()> function = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

	// 封装获取任务对象执行结果(std::future)，方便另外一个线程查看结果
	auto work_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(function);

	// 利用lambda表达式，返回一个函数对象
	std::function<void()> wrapper_function = [work_ptr]() {
		(*work_ptr)();
	};

	work_queue_.PushQueue(wrapper_function);

	conditional_lock_.notify_one();

	// 返回先前注册的任务指针
	return work_ptr->get_future();
}

void ThreadPool::WorkThread_::operator()() {
	std::function<void()> function;
	bool pop_queue;

	// 判断线程池是否关闭
	while(!pool_ptr_->shutdown_) {
		std::unique_lock<std::mutex> lock(pool_ptr_->conditional_mutex_);

		// 任务队列为空时，阻塞当前工作线程
		pool_ptr_->conditional_lock_.wait(
			lock,
			[this] { return !pool_ptr_->work_queue_.Empty(); });
		
		pop_queue = pool_ptr_->work_queue_.PopQueue(function);
	}

	if(pop_queue)
		function();
}

ThreadPool::ThreadPool(const int thread_number)
	: thread_number_(thread_number), shutdown_(false), work_thread_queue_(std::vector<std::thread>(thread_number)) {
	for(int i = 0; i < thread_number_; ++i) {
		work_thread_queue_[i] = std::thread(WorkThread_(i, this));
	}
}

int ThreadPool::Size() const {
  return thread_number_;
}

void ThreadPool::Shutdown() {
	shutdown_ = true;

	conditional_lock_.notify_all();

	for(auto& work_thread_ : work_thread_queue_) {
		if(work_thread_.joinable()) {
			work_thread_.join();
		}
	}
}

bool ThreadPool::IsShutdown() const {
	return shutdown_;
}

#endif