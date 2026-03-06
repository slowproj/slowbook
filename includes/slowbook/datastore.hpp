// slowbook/datastore.hpp //
// Created by Sanshiro Enomoto on 27 Jun 2024

#pragma once

#include <string>
#include <map>
#include <memory>
#include <iostream>
#include <fstream>
#include "frame.hpp"


namespace slowbook {

    
class DataStore {
  public:
    virtual ~DataStore() = default;
  public:
    void append(const RecordSet& record_set) { this->write_all(record_set, false); }
    void update(const RecordSet& record_set) { this->write_all(record_set, true); }
  protected:
    virtual void write_all(const RecordSet& record_set, bool is_to_update) {
        for (auto& record: record_set) {
            this->write(record, is_to_update);
        }
    }
    virtual void write(const Record& record, bool is_to_update) = 0;
};


    
class DataStore_JsonDump: public DataStore {
  public:
    DataStore_JsonDump(std::ostream& os_=std::cout): os(os_) {
        os << "[" << std::endl;
    }
    ~DataStore_JsonDump() override {
        os << "\"COMPLETE\": {}" << std::endl;
        os << "]" << std::endl;
    }
  protected:
    void write(const Record& record, bool is_to_update) override {
        os << (is_to_update ? "\"UPDATE\"" : "\"APPEND\"") << ": { " << std::endl;
        os << "    \"table\": \"" << record.schema.table_name << "\"," << std::endl;
        os << "    \"time\": [" << std::endl;
        for (unsigned i = 0; i < record.time_values.size(); i++) {
            if (i > 0) os << "," << std::endl;
            os << "        \"" << record.schema.time_names[i] << "\": ";
            os << record.time_values[i].as_literal();
        }
        os << std::endl << "    ]," << std::endl;
        os << "    \"tags\": [" << std::endl;
        for (unsigned i = 0; i < record.tag_values.size(); i++) {
            if (i > 0) os << "," << std::endl;
            os << "        \"" << record.schema.tag_names[i] << "\": ";
            os << record.tag_values[i].as_literal();
        }
        os << std::endl << "    ]," << std::endl;
        os << "    \"fields\": [" << std::endl;
        for (unsigned i = 0; i < record.field_values.size(); i++) {
            if (i > 0) os << "," << std::endl;
            os << "        \"" << record.schema.field_names[i] << "\": ";
            os << record.field_values[i].as_literal();
        }
        os << std::endl << "    ]" << std::endl;
        os << "}," << std::endl;
    }
  protected:
    std::ostream& os;
};



class DataStore_CsvFile: public DataStore {
  public:
    DataStore_CsvFile(const std::string& prefix_=""): prefix(prefix_) {}
  protected:
    void write(const Record& record, bool is_to_update) override {
        if (outputs.count(record.schema.table_name) == 0) {
            auto os = std::make_shared<std::ofstream>(prefix + record.schema.table_name + ".csv");
            outputs[record.schema.table_name] = os;
            for (unsigned i = 0; i < record.time_values.size(); i++) {
                *os << (i > 0 ? "," : "") << record.schema.time_names[i];
            }
            for (unsigned i = 0; i < record.tag_values.size(); i++) {
                *os << "," << record.schema.tag_names[i];
            }
            for (unsigned i = 0; i < record.field_values.size(); i++) {
                *os << "," << record.schema.field_names[i];
            }
            *os << std::endl;
        }
        
        std::ostream& os = *(outputs[record.schema.table_name]);
        for (unsigned i = 0; i < record.time_values.size(); i++) {
            os << (i > 0 ? "," : "") << record.time_values[i].as_literal('\0');
        }
        for (unsigned i = 0; i < record.tag_values.size(); i++) {
            os << "," << record.tag_values[i].as_literal('\0');
        }
        for (unsigned i = 0; i < record.field_values.size(); i++) {
            os << "," << record.field_values[i].as_literal('\0');
        }
        os << std::endl;
    }
  protected:
    std::string prefix;
    std::map<std::string, std::shared_ptr<std::ostream>> outputs;
};



class DataStore_SQL: public DataStore {
  protected:
    void write(const Record& record, bool is_to_update) override {
        std::ostringstream os;

        if (is_to_update) {
            // Here we do DELETE and INSERT instead of UPDATE, as the record might not exist.
            // It seems like there is no portable way to do UPSERT in RDBMS.
            os << "DELETE..." << std::endl;
        }
        
        os << "INSERT INTO " << record.schema.table_name << " ";
        os << "VALUES(";
        for (const auto& time: record.time_values) {
            os << ", " << time.as_literal('\'');
        }
        for (const auto& tag: record.tag_values) {
            os << ", " << tag.as_literal('\'');
        }
        for (const auto& field: record.field_values) {
            os << ", " << field.as_literal('\'');
        }
        os << ")";
        
        std::cout << os.str() << std::endl;
    }
};


}
