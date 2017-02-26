#ifndef _THREADSAFE_QUEUE_H_
#define _THREADSAFE_QUEUE_H_

//This file is from http://blog.csdn.net/liuxuejiang158blog/article/details/17301739
//Thanks to liuxuejiang

#include <queue>  
#include <memory>  
#include <mutex>  
#include <condition_variable>  
template<typename T>  
class threadsafe_queue  
{  
private:  
	mutable std::mutex mut;   
	std::queue<T> data_queue;  
	std::condition_variable data_cond;  
public:  
	threadsafe_queue(){}  
	threadsafe_queue(threadsafe_queue const& other)  
	{  
		std::lock_guard<std::mutex> lk(other.mut);  
		data_queue=other.data_queue;  
	}  
	void push(T new_value)//入队操作  
	{  
		std::lock_guard<std::mutex> lk(mut);  
		data_queue.push(new_value);  
		data_cond.notify_one();  
	}  
	void wait_and_pop(T& value)//直到有元素可以删除为止  
	{  
		std::unique_lock<std::mutex> lk(mut);  
		data_cond.wait(lk,[this]{return !data_queue.empty();});  
		value=data_queue.front();  
		data_queue.pop();  
	}  
	std::shared_ptr<T> wait_and_pop()  
	{  
		std::unique_lock<std::mutex> lk(mut);  
		data_cond.wait(lk,[this]{return !data_queue.empty();});  
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();  
		return res;  
	}  
	bool try_pop(T& value)//不管有没有队首元素直接返回  
	{  
		std::lock_guard<std::mutex> lk(mut);  
		if(data_queue.empty())  
			return false;  
		value=data_queue.front();  
		data_queue.pop();  
		return true;  
	}  
	std::shared_ptr<T> try_pop()  
	{  
		std::lock_guard<std::mutex> lk(mut);  
		if(data_queue.empty())  
			return std::shared_ptr<T>();  
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));  
		data_queue.pop();  
		return res;  
	}  
	bool empty() const  
	{  
		std::lock_guard<std::mutex> lk(mut);  
		return data_queue.empty();  
	}  
};

#endif


#if 0
//
// Copyright (c) 2013 Juan Palacios juan.palacios.puyana@gmail.com
// Subject to the BSD 2-Clause License
// - see < http://opensource.org/licenses/BSD-2-Clause>
//

#ifndef CONCURRENT_QUEUE_
#define CONCURRENT_QUEUE_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class Queue
{
 public:

  T pop() 
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    auto val = queue_.front();
    queue_.pop();
    return val;
  }

  void pop(T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    item = queue_.front();
    queue_.pop();
  }

  void push(const T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }
  Queue()=default;
  Queue(const Queue&) = delete;            // disable copying
  Queue& operator=(const Queue&) = delete; // disable assignment
  
 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

#endif
#endif

