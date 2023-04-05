#include <iostream>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>
#include <time.h>
#include "skiplist.h"

#define NUM_THREADS 1
#define TEST_COUNT 100000
SkipList<std::string, double> kSkipList(18);

void InsertElement(long tid, int test_count, int num_threads) {
    // std::cout << tid << std::endl;
    int tmp = test_count / num_threads;
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		kSkipList.InsertElement("a", rand() % test_count);
	}
}

void GetElement(long tid, int test_count, int num_threads) {
    // std::cout << tid << std::endl;
    int tmp = test_count / num_threads; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		kSkipList.SearchElement("a");
	}
}

int main(int argc, char* argv[]) {
	if(!strcmp(argv[1], "INSERT") && !strcmp(argv[1], "GET")) {
		std::cout << "Usage: ./stress_test_start.sh [INSERT|GET] [TEST_COUNT] [NUM_THREADS]" << std::endl;
		exit(1);
	}

	int test_count = atoi(argv[2]);
	int num_threads = atoi(argv[3]);

    srand((unsigned int)time(NULL));

	std::vector<std::thread> work_thread_queue_(num_threads);

	{
        auto start = std::chrono::high_resolution_clock::now();

        for(int i = 0; i < num_threads; i++) {
            // std::cout << "main(): creating thread, " << i << std::endl;
			std::thread t(InsertElement, i, test_count, num_threads);
			work_thread_queue_[i] = std::move(t);
        }

        for(int i = 0; i < num_threads; i++) {
            if (work_thread_queue_[i].joinable()) {
				work_thread_queue_[i].join();
            }
        }
        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "insert elapsed: " << elapsed.count() << std::endl;
    }

    if(!strcmp(argv[1], "GET")) {
        auto start = std::chrono::high_resolution_clock::now();

        for(int i = 0; i < num_threads; i++) {
            // std::cout << "main(): creating thread, " << i << std::endl;
            std::thread t(GetElement, i, test_count, num_threads);
			work_thread_queue_[i] = std::move(t);
        }

        for(int i = 0; i < num_threads; i++) {
            if (work_thread_queue_[i].joinable()) {
				work_thread_queue_[i].join();
            }
        }

        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "get elapsed: " << elapsed.count() << std::endl;
    }

    return 0;
}