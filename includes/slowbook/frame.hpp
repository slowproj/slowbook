// slowbook/frame.hpp //
// Created by Sanshiro Enomoto on 27 Jun 2024

#pragma once

#include <string>
#include <vector>
#include <time.h>
#include "variant.hpp"
#include "record.hpp"


namespace slowbook {


struct RecordSet: protected std::vector<Record> {
    const Schema& schema;
    std::vector<Variant> default_times, default_tags;
    using std::vector<Record>::begin;
    using std::vector<Record>::end;
  public:
    RecordSet(const Schema& a_schema): schema(a_schema) {
    }
    RecordSet(const Record& record): schema(record.schema) {
        this->add(Record(record));
    }
    RecordSet(Record&& record): schema(record.schema) {
        this->add(std::move(record));
    }
    RecordSet& add(const Record& record) {
        return this->add(Record(record));
    }
    RecordSet& add(Record&& record) {
        for (auto& time: default_times) {
            record.time(time);
        }
        for (auto& tag: default_tags) {
            record.tag(tag);
        }
        for (auto& time: record.time_values) {
            if (time.is_null) {
                time = long(::time(NULL));
            }
        }
        push_back(std::move(record));
        return *this;
    }
};


template<class XDataFrame>
// XDataFrame is a subclass of DataFrame (CRTP); returning XDataFrame& is necessary for overloaded operator<<().
struct DataFrame: public RecordSet {
    DataFrame(const Schema& schema): RecordSet(schema) {
    }
    XDataFrame& time(const Variant& time) {
        default_times.push_back(time);
        return *static_cast<XDataFrame*>(this);
    }
    XDataFrame& time(Variant&& time) {
        default_times.push_back(std::move(time));
        return *static_cast<XDataFrame*>(this);
    }
    XDataFrame& tag(const Variant& tag) {
        default_tags.push_back(tag);
        return *static_cast<XDataFrame*>(this);
    }
    XDataFrame& tag(Variant&& tag) {
        default_tags.push_back(std::move(tag));
        return *static_cast<XDataFrame*>(this);
    }

    // for each XDataFrame, for each value type X, implement the overloaded operator<<() below
    // to fill a value of type X into a data store for XDataFrame:
    //    template<typename X> XDataFrame& operator<<(XDataFrame& frame, const X& value)
};

    
}
