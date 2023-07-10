//
// Created by brent on 7/7/2023.
//

#ifndef CLUSTERDUCK_PROTOCOL_DUCKGPS_H
#define CLUSTERDUCK_PROTOCOL_DUCKGPS_H
#include <TinyGPS++.h>
#include <cdpcfg.h>
#include <ctime>
#include <memory>

class DuckGPS {
public:
    DuckGPS() : GPSSerial(1) {
        GPSSerial.begin(9600, SERIAL_8N1,CDPCFG_GPS_RX,CDPCFG_GPS_TX);
    };
    DuckGPS(int RX, int TX) : GPSSerial(1) {
        GPSSerial.begin(9600, SERIAL_8N1, RX, TX);
    };
    enum AltitudeUnit{
        meter,
        kilo,
        feet,
        miles
    };
    enum SpeedUnit{
        kmph,
        mps, // Speed in meters per second
        mph, // Speed in miles per hour
        knots
    };
    void readData(unsigned long ms);
    void printData();
    double lat(), lng();
    std::time_t epoch();
    double speed(SpeedUnit u);
    uint32_t satellites();
    double altitude(AltitudeUnit u);
protected:
    std::time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss);
private:
    TinyGPSPlus gps;
    HardwareSerial GPSSerial;

};


#endif //CLUSTERDUCK_PROTOCOL_DUCKGPS_H
