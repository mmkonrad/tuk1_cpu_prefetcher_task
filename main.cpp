#include <thread>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <limits>
#include <utility>
#include <memory>
#include <random>
#include <atomic>

using Time = std::chrono::nanoseconds;

const size_t minSize = 10 * 1024;
const size_t maxSize = static_cast<size_t>(1) * 1024 * 1024 * 1024;
const size_t cacheSize = 50 * 1024 * 1024;
const size_t sizeStep = 64;
const size_t iterations = 10;
const size_t differentValues = 20;
const size_t minThreads = 1;
const size_t maxThreads = 8;
const size_t threadStep = 2;

enum class Mode : int8_t { AGGREGATE, SCAN };

std::atomic_bool startFlag;

template<typename Elem>
struct TestResult
{
	TestResult(Mode mode, size_t size, size_t threads)
		: Average(0)
		, Min(std::numeric_limits<size_t>::max())
		, Max(0)
		, RunMode(mode)
		, Threads(threads)
		, Size(size)
	{
	}

	Time Average;
	Time Min;
	Time Max;
	Mode RunMode;
	const size_t Threads;
	const size_t Size;
};

template<Mode mode>
struct Functor
{
	template<typename Elem>
	static size_t execute(Elem* data, size_t elementCount);
};

template<>
template<typename Elem>
size_t Functor<Mode::AGGREGATE>::execute(Elem* data, size_t elementCount)
{
	size_t counter = 0;
	for (size_t current = 0; current < elementCount; ++current)
	{
		counter += data[current];
	}
	return counter;
}

template<>
template<typename Elem>
size_t Functor<Mode::SCAN>::execute(Elem* data, size_t elementCount)
{
	size_t counter = 0;
	for (size_t current = 0; current < elementCount; ++current)
	{
		if (data[current] == 0)
		{
			++counter;
		}
	}
	return counter;
}

void clearCache()
{
	static std::vector<uint8_t> cacheClearer(cacheSize);
	volatile size_t count = 0;
	for (auto elem : cacheClearer)
	{
		count += elem;
	}
}

template<typename Elem, Mode mode>
Time measureTime(Elem* data, size_t elementCount)
{
	while (!startFlag.load());
	const auto begin = std::chrono::high_resolution_clock::now();

	auto result = Functor<mode>::template execute<Elem>(data, elementCount);

	const auto end = std::chrono::high_resolution_clock::now();

	const volatile size_t optimizationBlocker = result;
	return std::chrono::duration_cast<Time>(end - begin);
}

template<typename Elem>
void randomizeData(Elem* data, size_t elementCount)
{
	// Fast random
	data[0] = 1;
	for (size_t current = 1; current < elementCount; ++current)
	{
		data[current] = (data[current - 1] + 7) * 13;
	}
}

template<typename Elem, Mode mode>
std::vector<TestResult<Elem>> run(uint8_t* rawData)
{
	std::vector<TestResult<Elem>> results;
	auto data = reinterpret_cast<Elem*>(rawData);
	randomizeData(data, maxSize / sizeof(Elem));

	for (size_t size = minSize; size < maxSize; size *= sizeStep)
	{
		const auto elementCount = size / sizeof(Elem);

		for (size_t threadCount = minThreads; threadCount <= maxThreads; threadCount *= threadStep)
		{
			TestResult<Elem> result(mode, size, threadCount);

			for (size_t iteration = 0; iteration < iterations; ++iteration)
			{
				std::vector<Time> times(threadCount);
				std::vector<std::thread> threads(threadCount);

				startFlag.store(false);
				for (size_t currentThread = 0; currentThread < threadCount; ++currentThread)
				{
					threads[currentThread] = std::thread([&times, currentThread, data, elementCount]()
					{
						times[currentThread] = measureTime<Elem, mode>(data + cacheSize / sizeof(Elem) * currentThread, elementCount);
					});
				}

				// Wait for all threads to start
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				startFlag.store(true);
				for (auto &thread : threads)
				{
					thread.join();
				}

				for (auto &thread : threads)
				{
					thread = std::thread(clearCache);
				}

				// Wait for all threads to start
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				for (auto &thread : threads)
				{
					thread.join();
				}

				auto time = *std::max_element(times.cbegin(), times.cend());

				result.Average += time;
				result.Min = std::min(result.Min, time);
				result.Max = std::max(result.Max, time);
			}
			result.Average /= iterations;

			results.push_back(result);
		}
	}

	return results;
}

template<typename Elem>
void printResult(const std::vector<TestResult<Elem>> &results)
{
	for (auto &result : results)
	{
		std::cout
			<< static_cast<int>(result.RunMode) << ";"
			<< typeid(Elem).name() << "; "
			<< result.Size << "; "
			<< result.Threads << "; "
			<< result.Average.count() << "; "
			<< (result.Size * result.Threads) / static_cast<double>(result.Average.count()) << std::endl;
	}
}

int main(int argc, char** argv)
{
	auto data = std::make_unique<uint8_t[]>(maxSize + maxThreads * cacheSize);

	printResult(run<uint8_t, Mode::AGGREGATE>(data.get()));
	printResult(run<uint16_t, Mode::AGGREGATE>(data.get()));
	printResult(run<uint32_t, Mode::AGGREGATE>(data.get()));
	printResult(run<uint64_t, Mode::AGGREGATE>(data.get()));

	printResult(run<uint8_t, Mode::SCAN>(data.get()));
	printResult(run<uint16_t, Mode::SCAN>(data.get()));
	printResult(run<uint32_t, Mode::SCAN>(data.get()));
	printResult(run<uint64_t, Mode::SCAN>(data.get()));
}

