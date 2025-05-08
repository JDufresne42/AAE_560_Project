/*
 * File: main.cpp
 * Author: Jonathan S. Dufresne
 * Description: top-level entry point
 * */

#include<iostream>
#include<filesystem>
#include<fstream>

#include "SoS.hpp"

void generateInput(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Warning: could not write input file" << std::endl;
    }
    out << "Receivers:\n";
    out << 1 << ' ' << 1 << ' ' <<  0 << ' ' << 0.0 << ' ' << 8.0 << '\n';
    out << 2 << ' ' << 1 << ' ' <<  -1 << ' ' << 0.0 << ' ' << 8.0 << "\n\n";
    out << "Satellites:\n";
    // sys 1
    int sat_id = 2;
    int sys_id = 1;
    double x = 0;
    out << sys_id << ' ' << 1 << ' ' <<  0 << ' ' << 550.0 << '\n';
    for (int i = 0; i < 50; ++i) {
        x = 5 * i + 1;
        out << sys_id << ' ' << sat_id << ' ' <<  x << ' ' << 550.0 << '\n';
        sat_id++;
        out << sys_id << ' ' << sat_id << ' ' <<  -x << ' ' << 550.0 << '\n';
        sat_id++;
    }
    // sys 2
    sys_id = 2;
    sat_id = 1;
    for (int i = 0; i < 300; ++i) {
        x = 2.5 * i + 1;
        out << sys_id << ' ' << sat_id << ' ' <<  x << ' ' << 610.0 << '\n';
        sat_id++;
        out << sys_id << ' ' << sat_id << ' ' <<  -x << ' ' << 610.0 << '\n';
        sat_id++;
    }
}

int main() {
    generateInput("input.txt");
    // build system
    SoS sos1;
    SoS sos2;
    SoS sos3;
    sos1.buildSystems("input.txt");
    sos2.buildSystems("input.txt");
    sos3.buildSystems("input.txt");
    // aim sats
    sos1.aimSats();
    sos2.aimSats();
    sos3.aimSats();
    // satellite selection
    sos1.runSatelliteSelection(1); // basic sat selection
    sos2.runSatelliteSelection(2); // protected sat selection
    sos3.runSatelliteSelection(3); // sys2 max SINR
    // analysis
    std::ofstream out("satSelection.txt");
    if (!out) {
        std::cerr << "Error: Could not open file for writing\n";
        return 0;
    }
    out << sos1.analyze();
    out << sos2.analyze();
    out << sos3.analyze();
    out.close();
    
    sos2.calc_data_out("calc_data.txt");
    sos2.feasibleCount_out("feasibleCount.txt");
    return 0;
}

