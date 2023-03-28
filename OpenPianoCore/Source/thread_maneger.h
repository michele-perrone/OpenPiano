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
	std::condition_variable gates_condition;
	const int N_EXPECTED;
	std::atomic<int> arrived_count;
	bool completion_fn_done;
	bool closed;
	Fn_type complete;
	std::condition_variable c_v;
	std::mutex thread_lock;
public:
	void open() { closed = false; }
	void wait_for_exit();
	gates() = delete;
	gates(int, Fn_type);
	void arrive_and_wait();
};

template <typename Func_t> 
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
	std::vector<Func_t> callable_vec;
	gates<std::function<void()>>* sync;
	
	// helper variables
	std::atomic<int> data_idx;
	std::shared_mutex data_lock;
	std::mutex read_lock;

	std::condition_variable run_contition;
	std::atomic<bool> run;
	std::atomic<bool> stop;

	Func_t worker;

	//init and stop could be made public
	void init();
	void stop_threading();

	// public functions
public:

	OPTManeger(const int);
	//void run();
	//void collect();
	void push_callable(Func_t);
	void run_and_collect();

	// destructor
	~OPTManeger();

};

