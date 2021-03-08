//
// Created by ray on 2021/3/7.
//

#ifndef MYWEBSERVER_HTTP_RESPONSE_H
#define MYWEBSERVER_HTTP_RESPONSE_H

#include <unordered_map>

class Buffer;

class HttpResponse {
   public:
    enum HttpStatusCode {
        kUnknown,
        k200Ok = 200,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

    explicit HttpResponse(bool is_short_conn)
        : status_code_(kUnknown), is_short_conn_(is_short_conn) {}

    void set_status_code(HttpStatusCode code) { status_code_ = code; }

    void set_status_message(const std::string& message) {
        status_message_ = message;
    }

    void set_short_connection() { is_short_conn_ = true; }
    void set_long_connection() { is_short_conn_ = false; }
    bool IsShortConnection() { return is_short_conn_; }

    void setContentType(const std::string& contentType) {
        AddHeader("Content-Type", contentType);
    }
    void AddHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }
    void set_body(const std::string& body) { body_ = body; }

    void AppendToBuffer(Buffer* output) const;

   private:
    std::unordered_map<std::string, std::string> headers_;
    HttpStatusCode status_code_;
    std::string status_message_;
    bool is_short_conn_;
    std::string body_;
};

#endif  // MYWEBSERVER_HTTP_RESPONSE_H
