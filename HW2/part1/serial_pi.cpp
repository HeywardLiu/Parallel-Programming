#include <iostream>
#include <climits>
#include <stdlib.h>  
using namespace std;

double estimatePI(long long int number_of_tosses) {
    long long int number_in_circle = 0;
    unsigned int seed = 123;
    for ( long long int toss = 0; toss < number_of_tosses; toss++) {
        double x = ((double) rand_r(&seed) / RAND_MAX) * 2.0 - 1.0 ;
        double y = ((double) rand_r(&seed) / RAND_MAX) * 2.0 - 1.0 ;
        double distance_squared = x * x + y * y;
        // cout << "dist = " << distance_squared << endl;
        if ( distance_squared <= 1)
            number_in_circle++;
    }

    cout << "in circle: " << number_in_circle
         << " / total tosses: " << number_of_tosses << endl;
    double pi_estimate = 4 * number_in_circle /(( double ) number_of_tosses);

    return pi_estimate;
}

int main(int argc, char** argv) {
    long long int number_of_tosses = atoll(argv[1]);
    cout << estimatePI(number_of_tosses) << endl;
}