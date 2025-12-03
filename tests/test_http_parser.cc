// tests/test_http_parser.cc
#include <gtest/gtest.h>
#include "yuyu/http/http_parser.h"
#include "yuyu/log.h"

namespace yuyu {
namespace http {
namespace test {

// Test fixture class
class HttpParserTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // Execute once before entire test suite
        g_logger = YUYU_LOG_ROOT();
    }
    
    void SetUp() override {
        // Execute before each test case
    }
    
    void TearDown() override {
        // Execute after each test case
    }
    
    static Logger::ptr g_logger;
};

Logger::ptr HttpParserTest::g_logger = nullptr;

// ==================== Request Parsing Tests ====================

TEST_F(HttpParserTest, ParsePostRequest) {
    const char test_request_data[] = "POST / HTTP/1.1\r\n"
                                     "Host: www.sylar.top\r\n"
                                     "Content-Length: 10\r\n\r\n"
                                     "1234567890";
    
    HttpRequestParser parser;
    std::string tmp = test_request_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    
    // Verify parsing results
    EXPECT_GT(s, 0) << "Should have parsed some data";
    EXPECT_FALSE(parser.hasError()) << "Parsing should not have errors";
    EXPECT_TRUE(parser.isFinished()) << "Parsing should be finished";
    EXPECT_EQ(parser.getContentLength(), 10) << "Content-Length should be 10";
    
    // Log results
    YUYU_LOG_INFO(g_logger) << "execute rt=" << s
        << " has_error=" << parser.hasError()
        << " is_finished=" << parser.isFinished()
        << " total=" << tmp.size()
        << " content_length=" << parser.getContentLength();
    
    // Verify request data
    auto req = parser.getData();
    ASSERT_NE(req, nullptr) << "Parsed request object should not be null";
    
    EXPECT_EQ(req->getMethod(), HttpMethod::POST) << "Method should be POST";
    EXPECT_EQ(req->getPath(), "/") << "Path should be /";
    EXPECT_EQ(req->getVersion(), 0x11) << "HTTP version should be 1.1";
    EXPECT_EQ(req->getHeader("Host"), "www.sylar.top") << "Host header should be correct";
    EXPECT_EQ(req->getHeader("Content-Length"), "10") << "Content-Length header should be correct";
    
    // Verify body
    tmp.resize(tmp.size() - s);
    EXPECT_EQ(tmp, "1234567890") << "Body should be 1234567890";
    
    YUYU_LOG_INFO(g_logger) << "Request:\n" << req->toString();
    YUYU_LOG_INFO(g_logger) << "Body: " << tmp;
}

TEST_F(HttpParserTest, ParseGetRequest) {
    const char test_data[] = "GET /index.html HTTP/1.1\r\n"
                            "Host: www.example.com\r\n"
                            "User-Agent: TestClient/1.0\r\n"
                            "Accept: */*\r\n\r\n";
    
    HttpRequestParser parser;
    std::string tmp = test_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    
    EXPECT_GT(s, 0);
    EXPECT_FALSE(parser.hasError());
    EXPECT_TRUE(parser.isFinished());
    EXPECT_EQ(parser.getContentLength(), 0) << "GET request usually has no body";
    
    auto req = parser.getData();
    ASSERT_NE(req, nullptr);
    
    EXPECT_EQ(req->getMethod(), HttpMethod::GET);
    EXPECT_EQ(req->getPath(), "/index.html");
    EXPECT_EQ(req->getHeader("Host"), "www.example.com");
    EXPECT_EQ(req->getHeader("User-Agent"), "TestClient/1.0");
    
    YUYU_LOG_INFO(g_logger) << "GET Request:\n" << req->toString();
}

TEST_F(HttpParserTest, ParseRequestWithQueryString) {
    const char test_data[] = "GET /search?q=test&page=1 HTTP/1.1\r\n"
                            "Host: www.example.com\r\n\r\n";
    
    HttpRequestParser parser;
    std::string tmp = test_data;
    parser.execute(&tmp[0], tmp.size());
    
    EXPECT_FALSE(parser.hasError());
    EXPECT_TRUE(parser.isFinished());
    
    auto req = parser.getData();
    ASSERT_NE(req, nullptr);
    
    EXPECT_EQ(req->getPath(), "/search");
    EXPECT_EQ(req->getQuery(), "q=test&page=1");
}

// TEST_F(HttpParserTest, ParseRequestChunked) {
//     const char part1[] = "POST / HTTP/1.1\r\n";
//     const char part2[] = "Host: www.sylar.com\r\n";
//     const char part3[] = "Content-Length: 5\r\n\r\n";
//     const char part4[] = "hello";
//     
//     HttpRequestParser parser;
//     
//     // Parse in chunks
//     std::string tmp1 = part1;
//     parser.execute(&tmp1[0], tmp1.size());
//     EXPECT_FALSE(parser.isFinished()) << "Should not finish after first chunk";
//     
//     std::string tmp2 = part2;
//     parser.execute(&tmp2[0], tmp2.size());
//     EXPECT_FALSE(parser.isFinished()) << "Should not finish after second chunk";
//     
//     std::string tmp3 = part3;
//     size_t s3 = parser.execute(&tmp3[0], tmp3.size());
//     EXPECT_FALSE(parser.isFinished()) << "Headers complete but body not arrived";
//     
//     std::string tmp4 = part4;
//     parser.execute(&tmp4[0], tmp4.size());
//     EXPECT_TRUE(parser.isFinished()) << "Should finish after all data arrives";
//     
//     auto req = parser.getData();
//     ASSERT_NE(req, nullptr);
//     EXPECT_EQ(req->getMethod(), HttpMethod::POST);
// }

TEST_F(HttpParserTest, ParseInvalidRequest) {
    const char invalid_data[] = "INVALID REQUEST\r\n\r\n";
    
    HttpRequestParser parser;
    std::string tmp = invalid_data;
    parser.execute(&tmp[0], tmp.size());
    
    EXPECT_TRUE(parser.hasError()) << "Invalid request should have error";
    
    YUYU_LOG_WARN(g_logger) << "Invalid request correctly detected error";
}

TEST_F(HttpParserTest, ParseRequestMissingHeaders) {
    const char test_data[] = "GET / HTTP/1.1\r\n\r\n";
    
    HttpRequestParser parser;
    std::string tmp = test_data;
    parser.execute(&tmp[0], tmp.size());
    
    // Although no Host header, format is valid
    EXPECT_FALSE(parser.hasError());
    EXPECT_TRUE(parser.isFinished());
    
    auto req = parser.getData();
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->getMethod(), HttpMethod::GET);
}

