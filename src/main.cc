#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <atomic>

// compute th area under the y = x*x on the (0, 1) interval
void IntegralMonteCarlo(int samples, std::atomic<int> &count)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    int local_count = 0;
    for (int i = 0; i < samples; ++i)
    {
        double x = dis(gen);
        double y = dis(gen);
        if (y <= x*x)
        {
            ++local_count;
        }
    }
    count += local_count;
}

// run Monte-Carlo in the number of threads
double MultiCarlo(int points, int threads_num) {
    std::vector<std::thread> threads;
    std::atomic<int> count(0);

    int thread_points= points / threads_num;

    for (int i = 0; i < threads_num; ++i)
    {
        threads.push_back(std::thread(IntegralMonteCarlo, thread_points, std::ref(count)));
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    return count * 1.0 / points;
}

// check if the precision works
bool check_precision(std::vector<double> result, double epsilon, int points) {
    double precision = ((1.0/3) * (1 - 1.0/3)) / (points * epsilon * epsilon * 1.0);
    size_t count = 0;
    for (auto res : result) {
        double deviation = abs(1.0/3 - res);
        if (deviation > epsilon) {
            count++;
        }
    }
    if (count * 1.0 / result.size() > precision) {
        return false;
    } 

    return true;
}

int main(int argc, char * argv[])
{   
    const int points = 1000000;
    int threads_num = 4;
    if (argc > 1) {
        threads_num = std::stoi(argv[1]);
    }
    if (threads_num < 1) {
        std::cerr << "Number of threads must be greater than 0. Got: " << threads_num << std::endl;
    }
    int times_num = 100;
    // call the MultiCarlo number of times to aggr the answer
    // and check result
    std::vector<double> result;
    for (int _ = 0; _ < times_num; _++) {
        result.emplace_back(MultiCarlo(points, threads_num));
    }
    bool prec = check_precision(result, 0.01, points);
    double aggr = std::accumulate(result.begin(), result.end(), 0.0) / times_num;
    std::cout << "Aggregate area: " << aggr << "\n Precision is: " << prec << std::endl;
    return 0;
}
