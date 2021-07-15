# QuickServer
A C++ High Performance NetServer (v0.3).

## Tech
 * 基于epoll的IO复用机制实现Reactor模式，connection fd 采用边缘触发（ET）模式和非阻塞模式

 * v0.3利用线程池实现多work线程, one loop per thread

 * 关闭连接
   * 通常情况下，由客户端主动发起FIN关闭连接
   * 客户端发送FIN关闭连接后，服务器把数据发完才close，而不是直接暴力close
   * 如果连接出错，则服务器可以直接close


## Run
	$ ./httpserver [port] [thread_num]
	
	例：$ ./httpserver 8080 4
	表示开启8080端口，采用4工作线程
	
