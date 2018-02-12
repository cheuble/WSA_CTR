// WebSocket.h
//

#ifndef LZZ_WebSocket_h
#define LZZ_WebSocket_h
#include <3ds.h>

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#include <wslay/wslay.h>

#include "dummy.h"

#include "thing.h"
}

#include <string>
#include <functional>

using namespace std;
#define LZZ_INLINE inline
namespace MM
{
  namespace WS
  {
    typedef function <void(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg* arg)> WSMSGFUNC;
  }
}
namespace MM
{
  namespace WS
  {
    typedef function <void(bool conn)> WSCONNEVT;
  }
}
namespace MM
{
  namespace WS
  {
    class WebSocket
    {
    protected:
      string addr;
      string getparam;
      u16 port;
      int sta;
      int soc_errno;
      wslay_event_callbacks wslcb;
      wslay_event_context_ptr wslctx;
    public:
      WSMSGFUNC msghandler;
      WSCONNEVT connevent;
      WebSocket (string addr, string getparam, u16 port);
    private:
      static ssize_t recv_callback (wslay_event_context_ptr ctx, unsigned char * buf, unsigned int len, int flags, void * user_data);
      static ssize_t send_callback (wslay_event_context_ptr ctx, unsigned char const * buf, unsigned int len, int flags, void * user_data);
      static int mask_callback (wslay_event_context_ptr ctx, unsigned char * buf, unsigned int len, void * user_data);
      static void wsl_onmsg (wslay_event_context_ptr ctx, struct wslay_event_on_msg_recv_arg const * arg, void * user_data);
    public:
      virtual int Connect (int verify = 0) = 0;
      virtual int Disconnect ();
      virtual int CheckConnect () = 0;
      virtual int polldata (int wat, int timeo = 0) = 0;
      virtual int recv_raw (void * buf, size_t len, int flags = 0) = 0;
      virtual int send_raw (void const * buf, size_t len, int flags = 0) = 0;
      int Tick ();
      int opensock ();
      int HTTP_Auth (int verify = 0);
      void Open ();
      void Close ();
      int QueueText (string const str);
      int QueueBuffer (void const * data, size_t size);
      static int pollsock (int sock, int wat, int timeout = 0);
      static void sock_mkasync (int fd);
      virtual ~ WebSocket ();
    };
  }
}
#undef LZZ_INLINE
#endif
