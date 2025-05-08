/*
 * File: Satellite.hpp
 * Author: Jonathan S. Dufresne
 * Description: Header for Satellite class
 *              Represents a satellite with transmitter antenna
 * */

#pragma once

#include<iostream>
#include<string>
#include<sstream>

#include "MyUtil.hpp"

class Satellite {
    public:
    // Constructors
    Satellite(int, int, Vec2);
    Satellite();

    ~Satellite();

    // Utilities
    Satellite& operator=(const Satellite&)= default;
    int getSysID() const;
    int getSatID() const;
    Vec2 getSatPos() const;
    Vec2 getSatDir() const;
    double getFc() const;
    double getB() const;
    double getGtLin() const;
    double getGt_dBi() const;
    double getPt_dBW() const;
    double getPt_dBm() const;

    void setSysID(int);
    void setSatID(int);
    void setSatPos(double, double);
    void setSatPos(Vec2);
    void aimSat(Vec2);
    
    bool inUse() const;
    void activate();
    void deactivate();

    Vec2 recToSat(Vec2) const;
    Vec2 satToRec(Vec2) const;
    
    std::string toString() const;
    void calcSignalStuff();

    private:
    // ID of system satellite is part of
    int sys_id;     
    // ID of satellite
    int sat_id;
    bool sat_pos_defined;
    bool in_use;
    Vec2 sat_pos; // km
    Vec2 sat_direction; // vector towards paired receiver
    
    // signal stuff
    double Gt_dBi; // boresight transmit gain dBi
    double Gt_lin; // boresight transmit gain linear
    double Pt_dBW;
    double fc = 20e9; //Hz
    double bandwidth = 400e6; // Hz
    double lambda;
    double A;
    double eta = 0.6; // aperture efficiency
    int M = 64, N = 64; // 64x64 antenna array
    double Sp;
    double EIRP_dBm;
};