#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "../include/filesystem.hpp"

const uint EXECS = 30;

class Timer {
public:
    Timer()
    {
        start_point = std::chrono::high_resolution_clock::now();
    };

    ~Timer() {};

    double stop()
    {
        auto end_point = std::chrono::high_resolution_clock::now();

        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_point).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_point).time_since_epoch().count();

        auto duration = end - start;
        double ms = duration * 0.001;

        return ms;
    };

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_point;
};

void exp_empty(std::string filename)
{
    std::unique_ptr<Filesystem> fs1 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs2 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs3 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs4 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs5 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs6 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs7 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs8 = std::make_unique<Filesystem>(filename);

    std::array<double, EXECS> results_copy_1;
    std::array<double, EXECS> results_copy_10;
    std::array<double, EXECS> results_copy_30;
    std::array<double, EXECS> results_rm_1;
    std::array<double, EXECS> results_rm_10;
    std::array<double, EXECS> results_rm_30;
    std::array<double, EXECS> results_rm_dir_nofile;
    std::array<double, EXECS> results_rm_dir_file;

    for (auto&& var : results_copy_1) {
        Timer timer;
        fs1->cp(source_file_1, dest);
        var = timer.stop();
    }

    for (auto&& var : results_copy_10) {
        Timer timer;
        fs2->cp(source_file_10, dest);
        var = timer.stop();
    }

    for (auto&& var : results_copy_30) {
        Timer timer;
        fs3->cp(source_file_1, dest);
        var = timer.stop();
    }

    for (auto&& var : results_rm_1) {
        Timer timer;
        fs4->rm(file_1);
        var = timer.stop();
    }

    for (auto&& var : results_rm_10) {
        var = 10;
    }

    for (auto&& var : results_rm_30) {
        var = 10;
    }

    for (auto&& var : results_rm_dir_nofile) {
        var = 10;
    }

    for (auto&& var : results_rm_dir_file) {
        var = 10;
    }
};

void exp_10(std::string filename) {
    //
};

void exp_30(std::string filename) {
    //
};

int main()
{
    std::string filename;

    exp_empty(filename);

    exp_10(filename);

    exp_30(filename);

    return 0;
}