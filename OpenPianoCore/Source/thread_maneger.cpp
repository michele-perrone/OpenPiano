#include "thread_maneger.h"

template <typename Func_t>
OPTManeger<Func_t>::OPTManeger(const int n_threads) :
	N_THREADS(n_threads),
	threads(),
	callable_vec(),
	data_lock(), read_lock(),
	run_contition(),
	run(false), stop(false)
{
	init();
}
template <typename Func_t>
void OPTManeger<Func_t>::init()
{
	sync = new gates<std::function<void()>>(
		N_THREADS,
		[this]
		{
			read_lock.lock();
			run = false;
			read_lock.unlock();
		}
	);

	for (int i = 0; i < N_THREADS; i++)
		threads.push_back(
			std::thread(
				[&, i, this]() mutable noexcept
				{
					int internal_idx;
					int data_size;
					while (true)
					{
						internal_idx = i;
						std::unique_lock<std::mutex> u_lock(read_lock);
						run_contition.wait(u_lock, [this]() -> bool { return run || stop; });
						u_lock.unlock();
						if (stop) return;

						data_size = callable_vec.size();
						while (internal_idx < data_size)
						{
							callable_vec[internal_idx](i);
							internal_idx += N_THREADS;
						}

						sync->arrive_and_wait();
					}
				}
			)
		);
}
template <typename Func_t>
void OPTManeger<Func_t>::push_callable(Func_t callable)
{
	// data_lock.lock();
	callable_vec.push_back(callable);
	// data_lock.unlock();
}
template <typename Func_t>
void OPTManeger<Func_t>::run_and_collect()
{
	read_lock.lock();
	run = true;
	read_lock.unlock();
	run_contition.notify_all();

	sync->wait_for_exit();
	// data_lock.lock();
	callable_vec.clear();
	// data_lock.unlock();
}
template<typename Func_t>
inline void OPTManeger<Func_t>::stop_threading()
{
	read_lock.lock();
	stop = true;
	read_lock.unlock();
	run_contition.notify_all();

	for (std::thread& thread : threads)
		if (thread.joinable()) thread.join();
}
template <typename Func_t>
OPTManeger<Func_t>::~OPTManeger()
{
	stop_threading();
	delete sync;
}

template <typename Fn_type>
gates<Fn_type>::gates(int expected, Fn_type in_fn) :
	arrived_count(0),
	N_EXPECTED(expected),
	complete(in_fn),
	completion_fn_done(false),
	closed(false),
	gates_condition(),
	thread_lock()
{}

template <typename Fn_type>
void gates<Fn_type>::arrive_and_wait()
{
	if (++arrived_count == N_EXPECTED)
	{
		complete();
		thread_lock.lock();
		completion_fn_done = true;
		arrived_count--;
		thread_lock.unlock();
		c_v.notify_all();
	}
	else
	{
		std::unique_lock<std::mutex> u_lock(thread_lock);
		c_v.wait(u_lock, [this]() ->bool {return completion_fn_done; });
		if (--arrived_count == 0)
		{
			// mutex already locked
			completion_fn_done = false; // resetwith last thread
			closed = true;
			u_lock.unlock();
			gates_condition.notify_one();
		}
	}
}

template<typename Fn_type>
void gates<Fn_type>::wait_for_exit()
{
	std::unique_lock<std::mutex> u_lock(thread_lock);
	gates_condition.wait(u_lock, [this]() -> bool { return closed; });
	closed = false;
}