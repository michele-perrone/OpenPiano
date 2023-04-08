#pragma once

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <vector>

// poor man's implementation of std::barrier
// as for the hell of me I can't make it work as expected

template <typename Fn_type> class gates
{

	const int N_EXPECTED;
	std::atomic<int> arrived_count;

	std::condition_variable c_v;
	std::condition_variable gates_condition;
	std::mutex thread_lock;

	bool completion_fn_done;
	bool closed;

	Fn_type complete;

public:
	void arrive_and_wait();
	void wait_for_exit();

	gates() = delete;
	gates(int, Fn_type);
};

class OPTManeger
{
	// prevent copy, assign, move
	OPTManeger(const OPTManeger&) = delete;
	OPTManeger(OPTManeger&&) = delete;
	OPTManeger& operator = (const OPTManeger&) = delete;
	OPTManeger& operator = (OPTManeger&&) = delete;

	// base variables
	const int N_THREADS;
	std::vector<std::thread> threads;
	std::vector<std::function<void(int)>> callable_vec;
	gates<std::function<void()>>* sync;
	std::atomic<int> data_idx;
	
	// helper variables
	std::shared_mutex data_lock;
	std::mutex read_lock;
	std::condition_variable run_contition;
	std::atomic<bool> run;
	std::atomic<bool> stop;


	//init and stop could be made public
	void init();


	// public functions
public:
	void stop_threading();
	OPTManeger(const int);
	//void run();
	//void collect();
	//void set_callable(std::function<void(int, Params...)>&& callable);
	template <typename F, typename... Params>
	void push_callable(F&& callable, Params&&... args)
	{
		//data_lock.lock();
		callable_vec.push_back(
			std::bind(
				std::move(callable),
				std::placeholders::_1,
				std::forward<Params>(args)...
			)
		);
		//data_lock.unlock();
	};

	void run_and_collect();

	// destructor
	~OPTManeger();

};

