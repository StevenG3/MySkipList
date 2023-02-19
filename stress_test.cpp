#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include "skiplist.h"

#define NUM_THREADS 1
#define TEST_COUNT 100000
SkipList kSkipList(18);

struct MyParam {
	long thread_id_;
	int test_count_;
	int num_threads_;
};

void *InsertElement(void* arg) {
	MyParam* pstru = (MyParam *)arg;

    long tid = (long)(pstru->thread_id_);
	int test_count = pstru->test_count_;
	int num_threads = pstru->num_threads_;

    // std::cout << tid << std::endl;
    int tmp = test_count / num_threads;
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		kSkipList.InsertElement("a", rand() % test_count);
	}
    pthread_exit(NULL);
}

void *GetElement(void* arg) {
	MyParam* pstru = (MyParam *)arg;

    long tid = (long)(pstru->thread_id_);
	int test_count = pstru->test_count_;
	int num_threads = pstru->num_threads_;

    // std::cout << tid << std::endl;
    int tmp = test_count / num_threads; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		kSkipList.SearchElement("a", rand() % test_count);
	}
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	if(!strcmp(argv[1], "INSERT") && !strcmp(argv[1], "GET")) {
		std::cout << "Usage: ./stress_test_start.sh [INSERT|GET] [TEST_COUNT] [NUM_THREADS]" << std::endl;
		exit(1);
	}
	int test_count = atoi(argv[2]);
	int num_threads = atoi(argv[3]);

	MyParam para {
		.test_count_ = test_count,
		.num_threads_ = num_threads,
	};

    srand((unsigned int)time(NULL));

	{
        pthread_t threads[num_threads];
        int rc;
        int i;

        auto start = std::chrono::high_resolution_clock::now();

        for(i = 0; i < num_threads; i++) {
            std::cout << "main(): creating thread, " << i << std::endl;
			para.thread_id_ = i;
            rc = pthread_create(&threads[i], NULL, InsertElement, (void *)(&para));

            if (rc) {
                std::cout << "Error: unable to create thread, " << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for(i = 0; i < num_threads; i++) {
            if (pthread_join(threads[i], &ret) != 0)  {
                perror("pthread_create() error"); 
                exit(3);
            }
        }
        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "insert elapsed: " << elapsed.count() << std::endl;
    }

    if(!strcmp(argv[1], "GET")) {
        pthread_t threads[num_threads];
        int rc;
        int i;
        auto start = std::chrono::high_resolution_clock::now();

        for(i = 0; i < num_threads; i++) {
            std::cout << "main(): creating thread, " << i << std::endl;
			para.thread_id_ = i;
            rc = pthread_create(&threads[i], NULL, GetElement, (void *)(&para));

            if (rc) {
                std::cout << "Error: unable to create thread, " << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for(i = 0; i < NUM_THREADS; i++) {
            if (pthread_join(threads[i], &ret) != 0) {
                perror("pthread_create() error"); 
                exit(3);
            }
        }

        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "get elapsed: " << elapsed.count() << std::endl;
    }

	pthread_exit(NULL);
    return 0;

}