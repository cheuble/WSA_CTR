// WebSocket.cpp
//

#include "WebSocket.h"
#define LZZ_INLINE inline
namespace MM
{
  namespace WS
  {
    WebSocket::WebSocket (string addr, string getparam, u16 port)
      : addr (addr), getparam (getparam), port (port)
            {
                sta = 0;
                soc_errno = 0;
                
                wslcb = (wslay_event_callbacks)                {
                    recv_callback,
                    send_callback,
                    mask_callback,
                    nullptr,
                    nullptr,
                    nullptr,
                    wsl_onmsg
                };
                
                wslctx = nullptr;
            }
  }
}
namespace MM
{
  namespace WS
  {
    ssize_t WebSocket::recv_callback (wslay_event_context_ptr ctx, unsigned char * buf, unsigned int len, int flags, void * user_data)
            {
                WebSocket* ws = (WebSocket*)user_data;
                
                ssize_t ret = ws->recv_raw(buf, len, 0);
                if(ret <= 0)
                {
                    if(ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        wslay_event_set_error(ws->wslctx, WSLAY_ERR_WOULDBLOCK);
                        return -1;
                    }
                    else
                        wslay_event_set_error(ws->wslctx, WSLAY_ERR_CALLBACK_FAILURE);
                    
                    if(!ret) ws->Close();
                    else ws->soc_errno = errno;
                    
                    DEBUG("WS recv fail: (%i) %s\n", errno, strerror(errno));
                    
                    return -1;
                }
                else return ret;
            }
  }
}
namespace MM
{
  namespace WS
  {
    ssize_t WebSocket::send_callback (wslay_event_context_ptr ctx, unsigned char const * buf, unsigned int len, int flags, void * user_data)
            {
                WebSocket* ws = (WebSocket*)user_data;
                
                ssize_t ret = ws->send_raw(buf, len, 0);
                if(ret <= 0)
                {
                    if(ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                        wslay_event_set_error(ws->wslctx, WSLAY_ERR_WOULDBLOCK);
                    else
                        wslay_event_set_error(ws->wslctx, WSLAY_ERR_CALLBACK_FAILURE);
                    
                    if(!ret) ws->Close();
                    else ws->soc_errno = errno;
                    
                    DEBUG("WS send fail: (%i) %s\n", errno, strerror(errno));
                    
                    return -1;
                }
                else return ret;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocket::mask_callback (wslay_event_context_ptr ctx, unsigned char * buf, unsigned int len, void * user_data)
            {
                sslcGenerateRandomData(buf, len);
                
                return 0;
            }
  }
}
namespace MM
{
  namespace WS
  {
    void WebSocket::wsl_onmsg (wslay_event_context_ptr ctx, struct wslay_event_on_msg_recv_arg const * arg, void * user_data)
            {
                WebSocket* ws = (WebSocket*)user_data;
                
                if(ws->msghandler) ws->msghandler(ctx, arg);
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocket::Disconnect ()
                                     { return -1; }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocket::Tick ()
            {
                int res = 0;
                
                if(CheckConnect())
                if(sta & 2)
                {
                    DEBUGS("Disconnecting");
                    res = Disconnect();
                    DEBUGS("Disconnected");
                    sta &= ~2;
                    if(res < 0) return res;
                    if(connevent) connevent(false);
                }
                
                if(!CheckConnect())
                if(sta & 1)
                {
                    DEBUGS("Connecting");
                    res = Connect();
                    DEBUGS("Connected");
                    sta &= ~1;
                    if(res < 0) return res;
                    if(connevent) connevent(true);
                }
                
                if(!CheckConnect()) return -1;
                
                if(wslay_event_want_read(wslctx) && (polldata(POLLIN) & POLLIN))
                {
                    if(wslay_event_recv(wslctx)) return soc_errno;
                }
                
                if(wslay_event_want_write(wslctx) && (polldata(POLLOUT) & POLLOUT))
                {
                    if(wslay_event_send(wslctx)) return soc_errno;
                }
                
                return 0;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocket::opensock ()
            {
                struct addrinfo hint;
                memset(&hint, 0, sizeof(hint));
                hint.ai_family = AF_INET;
                hint.ai_socktype = SOCK_STREAM;
                hint.ai_protocol = IPPROTO_IP;
                
                int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
                if(sock < 0)
                {
                    if(errno < 0) errno = -errno; //WTF pls
                    DEBUG("socket fail: (%i) %s\n", errno, strerror(errno));
                    return 0;
                }
                
                struct addrinfo* res = nullptr;
                struct addrinfo* ai = nullptr;
                
                char num[6];
                snprintf(num, 6, "%u", port);
                
                int ret = getaddrinfo(addr.c_str(), num, &hint, &res);
                if(ret)
                {
                    DEBUG("getaddrinfo fail: (%i) %s\n", ret, gai_strerror(ret));
                    return ret;
                }
                
                ai = res;
                while(ai)
                {
                    ret = connect(sock, ai->ai_addr, ai->ai_addrlen);
                    if(ret < 0)
                    {
                        DEBUG("connect (loop) fail: (%i) %s\n", errno, strerror(errno));
                        ai = ai->ai_next;
                        continue;
                    }
                    
                    break;
                }
                
                freeaddrinfo(res);
                
                if(!ai)
                {
                    close(sock);
                    return 0;
                }
                
                return sock;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocket::HTTP_Auth (int verify)
            {
                char acceptkey[29];
                u8 randomdata[16];
                sslcGenerateRandomData(randomdata, 16);
                base64((u8*)acceptkey, randomdata, 16);
                
                string tosend = "GET /";
                tosend += getparam;
                tosend += " HTTP/1.1\r\n";
                
                tosend += "Host: " + addr + "\r\n";
                tosend += "Upgrade: websocket\r\n";
                tosend += "Connection: Upgrade\r\n";
                tosend += "User-Agent: WSA_CTR\r\n";
                tosend += "Sec-WebSocket-Version: 13\r\n";
                tosend += "Sec-WebSocket-Key: ";
                tosend += acceptkey;
                tosend += "\r\n\r\n";
                
                DEBUG("HTTP sending headers:\n%s\n", tosend.c_str());
                
                int ret = send_raw(tosend.c_str(), tosend.size());
                if(ret < 0)
                {
                    DEBUG("HTTP send_raw fail: (%i) %s\n", errno, strerror(errno));
                    return -1;
                }
                
                char* recvbuf = new char[0x5001];
                memset(recvbuf, 0, 0x5001);
                
                int offs = 0;
                
                char* findptr = 0;
                
                DEBUGS("HTTP finding header end");
                
                while(offs < 0x4000)
                {
                    ret = recv_raw(recvbuf + offs, 0x1000, MSG_PEEK);
                    if(ret < 0)
                    {
                        DEBUG("HTTP recv_raw peek fail: (%i) %s\n", errno, strerror(errno));
                        break;
                    }
                    if(!ret)
                    {
                        DEBUGS("HTTP peek EOF");
                        break;
                    }
                    
                    if((findptr = strstr(recvbuf, "\r\n\r\n")))
                    {
                        int peekpos = findptr - recvbuf;
                        peekpos += 4;
                        peekpos &= 0xFFF;
                        if(!peekpos) peekpos = 0x1000;
                        if(ret < peekpos)
                            recv_raw(recvbuf + offs, ret, 0);
                        else
                            recv_raw(recvbuf + offs, peekpos, 0);
                        
                        findptr = strstr(recvbuf, "Sec-WebSocket-Accept");
                        if(!findptr)
                        findptr = strstr(recvbuf, "sec-websocket-accept");
                        if(!findptr)
                        {
                            DEBUGS("HTTP no Sec-WebSocket-Accept");
                            break;
                        }
                        
                        if(!verify)
                        {
                            delete[] recvbuf;
                            return 0;
                        }
                        
                        findptr += 22;
                        
                        findptr[28] = '\0';
                        
                        char tstkey[29];
                        create_accept_key(tstkey, findptr);
                        
                        int fml = strncmp(tstkey, findptr, 28);
                        
                        DEBUG("HTTP key comparison:\n- calc: %s\n- recv: %s\n", tstkey, findptr);
                        
                        delete[] recvbuf;
                        
                        return fml ? -1 : 0;
                    }
                    
                    ret = recv_raw(recvbuf + offs, 0x1000, 0);
                    if(ret < 0)
                    {
                        DEBUG("HTTP recv_raw read fail: (%i) %s\n", errno, strerror(errno));
                        break;
                    }
                    if(!ret)
                    {
                        DEBUGS("HTTP recv EOF");
                    }
                    
                    offs += ret;
                }
                
                DEBUGS(recvbuf);
                
                DEBUGS("HTTP found header end without header end");
                
                delete[] recvbuf;
                return -1;
            }
  }
}
namespace MM
{
  namespace WS
  {
    void WebSocket::Open ()
            {
                sta |= 1;
            }
  }
}
namespace MM
{
  namespace WS
  {
    void WebSocket::Close ()
            {
                sta |= 2;
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocket::QueueText (string const str)
            {
                struct wslay_event_msg msg = {1, (const u8*)str.c_str(), str.size()};
                return wslay_event_queue_msg(wslctx, &msg);
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocket::QueueBuffer (void const * data, size_t size)
            {
                struct wslay_event_msg msg = {2, (const u8*)data, size};
                return wslay_event_queue_msg(wslctx, &msg);
            }
  }
}
namespace MM
{
  namespace WS
  {
    int WebSocket::pollsock (int sock, int wat, int timeout)
            {
                struct pollfd pd;
                pd.fd = sock;
                pd.events = wat;
                
                if(poll(&pd, 1, timeout) == 1)
                    return pd.revents & wat;
                return 0;
            }
  }
}
namespace MM
{
  namespace WS
  {
    void WebSocket::sock_mkasync (int fd)
            {
                int status = fcntl(fd, F_GETFL);
                fcntl(fd, F_SETFL, status | O_NONBLOCK);
            }
  }
}
namespace MM
{
  namespace WS
  {
    WebSocket::~ WebSocket ()
            {
                Disconnect();
                if(wslctx) wslay_event_context_free(wslctx);
            }
  }
}
#undef LZZ_INLINE
