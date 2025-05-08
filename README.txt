## Use:

Systems are built from the text file "input.txt" using the formatting below

Position vectors are relative to local center {0,0}: surface level at the center of local area

Satellites are positioned at {x, y>0} with y on the order of 500km

Satellites all use 64x64 antenna arrays with half-wave spacing

Receivers may use an NxN antenna array with half-wave spacing


## Units:

Position Coordinates: Kilometers
Distances: Kilometers and Meters
Minimum Elevation Angle: radians
Most units indicated in variable names

## File Reader:

Lines starting with '#' will be skipped by file reader

File reading requires both "Receivers:" and "Satellites:" headers on their own individual lines

Format for receivers:
int (system ID) int (object ID) double (X position coord) double (Y position coord) int (type) double (diameter or array dimension)
ex: 1 1 5 0 8 -> receiver 1 in sys 1 located at {5,0} (on surface 5km right of center) and has an 8x8 antenna array

Format for satellites: 
int (system ID) int (object ID) double (X position coord) double (Y position coord)
ex: 1 1 10 500 -> satellite 1 in sys 1 located at {10,500} (500km above surface, 10km right of center)

Input Template:

Receivers:
# List receiver objects here


Satellites:
# List satellite objects here


## Outputs
calc_data.txt: comma separated data dump
#sat_index, sys1_sat_range, SNR_sys1_dB, INR_pv_dB, SINR_sys2_dB, sys2_sat_range, SNR_sys2_dB, INR_su_dB, SINR_sys1_dB

    sys1_sat_range, sys2_sat_range: x position coordinate (horizontal distance)

    SNR_sys1_dB: SNR from sys1 receiver to sys1_sats[i]
    SNR_sys2_dB: same respectively

    INR_pv_dB: Interference by sat sys1_sats[i] on rec V
    INR_su_dB: Interference by sat sys2_sats[i] on rec U

    SINR_sys1_dB: SINR for rec U to sat P interfered by sat S
    SINR_sys2_dB: same respectively

feasibleCount.txt: comma separated data dump
#INR_th, count, percent

INR_th: INR threshold for sys2 on sys1 [-3, -18]
count: number of sys2 satellites that meet threshold
percent: percent of sys2 satellites that meet threshold