//
// Created by rui836662398 on 2021/2/28.
//
#include "Net/buffer.h"

#include <gtest/gtest.h>

using namespace std;
TEST(BufferTest, AppendTakeOut) {
    Buffer buf;
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize);
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize);

    const string str(200, 'x');
    buf.Append(str);
    EXPECT_EQ(buf.ReadableBytes(), str.size());
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - str.size());
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize);

    const string str2 = buf.TakeOut(50);
    EXPECT_EQ(str2.size(), 50);
    EXPECT_EQ(buf.ReadableBytes(), str.size() - str2.size());
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - str.size());
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize + str2.size());
    EXPECT_EQ(str2, string(50, 'x'));

    buf.Append(str);
    EXPECT_EQ(buf.ReadableBytes(), 2 * str.size() - str2.size());
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - 2 * str.size());
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize + str2.size());

    const string str3 = buf.TakeOutAll();
    EXPECT_EQ(str3.size(), 350);
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize);
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize);
    EXPECT_EQ(str3, string(350, 'x'));
}

TEST(BufferTest, BufferGrow) {
    Buffer buf;
    buf.Append(string(400, 'y'));
    EXPECT_EQ(buf.ReadableBytes(), 400);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - 400);

    buf.TakeOut(50);
    EXPECT_EQ(buf.ReadableBytes(), 350);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - 400);
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize + 50);

    buf.Append(string(1000, 'z'));
    EXPECT_EQ(buf.ReadableBytes(), 1350);
    EXPECT_EQ(buf.WritableBytes(), 50);
    EXPECT_EQ(buf.PrePendedBytes(),
                      Buffer::kPrependSize);

    buf.TakeOutAll();
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WritableBytes(), 1400);
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize);
}

TEST(BufferTest, BufferInsideGrow) {
    Buffer buf;
    buf.Append(string(800, 'y'));
    EXPECT_EQ(buf.ReadableBytes(), 800);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - 800);

    buf.TakeOut(500);
    EXPECT_EQ(buf.ReadableBytes(), 300);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - 800);
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize + 500);

    buf.Append(string(300, 'z'));
    EXPECT_EQ(buf.ReadableBytes(), 600);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - 600);
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize);
}

TEST(BufferTest, BufferPrepend) {
    Buffer buf;
    buf.Append(string(200, 'y'));
    EXPECT_EQ(buf.ReadableBytes(), 200);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - 200);
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize);

    int x = 0;
    buf.Prepend(&x, sizeof x);
    EXPECT_EQ(buf.ReadableBytes(), 204);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kDefaultSize - 200);
    EXPECT_EQ(buf.PrePendedBytes(), Buffer::kPrependSize - 4);
}

TEST(BufferTest, FindCRLF) {
    Buffer buf;
    buf.Append("aaa");
    const char* res = buf.FindCRLF();
    EXPECT_EQ(res, nullptr);
    buf.Append("12345\r\n54321");
    res = buf.FindCRLF();
    EXPECT_EQ(strcmp(res, "\r\n54321"), 0);
    res = buf.FindCRLF(buf.ReadData() + 4);
    EXPECT_EQ(strcmp(res, "\r\n54321"), 0);
    res = buf.FindCRLF(buf.ReadData() + 9);
    EXPECT_EQ(res, nullptr);
}

TEST(BufferTest, FindEOL) {
    Buffer buf;
    buf.Append(string(100, '1'));
    const char* res = buf.FindEOL();
    EXPECT_EQ(res, nullptr);
    res = buf.FindEOL(buf.ReadData() + 10);
    EXPECT_EQ(res, nullptr);
    buf.Append("\n");
    buf.Append("22");
    res = buf.FindEOL();
    EXPECT_EQ(strcmp(res, "\n22"), 0);
    res = buf.FindEOL(buf.ReadData() + 102);
    EXPECT_EQ(res, nullptr);
}

void fun(Buffer&& buf, const void* data){
    Buffer new_buf(std::move(buf));
    EXPECT_EQ(data, new_buf.ReadData());
}

TEST(BufferTest, Move){
    Buffer buf;
    buf.Append("Ray", 3);
    auto data = buf.ReadData();
    fun(std::move(buf), data);
}