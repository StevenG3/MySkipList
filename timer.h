#ifndef __TIMER_H__
#define __TIMER_H__

#include "non_copyable.h"

#include <time.h>
#include <vector>

template <typename K, typename V>
class Timer: NonCopyable {
public:
	explicit Timer(): head_ptr_(nullptr), tail_ptr_(nullptr) {}

	~Timer();

	int Push(K key, int timeout = 10);

	int Pop(K key);

	void Tick(std::vector<K> &timeout_fds);

private:
  class UtilTimer {
  public:
    explicit UtilTimer(K key, int timeout = 10) : key_(key),
                                                   expire_(time(nullptr) + timeout),
                                                   prev_ptr_(nullptr),
                                                   next_ptr_(nullptr)
    {}

  	public:
		K key_;			// 被监控的键
		time_t expire_; // 超时时间，绝对时间
		UtilTimer *prev_ptr_;    // 前一个计时器
		UtilTimer *next_ptr_;    // 后一个计时器
  	};

  	UtilTimer *head_ptr_; // 头结点
  	UtilTimer *tail_ptr_; // 尾结点
};

template <typename K, typename V>
Timer<K, V>::~Timer() {
	UtilTimer *temp_ptr = head_ptr_;
	while (temp_ptr) {
		head_ptr_ = temp_ptr->next_ptr_;
		delete temp_ptr;
		temp_ptr = head_ptr_;
	}
}

template <typename K, typename V>
int Timer<K, V>::Push(K key, int timeout) {
	// 此处是否需要入参验证，如何验证

	UtilTimer *temp_ptr = new UtilTimer(key, timeout);

	if(!head_ptr_) {
		head_ptr_ = tail_ptr_ = temp_ptr;
		return 1;
	}

	temp_ptr->prev_ptr_ = tail_ptr_;
	tail_ptr_->next_ptr_ = temp_ptr;
	tail_ptr_ = temp_ptr;
	return 1;
}

template <typename K, typename V>
int Timer<K, V>::Pop(K key) {
	// 此处是否需要入参验证，如何验证
	
	if(!head_ptr_)
		return -1;
	
	UtilTimer* temp_ptr = head_ptr_;
	while(temp_ptr) {
		if(temp_ptr->key_ == key)
			break;
		
		temp_ptr = temp_ptr->next_ptr_;
	}

	if(temp_ptr == head_ptr_ && temp_ptr == tail_ptr_) {
		delete temp_ptr;
		head_ptr_ = tail_ptr_ = nullptr;
	} else if(temp_ptr == head_ptr_) { 
		head_ptr_ = head_ptr_->next_ptr_;
		head_ptr_->prev_ptr_ = nullptr;
		delete temp_ptr;
	} else if(temp_ptr == tail_ptr_) {
		tail_ptr_ = tail_ptr_->prev_ptr_;
		tail_ptr_->next_ptr_ = nullptr;
		delete temp_ptr;
	} else {
		temp_ptr->prev_ptr_->next_ptr_ = temp_ptr->next_ptr_;
		temp_ptr->next_ptr_->prev_ptr_ = temp_ptr->prev_ptr_;
		delete temp_ptr;
	}

	return 0;
}

template <typename K, typename V>
void Timer<K, V>::Tick(std::vector<K>& timeout_fds) {
	timeout_fds.clear();

	if(head_ptr_) {
		time_t cur_time = time(nullptr);
		UtilTimer* temp_ptr = head_ptr_;
		while(temp_ptr) {
			if(cur_time < temp_ptr->expire_)
				break;
			timeout_fds.push_back(temp_ptr->key_);
			Pop(temp_ptr->key_);
			temp_ptr = head_ptr_;
		}
	}
}

#endif