#include <stdio.h>
#include <stdlib.h>
#include "date.h"

// Macros and Enumerations
enum FORMATING { DAYS = 2, MONTHS = 2, YEARS = 4};

// Definitions for each structure
typedef struct date {
    int day;
    int month;
    int year;
} Date;

// File Specific Prototypes
int getNumber(char *str, int *pos, enum FORMATING format);

Date *date_create(char *datestr) {
    Date *date = malloc(sizeof(Date));
    int pos = (int) datestr;
    date->day = getNumber(datestr, &pos, DAYS);
    date->month = getNumber(datestr, &pos, MONTHS);
    date->year = getNumber(datestr, &pos, YEARS);
    
    // Make sure date is in correct format
    if (date->day == -1 || date->month == -1 || date->year == -1) {
        date_destroy(date);
        return NULL;
    } 
    
    // Make sure date is completely valid in calander
    int daysinmonth[] = {0,31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if ((!(date->year % 4) && date->year % 100) || !(date->year % 400)) {
        daysinmonth[1] = 29;
    }
    
    if (date->day <= 0 || date->day > daysinmonth[date->month]) {
        date_destroy(date);
        return NULL;
    }

    return date;
}

int date_compare(Date *d1, Date *d2) {

    if (d1->year < d2->year)
       return -1;

    else if (d1->year > d2->year)
       return 1;

    if (d1->year == d2->year)
    {
         if (d1->month<d2->month)
              return -1;
         else if (d1->month>d2->month)
              return 1;
         else if (d1->day<d2->day)
              return -1;
         else if(d1->day>d2->day)
              return 1;
         else
              return 0;
    }

}

Date *date_duplicate(Date *d) {
    Date *date = malloc(sizeof(d));
    if (date != NULL) {
        *date = *d;
        return date;
    } else {
        return NULL;
    }
}

void date_destroy(Date *d) {
    free(d);
}

int getNumber(char *str, int *pos, enum FORMATING format) {
    int number = 0;
    int format_count = format;
    str = (void*)*pos;
    while (*str != '/' && *str != '\0') {
        format_count--;
        if (*str >= '0' && *str <= '9') {
            number = number*10 + (int) (*str-'0');
        } else {
            return -1;
        }
        str++;
    }
    *pos = (int)++str;
    if (format_count != 0) {
        return -1;
    } else {
        return number;
    }
}