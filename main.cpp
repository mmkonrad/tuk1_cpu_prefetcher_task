#include <thread>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <iostream>
#include <typeinfo>


//
template <typename T>
struct Scan {
	/*
	*/
	std::vector<T> column;
	size_t entries;
	Scan(int size_in_kb){

		entries = size_in_kb/sizeof(T);
		column.resize(entries);
		size_t unique_count = entries / 20;

		for(auto itr = column.begin(); itr != column.end(); ++itr) {
			*itr = rand() * unique_count;
		}
	}	

	/*
	*/
	double scanVector() {

		const std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		int counter = 0;
		for(auto itr = column.begin(); itr != column.end(); ++itr) {
			if(*itr == 1) ++counter;
	  }

		const std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	  const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		return duration;
	}
};

template <typename T>
void run(Scan<T> & scan, double & result) {
	result = scan.scanVector();
}

template <typename T>
void runBechmark() {
	size_t thread_no = 4;
	std::vector<Scan<T>> scans(thread_no, Scan<T>(500 << 20));
	std::vector<double> results(thread_no);
	std::vector<std::thread> threads;
	
	for  (auto i = 0; i < thread_no; ++i) {
		//std::thread t(run<T>, std::ref(scans[i]), std::ref(results[i]));
		//t.join();
		threads.push_back(std::thread(run<T>, std::ref(scans[i]), std::ref(results[i])));
  	
	}
	

  for (auto & thread : threads) {
  	thread.join();	
  }
  std::cout << typeid(T).name() << ": " << sizeof(T) << " entries: " << scans.front().entries <<'\n';
  for (auto result : results) {
  	std::cout << result << '\n';
  }
}


int main(int argc, char** argv) {
    runBechmark<uint8_t>();
    runBechmark<uint16_t>();
    runBechmark<uint32_t>();
    runBechmark<uint64_t>();
}

