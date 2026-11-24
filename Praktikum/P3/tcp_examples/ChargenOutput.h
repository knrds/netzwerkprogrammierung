#ifndef CHARGEN_OUTPUT_H
#define CHARGEN_OUTPUT_H

#include <string.h>
#include <time.h>
#include "Socket.h"

#include <string.h>
#include <time.h>

/* Sendet eine Chargen-Zeile und verschiebt den Startindex. */
static inline int SendChargenLine(int fd, int *start)
{
    char line[74];

    for (int i = 0; i < 72; i++)
        line[i] = 32 + (*start + i) % 95;

    line[72] = '\r';
    line[73] = '\n';

    if (SendAll(fd, line, sizeof(line)) == -1)
        return -1;

    *start = (*start + 1) % 95;
    return 0;
}





static inline int SendDNA(int fd, int *frame)
{
    const int wave[] = {
         0,  1,  3,  5,  8, 10, 12, 13,
        14, 13, 12, 10,  8,  5,  3,  1,
         0, -1, -3, -5, -8,-10,-12,-13,
       -14,-13,-12,-10, -8, -5, -3, -1
    };

    int a = 14 + wave[*frame % 32];
    int b = 28 - a;

    char line[512] = "";

    for (int i = 0; i < 29; i++) {

        if (i == a && i == b)
            strcat(line, "\033[97m@");

        else if (i == a)
            strcat(line, "\033[96m@");

        else if (i == b)
            strcat(line, "\033[95m@");

        else if ((i > a && i < b) ||
                 (i > b && i < a))
            strcat(line, "\033[90m-");

        else
            strcat(line, " ");
    }

    strcat(line, "\033[0m\r\n");

    if (SendAll(fd, line, strlen(line)) == -1)
        return -1;

    *frame = (*frame + 1) % 32;

    struct timespec delay = {0, 90000000L};
    nanosleep(&delay, NULL);

    return 0;
}


#endif
