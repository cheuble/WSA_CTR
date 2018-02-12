// WebSocketWSS.h
//

#ifndef LZZ_WebSocketWSS_h
#define LZZ_WebSocketWSS_h
#include <3ds.h>

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <arpa/inet.h>

#include <wslay/wslay.h>
}

#include <string>
#include <functional>


#include "WebSocket.hpp"

using namespace std;
using MM::WS::WebSocket;
#define LZZ_INLINE inline
namespace MM
{
  namespace WS
  {
    struct WSS_Userdata
    {
      int sock;
      sslcContext sslc;
    };
  }
}
namespace MM
{
  namespace WS
  {
    class WebSocketWSS : public WebSocket
    {
    private:
      WSS_Userdata userdata;
    public:
      WebSocketWSS (string addr, string getparam, u16 port);
      int Connect (int verify = 0);
      int Disconnect ();
      int CheckConnect ();
      int polldata (int wat, int timeo = 0);
      int recv_raw (void * buf, size_t len, int flags = 0);
      int send_raw (void const * buf, size_t len, int flags = 0);
    };
  }
}
#undef LZZ_INLINE
#endif
