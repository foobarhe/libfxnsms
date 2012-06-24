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