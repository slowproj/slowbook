// slowbook/record.hpp //
// Created by Sanshiro Enomoto on 27 Jun 2024

#pragma once

#include <string>
#include <vector>
#include "variant.hpp"


namespace slowbook {


struct Schema {
    std::string table_name;
    std::vector<std::string> time_names, tag_names, field_names;
    std::vector<Variant> time_protos, tag_protos, field_protos;
  public:
    Schema(const std::string& a_table_name): table_name(a_table_name) {}
    template<typename T> Schema& add_time(const std::string& name, Variant default_value=Variant(T()).nullify()) {
        time_names.push_back(name);
        time_protos.emplace_back(default_value);
        return *this;
    }
    template<typename T> Schema& add_tag(const std::string& name, Variant default_value=Variant(T()).nullify()) {
        tag_names.push_back(name);
        tag_protos.emplace_back(default_value);
        return *this;
    }
    template<typename T> Schema& add_field(const std::string& name, Variant default_value=Variant(T()).nullify()) {
        field_names.push_back(name);
        field_protos.emplace_back(default_value);
        return *this;
    }
};


    
struct SimpleNumericSchema: public Schema {
  public:
    SimpleNumericSchema(const std::string& table_name): Schema(table_name) {
        add_time<long>("time");
        add_tag<std::string>("channel");
        add_field<double>("value");
    }
};
    
struct SimpleObjectSchema: public Schema {
  public:
    SimpleObjectSchema(const std::string& table_name): Schema(table_name) {
        add_time<long>("time");
        add_tag<std::string>("channel");
        add_field<std::string>("value");
    }
};



struct Record {
    const Schema& schema;
    std::vector<Variant> time_values;
    std::vector<Variant> tag_values;
    std::vector<Variant> field_values;
    bool is_to_update;
  private:
    unsigned time_index, tag_index, field_index;
  public:
    Record(const Schema& schema_):
        schema(schema_),
        time_values(schema_.time_protos),
        tag_values(schema_.tag_protos),
        field_values(schema_.field_protos)
    {
        is_to_update = false;
        time_index = tag_index = field_index = 0;
    }
    Record& time(const Variant& value) {
        if (time_index < time_values.size()) {
            try {
                time_values[time_index++].assign(value);
            }
            catch (std::exception& e) {
                throw std::runtime_error("slowbook::Record::time(): bad time value: \"" + value.as_text() + "\":  " + e.what());
            }
        }
        return *this;
    }
    Record& time(Variant&& value) {
        if (time_index < time_values.size()) {
            try {
                time_values[time_index++].assign(value);
            }
            catch (std::exception& e) {
                throw std::runtime_error("slowbook::Record::time(): bad time value: \"" + value.as_text() + "\": " + e.what());
            }
        }
        return *this;
    }
    Record& tag(const Variant& value) {
        if (tag_index < tag_values.size()) {
            try {
                tag_values[tag_index++].assign(value);
            }
            catch (std::exception& e) {
                throw std::runtime_error("slowbook::Record::tag(): bad tag value: \"" + value.as_text() + "\": " + e.what());
            }
        }
        return *this;
    }
    Record& tag(Variant&& value) {
        if (tag_index < tag_values.size()) {
            try {
                tag_values[tag_index++].assign(value);
            }
            catch (std::exception& e) {
                throw std::runtime_error("slowbook::Record::tag(): bad tag value: \"" + value.as_text() + "\": " + e.what());
            }
        }
        return *this;
    }
    Record& value(const Variant& value) {
        if (field_index < field_values.size()) {
            try {
                field_values[field_index++].assign(value);
            }
            catch (std::exception& e) {
                throw std::runtime_error("slowbook::Record::value(): bad field value: \"" + value.as_text() + "\": " + e.what());
            }
        }
        return *this;
    }
    Record& value(Variant&& value) {
        if (field_index < field_values.size()) {
            try {
                field_values[field_index++].assign(value);
            }
            catch (std::exception& e) {
                throw std::runtime_error("slowbook::Record::value(): bad field value: \"" + value.as_text() + "\": " + e.what());
            }
        }
        return *this;
    }
};

    
}
