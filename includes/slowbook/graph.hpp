// slowbook/graph.hpp //
// Created by Sanshiro Enomoto on 27 Jun 2024

#pragma once

#include <memory>
#include <vector>
#include <array>
#include <cmath>


namespace slowbook {

    
struct PointXY: public std::array<double,2> {
    static constexpr std::array<const char*,2> fields = { "x", "y" };
    PointXY(double x, double y): std::array<double,2>{{x, y}} {}
};

struct PointXYZ: public std::array<double,3> {
    static constexpr std::array<const char*,3> fields = { "x", "y", "z" };
    PointXYZ(double x, double y, double z): std::array<double,3>{{x, y, z}} {}
};

struct PointXYEy: public std::array<double,3> {
    static constexpr std::array<const char*,3> fields = { "x", "y", "ey" };
    PointXYEy(double x, double y, double ey): std::array<double,3>{{x, y, ey}} {}
};

struct PointXYExEy: public std::array<double,4> {
    static constexpr std::array<const char*,4> fields = { "x", "y", "ex", "ey" };
    PointXYExEy(double x, double y, double ex, double ey): std::array<double,4>{{x, y, ex, ey}} {}
};

struct PointXYZEz: public std::array<double,4> {
    static constexpr std::array<const char*,4> fields = { "x", "y", "z", "ez" };
    PointXYZEz(double x, double y, double z, double ez): std::array<double,4>{{x, y, z, ez}} {}
};


    
template<class XPoint>
struct Graph: protected std::vector<XPoint> {
    using Base = std::vector<XPoint>;
    using Base::operator[];
    using Base::clear;
    using Base::size;
    using Base::empty;
    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;
    template<typename... Args> void add_point(Args&&... args) {
        Base::emplace_back(std::forward<Args>(args)...);
    }
  public:
    struct Mark { typename Base::size_type position; };
    Mark mark() const {
        return Mark{size()};
    }
    Graph since(const Mark& mark) const {
        Graph graph;
        auto length = size() - mark.position;
        if (length > 0) {
            graph.assign(begin()+length, end());
        }
        return graph;
    }
};
    
}
