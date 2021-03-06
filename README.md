# x-MeterPU

x-MeterPU is a tool used to measure the execution time and the energy consumption of the Intel Xeon Phi. x-MeterPU supports energy measurements for both native and offload programming models of Intel Xeon Phi. It can automatically detect the execution environment, therefore a single API function is used for measurements for both native and offload applications. To measure the energy of offload-based applications, we use the "micsmc" utility, whereas the "micras" tool is used to measure the energy of the native-based applications. 

Similarly to MeterPU, the use of x-MeterPU is very simple. The start() and stop() methods are used to enclose code regions to be measured. The get_value() function is used to retrieve the energy consumption (in Joules). In addition to the total energy consumption, x-MeterPU returns a log file containing all the power data with exact timestamps, which enables the production of various plots.

The tool (including the source code) itslef and the instructions on how to use it will be available soon.

Usage:
```
// Include the x-meterpu
#include "x-meterpu.cpp"

xMeterPU time(XPHI_TIME);
xMeterPU energy(XPHI_ENERGY);

time.start();
energy.start();

//the code you want to measure

time.stop();
energy.stop();

printf("MIC consumed power is %f Watts\n", energy.get_value());
printf("MIC consumed energy is %f Joules\n", energy.get_value() * time.get_value() * 1000);
printf("MIC elapsed time is %f seconds\n", time.get_value());
```
If you use xMeterPU for a publication, you should cite it as follows

Suejb Memeti, Lu Li, Sabri Pllana, Joanna Kołodziej, and Christoph Kessler. 2017. Benchmarking OpenCL, OpenACC, OpenMP, and CUDA: Programming Productivity, Performance, and Energy Consumption. In Proceedings of the 2017 Workshop on Adaptive Resource Management and Scheduling for Cloud Computing (ARMS-CC '17). ACM, New York, NY, USA, 1-6. DOI: https://doi.org/10.1145/3110355.3110356
