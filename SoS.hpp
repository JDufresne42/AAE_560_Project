/*
 * File: SoS.hpp
 * Author: Jonathan S. Dufresne
 * Description: Header for SoS class
 *              Contains system of satellite systems
 * */

#pragma once

#include<string>

#include "Receiver.hpp"

class SoS {
public:
    SoS() {}

    void buildSystems(const std::string&);

    // read-only accessors
    const std::vector<Satellite>& constellationSys1() const noexcept {
        return sys1_sats;
    }
    const std::vector<Satellite>& constellationSys2() const noexcept {
        return sys2_sats;
    }
    const std::vector<Receiver>& receiversSys1() const noexcept {
        return sys1_recs;
    }
    const std::vector<Receiver>& receiversSys2() const noexcept {
        return sys2_recs;
    }

    void aimSats();
    void runSatelliteSelection(int);
    std::string analyze();
    void calc_data_out(const std::string&);
    void feasibleCount_out(const std::string&);

private:
    std::vector<Satellite> sys1_sats;
    std::vector<Receiver> sys1_recs;
    std::vector<Satellite> sys2_sats;
    std::vector<Receiver> sys2_recs;
    double SNR_min = 25; // minimum threshold for signal to noise ratio dB
    double INR_max = -12.2; // threshold for prohibitive interference

    /*
    * satellite selection
    * both systems maximize SNR, no knowledge sharing
    * */
    Satellite& satSelectBasic(int);

    /*
    * protected satellite selection
    * primary system takes best SNR
    * secondary system takes best SNR where INR on primary system is below threshold
    * secondary system will know primary system sat-rec pairs
    * */
    Satellite& satSelectProtected();

    /*
    * unprotected satellite selection
    * primary system takes best SNR
    * secondary system takes best SINR knowing which primary system satellite is in use
    * */
    Satellite&  satSelectBestSys2();
};