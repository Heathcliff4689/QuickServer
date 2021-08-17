# QuickServer
A C++ High Performance Network Server (v0.4).

## Introduce
 * 基于 epoll 的 I / O 复用机制， 采取 Reactor 事件处理模式，实现高性能服务器程序框架
 * 多线程为半同步 / 半异步模式，异步线程监听到客户请求后通过条件变量随机唤醒睡眠的工作线程
 * 应用层实现了简单的 HTTP 服务器 HttpSession，能够解析HTTP协议和Get方法请求
 	* 目前支持静态资源访问
	* 支持长连接
	* 支持优雅关闭连接
	* 支持定时器功能
 * 使用智能指针实现资源获取及释放
	

## Tech
 * 线程池及线程同步采用 C++ 11 Thread 库实现

 * v0.4 主线程监听 I / O，处理到期时间事件，并将网络事件插入工作队列；利用线程池实现多工作线程, one loop per thread 执行读写、处理请求等操作。

 * 成功连接的 Socket 采用边缘触发（ET）和非阻塞模式，并设置 EPOLLONESHOOT 标志位
 
 * 使用红黑树实现定时器，可自定义时间跨度关闭不活跃连接
   * 当有事件发生时，主线程首先处理到期的时间事件，然后处理网络事件
   * 每当新的网络事件发生，主线向定时器注册相应时间事件

 * 关闭连接的情形
   * 通常情况下，由数据发送方确定时间主动关闭连接
   * 数据发送方等待数据发送完成，关闭写端，shutdown(fd, SHUT_WR), 等下一次read返回0，再close
   * 避免数据发送方直接close且本地读缓存非空时，向对端发送RST，导致对端数据读不完整
   * 如果连接出错，本地直接close

## Run  
	$ cd build/
	$ ./QuickServer [port] [thread_num]
	  
	  
  * 例：$ ./QuickServer 8080 4 
  
  * 表示开启8080端口，采用4工作线程
  
  * 线程数限制为 2-16，不建议启动过多线程
  
  * IP 默认 127.0.0.1

## Test
* 测试工具  
	* webbench
* 测试流程
	* 模拟1000条客户端连接，测试 30 s
	* 测试读取磁盘数据
	* 测试仅读取内存数据
* 测试环境
	* Processor Intel Xeon(R) CPU E5-2620 v3 @ 2.40Ghz * 8
	* Memory 11.7 GiB
	* OS Ubuntu 18.04 LTS
	* Vmware Workstation 7
* 测试命令 0  
	* 服务器端	 

			./QuickServer 8080 4 
* 测试命令 1
	* 读取磁盘文件

			webbench -c 1000 -t 30 http://127.0.0.1:8080/index.html

![IO-affected](https://github.com/Heathcliff4689/QuickServer/blob/v0.3/test/IO_imfe.png)
* 测试命令 2
	* 不读取磁盘文件

			webbench -c 1000 -t 30 http://127.0.0.1:8080/

![Non-IO impacted](https://github.com/Heathcliff4689/QuickServer/blob/v0.3/test/NON-IO_imfe.png)



	
