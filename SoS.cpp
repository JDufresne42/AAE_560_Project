/*
 * File: SoS.cpp
 * Author: Jonathan S. Dufresne
 * Description: SoS class implementation
 *              Contains system of satellite systems
 * */

#include<fstream>
#include<sstream>

#include "SoS.hpp"

void SoS::buildSystems(const std::string& filename) {
    bool recs_done = false;

    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Error: could not open " << filename << "\n";
        return;
    }

    enum Mode { NONE, RECEIVERS, SATELLITES };
    Mode mode = NONE;
    std::string line;
    
    // read file
    while (std::getline(in, line)) {
        int id, system;
        double x_, y_, dim;
        
        // skip empty or comment lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        // read line and check contents/format
        if (line == "Receivers:") {
            mode = RECEIVERS;
            continue;
        } else if (line == "Satellites:") {
            mode = SATELLITES;
            continue;
        }

        std::istringstream iss(line);
        if (!((mode == SATELLITES && (iss >> system >> id >> x_ >> y_)) ||
            (mode == RECEIVERS && (iss >> system >> id >> x_ >> y_ >> dim)))) {
            std::cerr << "Warning: bad input line (" << line << ")\n";
            continue;
        }

        // position
        Vec2 pos = Vec2(x_, y_);

        switch (mode) {
            case RECEIVERS:
                switch (system) {
                    case 1:
                        sys1_recs.emplace_back(system, id, pos, dim);
                        break;
                    case 2:
                        sys2_recs.emplace_back(system, id, pos, dim);
                        break;
                    default:
                        std::cerr << "Warning: unknown system " << system << " in line: " << line << "\n";
                }
                break;
            case SATELLITES:
                switch (system) {
                    case 1:
                        sys1_sats.emplace_back(system, id, pos);
                        break;
                    case 2:
                        sys2_sats.emplace_back(system, id, pos);
                        break;
                    default:
                        std::cerr << "Warning: unknown system " << system << " in line: " << line << "\n";
                }
                break;
            default:
                std::cerr << "Warning: missing section headers\n";
        }
    }
}

void SoS::aimSats() {
    //sys1
    for (int i = 0; i < sys1_sats.size(); ++i) {
        sys1_sats[i].aimSat(sys1_recs[0].getRecPos());
    }
    //sys2
    for (int i = 0; i < sys2_sats.size(); ++i) {
        sys2_sats[i].aimSat(sys2_recs[0].getRecPos());
    }
}

void SoS::runSatelliteSelection(int mode) {
    if (sys1_sats.size() != 0 && sys2_sats.size() != 0 && sys1_recs.size() != 0 && sys2_recs.size() != 0) {
    } else {
        std::cout << "System empty" << std::endl;
        return;
    }
    switch (mode) {
        case 1: {
            // currently set up for only one receiver for each system
            satSelectBasic(1);
            sys2_recs[0].setOutSysSat(sys1_recs[0].getInSysSat());
            std::cout << "Sys1 chose Satellite: \n" << sys1_recs[0].getInSysSat().toString() << std::endl;
            
            satSelectBasic(2);
            sys1_recs[0].setOutSysSat(sys2_recs[0].getInSysSat());
            std::cout << "Sys2 chose Satellite: \n" << sys2_recs[0].getInSysSat().toString() << std::endl;
            break;
        }
        case 2: {
            satSelectProtected();
            break;
        }
        case 3: {
            satSelectBestSys2(); 
            break;
        }
        default:
            std::cerr << "Warning: unknown selection mode\n";
    }
}

