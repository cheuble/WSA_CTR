// WSHelper.h
//

#ifndef LZZ_WSHelper_h
#define LZZ_WSHelper_h
#include "WS/WebSocket.hpp"
#include "WS/WebSocketWS.hpp"
#include "WS/WebSocketWSS.hpp"
#define LZZ_INLINE inline
namespace MM
{
  namespace WS
  {
    class WSHelper
    {
    public:
      static WebSocketWS * CreateWS (string addr, string getparam, u16 port = 80);
      static WebSocketWSS * CreateWSS (string addr, string getparam, u16 port = 443);
    };
  }
}
#undef LZZ_INLINE
#endif
