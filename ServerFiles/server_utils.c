#include "server_utils.h"
#include <stdio.h>
#include <time.h>

void printOrario(){
      time_t mytime = time(NULL);
      struct tm *tmp = gmtime(&mytime);
      printf("[%02d:%02d:%02d]", tmp->tm_hour + 1, tmp->tm_min, tmp->tm_sec);
}