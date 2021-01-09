/*
 * Author: Rafael Schreiber (i16066)
 * Project: picalc_07
 * File: main.cpp
 * Date: 09-01-2021
 */

#include <iostream>
#include <thread>

#ifdef USE_FLOAT128
    typedef __float128  long_double_t;
#else
    typedef long double long_double_t;
#endif

using namespace std;

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



// int main(int argc, char** argv) {
int main(){
    uint64_t n = 1'000'000'000;
    long_double_t pos{calc_part_leibniz(n, true)};
    long_double_t neg{calc_part_leibniz(n, false)};
    cout.precision(20);
    long_double_t pi = (pos + neg) * 4;
    cout << pi << endl;
}
