// slowbook/uriparse.hpp //
// Created by Sanshiro Enomoto on 3 Mar 2026

#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

namespace slowbook {


struct URI {
    std::string protocol;
    std::string username, password;
    std::string host, port;
    std::string path, query;

    friend std::ostream& operator<<(std::ostream& os, const URI& uri) {
        os << "Protocol: " << uri.protocol << std::endl;
        os << "UserName: " << uri.username << std::endl;
        os << "Password: " << uri.password << std::endl;
        os << "Host: " << uri.host << std::endl;
        os << "Port: " << uri.port << std::endl;
        os << "Path: " << uri.path << std::endl;
        os << "Query: " << uri.query << std::endl;
        return os;
    }
    
    URI(const std::string& uri) {
        enum State { PROT, USER, PASS, HOST, PORT, PATH, QUERY } state = PROT;
        std::vector<char> expected;
        bool has_username = false;
            
        for (int i = 0; i < uri.size(); i++) {
            char ch = uri[i];
            if (! expected.empty()) {
                if (ch == expected.back()) {
                    expected.pop_back();
                }
                else {
                    throw std::runtime_error(std::string("slowbook: bad URI string: ") + uri);
                }
            }
            else if (state == PROT) {
                if (ch == ':') {
                    expected.push_back('/');
                    expected.push_back('/');
                    state = USER;
                }
                else {
                    protocol += ch;
                }
            }
            else if (state == USER) {
                if (ch == ':') {
                    state = PASS;
                }
                else if (ch == '@') {
                    has_username = true;
                    state = HOST;
                }
                else if (ch == '/') {
                    state = PATH;
                }
                else if (ch == '?') {
                    state = QUERY;
                }
                else {
                    username += ch;
                }
            }
            else if (state == PASS) {
                if (ch == '@') {
                    has_username = true;
                    state = HOST;
                }
                else if (ch == '/') {
                    state = PATH;
                }
                else if (ch == '?') {
                    state = QUERY;
                }
                else {
                    password += ch;
                }
            }
            else if (state == HOST) {
                if (ch == ':') {
                    state = PORT;
                }
                else if (ch == '/') {
                    state = PATH;
                }
                else if (ch == '?') {
                    state = QUERY;
                }
                else {
                    host += ch;
                }
            }
            else if (state == PORT) {
                if (ch == '/') {
                    state = PATH;
                }
                else if (ch == '?') {
                    state = QUERY;
                }
                else {
                    port += ch;
                }
            }
            else if (state == PATH) {
                if (ch == '?') {
                    state = QUERY;
                }
                else {
                    path += ch;
                }
            }
            else {
                query += ch;
            }
        }

        if (! has_username) {
            host = username;
            port = password;
            username.clear();
            password.clear();
        }
    }
};

    
}
