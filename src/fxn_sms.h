#ifndef _FXN_SMS_H_
#define _FXN_SMS_H_

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
int fxn_send(const char *mobile, const char *password, const char *to, const char *sms, const int tocell, const int verbose);

#endif