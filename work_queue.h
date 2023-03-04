#ifndef __WORK_QUEUE_H__
#define __WORK_QUEUE_H__

#include "non_copyable.h"

#include <mutex>
#include <queue>

template<typename T>
class WorkQueue: NonCopyable {
public:
	explicit WorkQueue() = default;

	~WorkQueue() = default;

	bool Empty();

	int Size();

	void PushQueue(const T &t);

	bool PopQueue(T &t);

private:
	std::queue<T> work_queue_;
	std::mutex mutex_;
};

template<typename T>
bool WorkQueue<T>::Empty() {
	// 读过程是否需要加锁
	std::lock_guard<std::mutex> lock(mutex_);

	return work_queue_.empty();
}

template<typename T>
int WorkQueue<T>::Size() {
	std::lock_guard<std::mutex> lock(mutex_);
	return work_queue_.size();
}

template<typename T>
void WorkQueue<T>::PushQueue(const T &t) {
	std::lock_guard<std::mutex> lock(mutex_);

	work_queue_.emplace(t);
}

template<typename T>
bool WorkQueue<T>::PopQueue(T &t) {
	std::lock_guard<std::mutex> lock(mutex_);
	if (work_queue_.empty()) return false;

	t = std::move(work_queue_.front());
  	work_queue_.pop();

	return true;
}

#endif