#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std::chrono;
using namespace std;


const int PAGESIZE = 4096;

void measure_tlb(int num_pages, int iterations)
{
    int *a = new int[PAGESIZE * num_pages];
    memset(a, 0, PAGESIZE * num_pages);

    //cout << "Timing " << num_pages << " pages, " << iterations << " iterations" << endl;

    // number of ints in a page
    int jump = PAGESIZE / sizeof(int);
    int i;

    steady_clock::time_point start = steady_clock::now();

    for (i=0; i < iterations;)
    {
        for (int n = 0; n < num_pages; n++)
        {
            a[n * jump] += 1;
            i++;
        }
    }

    steady_clock::time_point end = steady_clock::now();

    auto elapsed = duration_cast<nanoseconds>(end - start).count();

    //cout << "Elapsed time: " << elapsed << " ns avg: " << elapsed / (num_pages * iterations)
    //     << " ns" << endl;

    cout << num_pages << "," << elapsed / i << endl;

    delete [] a;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cerr << "Usage " << argv[0] << ": num_pages iterations" << endl;
        return -1;
    }

    string arg1(argv[1]);
    string arg2(argv[2]);

    int num_pages = stoi(arg1);
    int iterations = stoi(arg2);

    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(0, &set);

    if (sched_setaffinity(0, sizeof(set), &set) == -1)
    {
        perror("sched_setaffinity");
        return 1;
    }

    for (int pages = 2;pages <= num_pages;pages *= 2)
    {
        measure_tlb(pages, iterations);
    }

    return 0;
}
