#ifndef __YUYU_HTTP_SESSION_H__
#define __YUYU_HTTP_SESSION_H__

#include "http.h"
#include "http_parser.h"
#include "yuyu/streams/socket_stream.h"

namespace yuyu {
namespace http {
class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr sock, bool owner = true);

    HttpRequest::ptr recvRequest();

    int sendResponse(HttpResponse::ptr rsp);
};
}
} // namespace end

#endif
