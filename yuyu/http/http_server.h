#ifndef __YUYU_HTTP_SERVER_H__
#define __YUYU_HTTP_SERVER_H__

#include "yuyu/tcp_server.h"
#include "http_session.h"
#include "servlet.h"

namespace yuyu {
namespace http {
class HttpServer : public TcpServer {
public:
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keepalive = false
               ,yuyu::IOManager* worker = yuyu::IOManager::GetThis()
               ,yuyu::IOManager* io_worker = yuyu::IOManager::GetThis()
               ,yuyu::IOManager* accept_worker = yuyu::IOManager::GetThis());

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}

    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}

    virtual void setName(const std::string& v) override;
protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;
};

} // namespace http end
} // namespace end
#endif
