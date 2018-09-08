#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_THREAD_POOL_H
#define FAY_CORE_THREAD_POOL_H

#include "fay/core/fay.h"

#include <atomic>
// #include <boost/circular_buffer.hpp>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <deque>
#include <thread>

namespace fay
{

template<typename T>
class Sync_queue {
public:
    void put(const T& val);
    void put(T&& val);
    void get(T& val);
private:
    mutex mtx;
    condition_variable cond;    // 用于控制访问
    list<T> q;
};

template<typename T>
void Sync_queue<T>::put(const T& val)
{
    lock_guard<mutex> lck(mtx);
    q.push_back(val);
    cond.notify_one();
}

template<typename T>
void Sync_queue<T>::get(T& val)
{
    unique_lock<mutex> lck(mtx);
    cond.wait(lck, [this]{ return !q.empty(); });    // 防止假性唤醒
    val = q.front();
    q.pop_front();
}

// a half sync/half async thread pool

using T = std::function<void()>;

class sync_queue
{
public:
	sync_queue(int max_size) : capacity_{ max_size }, stop_{ false }
	{
	}

	void put(const T& t) { add(t); }
	// sq 实例化后 put 是一个普通函数，T&& 是一个右值引用，而非通用引用
	void put(T&& t) { add(/*std::forward<T>(t)*/std::move(t)); }

	void get(std::deque<T>& tasks)
	{
		std::unique_lock<std::mutex> locker(mutex_);
		not_empty_.wait(locker, [this]() { return stop_ || not_empty(); });

		if (stop_)
			return;

		tasks = std::move(queue_);
		not_full_.notify_one();
	}
	void get(T& task)
	{
		std::unique_lock<std::mutex> locker(mutex_);
		not_empty_.wait(locker, [this]() { return stop_ || not_empty(); });

		if (stop_)
			return;

		task = queue_.front();
		queue_.pop_front();
		not_full_.notify_one();
	}

	size_t size() // const
	{
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.size();
	}
	
	bool empty()
	{
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.empty();
	}
	bool full()
	{
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.size() == capacity_;
	}

	void stop()
	{
		{
			std::lock_guard<std::mutex> locker(mutex_);
			stop_ = true;
		}
		not_full_.notify_all();
		not_empty_.notify_all();
	}

private:
	bool not_full() const
	{
		bool full = queue_.size() >= capacity_;
		DLOG_IF(INFO, full) << 
			"full, waiting, thread id: " << std::this_thread::get_id() << std::endl;
		return !full;
	}

	bool not_empty() const
	{
		bool empty = queue_.empty();
		DLOG_IF(INFO, empty) <<
			"empty, waiting, thread id: " << std::this_thread::get_id() << std::endl;
		return !empty;
	}

	template<typename Type>
	void add(Type&& t)
	{
		std::unique_lock<std::mutex> locker(mutex_);
		not_full_.wait(locker, [this]() {return stop_ || not_full(); });
		if (stop_)
			return;

		queue_.push_back(std::forward<Type>(t));
		not_empty_.notify_one();
	}

private:
	std::mutex mutex_;
	std::condition_variable not_empty_;
	std::condition_variable not_full_;

	int capacity_;
	bool stop_;

	std::deque<T> queue_;
};

class thread_pool
{
public:
	using Task = std::function<void()>;

public:
	thread_pool(int max_task_count = 64, int nThreads = std::thread::hardware_concurrency()) : 
		queue_(max_task_count), running_{ true }
	{
		for (int i = 0; i < nThreads; ++i)
			threads_.push_back(std::make_shared<std::thread>(&thread_pool::run, this));
	}

	~thread_pool() { stop(); }

	void add(const Task& task) { queue_.put(task); }
	void add(Task&& task)	   { queue_.put(std::forward<Task>(task)); }

	void stop()
	{
		std::call_once(flag_, [this]() { stop_threads(); }); //保证多线程情况下只调用一次stop_threads
	}

private:
	void run()
	{
		while (running_)
		{
			//取任务分别执行
			std::deque<Task> queue;
			queue_.get(queue);

			for (auto& task : queue)
			{
				if (!running_)
					return;

				task();
			}
		}
	}

	void stop_threads()
	{
		queue_.stop(); //让同步队列中的线程停止
		running_ = false; //置为false，让内部线程跳出循环并退出

		for (auto thread : threads_) //等待线程结束
		{
			if (thread)
				thread->join();
		}
		threads_.clear();
	}

private:
	std::list<std::shared_ptr<std::thread>> threads_; //处理任务的线程组
	sync_queue queue_; //同步队列     
	std::atomic<bool> running_; //是否停止的标志
	std::once_flag flag_;
};

} // namespace fay

#endif // FAY_CORE_THREAD_POOL_H
