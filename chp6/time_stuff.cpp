#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std::chrono;
using namespace std;

void measure_syscall(int iterations, bool debug)
{
    int i;

    cout << "Timing syscalls with " << iterations << " iterations" << endl;

    steady_clock::time_point start = steady_clock::now();

    for(i=0; i < iterations; ++i)
    {
        read(-1, NULL, 0);
    }

    steady_clock::time_point end = steady_clock::now();

    auto elapsed = duration_cast<nanoseconds>(end - start).count();

    cout << "Elapsed time: " << elapsed << " ns avg: " << elapsed / i << " ns" << endl;
}

nanoseconds elapsed;

mutex ready_mutex;
condition_variable ready_cv;
bool ready = false;

void ctxswitch_thread(int iterations, bool do_timing, bool debug)
{
    cpu_set_t set;
    int i;

    CPU_ZERO(&set);
    CPU_SET(0, &set);

    if (sched_setaffinity(0, sizeof(set), &set) == -1)
    {
        perror("sched_setaffinity");
        return;
    }

    // The timing thread needs to wait for the busy thread to be started
    if (do_timing)
    {
        unique_lock<mutex> lck(ready_mutex);
        while (!ready) ready_cv.wait(lck);
    }
    // The busy thread notifies the timing thread that it's ready. It doesn't matter it starts looping before the timing thread.
    else
    {
        ready = true;
        ready_cv.notify_all();
    }

    steady_clock::time_point before, after;

    for(i=0;i < iterations;i++)
    {
        if (do_timing)
        {
            before = steady_clock::now();

            if (debug) cout << "A";
        }
        else
        {
            if (debug) cout << "B";
        }

        sched_yield();
        if (do_timing)
        {
            after = steady_clock::now();
            elapsed += after - before;
        }
    }
}

void measure_ctxswitch(int iterations, bool debug)
{
    cout << "Timing context switch" << endl;

    std::thread timing_thread (ctxswitch_thread, iterations, true, debug);
    std::thread busy_thread (ctxswitch_thread, iterations, false, debug);

    timing_thread.join();
    busy_thread.join();

    if (debug) cout << endl;

    cout << "Elapsed time: " << elapsed.count() << " ns avg: " << elapsed.count() / iterations << " ns" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Usage " << argv[0] << ": iterations [debug]" << endl;
        return -1;
    }

    bool debug = false;
    string arg1(argv[1]);
    int iterations = stoi(arg1);

    if (argc == 3)
    {
        string arg2(argv[2]);
        if (arg2 == "debug")
        {
            debug = true;
        }
    }

    measure_syscall(iterations, debug);

    measure_ctxswitch(iterations, debug);

    return 0;
}
