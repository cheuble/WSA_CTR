// WSHelper.cpp
//

#include "WSHelper.h"
#define LZZ_INLINE inline
using namespace MM::WS;
namespace MM
{
  namespace WS
  {
    WebSocketWS * WSHelper::CreateWS (string addr, string getparam, u16 port)
            {
                return new WebSocketWS(addr, getparam, port);
            }
  }
}
namespace MM
{
  namespace WS
  {
    WebSocketWSS * WSHelper::CreateWSS (string addr, string getparam, u16 port)
            {
                return new WebSocketWSS(addr, getparam, port);
            }
  }
}
#undef LZZ_INLINE
