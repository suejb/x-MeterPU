/* Copyright 2017 Suejb Memeti

    Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

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