Satellite& SoS::satSelectBasic(int sys) {
    int best_index = -1;
    double max_snr = -1;
    double theta;
    double Pt;

    // primary system
    if (sys == 1) { 
        std::vector<double> SNR(sys1_sats.size());
        for (int i = 0; i < sys1_sats.size(); ++i) {
            SNR[i] = sys1_recs[0].calc_SNR(sys1_sats[i]);
            theta = std::abs(sys1_recs[0].getElevationAngle(sys1_sats[i].getSatPos()));
            Pt = sys1_sats[i].getPt_dBm();
            if (SNR[i] > max_snr && theta >= g_min_el_angle && Pt >= sys1_recs[0].getPr_req_dBm()) {
                max_snr = SNR[i];
                best_index = i;
            }
        }
        if (best_index == -1 && max_snr == -1) {
            std::cerr << "Error: no satellite selected\n";
        }
        sys1_recs[0].pairSat(sys1_sats[best_index]);
        return sys1_sats[best_index];
    }

    // secondary system
    if (sys == 2) {
        std::vector<double> SNR(sys2_sats.size());
        for (int i = 0; i < sys2_sats.size(); ++i) {
            SNR[i] = sys2_recs[0].calc_SNR(sys2_sats[i]);
            theta = std::abs(sys2_recs[0].getElevationAngle(sys2_sats[i].getSatPos()));
            Pt = sys2_sats[i].getPt_dBm();
            if (SNR[i] > max_snr && theta >= g_min_el_angle && Pt >= sys2_recs[0].getPr_req_dBm()) {
                max_snr = SNR[i];
                best_index = i;
            }
        }
        if (best_index == -1 && max_snr == -1) {
            std::cerr << "Error: no satellite selected\n";
        }
        sys2_recs[0].pairSat(sys2_sats[best_index]);
        return sys2_sats[best_index];
    }
    throw std::runtime_error{"unknown system in satSelectBasic"};
}

Satellite& SoS::satSelectProtected() {
    // primary system selection does not change -> best SNR
    Satellite sat1 = satSelectBasic(1);
    sys2_recs[0].setOutSysSat(sys1_recs[0].getInSysSat());
    // secondary system selection
    int best_index = -1;
    double max_snr = -1;
    double theta;
    double Pt;
    std::vector<double> INR(sys2_sats.size());
    std::vector<int> S; // vector of indexes of secondary satellites that pass interference threshold

    for (int i = 0; i < sys2_sats.size(); ++i) {
        INR[i] = sys1_recs[0].calc_INR(sys1_recs[0].getInSysSat(), sys2_sats[i]);
        if (INR[i] < INR_max) {
            S.emplace_back(i);
        }
    }
    if (S.size() == 0) {
        std::cerr << "Error: no sys2 sats meet INR threshold" << std::endl;
    }

    std::vector<double> SNR(S.size());
    for (int i = 0; i < S.size(); ++i) {
        SNR[i] = sys2_recs[0].calc_SNR(sys2_sats[S[i]]);
        theta = std::abs(sys2_recs[0].getElevationAngle(sys2_sats[S[i]].getSatPos()));
        Pt = sys2_sats[S[i]].getPt_dBm();
        if (SNR[i] > max_snr && theta >= g_min_el_angle && Pt >= sys2_recs[0].getPr_req_dBm()) {
            max_snr = SNR[i];
            best_index = S[i];
        }
    }
    if (best_index < 0) {
        throw std::runtime_error{"satSelectBasic: no valid satellite found"};
    }

    sys2_recs[0].pairSat(sys2_sats[best_index]);
    sys1_recs[0].setOutSysSat(sys2_recs[0].getInSysSat());

    return sys2_recs[0].getInSysSat();
}

Satellite& SoS::satSelectBestSys2() {
    // primary system selection does not change -> best SNR
    Satellite sat1 = satSelectBasic(1);
    sys2_recs[0].setOutSysSat(sys1_recs[0].getInSysSat());
    
    // secondary system selection -> maximize SINR
    int best_index = -1;
    double max_sinr = -1;
    double epsilon;
    double Pt;
    std::vector<double> SINR(sys2_sats.size());
    for (int i = 0; i < sys2_sats.size(); ++i) {
        SINR[i] = sys2_recs[0].calc_SINR(sys2_sats[i], sat1);
        epsilon = std::abs(sys2_recs[0].getElevationAngle(sys2_sats[i].getSatPos()));
        Pt = sys2_sats[i].getPt_dBm();
        if (SINR[i] > max_sinr && epsilon >= g_min_el_angle && Pt >= sys2_recs[0].getPr_req_dBm()) {
            max_sinr = SINR[i];
            best_index = i;
        }
    }
    if (best_index < 0) {
        throw std::runtime_error{"satSelectBasic: no valid satellite found"};
    }
    
    sys2_recs[0].pairSat(sys2_sats[best_index]);
    sys1_recs[0].setOutSysSat(sys2_recs[0].getInSysSat());
    
    return sys2_recs[0].getInSysSat();
}

