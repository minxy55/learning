#include <examples/cdns2/Resolver2.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

#include <ares.h>
#include <netdb.h>
#include <arpa/inet.h>  // inet_ntop
#include <netinet/in.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

using namespace muduo;
using namespace muduo::net;
using namespace cdns2;

namespace
{
double getSeconds(struct timeval* tv)
{
  if (tv)
    return double(tv->tv_sec) + double(tv->tv_usec)/1000000.0;
  else
    return -1.0;
}

const char* getSocketType(int type)
{
  if (type == SOCK_DGRAM)
    return "UDP";
  else if (type == SOCK_STREAM)
    return "TCP";
  else
    return "Unknown";
}

const bool kDebug = false;
}

Resolver2::Resolver2(EventLoop* loop, Option opt)
  : loop_(loop),
    ac_(NULL),
    timerActive_(false)
{
	int status;

  status = ares_library_init(ARES_LIB_INIT_ALL);
  if (status != ARES_SUCCESS)
  {
    assert(0);
  }

  status = ares_init(&ac_);
  if (status != ARES_SUCCESS)
  {
    assert(0);
  }
  
  ares_set_socket_callback(ac_, &Resolver2::ares_sock_create_callback, this);
}

Resolver2::~Resolver2()
{
  ares_destroy(ac_);
  ares_library_cleanup();
}

bool Resolver2::resolve(StringArg hostname, const Callback& cb)
{
  loop_->assertInLoopThread();
  HostData* queryData = new HostData(this, cb);
  ares_gethostbyname(ac_, hostname.c_str(), AF_INET,
      &Resolver2::ares_host_callback, queryData);
  struct timeval tv;
  struct timeval* tvp = ares_timeout(ac_, NULL, &tv);
  double timeout = getSeconds(tvp);
  LOG_DEBUG << "timeout " <<  timeout << " active " << timerActive_;
  if (!timerActive_)
  {
    loop_->runAfter(timeout, boost::bind(&Resolver2::onTimer, this));
    timerActive_ = true;
  }
  return queryData != NULL;
}

void Resolver2::onRead(int sockfd, Timestamp t)
{
  LOG_DEBUG << "onRead " << sockfd << " at " << t.toString();
  ares_process_fd(ac_, sockfd, ARES_SOCKET_BAD);
}

void Resolver2::onTimer()
{
  assert(timerActive_ == true);
  ares_process_fd(ac_, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
  struct timeval tv;
  struct timeval* tvp = ares_timeout(ac_, NULL, &tv);
  double timeout = getSeconds(tvp);
  LOG_DEBUG << loop_->pollReturnTime().toString() << " next timeout " <<  timeout;

  if (timeout < 0)
  {
    timerActive_ = false;
  }
  else
  {
    loop_->runAfter(timeout, boost::bind(&Resolver2::onTimer, this));
  }
}

void Resolver2::onQueryResult(int status, struct hostent* result, const Callback& callback)
{
  LOG_DEBUG << "onQueryResult " << status;
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  if (result)
  {
    addr.sin_addr = *reinterpret_cast<in_addr*>(result->h_addr);
    
    if (kDebug)
    {
      printf("h_name %s\n", result->h_name);
      for (char** alias = result->h_aliases; *alias != NULL; ++alias)
      {
        printf("alias: %s\n", *alias);
      }
      // printf("ttl %d\n", ttl);
      // printf("h_length %d\n", result->h_length);
      for (char** haddr = result->h_addr_list; *haddr != NULL; ++haddr)
      {
        char buf[32];
        inet_ntop(AF_INET, *haddr, buf, sizeof buf);
        printf("  %s\n", buf);
      }
    }
  }
  InetAddress inet(addr);
  callback(inet);
}

void Resolver2::onSockCreate(int sockfd, int type)
{
  loop_->assertInLoopThread();
  assert(channels_.find(sockfd) == channels_.end());
  Channel* channel = new Channel(loop_, sockfd);
  channel->setReadCallback(boost::bind(&Resolver2::onRead, this, sockfd, _1));
  channel->enableReading();
  channels_.insert(sockfd, channel);
}

void Resolver2::onSockStateChange(int sockfd, bool read, bool write)
{
  loop_->assertInLoopThread();
  ChannelList::iterator it = channels_.find(sockfd);
  assert(it != channels_.end());
  if (read)
  {
    // update
    // if (write) { } else { }
  }
  else
  {
    // remove
    it->second->disableAll();
    it->second->remove();
    channels_.erase(it);
  }
}

void Resolver2::ares_host_callback(void* data, int status, int timeouts, struct hostent* hostent)
{
  HostData* query = static_cast<HostData*>(data);

  query->owner->onQueryResult(status, hostent, query->callback);
  delete query;
}

int Resolver2::ares_sock_create_callback(int sockfd, int type, void* data)
{
  LOG_TRACE << "sockfd=" << sockfd << " type=" << getSocketType(type);
  static_cast<Resolver2*>(data)->onSockCreate(sockfd, type);
  return 0;
}

void Resolver2::ares_sock_state_callback(void* data, int sockfd, int read, int write)
{
  LOG_TRACE << "sockfd=" << sockfd << " read=" << read << " write=" << write;
  static_cast<Resolver2*>(data)->onSockStateChange(sockfd, read, write);
}

