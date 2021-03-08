//
// Created by ray on 2021/3/7.
//

#ifndef MYWEBSERVER_HTTP_PARSER_H
#define MYWEBSERVER_HTTP_PARSER_H

#include "Net/Http/http_request.h"

class Buffer;

class HttpParser {
   public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

    HttpParser() : state_(kExpectRequestLine) {}

    bool ParseRequest(Buffer* buf);

    bool ParseComplete() { return state_ == kGotAll; }

    void Reset() {
        state_ = kExpectRequestLine;
        request_ = HttpRequest();
    }
    const HttpRequest& request() const { return request_; }

    HttpRequest& request() { return request_; }

   private:
    // return true if succeed
    bool ParseRequestLine(const char* begin, const char* end);

    HttpRequestParseState state_;
    HttpRequest request_;
};

#endif  // MYWEBSERVER_HTTP_PARSER_H
