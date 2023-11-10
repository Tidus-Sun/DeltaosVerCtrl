#include <stdio.h>
#include <time.h>

void getCurrentDateAsString(char *dateString)
{
    time_t t = time(NULL);
    struct tm currentTime = *localtime(&t);

    sprintf(dateString, "%04d%02d%02d", currentTime.tm_year + 1900, currentTime.tm_mon + 1, currentTime.tm_mday);
}
