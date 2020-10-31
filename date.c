#include <stdio.h>
#include <stdlib.h>
#include "date.h"

// Macros and Enumerations
enum FORMATING {DAYS = 2, MONTHS = 2, YEARS = 4};
#define NUMB_BASE 10

// Definitions for each structure
struct date {
    int day;
    int month;
    int year;
};

// File Specific Prototypes
int getNumber(char *str, int *pos, enum FORMATING format);

Date *date_create(char *datestr) {
    Date *date = (Date *)malloc(sizeof(Date));
    int pos = 0;
    date->day = getNumber(datestr, &pos, DAYS);
    date->month = getNumber(datestr, &pos, MONTHS);
    date->year = getNumber(datestr, &pos, YEARS);
    
    // Make sure date is in correct format
    if (date->day == -1 || date->month == -1 || date->year == -1) {
        date_destroy(date);
        return NULL;
    } 
    return date;
}

int date_compare(Date *date1, Date *date2) {
    int comparison = 0;
    if (date1->year == date2->year){
        if (date1->month<date2->month) {
            comparison = -1;
        } else if (date1->month>date2->month) {
            comparison = 1;
        } else if (date1->day<date2->day) {
            comparison = -1;
        } else if(date1->day>date2->day) {
            comparison = 1;
        } else {
            comparison = 0;
        }
    } else if (date1->year < date2->year) {
       comparison = -1;
    } else {
       comparison = 1;
    }
    return comparison;
}

Date *date_duplicate(Date *d) {
    Date *date = (Date *) malloc(sizeof(Date));
    if (date != NULL) {
        *date = *d;
    } else {
        return NULL;
    }
    return date;
}

void date_destroy(Date *d) {
    free(d);
}

// Given a format such as in the FORMARTING enum, return whether or not a substring of a date string is in the correct format
// If wrong format, return -1 
int getNumber(char *str, int *pos, enum FORMATING format) {
    int number = 0;
    int format_count = format;
    str = str + (*pos);
    while (format_count >= 0 && *str != '/' && *str != '\0') {
        if (*str >= '0' && *str <= '9') {
            number = number*NUMB_BASE + (int) (*str-'0');
        } else {
            break;
        }
        format_count--;
        str++; (*pos)++;
    }
    if (format_count == 0) {
        (*pos)++;
    } else {
        return -1;
    }
    return number;
}