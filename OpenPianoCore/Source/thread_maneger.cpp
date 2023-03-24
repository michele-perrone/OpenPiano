#include "thread_maneger.h"

template <typename Fn_type>
OPTManeger<Fn_type>::OPTManeger(const int n_threads, const int n_data, Fn_type in_fn) :
	N_THREADS(n_threads),
	N_DATA(n_data),
	data_idx(0),
	threads(),
	finish_lock(), start_lock(),
	c_v(), exit_cv(),
	start_promise(), start_future(),
	processing_done(false), can_begin(false), stop_threading(false),
	worker(in_fn)
{
	start_future = start_promise.get_future();
	init();
}

template <typename Fn_type>
void OPTManeger<Fn_type>::init()
{
	std::function<void()> begin = [this]() mutable noexcept
	{
		std::unique_lock<std::mutex> u_lock(start_lock); // wait until main frees
		c_v.wait_for(u_lock, std::chrono::milliseconds(1), [=]() ->bool { return can_begin || stop_threading; }); // check flags
		can_begin = false; // prevent from running immedeately again and give control to main
	};

	// sync before begining of computation and wait for main to signal start or stop
	sync_begin = new gates<std::function<void()>>(N_THREADS, begin);

	std::function<void()> complete = [this]() mutable noexcept
	{

		data_idx = 0; // reset data "quee"
		processing_done = true; // set flags
		exit_cv.notify_one(); // notify waiting main we're done
		std::unique_lock<std::mutex> u_lock(finish_lock); // block till main unlocks
	};

	// sync after completing and wait to collect()
	sync_finish = new gates<std::function<void()>>(N_THREADS, complete);

	for (int i = 0; i < N_THREADS; i++)
		threads.push_back(
			std::thread(
				[&, i]() mutable noexcept
				{
					int thread_data_idx; // for saving data index before it gets mutated 
					start_future.wait(); // wait for main to signal start
					int t_idx = i;
					while (true) // running loop
					{
						sync_begin->arrive_and_wait(); // wait to get signal to go again or stop
						if (stop_threading) return;
						while ((thread_data_idx = data_idx++) < N_DATA) // process loop
							worker(i, thread_data_idx); // call worker to process data
							// std::atomic should provide all needed thread safety

						sync_finish->arrive_and_wait(); // wait for all threads to come here and signal finish to main
					} // running loop
				}
			)
		);
	start_promise.set_value(); // signal to start
}
template <typename Fn_type>
void OPTManeger<Fn_type>::run_and_collect()
{

	std::unique_lock<std::mutex> u2_lock(finish_lock); // control exit before threads acqure it 
	can_begin = true; // by controlling beging processing flag
	processing_done = false; // needs a reset here or it will race it
	c_v.notify_one(); // signal start

	// threads start
	exit_cv.wait(u2_lock, [this] { return processing_done || stop_threading ; });
	u2_lock.unlock();
	c_v.notify_one();
}
template <typename Fn_type>
void OPTManeger<Fn_type>::stop()
{
	stop_threading = true; // beging processing flag
	c_v.notify_all(); // signal dropout
	for (std::thread& thread : threads)
		if (thread.joinable()) thread.join();
}
template <typename Fn_type>
OPTManeger<Fn_type>::~OPTManeger()
{
	stop();
	delete sync_begin;
	delete sync_finish;
}

template <typename Fn_type>
gates<Fn_type>::gates(int expected, Fn_type in_fn) :
	arrived_count(0),
	N_EXPECTED(expected),
	complete(in_fn),
	completion_fn_done(false),
	thread_lock(), loop_lock()
{}

template <typename Fn_type>
void gates<Fn_type>::arrive_and_wait()
{

	if (++arrived_count == N_EXPECTED)
	{
		complete();
		completion_fn_done = true;
		c_v.notify_all();
	}
	else
	{
		std::unique_lock<std::mutex> u_lock(thread_lock);
		c_v.wait(u_lock, [this]() ->bool {return completion_fn_done; });
	}
	if (--arrived_count == 0)
		completion_fn_done = false; // resetwith last thread
}