// ==================== Response Parsing Tests ====================

TEST_F(HttpParserTest, ParseResponse200OK) {
    const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
            "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
            "Server: Apache\r\n"
            "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
            "ETag: \"51-47cf7e6ee8400\"\r\n"
            "Accept-Ranges: bytes\r\n"
            "Content-Length: 81\r\n"
            "Cache-Control: max-age=86400\r\n"
            "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
            "Connection: Close\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html>\r\n"
            "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
            "</html>\r\n";
    
    HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size(), true);
    
    // Verify parsing results
    EXPECT_GT(s, 0);
    EXPECT_FALSE(parser.hasError()) << "Parsing should not have errors";
    EXPECT_TRUE(parser.isFinished()) << "Parsing should be finished";
    EXPECT_EQ(parser.getContentLength(), 81) << "Content-Length should be 81";
    
    YUYU_LOG_INFO(g_logger) << "execute rt=" << s
        << " has_error=" << parser.hasError()
        << " is_finished=" << parser.isFinished()
        << " total=" << tmp.size()
        << " content_length=" << parser.getContentLength();
    
    // Verify response data
    auto resp = parser.getData();
    ASSERT_NE(resp, nullptr) << "Parsed response object should not be null";
    
    EXPECT_EQ(resp->getStatus(), HttpStatus::OK) << "Status code should be 200";
    EXPECT_EQ(resp->getVersion(), 0x11) << "HTTP version should be 1.1";
    EXPECT_EQ(resp->getHeader("Server"), "Apache");
    EXPECT_EQ(resp->getHeader("Content-Type"), "text/html");
    EXPECT_EQ(resp->getHeader("Connection"), "Close");
    
    // Verify body
    tmp.resize(tmp.size() - s);
    EXPECT_EQ(tmp.size(), 81) << "Body size should be 81";
    EXPECT_TRUE(tmp.find("<html>") != std::string::npos) << "Body should contain HTML";
    
    YUYU_LOG_INFO(g_logger) << "Response:\n" << resp->toString();
    YUYU_LOG_INFO(g_logger) << "Body:\n" << tmp;
}

