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
    
    if (date->day == -1 || date->month == -1 || date->year == -1) {
        date_destroy(date);
        return NULL;
    } 
    
    return date;
}

int date_compare(Date *date1, Date *date2) {
    int d_1 = date1->day + date1->month*100 + date1->year*1000;
    int d_2 = date2->day + date2->month*100 + date2->year*1000;

    if (d_1 > d_2) {
        return 1;
    } else if (d_1 == d_2) {
        return 0;
    } else {
        return -1;
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