#include "datetime.h"

/*******************************************************************************
 * Integer date/time to string conversion mappings
 ******************************************************************************/

const char* days_short[] = {
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
    "SUN"
};

const char* days_long[] = {
    "Monday",
    "Tuesday",
    "Wednsday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};

const char* months_long[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "Septmber",
    "October",
    "November",
    "December"
};

const char* months_short[] = {
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JUL",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC"
};

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

inline unsigned char bcd2char(unsigned char x) {
    return (x & 0xf) + (((x & 0xf0) >> 4) * 10);
}

inline unsigned char char2bcd(unsigned char x) {
    return ((x / 10) << 4) | (x % 10);
}

rtccTime bcdTime2DecTime(rtccTime tm) {
    tm.sec = bcd2char(tm.sec);
    tm.min = bcd2char(tm.min);
    tm.hour = bcd2char(tm.hour);
    
    return tm;
}

rtccDate bcdDate2DecDate(rtccDate dt) {
    dt.mday = bcd2char(dt.mday);
    dt.mon = bcd2char(dt.mon);
    dt.year = bcd2char(dt.year);
    
    return dt;
}

rtccTime decTime2BcdTime(rtccTime tm) {
    tm.sec = char2bcd(tm.sec);
    tm.min = char2bcd(tm.min);
    tm.hour = char2bcd(tm.hour);
    
    return tm;
}

rtccDate decDate2BcdDate(rtccDate dt) {
    dt.mday = char2bcd(dt.mday);
    dt.mon = char2bcd(dt.mon);
    dt.year = char2bcd(dt.year);
    
    return dt;
}

unsigned char twentyFour2TwelveHour(unsigned char hour) {
    if (hour == 0) {
        return 12;
    }
    else if (hour > 12) {
        return hour - 12;
    }
    else {
        return hour;
    }
}

void rtcc_init() {
    rtccTime tm;
    rtccDate dt;
    
    // Time this code was finalized
    tm.l=0x00;
    tm.sec=0x00;                                                                 
    tm.min=0x15;
    tm.hour=0x19;

    dt.wday=0x6;
    dt.mday=0x06;
    dt.mon=0x12;
    dt.year=0x15;
    
    RtccOpen(tm.l, dt.l, 0);
}

void setTime(unsigned char hour, unsigned char min, unsigned char sec) {
    static rtccTime tm;
    
    if (hour > 23 || min > 59 || sec > 59) {
        return;
    }

    tm.l=0x00;
    tm.sec=sec;                                                                 
    tm.min=min;
    tm.hour=hour;
    
    RtccSetTime(decTime2BcdTime(tm).l);
}

void setDate(unsigned char month, unsigned char mday,
             unsigned char year, unsigned char wday) {
    static rtccDate dt;

    if (month > 12 || mday > 31 || year > 99 || wday > 6) {
        return;
    }
    
    dt.wday=wday;
    dt.mday=mday;
    dt.mon=month;
    dt.year=year;
    
    RtccSetDate(decDate2BcdDate(dt).l);
}