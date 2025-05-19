#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>

#if defined(__OpenBSD__)
    #include <string.h>
    #include <sys/time.h>
    #include <sys/sysctl.h>
    #include <sys/sensors.h>
#endif

#include "ansi.h"

int get_gib(long long value) {
    return value / (1024.0 * 1024.0 * 1024.0);
}

int get_mib(long long value) {
    return value / (1024.0 * 1024.0);
}

#if defined(__OpenBSD__)

    int microkelvin_to_celsius(long int value) {
        return (value - 273150000) / 1E6;
    }

    struct sensor get_sensor_openbsd(char *sensor_name, int sensor_type) {
        int mib[5];
        size_t len, slen;
        int ret;
        struct sensor sensor;
        struct sensordev sensordev;

        mib[0] = CTL_HW;
        mib[1] = HW_SENSORS;


        for (int i = 0; i < 1024; i++) {

            mib[2] = i;

            len = sizeof(struct sensordev);
            ret = sysctl(mib, 3, &sensordev, &len, NULL, 0);

            if (ret == -1) {
                continue;
            }

            mib[3] = sensor_type;

            for (int j = 0; j < sensordev.maxnumt[sensor_type]; j++) {
                mib[4] = j;
                slen = sizeof(struct sensor);
                ret = sysctl(mib, 5, &sensor, &slen, NULL, 0);

                if (ret == -1) {
                    continue;
                }

                if (strcmp(sensordev.xname, sensor_name) == 0) {
                    return sensor;
                }

            }
        }

        return sensor;
    }

#endif

void draw_box(int top) {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);


    printf(top ? "%s%s┏" : "\n\n%s%s┗", RESET, FG);

    int cols = w.ws_col - 2 - (top ? 6 : 0);

    for (int i = 0; i < cols; i++) {
        if (i == (w.ws_col / 2) - 4 && top) {
            printf("nixmon");
        }
        printf("━");
    }

    printf(top ? "┓\n\n%s" : "┛\n\n%s", RESET);
}

void pretty_print(char *content, char *content2) {

    printf("\t%s%s %s", FG, content, RESET);

    if (strlen(content) < 7) {
        printf("\t");
    }

    printf("\t%s\n", content2);
}

void print_title(char *content) {
    printf("\t\b\b%s%s\n\n%s", BG, content, RESET);
}

void print_progress(char *name, float value, float max_value) {
    int percent = (value / max_value) * 100;

    printf("\t%s%s %s", FG, name, RESET);


    if (strlen(name) < 7) {
        printf("\t");
    }

    printf("\t[");
    for (int i = 0; i < 50; i++) {
        if (percent / 2 > i) {
            printf("%s %s", BG, RESET);
        } else {
            printf(" ");
        }
    }
    printf("]");
}
