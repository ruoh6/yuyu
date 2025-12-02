#include "yuyu/http/http_server.h"
#include "yuyu/log.h"

static yuyu::Logger::ptr g_logger = YUYU_LOG_ROOT();

#define XX(...) #__VA_ARGS__


yuyu::IOManager::ptr worker;
void run() {
    g_logger->setLevel(yuyu::LogLevel::INFO);
    //yuyu::http::HttpServer::ptr server(new yuyu::http::HttpServer(true, worker.get(), yuyu::IOManager::GetThis()));
    yuyu::http::HttpServer::ptr server(new yuyu::http::HttpServer(true));
    yuyu::Address::ptr addr = yuyu::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/yuyu/xx", [](yuyu::http::HttpRequest::ptr req
                ,yuyu::http::HttpResponse::ptr rsp
                ,yuyu::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/yuyu/*", [](yuyu::http::HttpRequest::ptr req
                ,yuyu::http::HttpResponse::ptr rsp
                ,yuyu::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });

    sd->addGlobServlet("/yuyux/*", [](yuyu::http::HttpRequest::ptr req
                ,yuyu::http::HttpResponse::ptr rsp
                ,yuyu::http::HttpSession::ptr session) {
            rsp->setBody(XX(<html>
<head><title>404 Not Found</title></head>
<body>
<center><h1>404 Not Found</h1></center>
<hr><center>nginx/1.16.0</center>
</body>
</html>
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
));
            return 0;
    });

    server->start();
}

int main(int argc, char** argv) {
    yuyu::IOManager iom(1, true, "main");
    worker.reset(new yuyu::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}
