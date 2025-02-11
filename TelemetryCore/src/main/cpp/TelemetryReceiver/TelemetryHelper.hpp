//
// Created by geier on 08/10/2020.
//

#ifndef FPV_VR_OS_TELEMETRYHELPER_HPP
#define FPV_VR_OS_TELEMETRYHELPER_HPP

#include <string>
#include <WFBTelemetryData.h>
#include <cmath>

// Contains only helper functions that take up so much code lines that they should be stored elsewhere than inside TelemetryReceiver
class TelemetryHelper{
public:
    static std::wstring getMAVLINKFlightModeAsWString(const bool isQuadcopter, const int FlightMode_MAVLINK, const bool FlightMode_MAVLINK_armed){
        std::wstring mode;
        if(isQuadcopter){
            switch (FlightMode_MAVLINK) {
                case 0:mode = L"STAB";break;
                case 1:mode = L"ACRO";break;
                case 2:mode = L"ALTHOLD";break;
                case 3:mode = L"AUTO";break;
                case 4:mode = L"GUIDED";break;
                case 5:mode = L"LOITER";break;
                case 6:mode = L"RTL";break;
                case 7:mode = L"CIRCLE";break;
                case 9:mode = L"LAND";break;
                case 11:mode = L"DRIFT";break;
                case 13:mode = L"SPORT";break;
                case 14:mode = L"FLIP";break;
                case 15:mode = L"AUTOTUNE";break;
                case 16:mode = L"POSHOLD";break;
                case 17:mode = L"BRAKE";break;
                case 18:mode = L"THROW";break;
                case 19:mode = L"AVOIDADSB";break;
                case 20:mode = L"GUIDEDNOGPS";break;
                default:mode = L"-----";break;
            }
        }else{
            switch (FlightMode_MAVLINK) {
                case 0: mode = L"MAN"; break;
                case 1: mode = L"CIRC"; break;
                case 2: mode = L"STAB"; break;
                case 3: mode = L"TRAI"; break;
                case 4: mode = L"ACRO"; break;
                case 5: mode = L"FBWA"; break;
                case 6: mode = L"FBWB"; break;
                case 7: mode = L"CRUZ"; break;
                case 8: mode = L"TUNE"; break;
                case 10: mode = L"AUTO"; break;
                case 11: mode = L"RTL"; break;
                case 12: mode = L"LOIT"; break;
                case 15: mode = L"GUID"; break;
                case 16: mode = L"INIT"; break;
                default:
                    mode = L"-----";
                    break;
            }
        }
        if(!FlightMode_MAVLINK_armed){
            mode=L"["+mode+L"]";
        }
        return mode;
    }
    static std::string getEZWBDataAsString(const wifibroadcast_rx_status_forward_t2& wifibroadcastTelemetryData){
        std::stringstream ss;
        ss << "damaged_block_cnt:" << wifibroadcastTelemetryData.damaged_block_cnt << "\n";
        ss << "lost_packet_cnt:" << wifibroadcastTelemetryData.lost_packet_cnt << "\n";
        ss << "skipped_packet_cnt:" << wifibroadcastTelemetryData.skipped_packet_cnt << "\n";
        ss << "received_packet_cnt:" << wifibroadcastTelemetryData.received_packet_cnt << "\n";
        ss << "kbitrate:" << wifibroadcastTelemetryData.kbitrate << "\n";
        ss << "kbitrate_measured:" << wifibroadcastTelemetryData.kbitrate_measured << "\n";
        ss << "kbitrate_set:" << wifibroadcastTelemetryData.kbitrate_set << "\n";
        ss << "lost_packet_cnt_telemetry_up:" << wifibroadcastTelemetryData.lost_packet_cnt_telemetry_up << "\n";
        ss << "lost_packet_cnt_telemetry_down:" << wifibroadcastTelemetryData.lost_packet_cnt_telemetry_down << "\n";
        ss << "lost_packet_cnt_msp_up:" << wifibroadcastTelemetryData.lost_packet_cnt_msp_up << "\n";
        ss << "lost_packet_cnt_msp_down:" << wifibroadcastTelemetryData.lost_packet_cnt_msp_down << "\n";
        ss << "lost_packet_cnt_rc:" << wifibroadcastTelemetryData.lost_packet_cnt_rc << "\n";
        ss << "current_signal_joystick_uplink:" << (int) wifibroadcastTelemetryData.current_signal_joystick_uplink << "\n";
        ss << "current_signal_telemetry_uplink:" << (int) wifibroadcastTelemetryData.current_signal_telemetry_uplink << "\n";
        ss << "joystick_connected:" << (int) wifibroadcastTelemetryData.joystick_connected << "\n";
        ss << "cpuload_gnd:" << (int) wifibroadcastTelemetryData.cpuload_gnd << "\n";
        ss << "temp_gnd:" << (int) wifibroadcastTelemetryData.temp_gnd << "\n";
        ss << "cpuload_air:" << (int) wifibroadcastTelemetryData.cpuload_air << "\n";
        ss << "temp_air:" << (int) wifibroadcastTelemetryData.temp_air << "\n";
        ss << "wifi_adapter_cnt:" << wifibroadcastTelemetryData.wifi_adapter_cnt << "\n";
        for (int i = 0; i < wifibroadcastTelemetryData.wifi_adapter_cnt; i++) {
            const auto& adapter=wifibroadcastTelemetryData.adapter[i];
            ss << "Adapter" << i << " dbm:" << (int) adapter.current_signal_dbm
               << " pkt:"<< (int) adapter.received_packet_cnt
               << "type"<< ((adapter.type==0) ? "A": "R")
               << "\n";
        }
        return ss.str();
    }
    //taken from tinygps: https://github.com/mikalhart/TinyGPS/blob/master/TinyGPS.cpp#L296
    //changed slightly to use double precision
    static const double distance_between(double lat1, double long1, double lat2, double long2) {
        // returns distance in meters between two positions, both specified
        // as signed decimal-degrees latitude and longitude. Uses great-circle
        // distance computation for hypothetical sphere of radius 6372795 meters.
        // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
        // Courtesy of Maarten Lamers
        double delta = (long1-long2)*0.017453292519;
        double sdlong = sin(delta);
        double cdlong = cos(delta);
        lat1 = (lat1)*0.017453292519;
        lat2 = (lat2)*0.017453292519;
        double slat1 = sin(lat1);
        double clat1 = cos(lat1);
        double slat2 = sin(lat2);
        double clat2 = cos(lat2);
        delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
        delta = delta*delta;
        delta += (clat2 * sdlong)*(clat2 * sdlong);
        delta = sqrt(delta);
        double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
        delta = atan2(delta, denom);
        return delta * 6372795;
    }
    //taken from tinygps: https://github.com/mikalhart/TinyGPS/blob/master/TinyGPS.cpp#L321
    //changed slightly to use double precision
    static const double course_to (double lat1, double long1, double lat2, double long2)
    {
        // returns course in degrees (North=0, West=270) from position 1 to position 2,
        // both specified as signed decimal-degrees latitude and longitude.
        // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
        // Courtesy of Maarten Lamers
        double dlon = (long2-long1)*0.017453292519;
        lat1 = (lat1)*0.017453292519;
        lat2 = (lat2)*0.017453292519;
        double a1 = sin(dlon) * cos(lat2);
        double a2 = sin(lat1) * cos(lat2) * cos(dlon);
        a2 = cos(lat1) * sin(lat2) - a2;
        a2 = atan2(a1, a2);
        if (a2 < 0.0)
        {
            a2 += M_PI*2;
        }
        return (180.0/M_PI)*(a2);
    }
};
#endif //FPV_VR_OS_TELEMETRYHELPER_HPP
