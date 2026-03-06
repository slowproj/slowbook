// slowbook/trend.hpp //
// Created by Sanshiro Enomoto on 27 Jun 2024

#pragma once

#include <memory>
#include <vector>
#include <array>
#include <cmath>


namespace slowbook {

    
struct TrendBucket {
    double t, n, sum, sum2, min, max, last;
    TrendBucket(double time): t(time), n(0), sum(0), sum2(0), min(NAN), max(NAN), last(NAN) {}
    double mean() const { return n>0 ? sum/n : NAN; }
    double var() const { auto m = mean(); return n>0 ? sum2/n-m*m : NAN; }
    double std() const { auto v = var(); return v>=0 ? std::sqrt(v) : NAN; }
    double sem() const { auto v = var(); return ((v>=0) && (n > 0)) ? std::sqrt(v)/n : NAN; }
    void fill(double x) {
        n += 1;
        sum += x;
        sum2 += x*x;
        last = x;
        if (! (x > min)) min = x;
        if (! (x < max)) max = x;
    }
};


struct TrendPointMean: public std::array<double,2> {
    static constexpr std::array<const char*,2> fields = { "t", "x" };
    static constexpr std::array<const char*,2> graph_fields = { "x", "y" };
    TrendPointMean(const TrendBucket& b): std::array<double,2>{{b.t, b.mean()}} {}
    TrendPointMean(double t, double x): std::array<double,2>{{t, x}} {}
};

struct TrendPointSum: public std::array<double,2> {
    static constexpr std::array<const char*,2> fields = { "t", "x" };
    static constexpr std::array<const char*,2> graph_fields = { "x", "y" };
    TrendPointSum(const TrendBucket& b): std::array<double,2>{{b.t, b.sum}} {}
    TrendPointSum(double t, double x): std::array<double,2>{{t, x}} {}
};

struct TrendPointN: public std::array<double,2> {
    static constexpr std::array<const char*,2> fields = { "t", "x" };
    static constexpr std::array<const char*,2> graph_fields = { "x", "y" };
    TrendPointN(const TrendBucket& b): std::array<double,2>{{b.t, b.n}} {}
    TrendPointN(double t, double x): std::array<double,2>{{t, x}} {}
};

struct TrendPointMeanErr: public std::array<double,4> {
    static constexpr std::array<const char*,4> fields = { "t", "x", "x_err", "x_n" };
    static constexpr std::array<const char*,4> graph_fields = { "x", "y", "y_err", "y_n" };
    TrendPointMeanErr(const TrendBucket& b): std::array<double,4>{{b.t, b.mean(), b.sem(), b.n}} {}
    TrendPointMeanErr(double t, double x, double sem, double n): std::array<double,4>{{t, x, sem, n}} {}
};

struct TrendPointMeanMinMax: public std::array<double,5> {
    static constexpr std::array<const char*,5> fields = { "t", "x", "x_n", "x_min", "x_max" };
    static constexpr std::array<const char*,5> graph_fields = { "x", "y", "y_n", "y_min", "y_max" };
    TrendPointMeanMinMax(const TrendBucket& b): std::array<double,5>{{b.t, b.mean(), b.n, b.min, b.max}} {}
    TrendPointMeanMinMax(double t, double x, double n, double min, double max): std::array<double,5>{{t, x, n, min, max}} {}
};

struct TrendPointMeanErrMinMax: public std::array<double,6> {
    static constexpr std::array<const char*,6> fields = { "t", "x", "x_err", "x_n", "x_min", "x_max" };
    static constexpr std::array<const char*,6> graph_fields = { "x", "y", "y_err", "y_n", "y_min", "y_max" };
    TrendPointMeanErrMinMax(const TrendBucket& b): std::array<double,6>{{b.t, b.mean(), b.sem(), b.n, b.min, b.max}} {}
    TrendPointMeanErrMinMax(double t, double x, double sem, double n, double min, double max): std::array<double,6>{{t, x, sem, n, min, max}} {}
};

struct TrendPointMeanStdMinMax: public std::array<double,6> {
    static constexpr std::array<const char*,6> fields = { "t", "x", "x_n", "x_std", "x_min", "x_max" };
    static constexpr std::array<const char*,6> graph_fields = { "x", "y", "y_n", "y_std", "y_min", "y_max" };
    TrendPointMeanStdMinMax(const TrendBucket& b): std::array<double,6>{{b.t, b.mean(), b.n, b.std(), b.min, b.max}} {}
    TrendPointMeanStdMinMax(double t, double x, double n, double std, double min, double max): std::array<double,6>{{t, x, n, std, min, max}} {}
};

struct TrendPointLastMinMax: public std::array<double,4> {
    static constexpr std::array<const char*,4> fields = { "t", "x", "x_min", "x_max" };
    static constexpr std::array<const char*,4> graph_fields = { "x", "y", "y_min", "y_max" };
    TrendPointLastMinMax(const TrendBucket& b): std::array<double,4>{{b.t, b.last, b.min, b.max}} {}
    TrendPointLastMinMax(double t, double x, double min, double max): std::array<double,4>{{t, x, min, max}} {}
};

struct TrendPointLastErrMinMax: public std::array<double,6> {
    static constexpr std::array<const char*,6> fields = { "t", "x", "x_err", "x_n", "x_min", "x_max" };
    static constexpr std::array<const char*,6> graph_fields = { "x", "y", "y_err", "y_n", "y_min", "y_max" };
    TrendPointLastErrMinMax(const TrendBucket& b): std::array<double,6>{{b.t, b.last, b.sem(), b.n, b.min, b.max}} {}
    TrendPointLastErrMinMax(double t, double x, double sem, double n, double min, double max): std::array<double,6>{{t, x, sem, n, min, max}} {}
};


    
template<class XTrendPoint>
struct BareTrend: protected std::vector<XTrendPoint> {
    double start;
    using std::vector<XTrendPoint>::operator[];
    using std::vector<XTrendPoint>::clear;
    using std::vector<XTrendPoint>::size;
    using std::vector<XTrendPoint>::empty;
    using std::vector<XTrendPoint>::begin;
    using std::vector<XTrendPoint>::end;
    using point_type = XTrendPoint;
  public:
    BareTrend(double a_start=0): start(a_start) {}
  public:
    struct Mark { typename std::vector<XTrendPoint>::size_type position; };
    Mark mark() const {
        return Mark{size()};
    }
    BareTrend since(const Mark& mark) const {
        BareTrend trend(start);
        auto length = size() - mark.position;
        if (length > 0) {
            trend.assign(begin()+length, end());
        }
        return trend;
    }
};


    
template<class XTrendPoint>
struct Trend: public BareTrend<XTrendPoint> {
    using BareTrend<XTrendPoint>::start;
    using BareTrend<XTrendPoint>::size;
  public:
    Trend(double a_step, double a_start=NAN): BareTrend<XTrendPoint>(a_start), step(a_step), current_bucket(step/2) {}
    void fill(double t, double x=NAN) { // NAN value is to fill empty time buckets
        if (std::isnan(start)) {
            start = t;
        }
        long index = std::floor((t - start)/step);
        if (index < size()) {
            // ERROR: fill() must be ordered in increasing time
            return;
        }
        while (index > size()) {
            std::vector<XTrendPoint>::push_back(XTrendPoint(current_bucket));
            current_bucket = TrendBucket(step * (size() + 0.5));
        }
        if (! std::isnan(x)) {
            current_bucket.fill(x);
        }
    }
    Trend& flush() {
        if (current_bucket.n > 0) {
            push_back(XTrendPoint(current_bucket));
        }
        return *this;
    }
    Trend& truncate() {  // this keeps the current bucket; to delete it, do flush().truncate()
        double length = step * size();
        start += length;
        current_bucket.t -= length;
        BareTrend<XTrendPoint>::clear();
        return *this;
    }
  protected:
    double step;
    TrendBucket current_bucket;
};



struct RateTrend: public Trend<TrendPointSum> {
    RateTrend(double a_step, double a_start=NAN): Trend<TrendPointSum>(a_step, a_start), weight(1.0/step) {}
    void fill(double t, double x=NAN) {
        Trend<TrendPointSum>::fill(t, weight);
    }
  protected:
    double weight;
};
    
    
}
