// slowbook/trend.hpp //
// Created by Sanshiro Enomoto on 27 Jun 2024

#pragma once

#include <string>
#include <sstream>
#include <stdexcept>


namespace slowbook {

    
struct Variant {
    // this is basically std::any, but avoids using "new" and "virtual".
    enum { TYPE_VOID, TYPE_BOOL, TYPE_INTEGER, TYPE_REAL, TYPE_TEXT } type;
    struct Value {
        union {
            bool as_bool;
            long as_integer;
            double as_real;
        };
        std::string as_text;
        bool is_null;
    } value;
    bool is_null;  // has a type, has no value; conceptually "T* is NULL" where T can be non-VOID
  public:
    Variant(): type(TYPE_VOID), is_null(false) {}
    Variant(bool value_): type(TYPE_BOOL), is_null(false) { value.as_bool = value_; }
    Variant(int value_): type(TYPE_INTEGER), is_null(false) { value.as_integer = value_; }
    Variant(long value_): type(TYPE_INTEGER), is_null(false) { value.as_integer = value_; }
    Variant(float value_): type(TYPE_REAL), is_null(false) { value.as_real = value_; }
    Variant(double value_): type(TYPE_REAL), is_null(false) { value.as_real = value_; }
    Variant(const std::string& value_): type(TYPE_TEXT), is_null(false) { value.as_text = value_; }
    Variant(const char* value_): type(TYPE_TEXT), is_null(false) { value.as_text = value_; }
    Variant& nullify() { is_null = true; return *this; }

    bool is_numeric() const { return (type == TYPE_INTEGER) || (type == TYPE_REAL); }
    std::string type_name() const {
        switch(type) {
          case TYPE_VOID: return "void";
          case TYPE_BOOL: return "bool";
          case TYPE_INTEGER: return "integer";
          case TYPE_REAL: return "real";
          case TYPE_TEXT: return "text";
          default: return "undefined";  // type does not exist (where "void" is a type)
        }
    }
    
    bool as_bool() const {
        if (is_null) {
            return false;
        }
        switch (type) {
          case TYPE_BOOL:
            return value.as_bool;
          case TYPE_INTEGER:
            return bool(value.as_integer);
          case TYPE_REAL:
            return bool(value.as_real);
          case TYPE_TEXT:
            return value.as_text.empty();
          default /* TYPE_VOID */:
            return false;
        }
    }
    long as_integer() const {
        if (is_null) {
            return 0;
        }
        switch (type) {
          case TYPE_BOOL:
            return long(value.as_bool);
          case TYPE_INTEGER:
            return value.as_integer;
          case TYPE_REAL:
            return long(value.as_real);
          case TYPE_TEXT:
            try {
                return stol(value.as_text);
            }
            catch (std::exception&) {
                throw std::invalid_argument("slowbook::Variant: bad string to convert to int: " + value.as_text);
            }
          default /* TYPE_VOID */:
            break;
        }
        return 0;
    }
    double as_real() const {
        if (is_null) {
            return 0;
        }
        switch (type) {
          case TYPE_BOOL:
            return long(value.as_bool);
          case TYPE_INTEGER:
            return value.as_integer;
          case TYPE_REAL:
            return long(value.as_real);
          case TYPE_TEXT:
            try {
                return stod(value.as_text);
            }
            catch (std::exception&) {
                throw std::invalid_argument("slowbook::Variant: bad string to convert to float: " + value.as_text);
            }
          default /* TYPE_VOID */:
            break;
        }
        return 0;
    }
    std::string as_text(int precision=0) const {
        if (is_null) {
            return "null";
        }
        if (type == TYPE_TEXT) {
            return value.as_text;
        }
        
        else if (type == TYPE_VOID) {
            return "null";
        }
        else if (type == TYPE_BOOL) {
            return value.as_bool ? "true" : "false";
        }
        else if (type == TYPE_INTEGER) {
            return std::to_string(value.as_integer);
        }
        
        if (type == TYPE_REAL) {
            std::ostringstream os;
            if (precision > 0) {
                os.precision(precision);
            }
            os << value.as_real;
            return os.str();
        }
        
        return "null";
    }

    Variant& assign(const Variant& value_) {
        if (value_.is_null) {
            this->is_null = true;
            return *this;
        }
        this->is_null = false;
        
        if (type == value_.type) {
            *this = value_;
        }
        
        else if (type == TYPE_BOOL) {
            this->value.as_bool = value_.as_bool();
        }
        else if (type == TYPE_INTEGER) {
            this->value.as_integer = value_.as_integer();
        }
        else if (type == TYPE_REAL) {
            this->value.as_real = value_.as_real();
        }
        else if (type == TYPE_TEXT) {
            this->value.as_text = value_.as_text();
        }
        
        else /* if (type == TYPE_VOID) */ {
            this->is_null = true;
        }
        
        return *this;
    }
        
    std::string as_literal(char quote='"', int precision=0) const {
        if (type == TYPE_TEXT) {
            std::ostringstream os;
            if (quote != '\0') {
                os << quote;
            }
            for (const auto& ch: value.as_text) {
                if (ch == quote) {
                    os << "\\" << quote;
                    continue;
                }
                else if (ch == '\\') {
                    os << "\\\\";
                    continue;
                }
                else if (ch == '\n') {
                    os << "\\n";
                    continue;
                }
                else if ((ch == ',') && (quote == '\0')) {
                    os << "\\,";
                    continue;
                }
                os << ch;
            }
            if (quote != '\0') {
                os << quote;
            }

            return os.str();
        }

        std::string literal = this->as_text(precision);
        if ((type == TYPE_REAL) && ! is_null) {
            if (
                (literal.find_first_of('.') == std::string::npos) &&
                (literal.find_first_of('e') == std::string::npos)
            ){
                literal += ".0";
            }
        }

        return literal;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Variant& value) {
        return os << value.as_text();
    }
};

    
}
