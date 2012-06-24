#include "fxn_sms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

static char *mobile = NULL;
static char *password = NULL;
static char *to = NULL;
static char *sms = NULL;
static int cell = 0;
static int verbose = 0;

static void show_help();
static void safe_exit();
static char *trim(char *src);

int main(int argc, char *argv[]) {
	//获取参数
	int c;
	char *err = NULL;
	while ( (c = getopt(argc, argv, "m:p:t:s:cvh")) != -1) {
		switch (c) {
			case 'm' : 
				mobile = trim(strdup(optarg));
				if (strlen(mobile) != 11) {
					err = "Error: your mobile is invalid";
				}
				break;
			case 'p' : 
				password = trim(strdup(optarg));
				if (!strlen(password)) {
					err = "Error: password should not be empty";
				}
				break;
			case 't' : 
				to = trim(strdup(optarg));
				if (!strlen(to)) {
					err = "Error: to should not be empty";
				}
				break;
			case 's' : 
				sms = trim(strdup(optarg));
				if (!strlen(sms)) {
					err = "Error: sms should not be empty";
				}
				break;
			case 'c' : 
				cell = 1;
				break;
			case 'v' : 
				verbose = 1;
				break;
			case 'h' :
			default : 
				show_help();
				safe_exit();
				exit(EXIT_FAILURE);
		}
	}

	if (!mobile || !password || !to || !sms) {
		show_help();
		safe_exit();
		exit(EXIT_FAILURE);
	}
	if (err) {
		fprintf(stderr, "%s\n", err);
		show_help();
		safe_exit();
		exit(EXIT_FAILURE);
	}

	//发送短信
	if (fxn_send(mobile, password, to, sms, cell, verbose)) {
		printf("success\n");	
	} else {
		printf("%s\n", fxn_error);
	}
	safe_exit();
	return 0;
}

static void show_help() {
	char *help = "\nSending SMS to your Fetion friends written by Min (http://54min.com)\n\n"
		"  -m     your mobile number (required)\n"
		"  -p     your Fetion password (required)\n"
		"  -t     mobile number or Fetion Id of your friend (required)\n"
		"  -s     sms to send (required)\n"
		"  -c     in default the sms is sent to Fetion, \n"
		"         if set, sms will be sent to cell phone directly\n"
		"  -v     if set, show detail\n"
		"  -h     print this help and exit\n"
		"\n";
	printf("%s", help);
}

static void safe_exit() {
	if (mobile) {
		free(mobile);
	}
	if (password) {
		free(password);
	}
	if (to) {
		free(to);
	}
	if (sms) {
		free(sms);
	}
}

static char *trim(char *src) {
	char *start;
	start = src;
	while (isspace(*start)) {
		if ( (start - src) == (strlen(src) -1) ) {
			src[0] = '\0';
			return src;
		}
		start++;
	}
	char *end;
	end = src + strlen(src) - 1;
	while (isspace(*end)) {
		if (end == src) {
			src[0] = '\0';
		}
		end--;
	}

	char *p;
	p = src;
	while (start <= end) {
		*p = *start;
		p++;
		start++;
	}
	*p = '\0';
	return src;
}
