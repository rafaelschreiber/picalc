/*
 * Author: Rafael Schreiber (i16066)
 * Project: picalc_07
 * File: main.cpp
 * Date: 09-01-2021
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <csignal>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "tfile.h"
#include "CLI11.hpp"

// use __float128 if compiler and machine support it, for more precision
#ifdef USE_FLOAT128
    typedef __float128  long_double_t;
#else
    typedef long double long_double_t;
#endif

using namespace std;

// to cast the (partial) results from double to str with more precision
template <typename T>
string to_string_w_precision(const T a_value, const int n = 256){
    ostringstream out;
    out.precision(n);
    out << fixed << a_value;
    string flt = out.str();
    reverse(flt.begin(), flt.end());
    uint8_t trailing_zeros = 0;
    for (char chr : flt){
        if (chr != '0'){
            break;
        }
        trailing_zeros++;
    }
    flt = flt.substr(trailing_zeros, flt.size());
    reverse(flt.begin(), flt.end());
    return flt;
}

// iterations for the Leibniz formular
uint64_t n;
uint64_t n_pos; // positive terms
uint64_t n_neg; // negative terms

string result_file_path{"/tmp/"}; // default path where the partial results are stored
size_t finished_processes{0}; // condition variable to check if all processes are finished
auto console = spdlog::stderr_color_mt("console");
int child_process_1, child_process_2; // pids of child processes
bool delete_on_exit{false}; // flag if partial result files should be deleted after exit

long_double_t calc_part_leibniz(uint64_t n, bool is_positive){
    // is_positive determines if term is negative or positive 
    uint8_t startpoint{1};
    if (!is_positive){
        startpoint = 3;
    }
    long_double_t part_result = 0.0;
    for (uint64_t i{0}; i < n; i++){
        part_result += (long_double_t) 1 / (startpoint + 4 * i);
    } 

    if (!is_positive){
        part_result = -part_result;
    }

    return part_result;
}

void calc_positive(int signal){
    // signal callable for calculating the positive terms
    console->info("Started calculation of {} positive term(s) by child process 1", n_pos);
    string result_string{to_string_w_precision(calc_part_leibniz(n_pos, true))};
    string storepath{result_file_path + "part_pos_" + to_string(getppid()) + ".txt"};
    try{
        tfile::write(storepath.c_str(), result_string);
        console->info("Positive calculations finished and written to file: {}", storepath);
    } catch (const runtime_error& error){ // occurs if file is not writable
        kill(getppid(), SIGTERM);
    }
    
}

void calc_negative(int signal){
    // signal callable for calculating the negative term
    usleep(1'000); // wait for clean output
    console->info("Started calculation of {} negative term(s) by child process 2", n_neg);
    string result_string{to_string_w_precision(calc_part_leibniz(n_neg, false))};
    string storepath{result_file_path + "part_neg_" + to_string(getppid()) + ".txt"};
    try{
        tfile::write(storepath.c_str(), result_string);
        console->info("Negative calculations finished and written to file: {}", storepath);
    } catch (const runtime_error& error){ // occurs if file is not writable
        kill(getppid(), SIGTERM);
    }
}

inline void increment_finished_processes(int signal){
    finished_processes++;
}

void calc_pi(){
    // read partial results from files and calculate the final result
    long_double_t pos, neg, pi;
    string pos_str, neg_str;
    string storepath_pos{result_file_path + "part_pos_" + to_string(getpid()) + ".txt"};
    string storepath_neg{result_file_path + "part_neg_" + to_string(getpid()) + ".txt"};
    pos_str = tfile::read(storepath_pos.c_str());
    neg_str = tfile::read(storepath_neg.c_str());
    pos = stold(pos_str);
    neg = stold(neg_str);
    pi = (pos + neg) * 4; // the final calculation :)
    console->info("Final calculation finished. π ≈ \e[1m{}\e[0m", to_string_w_precision(pi));
}

void terminate_program(int signal){
    console->critical("Cannot write partial result file to directory {}", result_file_path);
    kill(child_process_1, SIGTERM);
    kill(child_process_2, SIGTERM);
    exit(2);
}


int main(int argc, char** argv) {
    CLI::App app("Calculate the constant pi with the Leibniz formula");
    app.add_option("-i,--iterations", n, "Number of iterations (n >= 2)")
        ->required()
        ->check(CLI::PositiveNumber);
    app.add_option("-l,--location", result_file_path, "Location for the partial result files")
        ->check(CLI::ExistingDirectory);
    app.add_flag("-d,--delete", delete_on_exit, "Delete partial result files on program exit");
    
    CLI11_PARSE(app, argc, argv);

    console->set_level(spdlog::level::trace);
    
    /*
    console->trace("Welcome to picalc!");
    console->debug("Welcome to spdlog!");
    console->info("Welcome to spdlog!");
    console->warn("Welcome to spdlog!");
    console->error("Welcome to spdlog!");  // Achtung: spdlog::level::err
    console->critical("Welcome to spdlog!");
    // + spdlog::level::off
    */

    if (n < 2){
        console->critical("The number of iterations must be greater than or equal to 2");
        return 1;
    }

    if (n % 2 == 1){
        n_pos = (n / 2) + 1;
        n_neg = (n / 2);
    } else {
        n_pos = n / 2;
        n_neg = n / 2;
    }

    if (result_file_path.back() != '/'){ // add / to path if no one is specified
        result_file_path += '/';
    }

    console->info("Welcome to picalc {}!", getenv("USER"));

    child_process_1 = fork();
    if (child_process_1 == 0){
        console->info("Started child process 1 with PID:{} and PPID:{}", getpid(), getppid());
        signal(SIGHUP, calc_positive);
        pause();
        kill(getppid(), SIGHUP); // send parent process that calculation is finished
        pause();
    }

    if (child_process_1 > 0){
        child_process_2 = fork();
        if(child_process_2 > 0){
            console->info("Parent process with PID:{} and PPID:{} (shell)", getpid(), getppid());

            // wait for 3 seconds to start calculation with SIGHUP
            usleep(1'000'000);
            kill(child_process_1, SIGHUP);
            kill(child_process_2, SIGHUP);

            signal(SIGHUP, increment_finished_processes); // signal handler if one client is finished
            signal(SIGTERM, terminate_program); // signal handler if one client has an error
            
            console->info("Parent process waiting for child processes to finish calculations");
            while (finished_processes < 2){ // wait for subprocesses to finish
                usleep(1'000);
            }

            // kill child processes
            kill(child_process_1, SIGKILL);
            kill(child_process_2, SIGKILL);
            console->info("All calculations finished. All child processes were killed");

            waitpid(child_process_1, nullptr, 0);
            waitpid(child_process_2, nullptr, 0);

            calc_pi();

            if (delete_on_exit){
                string storepath_pos{result_file_path + "part_pos_" + to_string(getpid()) + ".txt"};
                string storepath_neg{result_file_path + "part_neg_" + to_string(getpid()) + ".txt"};
                remove(storepath_pos.c_str());
                remove(storepath_neg.c_str());
                console->info("All partial result files deleted");
            }
            exit(EXIT_SUCCESS);
        } else if(child_process_2 == 0){
            console->info("Started child process 2 with PID:{} and PPID:{}", getpid(), getppid());
            signal(SIGHUP, calc_negative);
            pause();
            kill(getppid(), SIGHUP); // send parent process that calculation is finished
            pause();
        }
    }
}