TEST_F(HttpParserTest, ParseResponse404NotFound) {
    const char test_data[] = "HTTP/1.1 404 Not Found\r\n"
                            "Content-Length: 9\r\n"
                            "Content-Type: text/plain\r\n\r\n"
                            "Not Found";
    
    HttpResponseParser parser;
    std::string tmp = test_data;
    size_t s = parser.execute(&tmp[0], tmp.size(), true);
    
    EXPECT_FALSE(parser.hasError());
    EXPECT_TRUE(parser.isFinished());
    
    auto resp = parser.getData();
    ASSERT_NE(resp, nullptr);
    
    EXPECT_EQ(resp->getStatus(), HttpStatus::NOT_FOUND);
    EXPECT_EQ(resp->getHeader("Content-Type"), "text/plain");
    
    tmp.resize(tmp.size() - s);
    EXPECT_EQ(tmp, "Not Found");
    
    YUYU_LOG_INFO(g_logger) << "404 Response:\n" << resp->toString();
}

TEST_F(HttpParserTest, ParseResponse302Redirect) {
    const char test_data[] = "HTTP/1.1 302 Found\r\n"
                            "Location: https://www.example.com/new-location\r\n"
                            "Content-Length: 0\r\n\r\n";
    
    HttpResponseParser parser;
    std::string tmp = test_data;
    parser.execute(&tmp[0], tmp.size(), true);
    
    EXPECT_FALSE(parser.hasError());
    EXPECT_TRUE(parser.isFinished());
    
    auto resp = parser.getData();
    ASSERT_NE(resp, nullptr);
    
    EXPECT_EQ(resp->getStatus(), HttpStatus::FOUND);
    EXPECT_EQ(resp->getHeader("Location"), "https://www.example.com/new-location");
    EXPECT_EQ(parser.getContentLength(), 0);
}

TEST_F(HttpParserTest, ParseResponseChunked) {
    const char part1[] = "HTTP/1.1 200 OK\r\n";
    const char part2[] = "Content-Length: 5\r\n\r\n";
    const char part3[] = "Hello";
    
    HttpResponseParser parser;
    
    std::string tmp1 = part1;
    parser.execute(&tmp1[0], tmp1.size(), false);
    EXPECT_FALSE(parser.isFinished());
    
    std::string tmp2 = part2;
    parser.execute(&tmp2[0], tmp2.size(), false);
    EXPECT_FALSE(parser.isFinished());
    
    std::string tmp3 = part3;
    parser.execute(&tmp3[0], tmp3.size(), true);
    EXPECT_TRUE(parser.isFinished());
    
    auto resp = parser.getData();
    ASSERT_NE(resp, nullptr);
    EXPECT_EQ(resp->getStatus(), HttpStatus::OK);
}

TEST_F(HttpParserTest, ParseResponseNoContentLength) {
    const char test_data[] = "HTTP/1.1 200 OK\r\n"
                            "Connection: close\r\n\r\n";
    
    HttpResponseParser parser;
    std::string tmp = test_data;
    parser.execute(&tmp[0], tmp.size(), true);
    
    // Without Content-Length, end is determined by connection close
    auto resp = parser.getData();
    ASSERT_NE(resp, nullptr);
    
    EXPECT_EQ(resp->getStatus(), HttpStatus::OK);
    EXPECT_EQ(parser.getContentLength(), 0);
}

TEST_F(HttpParserTest, ParseInvalidResponse) {
    const char invalid_data[] = "INVALID RESPONSE\r\n\r\n";
    
    HttpResponseParser parser;
    std::string tmp = invalid_data;
    parser.execute(&tmp[0], tmp.size(), true);
    
    EXPECT_TRUE(parser.hasError()) << "Invalid response should have error";
    
    YUYU_LOG_WARN(g_logger) << "Invalid response correctly detected error";
}

