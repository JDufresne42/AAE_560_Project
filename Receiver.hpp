/*
 * File: Receiver.hpp
 * Author: Jonathan S. Dufresne
 * Description: Header for Receiver class
 *              Represents a ground receiver antenna
 * */

#pragma once
#include<vector>
#include "Satellite.hpp"

class Receiver {
    public:
    // Constructors
    Receiver(int, int, Vec2, double);
    Receiver();

    ~Receiver();

    // Utilities
    int getSysID() const;
    int getRecID() const;
    Vec2 getRecPos() const;
    Satellite& getInSysSat();
    Satellite& getOutSysSat();
    double getPr_req_dBm() const;
    double getGr_dBi() const;

    void setSysID(int);
    void setRecID(int);
    void setRecPos(Vec2);
    void setRecPos(double, double);

    void pairSat(Satellite&);
    void setOutSysSat(Satellite&);
    void calcSignalStuff();
   
    // before / during satellite selection -> no in_sys_sat or out_sys_sat
    double getElevationAngle(Vec2);
    double calc_sat_int_angle(const Satellite&);
    double calc_rec_int_angle(const Satellite&, const Satellite&);
    double calc_Gt_int(const Satellite&);
    double calc_Gr_int(const Satellite&, const Satellite&);
    double calc_FSPL_dB(const Satellite&); // FSPL(this, sat) in dB
    double calc_SNR(const Satellite&); // SNR(this, sat) in dB
    double calc_INR(const Satellite&, const Satellite&);
    double calc_SINR(const Satellite&, const Satellite&);
    
    Vec2 worstCaseAngle(const Satellite&, const Satellite&, double);
    double calc_Gt_int_UN(const Satellite&, int);
    double calc_Gr_int_UN(const Satellite&, const Satellite&, int);
    double calc_INR_UN(const Satellite&, const Satellite&, int);
    double calc_SINR_UN(const Satellite&, const Satellite&, int);
    
    private:
    int sys_id;
    int rec_id;
    Vec2 rec_pos; // km
    //int rec_type;
    double N; // NxN antenna array
    
    Satellite in_sys_sat;
    Vec2 in_sys_rel_pos; // km
    Vec2 in_sys_unit;
    double in_sys_dist; // meters
    bool in_sys_sat_def;

    Satellite out_sys_sat;
    Vec2 out_sys_rel_pos; // km
    Vec2 out_sys_unit;
    double out_sys_dist; // meters
    bool out_sys_sat_def;

    // signal stuff
    double lambda;
    double Gr_dBi;      // boresight receive gain in dBi
    double Gr_lin;      // boresight receive gain linear
    double Aeff;        // effective area of receiver
    double Pn_dBm;      // noise power in dBm
    double Pr_req_dBm;  // minimum required receive power
    
    double fc = 20e9; // Hz
    double B = 400e6; // Hz
    double M = 64; // satellite array dimension
    double eta = 0.6;
    double T0 = 290; // noise temperature K
    double nf = 1.2; // noise figure dB
    double SNR_min = 25; // minimum threshold for signal to noise ratio dB
    double INR_max = -12.2; // threshold for prohibitive interference
};