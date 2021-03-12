//
// Created by ray on 2021/3/7.
//

#include "Net/Http/http_parser.h"

#include <gtest/gtest.h>

#include "Net/buffer.h"

using namespace std;

TEST(HttpParser, test) {
    HttpParser parser;
    Buffer input;
    input.Append(
        "GET HTTP/1.1\r\n"
        "Host: www.baidu.com\r\n"
        "\r\n");

    EXPECT_FALSE(parser.ParseRequest(&input));
    EXPECT_FALSE(parser.ParseComplete());
}

TEST(HttpParser, test1) {
    HttpParser parser;
    Buffer input;
    input.Append(
        "GET / HTTP/1.1\r\n"
        "Host: www.baidu.com\r\n"
        "User-Agent:\r\n"
        "\r\n");

    EXPECT_TRUE(parser.ParseRequest(&input));
    EXPECT_TRUE(parser.ParseComplete());
    auto request = parser.request();
    EXPECT_EQ(request.method(), HttpRequest::kGet);
    EXPECT_EQ(request.path(), string("/"));
    EXPECT_EQ(request.version(), HttpRequest::kHttp11);
    EXPECT_EQ(request.GetHeader("Host"), string("www.baidu.com"));
    EXPECT_EQ(request.GetHeader("User-Agent"), string(""));
    EXPECT_EQ(request.GetHeader("Accept-Encoding"), string(""));
}

TEST(HttpParser, test2) {
    string all(
        "GET / HTTP/1.0\r\n"
        "Host: www.baidu.com\r\n"
        "\r\n");
    int x = all.size();
    for (size_t i = 0; i < all.size(); ++i) {
        HttpParser parser;
        Buffer input;
        input.Append(all.c_str(), i);
        EXPECT_TRUE(parser.ParseRequest(&input));
        EXPECT_FALSE(parser.ParseComplete());

        size_t len2 = all.size() - i;
        input.Append(all.c_str() + i, len2);
        EXPECT_TRUE(parser.ParseRequest(&input));
        EXPECT_TRUE(parser.ParseComplete());
        const HttpRequest& request = parser.request();
        EXPECT_EQ(request.method(), HttpRequest::kGet);
        EXPECT_EQ(request.path(), string("/"));
        EXPECT_EQ(request.version(), HttpRequest::kHttp10);
        EXPECT_EQ(request.GetHeader("Host"), string("www.baidu.com"));
        EXPECT_EQ(request.GetHeader("User-Agent"), string(""));
    }
}