/*
 * File: Receiver.cpp
 * Author: Jonathan S. Dufresne
 * Description: Receiver class implementation
 *              Represents a ground receiver antenna
 * */

#include<cmath>
#include<algorithm>
#include<limits>

#include "Receiver.hpp"

Receiver::Receiver(int sys_id_, int rec_id_, Vec2 pos_, double dim_) {
    sys_id = sys_id_;
    rec_id = rec_id_;
    rec_pos = pos_;
    //rec_type = type_;
    N = dim_;
    in_sys_sat_def = false;
    out_sys_sat_def = false;
    calcSignalStuff();
}

Receiver::Receiver() {
    sys_id = 0;
    rec_id = 0;
    rec_pos = Vec2();
    //rec_type = 1;
    N = 1;
    in_sys_sat_def = false;
    out_sys_sat_def = false;
    calcSignalStuff();
}

Receiver::~Receiver() {
    // Satellite objects will deconstruct automatically
}

int Receiver::getSysID() const { return sys_id; }

int Receiver::getRecID() const { return rec_id; }

Vec2 Receiver::getRecPos() const { return rec_pos; }

double Receiver::getPr_req_dBm() const { return Pr_req_dBm; }

double Receiver::getGr_dBi() const { return Gr_dBi; };

Satellite& Receiver::getInSysSat() {
    return in_sys_sat;
}

Satellite& Receiver::getOutSysSat() {
    return out_sys_sat;
}

void Receiver::setSysID(int id) {
    sys_id = id;
}

void Receiver::setRecID(int id) {
    rec_id = id;
}

void Receiver::setRecPos(Vec2 pos_) {
    rec_pos = pos_;
}

void Receiver::setRecPos(double x_, double y_) {
    rec_pos = Vec2(x_, y_);
}

void Receiver::pairSat(Satellite& sat) {
    in_sys_sat = sat;
    in_sys_sat.activate();
    in_sys_rel_pos = in_sys_sat.recToSat(rec_pos);
    in_sys_dist = in_sys_rel_pos.magnitude_m();
    in_sys_unit = in_sys_rel_pos.unitVec();
    in_sys_sat.aimSat(rec_pos);
    in_sys_sat_def = true;
}

void Receiver::setOutSysSat(Satellite& sat) {
    out_sys_sat = sat;
    out_sys_rel_pos = out_sys_sat.recToSat(rec_pos);
    out_sys_dist = out_sys_rel_pos.magnitude_m();
    out_sys_unit = out_sys_rel_pos.unitVec();
    out_sys_sat_def = true;
}

double Receiver::getElevationAngle(Vec2 sat_pos) {
    double h,v,theta;
    h = sat_pos.x-rec_pos.x;
    v = sat_pos.y-rec_pos.y;
    if (h == 0) {
        return g_PI / 2;
    }
    theta = std::atan(v/h);
    return theta;
}

void Receiver::calcSignalStuff() {
    lambda = g_C/fc;

    double dx = 0.5 * lambda;
    Aeff = (N*dx)*(N*dx);

    Gr_lin = eta * 4.0 * g_PI * Aeff / (lambda*lambda);
    Gr_dBi = 10.0 * std::log10(Gr_lin);

    Pn_dBm = 10.0 * std::log10(g_K * T0 * B) + 30; // dBm -> -174 dBm/Hz + 10 log_10(B)
    Pn_dBm += nf;
    Pr_req_dBm = Pn_dBm + SNR_min;
}

// FSPL(this, sat) in dB
double Receiver::calc_FSPL_dB(const Satellite& sat) {
    double temp;
    Vec2 vec = sat.recToSat(rec_pos);
    double range = vec.magnitude_m();
    temp = 4.0 * g_PI * range / lambda; // squared in return
    return 20.0 * std::log10(temp);
}

// SNR(this, sat) in dB
double Receiver::calc_SNR(const Satellite& sat) {
    double Pt_dBm = sat.getPt_dBm();
    double Gt_dBi = sat.getGt_dBi();
    double fspl = calc_FSPL_dB(sat);

    // Ptx(sat) + Gtx(this, sat) + Grx(this, sat) - L(this, sat) - Pn(this)
    return Pt_dBm + Gt_dBi + Gr_dBi - fspl - Pn_dBm;
}

// angle between aim of sat and direction to rec
double Receiver::calc_sat_int_angle(const Satellite& sat) {
    Vec2 s_unit = sat.getSatDir().unitVec();
    Vec2 i_unit = sat.satToRec(rec_pos).unitVec();
    double cosAng = std::clamp(s_unit.dot(i_unit), -1.0, 1.0);
    return std::acos(cosAng); // radians, no need for magnitudes bc unit vectors
}

// transmit gain of interference in dBi
double Receiver::calc_Gt_int(const Satellite& sat) {
    double theta = calc_sat_int_angle(sat);
    double psi = g_PI * std::sin(theta); // for half wave spacing
    double num = std::sin(M * psi / 2.0);
    double den = std::sin(psi / 2.0);
    double AF_norm;
    if (std::abs(den) < 1e-8) {
        AF_norm = 1.0;
    } else {
        AF_norm = (1 / M) * num / den;
    }
    double AF_dB = 20.0 * std::log10(std::abs(AF_norm));
    return sat.getGt_dBi() + AF_dB;
}

// angle between aim of rec and direction to sat
double Receiver::calc_rec_int_angle(const Satellite& in_sat, const Satellite& out_sat) {
    Vec2 s_unit = out_sat.recToSat(rec_pos).unitVec();
    Vec2 p_unit = in_sat.recToSat(rec_pos).unitVec();
    double cosAng = std::clamp(p_unit.dot(s_unit), -1.0, 1.0);
    return std::acos(cosAng); // radians, no need for magnitudes bc unit vectors
}

// receive gain of interference in dBi
double Receiver::calc_Gr_int(const Satellite& in_sat, const Satellite& out_sat) {
    double theta = calc_rec_int_angle(in_sat, out_sat);
    double psi = g_PI * std::sin(theta); // for half wave spacing
    double num = std::sin(N * psi / 2.0);
    double den = std::sin(psi / 2.0);
    double AF_norm;
    if (std::abs(den) < 1e-8) {
        AF_norm = 1.0;
    } else {
        AF_norm = (1 / N) * num / den;
    }
    double AF_dB = 20.0 * std::log10(std::abs(AF_norm));
    return Gr_dBi + AF_dB;
}

// INR(this, in_sys_sat; sat) in dB
double Receiver::calc_INR(const Satellite& in_sat, const Satellite& out_sat) {
    double Pt_dBm = out_sat.getPt_dBm();
    double Gt_int_dBi = calc_Gt_int(out_sat);
    double Gr_int_dBi = calc_Gr_int(in_sat, out_sat);
    double FSPL = calc_FSPL_dB(out_sat);

    // Ptx(s) + Gtx(u, s;v) + Grx(u, s;p) - L(u, s) - Pn(u)
    return Pt_dBm + Gt_int_dBi + Gr_int_dBi - FSPL - Pn_dBm;
}

double Receiver::calc_SINR(const Satellite& in_sat, const Satellite& out_sat) {
    double sinr;
    double snr_dB = calc_SNR(in_sat);
    double inr_dB = calc_INR(in_sat, out_sat);

    // linear -> snr / (1 + inr) but we have in dB
    // -> snr_dB - 10*log10(1 + inr_lin)
    sinr = snr_dB - 10.0 * std::log10(1.0 + std::pow(10.0, inr_dB/10.0));
    return sinr;
}