/*
 * File: MyUtil.hpp
 * Author: Jonathan S. Dufresne
 * Description: common utility header
 * */

#pragma once
#include<cmath>
#include<string>
#include<sstream>

const double g_PI = 3.14159; // tasty
const double g_min_el_angle = 0.610865; // radians
const double g_C = 299792458; // Speed of light m/s
const double g_K = 1.38e-23; // Boltsmann's constant J/K
const double INF = std::numeric_limits<double>::max() / 2;

struct Vec2
{
    double x, y;

    // Constructors
    Vec2() : x(0), y(0) {}
    Vec2(double x_, double y_) : x(x_), y(y_) {}

    // Operations
    Vec2 operator+(Vec2 o) const { return {x+o.x, y+o.y}; }
    Vec2 operator-(Vec2 o) const { return {x-o.x, y-o.y}; }
    Vec2 operator*(double s) const { return {x*s, y*s}; }

    inline double dot(Vec2 o) const noexcept { return x*o.x + y*o.y; }
    inline double magnitude_km() const noexcept { return std::hypot(x,y); }
    inline double magnitude_m() const noexcept { return std::hypot(x,y) * 1000; }
    inline Vec2 unitVec() const noexcept { 
        double mag = magnitude_km();
        return mag>0 ? Vec2{x/mag, y/mag} : Vec2{0,0};
    }
    inline std::string toString() { 
        std::ostringstream oss;
        oss << "{" << x << ", " << y << "}";
        return oss.str();
    }
};
