
## libfxnsms: a C library for Sending SMS through Fetion(用于发短信的C库)


### 功能

+ 使用libfxnsms提供的接口，给指定手机号/飞信号发送短信
+ 可以选择发送到飞信或直接到手机
+ UTF-8编码
+ 简单易用

### API(UTF-8编码)

		/*
		当fxn_send返回非0，该变量记录错误信息
		*/
		char *fxn_error;

		/*
		mobile:		你的手机号
		password:	你的飞信密码
		to:			接收者的手机号或飞信号
		sms:		要发送的短信内容
		tocell:		0表示如飞信在线发送到飞信；1表示强制发送到手机
		verbose:	0不输出细节；1输出细节方便调试
		*/
		int fxn_send(const char *mobile, 
				const char *password, 
				const char *to, 
				const char *sms, 
				const int tocell, 
				const int verbose);

### 下载使用

+ 下载地址：<http://code.google.com/p/libfxnsms/>

+ 安装：

		tar xzvf libfxnsms-0.1.tar.gz
		cd libfxnsms
		sudo make install
		
		卸载：
		cd libfxnsms
		sudo make clean
		
+ 示例代码：
		
		//注意：代码需要保存为UTF-8编码
		#include "fxn_sms.h"
		#include <stdio.h>

		int main(int argc, char *argv[]) {
			//发送短信
			if (fxn_send("13800138000", "password", "13500135000", "测试短信，test sms", 1, 1)) {
				printf("success\n");	
			} else {
				printf("%s\n", fxn_error);
			}
			return 0;
		}
		
+ 编译和运行：

		//使用静态库
		gcc -o test test.c -lfxnsms -static
		./test
		
		//使用动态库
		gcc -o test test.c -lfxnsms
		./test

### 命令行工具：

下载包中的`example/cli.c`为使用libfxnsms库实现的命令行下发送短信的小工具，编译：

		gcc -o cli cli.c -lfxnsms

使用cli从命令行发短信：

		./cli -h

		Sending SMS to your Fetion friends written by Min (http://54min.com)

		  -m     your mobile number (required)
		  -p     your Fetion password (required)
		  -t     mobile number or Fetion Id of your friend (required)
		  -s     sms to send (required)
		  -c     in default the sms is sent to Fetion,
			 if set, sms will be sent to cell phone directly
		  -v     if set, show detail
		  -h     print this help and exit
		
		发送短信
		./cli -m 13800138000 -p password -t 13500135000 -s "the sms to send" -cv

更多实例下载包中的`example`文件。