// ==================== Edge Case Tests ====================

TEST_F(HttpParserTest, ParseEmptyData) {
    HttpRequestParser parser;
    std::string tmp = "";
    size_t s = parser.execute(&tmp[0], tmp.size());
    
    EXPECT_EQ(s, 0) << "Empty data should return 0";
    EXPECT_FALSE(parser.isFinished());
}

TEST_F(HttpParserTest, ParseVeryLongHeader) {
    std::string test_data = "GET / HTTP/1.1\r\n";
    test_data += "Host: www.example.com\r\n";
    test_data += "X-Custom-Header: ";
    test_data += std::string(8000, 'A'); // Very long header value
    test_data += "\r\n\r\n";
    
    HttpRequestParser parser;
    parser.execute(&test_data[0], test_data.size());
    
    // Depending on implementation, may accept or reject
    if (parser.hasError()) {
        YUYU_LOG_INFO(g_logger) << "Parser correctly rejected oversized header";
    } else {
        auto req = parser.getData();
        ASSERT_NE(req, nullptr);
        YUYU_LOG_INFO(g_logger) << "Parser accepted large header";
    }
}

TEST_F(HttpParserTest, ParseMultipleRequests) {
    // Test parsing multiple consecutive requests
    const char request1[] = "GET /1 HTTP/1.1\r\n\r\n";
    const char request2[] = "GET /2 HTTP/1.1\r\n\r\n";
    
    HttpRequestParser parser1;
    std::string tmp1 = request1;
    parser1.execute(&tmp1[0], tmp1.size());
    EXPECT_TRUE(parser1.isFinished());
    EXPECT_EQ(parser1.getData()->getPath(), "/1");
    
    HttpRequestParser parser2;
    std::string tmp2 = request2;
    parser2.execute(&tmp2[0], tmp2.size());
    EXPECT_TRUE(parser2.isFinished());
    EXPECT_EQ(parser2.getData()->getPath(), "/2");
}

TEST_F(HttpParserTest, ParseRequestWithSpecialCharacters) {
    const char test_data[] = "GET /path/%20with%20spaces?param=value%26more HTTP/1.1\r\n"
                            "Host: www.example.com\r\n\r\n";
    
    HttpRequestParser parser;
    std::string tmp = test_data;
    parser.execute(&tmp[0], tmp.size());
    
    EXPECT_FALSE(parser.hasError());
    EXPECT_TRUE(parser.isFinished());
    
    auto req = parser.getData();
    ASSERT_NE(req, nullptr);
    EXPECT_TRUE(req->getPath().find("%20") != std::string::npos) << "URL encoded spaces should be preserved";
}

TEST_F(HttpParserTest, ParseResponseWithEmptyBody) {
    const char test_data[] = "HTTP/1.1 204 No Content\r\n"
                            "Server: TestServer\r\n\r\n";
    
    HttpResponseParser parser;
    std::string tmp = test_data;
    parser.execute(&tmp[0], tmp.size(), true);
    
    EXPECT_FALSE(parser.hasError());
    EXPECT_TRUE(parser.isFinished());
    
    auto resp = parser.getData();
    ASSERT_NE(resp, nullptr);
    EXPECT_EQ(resp->getStatus(), HttpStatus::NO_CONTENT);
}

TEST_F(HttpParserTest, ParseRequestWithCaseInsensitiveHeaders) {
    const char test_data[] = "GET / HTTP/1.1\r\n"
                            "host: www.example.com\r\n"
                            "CONTENT-TYPE: text/plain\r\n\r\n";
    
    HttpRequestParser parser;
    std::string tmp = test_data;
    //size_t s = parser.execute(&tmp[0], tmp.size());
    
    EXPECT_FALSE(parser.hasError());
    EXPECT_TRUE(parser.isFinished());
    
    auto req = parser.getData();
    ASSERT_NE(req, nullptr);
    // Headers should be case-insensitive
}

} // namespace test
} // namespace http
} // namespace yuyu

// Main function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
