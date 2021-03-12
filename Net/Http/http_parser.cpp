//
// Created by ray on 2021/3/7.
//

#include "http_parser.h"

#include "Net/buffer.h"

bool HttpParser::ParseRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.set_method(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char* question = std::find(start, space, '?');
            if (question != space) {
                request_.set_path(start, question);
                request_.set_query(question, space);
            } else {
                request_.set_path(start, space);
            }
            start = space + 1;

            succeed =
                (end - start == 8 && std::equal(start, end - 1, "HTTP/1."));
            if (succeed) {
                if (*(end - 1) == '1') {
                    request_.set_version(HttpRequest::kHttp11);
                } else if (*(end - 1) == '0') {
                    request_.set_version(HttpRequest::kHttp10);
                } else {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

bool HttpParser::ParseRequest(Buffer* buf) {
    bool succeed = true;
    bool has_more = true;
    while (has_more) {
        if (state_ == kExpectRequestLine) {
            const char* crlf = buf->FindCRLF();
            if (crlf) {
                succeed = ParseRequestLine(buf->ReadData(), crlf);
                if (succeed) {
                    buf->SkipUtil(crlf + 2);
                    state_ = kExpectHeaders;
                } else {
                    has_more = false;
                }
            } else {
                // wait for crlf in next ParseRequest()
                has_more = false;
            }
        } else if (state_ == kExpectHeaders) {
            const char* crlf = buf->FindCRLF();
            if (crlf) {
                const char* colon = std::find(buf->ReadData(), crlf, ':');
                if (colon != crlf) {
                    request_.AddHeader(buf->ReadData(), colon, crlf);
                } else {
                    // empty line, end of header
                    // GET doesn't have body
                    state_ = kGotAll;
                    has_more = false;
                }
                buf->SkipUtil(crlf + 2);
            } else {
                has_more = false;
            }
        }
    }
    return succeed;
}
