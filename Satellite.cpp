/*
 * File: Satellite.cpp
 * Author: Jonathan S. Dufresne
 * Description: Satellite class implementation
 *              Represents a satellite with transmitter antenna
 * */

#include<cmath>

#include "Satellite.hpp"

Satellite::Satellite(int sys_id_, int sat_id_, Vec2 pos_) {
    sys_id = sys_id_;
    sat_id = sat_id_;
    in_use = false;
    sat_pos_defined = true;
    sat_pos = pos_;
    calcSignalStuff();
}

Satellite::Satellite() {
    sys_id = 0;
    sat_id = 0;
    sat_pos_defined = false;
    sat_pos = Vec2();
}

Satellite::~Satellite() {
    // std structures automatically delete
}

int Satellite::getSysID() const { return sys_id; }

int Satellite::getSatID() const { return sat_id; }

Vec2 Satellite::getSatPos() const { return sat_pos; }

Vec2 Satellite::getSatDir() const { return sat_direction; }

double Satellite::getFc() const { return fc; }

double Satellite::getB() const { return bandwidth; }

double Satellite::getGtLin() const { return Gt_lin; }

double Satellite::getGt_dBi() const { return Gt_dBi; }

double Satellite::getPt_dBW() const { return Pt_dBW; }

double Satellite::getPt_dBm() const { return Pt_dBW + 30; }

bool Satellite::inUse() const { return in_use; }

void Satellite::activate() {
    in_use = true;
}

void Satellite::deactivate() {
    in_use = false;
}

void Satellite::setSysID(int sys_id_) {
    sys_id = sys_id_;
}

void Satellite::setSatID(int sat_id_) {
    sat_id = sat_id_;
}

void Satellite::setSatPos(double x_, double y_) {
    sat_pos_defined = true;
    sat_pos = Vec2(x_, y_);
}

void Satellite::setSatPos(Vec2 pos_) {
    sat_pos_defined = true;
    sat_pos = pos_;
}

void Satellite::aimSat(Vec2 pos) {
    sat_direction = pos - sat_pos;
}

Vec2 Satellite::recToSat(Vec2 rec_pos) const {
    return sat_pos-rec_pos;
}

Vec2 Satellite::satToRec(Vec2 rec_pos) const {
    return rec_pos-sat_pos;
}

std::string Satellite::toString() const {
    std::ostringstream oss;
    oss << "system ID: " << sys_id << "\nsatellite ID: " << sat_id << "\n";
    if (!sat_pos_defined) {
        oss << "position undefined\n";
    } else {
        oss << "position: <" << sat_pos.x << ", " << sat_pos.y << ">\n";
    }
    std::string s = oss.str();
    
    return s;
}

void Satellite::calcSignalStuff() {
    lambda = g_C / fc; // wavelength
    double dx = 0.5 * lambda; // half-wave spacing
    double dy = dx;
    A = (M*dx)*(N*dy); // aperature area
    Gt_lin = eta * (4.0 * g_PI * A) / (lambda*lambda); // linear transmit gain
    Gt_dBi = 10.0 * std::log10(Gt_lin); // transmit gain dBi ~38.8 ish

    switch(sys_id) {
        case 1:
            Sp = -54.3;
            break;
        case 2:
            Sp = -53.3;
            break;
        default:
            std::cerr << "Warning: unknown sys_id\n";
    }
    Pt_dBW = Sp + 10.0 * std::log10(bandwidth);
}