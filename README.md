# my-http-server-from-scratch
## 版本

版本V0.5改动：  
* 为一个简单的http server提供动力，用于展示静态内容；  
* 支持I/O多路复用，实践Reactor模式  

版本V0.8改动：  
* 远程连接使用抽象的连接池进行管理  
* 更改Reactor类API的传入参数，修改接口  
* 业务与框架解耦（不完全，描述不准确）  
* 资源回收逻辑优化  
* 本版本暂时未引入线程池

## 程序说明

必须使用支持C++11的的编译器进行构建  
目前，除googletest未依赖任何第三方库，后续增加sqlite做为本地数据库  
程序使用CMAKE构建，最好安装高版本的CMAKE  

## 编译

程序根目录，分别运行：  
&emsp;&emsp;&emsp;&emsp;mkdir build  
&emsp;&emsp;&emsp;&emsp;cd build  
&emsp;&emsp;&emsp;&emsp;cmake ..  
&emsp;&emsp;&emsp;&emsp;cmake --build .

## 运行

目前版本，各种配置都在程序内硬编码，包括程序运行时间、是否守护进程启动等。  
在build目录中运行程序  
&emsp;&emsp;&emsp;&emsp;./TestDemo/testdemo 9901
