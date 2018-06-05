#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>

#include <omp.h>

using namespace std;

#define MICPOWER_MAX_COUNTERS 16
typedef struct MICPOWER_control_state {
    long long counts[MICPOWER_MAX_COUNTERS];    // used for caching
    long long lastupdate;
} MICPOWER_control_state_t;

#define XPHI_ENERGY 0
#define XPHI_TIME 1

class xMeterPU {

private:
    int type;
    pthread_t micPthread;
    volatile bool keepAlive = true;
    double passEnergy = 0.0;
    double passTime = 0.0;

    static int read_sysfs_file(long long* counts) {
        FILE* fp = NULL;
        int i;
        int status;
        int retval = 1;
        char path[1024];

        #ifdef __MIC__
            fp = fopen("/sys/class/micras/power", "r");
            if (!fp) {
                return 0;
            }

            for (i=0; i < MICPOWER_MAX_COUNTERS-9; i++) {
                retval &= fscanf(fp, "%lld", &counts[i]);
            }
            for (i = MICPOWER_MAX_COUNTERS - 9; i < MICPOWER_MAX_COUNTERS; i += 3) {
                retval &= fscanf(fp, "%lld %lld %lld", &counts[i], &counts[i+1], &counts[i+2]);
            }
            status = fclose(fp);
        #else
            fp = popen("/usr/bin/micsmc -f | grep \"Total\" | awk '{print $4}'", "r");

            fgets(path, 1024, fp);
            status = pclose(fp);

            counts[0] = (unsigned int)((atof(path))*1000*1000);
        #endif

        return retval;
    }

    static void write_file(char* file_name, string content) {
        ofstream myfile;
        myfile.open (file_name);
        myfile << content;
        myfile.close();
    }

    string print_time(){
        char buffer[26];
        int millisec;
        struct tm* tm_info;
        struct timeval tv;

        gettimeofday(&tv, NULL);

          millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
          if (millisec>=1000) { // Allow for rounding up to nearest second
            millisec -=1000;
            tv.tv_sec++;
        }

        tm_info = localtime(&tv.tv_sec);

        strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);

        char buff[26+7];
        snprintf(buff, sizeof(buff), "%s.%03d", buffer, millisec);
        return buff;
    }

    void recordEnergyDevice() {
        string records = "Timestamp,Total_Power_mic0\n";

        int retval = 0;
        long long counts[MICPOWER_MAX_COUNTERS];    // used for caching
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 50000000L; // 50 milliseconds
        double energy = 0.0;

        int count_total = 0;

        while (keepAlive) {
            retval = read_sysfs_file(counts);
            count_total ++;
            records += print_time() + "," + to_string(counts[0] * 1e-6) + "\n";
            energy += counts[0];
            nanosleep(&ts, NULL);
        }
        keepAlive = true;
        passEnergy = energy / count_total;
        if(writeToFile)
            write_file("power_consumption.csv", records);
        energy = 0.0;
    }

    static void * startRecordingEnergy(void * This) {
        ((xMeterPU *)This)->recordEnergyDevice();
        return NULL;
    }


    void startEnergy() {
        int iret1 = pthread_create(&micPthread, NULL, startRecordingEnergy, this);
    }

    void stopEnergy() {
        keepAlive = false;
        pthread_join(micPthread, NULL);
        passEnergy = passEnergy*1e-6; //converting nanowatts to watts
    }

    double getEnergy() {
        return passEnergy;
    }

    void startTime() {
        passTime = omp_get_wtime();
    }

    void stopTime() {
        passTime = (omp_get_wtime() - passTime);
    }

    double getTime() {
        return passTime;
    }

public:
    bool writeToFile = false;

    xMeterPU(int itype) {
        type = itype;
    }

    void start() {
        if(type == XPHI_ENERGY)
            startEnergy();
        else if (type == XPHI_TIME)
            startTime();
        else
            printf("Type %d provided in the constructor is not supported. Please use XPHI_ENERGY or XPHI_TIME instead. \n", type);
    }

    void stop() {
        if(type == XPHI_ENERGY)
            stopEnergy();
        else if (type == XPHI_TIME)
            stopTime();
        else
            printf("Type %d provided in the constructor is not supported. Please use XPHI_ENERGY or XPHI_TIME instead. \n", type);
    }

    double get_value() {
        if(type == XPHI_ENERGY)
            return getEnergy();
        else if (type == XPHI_TIME)
            return getTime();
        else
            return -1;
    }

    void print() {
        if(type == XPHI_ENERGY)
            printf("MIC Energy consumed is %f Watts\n", getEnergy());
        else if (type == XPHI_TIME)
            printf("MIC Elapsed time is %f seconds\n", getTime());
        else
            printf("Type %d provided in the constructor is not supported. Please use XPHI_ENERGY or XPHI_TIME instead. \n", type);

        if(writeToFile && type == XPHI_ENERGY)
            printf("A log file (named power_consumption.csv) of power consumption is generated in your home directory. \n");

    }
};
