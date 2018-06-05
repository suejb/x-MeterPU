#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <omp.h>

#include "../xMeterPU.cpp"

int main(int argc, char* argv[])
{
    using namespace std;

    xMeterPU time(XPHI_TIME);
    xMeterPU energy(XPHI_ENERGY);
    // energy.writeToFile = true; //enables writing logs to a file

	time.start();
    energy.start();

    int iterations = 10000;
    double sum;

    #pragma offload target (mic:0)
    {
        //some simple example
        #pragma omp parallel for reduction(+:sum)
        for(int i= 0; i < iterations; i++) {
            sum += i * 3.14;
        }
    }

    time.stop();
    energy.stop();

    printf("MIC consumed power is %f Watts\n", energy.get_value());

    printf("MIC consumed energy is %f Joules\n", energy.get_value() * time.get_value() * 1000);

    printf("MIC elapsed time is %f seconds\n", time.get_value());
    return 0;
}
