// WebSocketWS.cpp
//

#include "WebSocketWS.h"
#define LZZ_INLINE inline
namespace MM
{
  namespace WS
  {
    WebSocketWS::WebSocketWS (string addr, string getparam, u16 port)
      : WebSocket (addr, getparam, port)
            {
                userdata = 0;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWS::Connect (int verify)
            {
                int sock = opensock();
                if(sock <= 0) return -1;
                
                userdata = sock;
                
                int ret = HTTP_Auth(verify);
                if(ret)
                {
                    Disconnect();
                    return -1;
                }
                
                WebSocket::sock_mkasync(sock);
                
                if(wslctx) { wslay_event_context_free(wslctx); wslctx = nullptr; }
                wslay_event_context_client_init(&wslctx, &wslcb, this);
                
                return 0;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWS::Disconnect ()
            {
                if(userdata)
                {
                    close(userdata);
                    userdata = 0;
                }
                else return -1;
                
                if(wslctx) { wslay_event_context_free(wslctx); wslctx = nullptr; }
                
                return 0;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWS::CheckConnect ()
            {
                return userdata ? 1 : 0;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWS::polldata (int wat, int timeo)
            {
                return WebSocket::pollsock(userdata, wat, timeo);
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWS::recv_raw (void * buf, size_t len, int flags)
            {
                return recv(userdata, buf, len, flags);
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWS::send_raw (void const * buf, size_t len, int flags)
            {
                return send(userdata, buf, len, flags);
            }
  }
}
#undef LZZ_INLINE
