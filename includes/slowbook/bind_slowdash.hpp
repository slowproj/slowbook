// slowbook/bind_slowdash.hpp //
// Created by Sanshiro Enomoto on 27 Jun 2024

#pragma once

#include <string>
#include <sstream>
#include "histogram.hpp"
#include "graph.hpp"
#include "trend.hpp"
#include "variant.hpp"
#include "record.hpp"


namespace slowbook {


//// simple scalars (numeric and string) ////


struct SlowDashDataFrame: public DataFrame<SlowDashDataFrame> {
  public:
    SlowDashDataFrame(const Schema& schema): DataFrame(schema) {}
};

SlowDashDataFrame& operator<<(SlowDashDataFrame& frame, double value) {
    frame.add(std::move(Record(frame.schema).value(value)));
    return frame;
}
    
SlowDashDataFrame& operator<<(SlowDashDataFrame& frame, const std::string& value) {
    frame.add(std::move(Record(frame.schema).value(value)));
    return frame;
}

    

//// histograms and graphs are stored as JSON string using enslow() function ////
    
template<class X>
SlowDashDataFrame& operator<<(SlowDashDataFrame& frame, const X& value) {
    frame.add(std::move(Record(frame.schema).value(enslow(value))));
    return frame;
}
    
// histogram serializaiton //
inline std::string enslow(const Histogram& hist) {
    std::ostringstream os;
    
    os << "{ ";
    os << "\"_attr\": { \"Underflow\": " << hist.underflow << ", ";
    os << "\"Overflow\": " << hist.overflow << " }, ";
    os << "\"bins\": { \"min\": " << hist.min << ", \"max\": " << hist.max << " }, ";
    os << "\"counts\": [ ";
    for (unsigned i = 0; i < hist.size(); i++) {
        os << (i==0 ? "" : ", ") << hist[i];
    }
    os << " ]";
    os << " }";
    
    return os.str();
}
    
// histogram(2d) serializaiton //
inline std::string enslow(const Histogram2d& hist) {
    std::ostringstream os;
    
    os << "{ ";
    os << "\"xbins\": { \"min\": " << hist.xmin << ", \"max\": " << hist.xmax << " }, ";
    os << "\"ybins\": { \"min\": " << hist.ymin << ", \"max\": " << hist.ymax << " }, ";
    os << "\"counts\": [ ";
    for (unsigned i = 0; i < hist.size(); i++) {
        os << (i==0 ? "" : ", ") << "[ ";
        for (unsigned j = 0; j < hist[i].size(); j++) {
            os << (j==0 ? "" : ", ") << hist[i][j];
        }
        os << " ]";
    }
    os << " ]";
    os << " }";
    
    return os.str();
}
    
// graph serializaiton //
template<class XPoint>
inline std::string enslow(const Graph<XPoint>& graph) {
    std::ostringstream os;
    
    os << "{ ";
    for (unsigned k = 0; k < std::tuple_size<decltype(XPoint::fields)>::value; k++) {
        os << (k==0 ? "" : ", ");
        os << "\"" << XPoint::fields[k] << "\": [ ";
        for (unsigned i = 0; i < graph.size(); i++) {
            os << (i==0 ? "" : ", ") << graph[i][k];
        }
        os << " ]";
    }
    os << " }";
    
    return os.str();
}

// trend serializaiton as a graph object //
template<class XTrendPoint>
inline std::string enslow(const BareTrend<XTrendPoint>& trend) {
    std::ostringstream os;
    
    os << "{ ";
    auto p = os.precision(12);
    os << "\"_attr\": { \"StartTime\": " << trend.start << " }, ";
    os.precision(p);
    for (unsigned k = 0; k < std::tuple_size<decltype(XTrendPoint::graph_fields)>::value; k++) {
        os << (k==0 ? "" : ", ");
        os << "\"" << XTrendPoint::graph_fields[k] << "\": [ ";
        for (unsigned i = 0; i < trend.size(); i++) {
            os << (i==0 ? "" : ", ") << trend[i][k];
        }
        os << " ]";
    }
    os << " }";
    
    return os.str();
}

    

//// trend serializaiton into multiple rows ////
// If the SimpleTrendShema is used, all the fields are stored in columns.
// If SimpleSchema is used, which has only one numeric value field,
// only the "main" trend field (typically the mean) is stored.
// To store the trend object as a graph object (usually not meaningful), use Record().value(enslow(trend)).    
    
template<class XTrendPoint>
struct SimpleTrendSchema: public Schema {
  public:
    SimpleTrendSchema(const std::string& schema_name): Schema(schema_name) {
        add_time<long>("time");
        add_tag<std::string>("channel");
        for (unsigned k = 1; k < std::tuple_size<decltype(XTrendPoint::fields)>::value; k++) {
            add_field<double>(std::string(XTrendPoint::fields[k]));
        }
    }
};

template<class XTrendPoint>
SlowDashDataFrame& operator<<(SlowDashDataFrame& frame, const BareTrend<XTrendPoint>& trend) {
    unsigned ncols = std::tuple_size<decltype(XTrendPoint::fields)>::value;
    for (const auto& point: trend) {
        Record record(frame.schema);
        record.time(trend.start + point[0]);
        for (unsigned k = 1; k < ncols; k++) {
            record.value(point[k]);
        }
        frame.add(std::move(record));
    }
    
    return frame;
}
    
// this is necessary to prevent "<< enslow(Trend)" being called
template<class XTrendPoint>
SlowDashDataFrame& operator<<(SlowDashDataFrame& frame, const Trend<XTrendPoint>& trend) {
    return operator<<(frame, (const BareTrend<XTrendPoint>&) trend);
}
    
SlowDashDataFrame& operator<<(SlowDashDataFrame& frame, const RateTrend& trend) {
    return operator<<(frame, (const BareTrend<TrendPointSum>&) trend);
}

    
}