std::string SoS::analyze() {
    Satellite sys1_pair = sys1_recs[0].getInSysSat();
    Satellite sys2_pair = sys2_recs[0].getInSysSat();
    std::ostringstream oss;

    // SNR, INR, SINR of sys1
    std::cout << "Analyzing primary system\n";
    double sys1_SNR = sys1_recs[0].calc_SNR(sys1_pair);
    double sys1_INR = sys1_recs[0].calc_INR(sys1_pair, sys2_pair);
    double sys1_SINR = sys1_recs[0].calc_SINR(sys1_pair, sys2_pair);
    std::cout << "SNR = " << sys1_SNR << "\nINR = " << sys1_INR << "\nSINR = " << sys1_SINR << "\n";
    
    Vec2 p_pos = sys1_pair.getSatPos();
    Vec2 u_pos = sys1_recs[0].getRecPos();
    oss << p_pos.x << ',' << p_pos.y << ',' << u_pos.x << ',' <<  u_pos.y << ',';
    oss << sys1_SNR << ',' << sys1_INR << ',' << sys1_SINR << ',';
    
    // SNR, INR, SINR of sys2
    std::cout << "Analyzing secondary system\n";
    double sys2_SNR = sys2_recs[0].calc_SNR(sys2_pair);
    double sys2_INR = sys2_recs[0].calc_INR(sys2_pair, sys1_pair);
    double sys2_SINR = sys2_recs[0].calc_SINR(sys2_pair, sys1_pair);
    std::cout << "SNR = " << sys2_SNR << "\nINR = " << sys2_INR << "\nSINR = " << sys2_SINR << "\n";

    Vec2 s_pos = sys2_pair.getSatPos();
    Vec2 v_pos = sys2_recs[0].getRecPos();
    oss << s_pos.x << ',' << s_pos.y << ',' << v_pos.x << ',' <<  v_pos.y << ',';
    oss << sys2_SNR << ',' << sys2_INR << ',' << sys2_SINR << '\n';

    return oss.str();
}

void SoS::calc_data_out(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Could not open " << filename << " for writing\n";
        return;
    }

    Receiver U_rec = sys1_recs[0];
    Satellite P_sat = U_rec.getInSysSat();
    Receiver V_rec = sys2_recs[0];
    Satellite S_sat = V_rec.getInSysSat();
    int n1 = sys1_sats.size();
    int n2 = sys2_sats.size();
    int n = std::max(n1, n2);

    for (int i = 0; i < n; ++i) {
        out << i;
        if (i < n1) {
            double sys1_range = sys1_sats[i].getSatPos().x; 
            double snr1_dB = U_rec.calc_SNR(sys1_sats[i]);
            double inr_dB_pv = V_rec.calc_INR(S_sat, P_sat);
            double sinr2_dB = V_rec.calc_SINR(S_sat, sys1_sats[i]);
            out << ',' << sys1_range << ',' << snr1_dB << ',' << inr_dB_pv << ',' << sinr2_dB;
        } else {
            out << ",,,,";
        }
        if (i < n2) {
            double sys2_range = sys2_sats[i].getSatPos().x;
            double inr_dB_su = U_rec.calc_INR(P_sat, sys2_sats[i]);
            double snr2_dB = V_rec.calc_SNR(sys2_sats[i]);
            double sinr1_dB = U_rec.calc_SINR(P_sat, sys2_sats[i]);
            double angle = U_rec.calc_rec_int_angle(P_sat, sys2_sats[i]) * 180 / g_PI;
            out << ',' << sys2_range << ',' << snr2_dB << ',' << inr_dB_su << ',' << sinr1_dB << ',' << angle;
        } else {
            out << ",,,,,";
        }
        out << '\n';
    }
    out.close();
}

void SoS::feasibleCount_out(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Could not open " << filename << " for writing\n";
        return;
    }

    Receiver U_rec = sys1_recs[0];
    Satellite P_sat = U_rec.getInSysSat();

    for (double INR_th = -2; INR_th >= -18; INR_th -= 1) {
        double count = 0;
        for (int i = 0; i < sys2_sats.size(); ++i) {
            double inr = U_rec.calc_INR(P_sat, sys2_sats[i]);
            if (inr <= INR_th) {
                count++;
            }
        }
        double percent = count / double(sys2_sats.size());
        out << INR_th << ',' << count << ',' << percent << '\n';
    }
    out.close();
}