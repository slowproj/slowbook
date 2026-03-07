// slowbook/histogram.hpp //
// Created by Sanshiro Enomoto on 27 Jun 2024

#pragma once

#include <memory>
#include <vector>
#include <array>
#include <cmath>


namespace slowbook {

    
struct Histogram: protected std::vector<double> {
    double min, max;
    double overflow, underflow;
    using Base = std::vector<double>;
    using Base::operator[];
    using Base::size;
    using Base::empty;
    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;
  public:
    Histogram(unsigned nbins_, double min_, double max_): std::vector<double>(nbins_, 0), min(min_), max(max_) {
        overflow = 0;
        underflow = 0;
    }
    void fill(double value, double weight=1) {
        if (value < min) {
            underflow += weight;
        }
        else if (value >= max) {
            overflow += weight;
        }
        else {
            at(int(size()*(value-min)/(max-min))) += weight;
        }
    }
    void clear(void) {
        assign(size(), 0);
    }
  public:
    struct Mark { std::shared_ptr<Histogram> hist; };
    Mark mark() const {
        return Mark{ std::make_shared<Histogram>(*this) };
    }
    Histogram since(const Mark& mark) const {
        Histogram hist(*this);
        for (unsigned i = 0; i < size(); i++) {
            hist[i] -= mark.hist->at(i);
        }
        hist.overflow -= mark.hist->overflow;
        hist.underflow -= mark.hist->underflow;
        return hist;
    }
};



struct Histogram2d: protected std::vector<std::vector<double>> {
    double xmin, xmax, ymin, ymax;
    using Base = std::vector<std::vector<double>>;
    using Base::operator[];
    using Base::size;
    using Base::empty;
    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;
  public:
    Histogram2d(unsigned xnbins_, double xmin_, double xmax_, unsigned ynbins_, double ymin_, double ymax_): std::vector<std::vector<double>>(ynbins_), xmin(xmin_), xmax(xmax_), ymin(ymin_), ymax(ymax_) {
        for (auto& ystrip: *this) {
            ystrip.resize(xnbins_, 0);
        }
    }
    void fill(double x, double y, double weight=1) {
        if ((x >= xmin) && (x < xmax) && (y >= ymin) && (y < ymax)) {
            int ix = int(front().size() * (x-xmin)/(xmax-xmin));
            int iy = int(size() * (y-ymin)/(ymax-ymin));
            at(iy).at(ix) += weight;
        }
    }
    void clear(void) {
        for (auto& ystrip: *this) {
            ystrip.assign(ystrip.size(), 0);
        }
    }
  public:
    struct Mark { std::shared_ptr<Histogram2d> hist; };
    Mark mark() const {
        return Mark{ std::make_shared<Histogram2d>(*this) };
    }
    Histogram2d since(const Mark& mark) const {
        Histogram2d hist(*this);
        for (unsigned i = 0; i < size(); i++) {
            auto& ystrip = hist[i];
            for (unsigned j = 0; j < ystrip.size(); j++) {
                ystrip[j] -= (*mark.hist)[i][j];
            }
        }
        return hist;
    }
};

    
}
