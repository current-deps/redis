// Simple blocking redis client for all types of commands with KeepAlive and TCP timeout and auto reconnect support

#pragma once
extern "C" {
#include <hiredis.h>
}
#include <string>
#include <iostream>
#include "current.h"

CURRENT_STRUCT(RedisResponse) {
  // Struct for RESP2 protocol

  CURRENT_FIELD(status, uint16_t);
  CURRENT_FIELD(string, std::string);
  CURRENT_FIELD(integer, uint64_t);
  CURRENT_FIELD(elements, std::vector<RedisResponse>);
  CURRENT_FIELD(length, uint64_t);

  void Print() {
    std::cout << "Response {" << status << ", " << string << ", " << integer << ", " << length << " }" << std::endl;
    if (!elements.size()) return;
    std::cout << "Elements: " << std::endl;
    for (auto& element : elements) {
      element.Print();
    }
    std::cout << std::endl;
  }

  bool IsOK() { return status != REDIS_REPLY_ERROR; }
  bool IsNIL() { return status == REDIS_REPLY_NIL; }
  bool IsString() { return status == REDIS_REPLY_STRING; }
  bool IsNumber() { return status == REDIS_REPLY_INTEGER; }
  bool IsArray() { return status == REDIS_REPLY_ARRAY; }

  static RedisResponse CreateConnError() { return RedisResponse::Create(REDIS_REPLY_ERROR, "Connection error", 0, 16); }

  static RedisResponse Create(uint16_t status, std::string str, long long integer, uint64_t length) {
    RedisResponse response;
    response.status = status;
    response.string = str;
    response.integer = integer;
    response.length = length;
    return response;
  }
  static RedisResponse CreateWithElements(
      uint16_t status, std::string str, uint64_t integer, uint64_t length, std::vector<RedisResponse> elements) {
    auto response = RedisResponse::Create(status, str, integer, length);
    response.elements = elements;
    return response;
  }
};

class RedisClient final {
  // Blocking client for redis with auto reconnect and resp 2 -> Current struct parser
 protected:
  redisContext* context_;
  std::string host_;
  int port_;
  std::string user_;
  std::string password_;

 public:
  explicit RedisClient(std::string host, int port)
      : host_(host), port_(port), context_(redisConnect(host.c_str(), port)) {
    IsConnected(true);
  }
  explicit RedisClient(std::string host, int port, std::string user, std::string password)
      : host_(host), port_(port), user_(user), password_(password), context_(redisConnect(host.c_str(), port)) {
    // Constructor with user/password authorization
    if (IsConnected(true)) {
      auto resp = SendCommand("AUTH " + user + " " + password);
      if (resp.status != REDIS_REPLY_STATUS && resp.string != "OK") {
        std::cout << "Authorization failed" << std::endl;
      }
    }
  };

  void EnableKeepAlive(int interval = 0) {
    // Enables keep-alive with optional timeout
    if (interval > 0) {
      redisEnableKeepAliveWithInterval(context_, interval);
    } else {
      redisEnableKeepAlive(context_);
    }
  }

  void SetTCPTimeout(unsigned int timeout) {
    // Sets TCP timeout
    redisSetTcpUserTimeout(context_, timeout);
  }

  bool IsConnected(bool verbose = false) {
    // Returns true if client is connected
    auto res = context_ == nullptr || context_->err;
    if (res && verbose) {
      std::cout << "Failed to connect to redis server" << std::endl;
    }
    return !res;
  }

  RedisResponse SendCommand(std::string command) {
    // Sends command to redis and returns RESP2 proto output as Current struct
    if (!IsConnected(true)) {
      return RedisResponse::CreateConnError();
    }

    redisReply* reply = (redisReply*)redisCommand(context_, command.c_str());
    std::string data = "";
    if (reply->str != nullptr) data = std::string(reply->str);
    auto resp = RedisResponse::Create(reply->type, data, reply->integer, reply->len);
    if (resp.IsArray()) {
      auto elems = std::vector<RedisResponse>();

      // reply->elements is not std container
      for (int i = 0; i < reply->elements; i++) {
        // FIXME, ?check if it supports nested arrays - need to implement it
        auto elem = reply->element[i];
        elems.push_back(RedisResponse::Create(elem->type, elem->str, elem->integer, elem->len));
      }
      resp.elements = elems;
    }
    freeReplyObject(reply);
    return resp;
  }
};