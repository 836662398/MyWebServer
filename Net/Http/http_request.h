//
// Created by ray on 2021/3/7.
//

#ifndef MYWEBSERVER_HTTP_REQUEST_H
#define MYWEBSERVER_HTTP_REQUEST_H

#include <cassert>
#include <unordered_map>

class HttpRequest {
   public:
    enum Method { kGet, kPost, kHead, kPut, kDelete, kInvalid };

    HttpRequest() : method_(kInvalid) {}

    bool set_method(const char* start, const char* end) {
        assert(method_ == kInvalid);
        std::string m(start, end);
        if (m == "GET") {
            method_ = kGet;
        } else if (m == "POST") {
            method_ = kPost;
        } else if (m == "HEAD") {
            method_ = kHead;
        } else if (m == "PUT") {
            method_ = kPut;
        } else if (m == "DELETE") {
            method_ = kDelete;
        } else {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }

    Method method() const { return method_; }
    const char* PrintMethod() const {
        const char* result = "UNKNOWN";
        switch (method_) {
            case kGet:
                result = "GET";
                break;
            case kPost:
                result = "POST";
                break;
            case kHead:
                result = "HEAD";
                break;
            case kPut:
                result = "PUT";
                break;
            case kDelete:
                result = "DELETE";
                break;
            default:
                break;
        }
        return result;
    }

    void set_path(const char* start, const char* end) {
        path_.assign(start, end);
    }
    void set_query(const char* start, const char* end) {
        query_.assign(start, end);
    }
    const std::string& path() const { return path_; }
    const std::string& query() const { return query_; }

    void AddHeader(const char* start, const char* colon, const char* end) {
        std::string key(start, colon);
        ++colon;
        while (colon < end && isspace(*colon)) {
            ++colon;
        }
        std::string value(colon, end);
        int val_len = value.size();
        while (!value.empty() && isspace(value[val_len - 1])) {
            val_len--;
        }
        value.resize(val_len);
        headers_[key] = value;
    }

    std::string GetHeader(const std::string& key) const {
        std::string result;
        auto it = headers_.find(key);
        if (it != headers_.end()) {
            result = it->second;
        }
        return result;
    }

    const std::unordered_map<std::string, std::string>& headers() const {
        return headers_;
    }

   private:
    Method method_;
    std::string path_;
    std::string query_;
    std::unordered_map<std::string, std::string> headers_;
};

#endif  // MYWEBSERVER_HTTP_REQUEST_H
