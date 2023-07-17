# my-http-server-from-scratch
## 版本

这是上传的第一个版本，暂定为V0.5，基本内容已经完成了  
它已经可以为一个http server提供动力，虽然目前只能展示静态内容  
目前，线程池和程序缓冲区还未开始构建，待后面的版本开始做  

## 程序说明

必须使用支持C++11的的编译器进行构建  
目前，除googletest未依赖任何第三方库，后续按需添加内容  
程序使用CMAKE构建，最好安装高版本的CMAKE  

## 编译

程序根目录，分别运行：  
&emsp;&emsp;&emsp;&emsp;mkdir build  
&emsp;&emsp;&emsp;&emsp;cd build  
&emsp;&emsp;&emsp;&emsp;cmake ..  
&emsp;&emsp;&emsp;&emsp;cmake --build .

## 运行

目前版本，各种配置都在程序内硬编码，后续改成读取配置文件。  
&emsp;&emsp;&emsp;&emsp;./TestDemo/testdemo
