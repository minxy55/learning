#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Thread.h>
#include <muduo/base/CountDownLatch.h>

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

void print(EventLoop* p = NULL)
{
  printf("print: pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::tid(), p);
}

void quit(EventLoop* p)
{
  print(p);
  p->quit();
}

int main()
{
  print();

  {
  EventLoopThread thr1;  // never start
  }

  {
  // dtor calls quit()
  printf("dtor calls quit()\n");
  EventLoopThread thr2;
  EventLoop* loop = thr2.startLoop();
  loop->runInLoop(boost::bind(print, loop));
  CurrentThread::sleepUsec(500 * 1000);
  }

  {
  	printf("thr4\n");
  	EventLoopThread thr4;
	EventLoop *loop = thr4.startLoop();
	loop->runAfter(1, boost::bind(print, loop));
	loop->runAfter(2, boost::bind(print, loop));
	CurrentThread::sleepUsec(3000 * 1000);
  }

  {
  // quit() before dtor
  printf("quit() before dtor\n");
  EventLoopThread thr3;
  EventLoop* loop = thr3.startLoop();
  loop->runInLoop(boost::bind(quit, loop));
  CurrentThread::sleepUsec(500 * 1000);
  }
}

