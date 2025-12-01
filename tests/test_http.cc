#include "yuyu/http/http.h"
#include "yuyu/log.h"

void test_request() {
    yuyu::http::HttpRequest::ptr req(new yuyu::http::HttpRequest);
    req->setHeader("host" , "www.sylar.top");
    req->setBody("hello yuyu");
    req->dump(std::cout) << std::endl;
}

void test_response() {
    yuyu::http::HttpResponse::ptr rsp(new yuyu::http::HttpResponse);
    rsp->setHeader("X-X", "sylar");
    rsp->setBody("hello yuyu");
    rsp->setStatus((yuyu::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
    test_request();
    test_response();
    return 0;
}
