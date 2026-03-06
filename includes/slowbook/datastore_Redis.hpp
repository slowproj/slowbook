// slowbook/datastore_Redis.hpp //
// Created by Sanshiro Enomoto on 3 March 2026

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

#include <hiredis.h>

#include "datastore.hpp"
#include "uriparse.hpp"


namespace slowbook {


class Redis {
  public:
    struct ContextDeleter {
        void operator()(redisContext* context) const { if (context) redisFree(context); }
    };
    struct ReplyDeleter {
        void operator()(redisReply* reply) const { if (reply) freeReplyObject(reply); }
    };
    using ContextPtr = std::unique_ptr<redisContext, ContextDeleter>;
    using ReplyPtr = std::unique_ptr<redisReply, ReplyDeleter>;

    class Reply {
      public:
        Reply(ReplyPtr a_ptr): ptr(std::move(a_ptr)) {}
        int type() const { return ptr->type; }
        bool is_error() const { return ptr->type == REDIS_REPLY_ERROR; }
        std::string str() const {
            return std::string(ptr->str ? ptr->str : "");
        }
        long integer() const {
            if (ptr->type == REDIS_REPLY_INTEGER) {
                return ptr->integer;
            }
            throw std::runtime_error("slowbook::RedisReply::integer: not an integer");
        }
        operator std::string() const { return this->str(); }
        friend std::ostream& operator<<(std::ostream& os, const Reply& reply) { return os << reply.str(); }
      private:
        ReplyPtr ptr;
    };
    
  public:
    explicit Redis(const std::string& redis_uri) {
        URI uri(redis_uri);
        int port, db;
        try {
            port = uri.port.empty() ? 6379 : std::stoi(uri.port);
            db = uri.path.empty() ? -1 : std::stoi(uri.path);
        }
        catch (std::runtime_error &e) {
            throw std::runtime_error(std::string("bad Redis URI: ") + e.what());
        }
        this->initialize(uri.host, port, db);
    }
    explicit Redis(const std::string& host, int port, int db) {
        this->initialize(host, port, db);
    }

    Redis(const Redis&) = delete;
    Redis& operator=(const Redis&) = delete;
    
    Reply command(const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        redisReply* reply = (redisReply*) redisvCommand(this->context.get(), fmt, ap);
        va_end(ap);
        
        if (! reply) {
            throw std::runtime_error("slowbook::redisCommand failed (null reply)");
        }
        return Reply(ReplyPtr(reply));
    }

    Reply command(std::vector<std::string> args) {
        std::vector<const char*> argv;
        std::vector<size_t> argvlen;
        for (auto& arg: args) {
            argv.push_back(arg.c_str());
            argvlen.push_back(arg.size());
        }

        redisReply* reply = (redisReply*) redisCommandArgv(
            this->context.get(),
            args.size(), argv.data(), argvlen.data()
        );
        if (! reply) {
            throw std::runtime_error("slowbook::redisCommand failed (null reply)");
        }
        return Reply(ReplyPtr(reply));
    }
    
  private:
    void initialize(const std::string& host, int port, int db) {
        redisContext* raw = redisConnect(host.c_str(), port);
        if (! raw) {
            throw std::runtime_error("slowbook::redisConnect: unable to create a Redis context");
        }
        this->context.reset(raw);
        if (this->context->err) {
            throw std::runtime_error(std::string("slowbook::Redis error: ") + this->context->errstr);
        }
        if (db >= 0) {
            this->command("SELECT %d", db);
        }
    }
    ContextPtr context;
};


    
class DataStore_Redis: public DataStore {
  public:
    DataStore_Redis(const std::string& uri): redis_(uri) {}
    Redis::Reply command(std::vector<std::string> args) { return redis_.command(args); }
  protected:
    void write(const Record& record, bool is_to_update) override {
        if (
            (record.time_values.size() != 1) ||
            (record.tag_values.size() != 1) ||
            (record.field_values.size() != 1)
        ){
            throw std::runtime_error("slowbook::DataStore_Redis: bad time/tag/field length (must be all one)");
        }
        bool is_numeric = record.field_values[0].is_numeric();
        std::string time = std::to_string(long(record.time_values[0].as_real() * 1000));  // ms
        std::string key = record.tag_values[0].as_text();
        std::string value = record.field_values[0].as_text(/*precision*/10);

        if (is_to_update) {
            auto reply = redis_.command({"SET", key, value});
        }
        else if (is_numeric) {
            auto reply = redis_.command({"TS.ADD", key, time, value});
        }
        else {
            std::cerr << "NOT INSERTED" << std::endl;
        }
    }
  protected:
    Redis redis_;
};

    
}
