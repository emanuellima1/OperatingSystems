#include <array>
#include <chrono>
#include <fstream>
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
        Timer timer;
        fs5->rm(file_10);
        var = timer.stop();
    }

    for (auto&& var : results_rm_30) {
        Timer timer;
        fs6->rm(file_30);
        var = timer.stop();
    }

    for (auto&& var : results_rm_dir_nofile) {
        Timer timer;
        fs7->rmdir(file_30);
        var = timer.stop();
    }

    for (auto&& var : results_rm_dir_file) {
        Timer timer;
        fs8->rmdir(file_30);
        var = timer.stop();
    }

    std::ofstream file("results_empty.csv");
    file << "results_copy_1" << ','
         << "results_copy_10" << ','
         << "results_copy_30" << ','
         << "results_rm_1" << ','
         << "results_rm_10" << ','
         << "results_rm_30" << ','
         << "results_rm_dir_nofile" << ','
         << "results_rm_dir_file" << ','
         << '\n';
    for (uint i = 0; i < EXECS; i++) {
        file << results_copy_1[i] << ','
             << results_copy_10[i] << ','
             << results_copy_30[i] << ','
             << results_rm_1[i] << ','
             << results_rm_10[i] << ','
             << results_rm_30[i] << ','
             << results_rm_dir_nofile[i] << ','
             << results_rm_dir_file[i] << ','
             << '\n';
    }
    file.close();
};

void exp_10(std::string filename)
{
    // TODO: Fix function calls

    std::unique_ptr<Filesystem> fs1 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs2 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs3 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs4 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs5 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs6 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs7 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs8 = std::make_unique<Filesystem>(filename);

    fs1->cp(source_file_10, dest);
    fs2->cp(source_file_10, dest);
    fs3->cp(source_file_10, dest);
    fs4->cp(source_file_10, dest);
    fs5->cp(source_file_10, dest);
    fs6->cp(source_file_10, dest);
    fs7->cp(source_file_10, dest);
    fs8->cp(source_file_10, dest);

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
        Timer timer;
        fs5->rm(file_10);
        var = timer.stop();
    }

    for (auto&& var : results_rm_30) {
        Timer timer;
        fs6->rm(file_30);
        var = timer.stop();
    }

    for (auto&& var : results_rm_dir_nofile) {
        Timer timer;
        fs7->rmdir(file_30);
        var = timer.stop();
    }

    for (auto&& var : results_rm_dir_file) {
        Timer timer;
        fs8->rmdir(file_30);
        var = timer.stop();
    }

    std::ofstream file("results_10.csv");
    file << "results_copy_1" << ','
         << "results_copy_10" << ','
         << "results_copy_30" << ','
         << "results_rm_1" << ','
         << "results_rm_10" << ','
         << "results_rm_30" << ','
         << "results_rm_dir_nofile" << ','
         << "results_rm_dir_file" << ','
         << '\n';
    for (uint i = 0; i < EXECS; i++) {
        file << results_copy_1[i] << ','
             << results_copy_10[i] << ','
             << results_copy_30[i] << ','
             << results_rm_1[i] << ','
             << results_rm_10[i] << ','
             << results_rm_30[i] << ','
             << results_rm_dir_nofile[i] << ','
             << results_rm_dir_file[i] << ','
             << '\n';
    }
    file.close();
};

void exp_50(std::string filename)
{
    // TODO: Fix function calls

    std::unique_ptr<Filesystem> fs1 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs2 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs3 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs4 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs5 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs6 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs7 = std::make_unique<Filesystem>(filename);
    std::unique_ptr<Filesystem> fs8 = std::make_unique<Filesystem>(filename);

    fs1->cp(source_file_50, dest);
    fs2->cp(source_file_50, dest);
    fs3->cp(source_file_50, dest);
    fs4->cp(source_file_50, dest);
    fs5->cp(source_file_50, dest);
    fs6->cp(source_file_50, dest);
    fs7->cp(source_file_50, dest);
    fs8->cp(source_file_50, dest);

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
        Timer timer;
        fs5->rm(file_10);
        var = timer.stop();
    }

    for (auto&& var : results_rm_30) {
        Timer timer;
        fs6->rm(file_30);
        var = timer.stop();
    }

    for (auto&& var : results_rm_dir_nofile) {
        Timer timer;
        fs7->rmdir(file_30);
        var = timer.stop();
    }

    for (auto&& var : results_rm_dir_file) {
        Timer timer;
        fs8->rmdir(file_30);
        var = timer.stop();
    }

    std::ofstream file("results_50.csv");
    file << "results_copy_1" << ','
         << "results_copy_10" << ','
         << "results_copy_30" << ','
         << "results_rm_1" << ','
         << "results_rm_10" << ','
         << "results_rm_30" << ','
         << "results_rm_dir_nofile" << ','
         << "results_rm_dir_file" << ','
         << '\n';
    for (uint i = 0; i < EXECS; i++) {
        file << results_copy_1[i] << ','
             << results_copy_10[i] << ','
             << results_copy_30[i] << ','
             << results_rm_1[i] << ','
             << results_rm_10[i] << ','
             << results_rm_30[i] << ','
             << results_rm_dir_nofile[i] << ','
             << results_rm_dir_file[i] << ','
             << '\n';
    }
    file.close();
};

int main()
{
    std::string filename;

    exp_empty("empty_fs");

    exp_10("10_fs");

    exp_50("50_fs");

    return 0;
}