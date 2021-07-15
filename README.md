# QuickServer
A C++ High Network Server (v0.3).

## Introduce
 * 基于 epoll 的 I / O 复用机制， 采取 Reactor 事件处理模式，实现高性能服务器程序框架
 * 并发编程为半同步 / 半异步模式，异步线程监听到客户请求后通过条件变量随机唤醒睡眠的工作线程
 * 应用层实现了简单的HTTP服务器HttpSession，服务器实现了HTTP的解析和Get方法请求，目前支持静态资源访问

## Tech
 * 多线程采用 C++ Thread 库实现

 * v0.3 主线程监听 I / O，并将任务插入工作队列；利用线程池实现多工作线程, one loop per thread 执行读写、处理请求等操作。

 * 成功连接的 Socket 采用边缘触发（ET）模式和非阻塞模式

 * 关闭连接的情形
   * 通常情况下，由客户端主动发起FIN关闭连接
   * 客户端发送FIN关闭连接后，服务器把数据发完才close，而不是直接暴力close
   * 如果连接出错，则服务器可以直接close

## Run
	$ ./build/QuickServer [port] [thread_num]

  * 例：$ ./build/QuickServer 8080 4
  
  * 表示开启8080端口，采用4工作线程
  
  * 线程数限制为 2-16，不建议启动过多线程
  
  * IP 默认 127.0.0.1


	
