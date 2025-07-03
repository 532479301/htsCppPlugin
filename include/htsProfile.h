// Copyright(c) 2025-present, ����.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#pragma once
#include <future>
#include <chrono>

// ���ڲ���һ������ִ��ʱ���һ�鹤�ߺ���

// �������⺯��һ��ִ�к�ʱ
template <typename F, typename... Args>
float measure(F func, const Args&... args)
{
	auto start = std::chrono::system_clock::now();
	func(args...);
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start);
	return duration.count() / 1000.0f;
}

// �������⺯���ڶ��߳̾����龰�µ�һ��ִ�к�ʱ
// �����Ҫ����߳�������������߳�ͬʱ�ܱ��⺯��
// ���Ϊ���⺯���ڶ���߳��е�ƽ��ִ��ʱ��
template <typename F, typename... Args>
inline float measure_multithread(int threads, F func, Args&&... args)
{
	float total_time = 0;
	std::vector<std::future<float>> tasks;
	tasks.reserve(threads);

	// ÿ���߳�����ʱ�䲻һ�����Ͼ���ѭ������һ��һ����ģ�����趨һ�������ʱ�䣬������ͬʱ������Щ�߳�
	auto start = std::chrono::system_clock::now() + std::chrono::milliseconds(50);

	for (int i = 0; i < threads; ++i)
	{
		auto task = std::packaged_task([&, i, start]() {
			std::this_thread::sleep_until(start);		// �����̵߳ȴ���ͬһʱ�������в��Դ���
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

// ��β���ȡƽ��ֵ�Ĳ��Ժ�����֧��ÿ���߳�ִ�в�ͬ����
template <typename F, typename... Args>
float benchmark(unsigned int threadCount, unsigned int testCount, F func, std::initializer_list<Args>... args)
{
	bool error = false;
	size_t threadCounts[] = { args.size()..., };
	for (int i = 0; i < sizeof...(args); i++)
	{
		if (threadCounts[i] != threadCount)
		{
			printf("��%d����ʼ���б���Ĳ�������(%zd)��theadCount(%d)����\n", i, threadCounts[i], threadCount);
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

		// ÿ���߳�����ʱ�䲻һ�����Ͼ���ѭ������һ��һ����ģ�����趨һ�������ʱ�䣬������ͬʱ������Щ�߳�
		auto start = std::chrono::system_clock::now() + std::chrono::milliseconds(50);

		for (unsigned int i = 0; i < threadCount; ++i)
		{
			auto tuple_param = std::make_tuple(func, i, (*(args.begin() + i))...);
			auto task = std::packaged_task([&, i, start, tuple_param]() {
				std::this_thread::sleep_until(start);		// �����̵߳ȴ���ͬһʱ�������в��Դ���
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

// ��β���ȡƽ��ֵ�Ĳ��Ժ�����ÿ���߳�ִ����ͬ�Ĳ���
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

		std::this_thread::sleep_for(std::chrono::milliseconds(20));	// ���Բ�Ҫ��ֻ����ÿ�ֲ���֮���и�ͣ��
	}
	return total_time / testCount;
}