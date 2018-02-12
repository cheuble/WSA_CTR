// WebSocketWSS.cpp
//

#include "WebSocketWSS.h"
#define LZZ_INLINE inline
namespace MM
{
  namespace WS
  {
    WebSocketWSS::WebSocketWSS (string addr, string getparam, u16 port)
      : WebSocket (addr, getparam, port)
            {
                userdata.sock = 0;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWSS::Connect (int verify)
            {
                int sock = opensock();
                if(sock <= 0) return -1;
                
                userdata.sock = sock;
                int ret = sslcCreateContext(&userdata.sslc, sock, SSLCOPT_DisableVerify, addr.c_str());
                if(ret < 0)
                {
                    DEBUG("sslcCreateContext fail: %08X\n", ret);
                    close(sock);
                    userdata.sock = 0;
                    return -1;
                }
                int retval = -1;
                u32 something = 0;
                
                ret = sslcStartConnection(&userdata.sslc, &retval, &something);
                if(ret < 0)
                {
                    DEBUG("sslcStartConnection fail: %08X\n", ret);
                    Disconnect();
                    return -1;
                }
                
                DEBUG("StartConnection: ret=%i, val=%i (%08X)\n", retval, something, something);
                
                ret = HTTP_Auth(verify);
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
    int WebSocketWSS::Disconnect ()
            {
                if(userdata.sock)
                {
                    sslcDestroyContext(&userdata.sslc);
                    close(userdata.sock);
                    userdata.sock = 0;
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
    int WebSocketWSS::CheckConnect ()
            {
                return userdata.sock ? 1 : 0;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWSS::polldata (int wat, int timeo)
            {
                return WebSocket::pollsock(userdata.sock, wat, timeo);
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWSS::recv_raw (void * buf, size_t len, int flags)
            {
                int ret = sslcRead(&userdata.sslc, buf, len, (flags & MSG_PEEK) ? true : false);
                if(ret == 0xD840B802)
                {
                    errno = EWOULDBLOCK;
                    return -1;
                }
                
                if(ret < 0)
                {
                    DEBUG("sslcRead fail: %08X\n", ret);
                    return -1;
                }
                if(!ret)
                {
                    DEBUGS("sslcRead EOF");
                    return 0;
                }
                return ret;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocketWSS::send_raw (void const * buf, size_t len, int flags)
            {
                int ret = sslcWrite(&userdata.sslc, buf, len);
                if(ret == 0xD840B803)
                {
                    errno = EWOULDBLOCK;
                    return -1;
                }
                
                if(ret < 0)
                {
                    DEBUG("sslcWrite fail: %08X\n", ret);
                    return -1;
                }
                if(!ret)
                {
                    DEBUGS("sslcWrite EOF");
                    return 0;
                }
                return ret;
            }
  }
}
#undef LZZ_INLINE
