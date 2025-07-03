// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#pragma once
#include <future>
#include <chrono>

// 用于测试一个函数执行时间的一组工具函数

// 测量被测函数一次执行耗时
template <typename F, typename... Args>
float measure(F func, const Args&... args)
{
	auto start = std::chrono::system_clock::now();
	func(args...);
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start);
	return duration.count() / 1000.0f;
}

// 测量被测函数在多线程竞争情景下的一次执行耗时
// 会根据要求的线程数，启动多个线程同时跑被测函数
// 结果为被测函数在多个线程中的平均执行时间
template <typename F, typename... Args>
inline float measure_multithread(int threads, F func, Args&&... args)
{
	float total_time = 0;
	std::vector<std::future<float>> tasks;
	tasks.reserve(threads);

	// 每个线程启动时间不一样，毕竟是循环里面一个一个起的，因此设定一个靠后的时间，来尽量同时运行这些线程
	auto start = std::chrono::system_clock::now() + std::chrono::milliseconds(50);

	for (int i = 0; i < threads; ++i)
	{
		auto task = std::packaged_task([&, i, start]() {
			std::this_thread::sleep_until(start);		// 所有线程等待到同一时刻再运行测试代码
			return measure(func, i, std::forward<Args>(args)...); });
		tasks.push_back(task.get_future());
		std::thread(std::move(task)).detach();
	}
	for (int i = 0; i < threads; ++i)
	{
		total_time += tasks[i].get();
	}
	return total_time / threads;
}

// 多次测试取平均值的测试函数，支持每个线程执行不同参数
template <typename F, typename... Args>
float benchmark(unsigned int threadCount, unsigned int testCount, F func, std::initializer_list<Args>... args)
{
	bool error = false;
	size_t threadCounts[] = { args.size()..., };
	for (int i = 0; i < sizeof...(args); i++)
	{
		if (threadCounts[i] != threadCount)
		{
			printf("第%d个初始化列表里的参数数量(%zd)和theadCount(%d)不符\n", i, threadCounts[i], threadCount);
			error = true;
		}
	}
	if (error) return 0.0f;

	float total_time = 0;
	for (std::size_t i = 0u; i < testCount; ++i)
	{
		float test_time = 0;
		std::vector<std::future<float>> tasks;
		tasks.reserve(threadCount);

		// 每个线程启动时间不一样，毕竟是循环里面一个一个起的，因此设定一个靠后的时间，来尽量同时运行这些线程
		auto start = std::chrono::system_clock::now() + std::chrono::milliseconds(50);

		for (unsigned int i = 0; i < threadCount; ++i)
		{
			auto tuple_param = std::make_tuple(func, i, (*(args.begin() + i))...);
			auto task = std::packaged_task([&, i, start, tuple_param]() {
				std::this_thread::sleep_until(start);		// 所有线程等待到同一时刻再运行测试代码
				using funType = decltype(func);
				return std::apply(measure<funType, int, Args...>, tuple_param);
				});
			tasks.push_back(task.get_future());
			std::thread(std::move(task)).detach();
		}
		for (unsigned int i = 0; i < threadCount; ++i)
		{
			test_time += tasks[i].get();
		}
		total_time += test_time / threadCount;
	}
	return total_time / threadCount;
}

// 多次测试取平均值的测试函数，每个线程执行相同的参数
template <typename F, typename... Args>
float benchmark(unsigned int threadCount, unsigned int testCount, F func, Args&&... args)
{
	if (threadCount == 0 || testCount == 0) return 0.0f;

	float total_time = 0;
	for (std::size_t i = 0u; i < testCount; ++i)
	{
		if (threadCount == 1)
			total_time += measure(func, 0, std::forward<Args>(args)...);
		else
			total_time += measure_multithread(threadCount, func, std::forward<Args>(args)...);

		std::this_thread::sleep_for(std::chrono::milliseconds(20));	// 可以不要，只是想每轮测试之间有个停顿
	}
	return total_time / testCount;
}