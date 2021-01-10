/*
 * Author: Rafael Schreiber (i16066)
 * Project: picalc_07
 * File: main.cpp
 * Date: 09-01-2021
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <sstream>
#include <csignal>

#include "tfile.h"
#include "CLI11.hpp"

// use __float128 if compiler and machine support it for more precision
#ifdef USE_FLOAT128
    typedef __float128  long_double_t;
#else
    typedef long double long_double_t;
#endif

using namespace std;

template <typename T>
string to_string_w_precision(const T a_value, const int n = 128){
    ostringstream out;
    out.precision(n);
    out << fixed << a_value;
    return out.str();
}

uint64_t n; // global variable for determine iterations
string result_file_path;
size_t finished_processes{0};

long_double_t calc_part_leibniz(uint64_t n, bool is_positive){
    // is_positive determines if term is negative or positive 
    uint8_t startpoint{1};
    if (!is_positive){
        startpoint = 3;
    }
    long_double_t part_result = 0.0;
    for (uint64_t i{0}; i <= n; i++){
        part_result += (long_double_t) 1 / (startpoint + 4 * i);
    } 

    if (!is_positive){
        part_result = -part_result;
    }

    return part_result;
}

void calc_positive(int signal){
    string result_string{to_string_w_precision(calc_part_leibniz(n, true))};
    string storepath{result_file_path + "part_pos.txt"};
    tfile::write(storepath.c_str(), result_string);
}

void calc_negative(int signal){
    string result_string{to_string_w_precision(calc_part_leibniz(n, false))};
    string storepath{result_file_path + "part_neg.txt"};
    tfile::write(storepath.c_str(), result_string);
}

inline void increment_finished_processes(int signal){
    finished_processes++;
}

void calc_pi(){
    long_double_t pos, neg, pi;
    string pos_str, neg_str;
    string storepath_pos{result_file_path + "part_pos.txt"};
    string storepath_neg{result_file_path + "part_neg.txt"};
    pos_str = tfile::read(storepath_pos.c_str());
    neg_str = tfile::read(storepath_neg.c_str());
    pos = stold(pos_str);
    neg = stold(neg_str);
    pi = (pos + neg) * 4;
    cout << to_string_w_precision(pi) << endl;
}


// int main(int argc, char** argv) {
int main(){
    n = 500'000'000;
    result_file_path = "/Users/rafael/Desktop/";
    int child_process_1;
    int child_process_2;
    child_process_1 = fork();
    if (child_process_1 == 0){
        printf("Child Process 1:\npid :%d\nppid:%d\n", getpid(), getppid());
        signal(SIGHUP, calc_positive);
        pause();
        cout << "done_pos" << endl;
        kill(getppid(), SIGHUP);
        pause();
    }
    if (child_process_1 > 0){
        child_process_2 = fork();
        if(child_process_2 > 0){
            printf("\nParent Process:\npid:%d\nppid :%d\n", getpid(), getppid());
            usleep(1'000'000);
            kill(child_process_1, SIGHUP);
            kill(child_process_2, SIGHUP);
            signal(SIGHUP, increment_finished_processes);
            while (finished_processes < 2){ // wait for subprocesses to finish
                usleep(1'000'000);
            }
            kill(child_process_1, SIGKILL);
            kill(child_process_2, SIGKILL);
            waitpid(child_process_1, nullptr, 0);
            waitpid(child_process_2, nullptr, 0);
            cout << "was geht jungs!" << endl;
            calc_pi();
        }
        else if(child_process_2 == 0){
            printf("Child Process 2:\npid :%d\nppid:%d\n", getpid(), getppid());
            signal(SIGHUP, calc_negative);
            pause();
            cout << "done_neg" << endl;
            kill(getppid(), SIGHUP);
            pause();
        }
    }
}
