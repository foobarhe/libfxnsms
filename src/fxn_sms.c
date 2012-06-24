#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

char *fxn_error = NULL;

static const char *fxn_hostname = "f.10086.cn";
static const char *fxn_loginpath = "/im/login/inputpasssubmit1.action";
static const char *fxn_logoutpath = "/im/index/logoutsubmit.action";
static const char *fxn_loginstatus = "4";
static const char *fxn_searchpath = "/im/index/searchOtherInfoList.action";
static const char *fxn_sendselfpath = "/im/user/sendMsgToMyselfs.action?touserid=";
static const char *fxn_send2fetionpath = "/im/chat/sendMsg.action?touserid=";
static const char *fxn_send2cellpath = "/im/chat/sendShortMsg.action?touserid=";

static char *m = NULL;
static char *pass = NULL;
static char *t = NULL;
static char *msg = NULL;
static int  c = 0;
static int sock = -1;
static char *cookie = NULL;
static char *userid = NULL;

static void fxn_safeexit();
static char *fxn_trim(char *src);
static int fxn_getip(const char *input, struct in_addr *ip);
static int fxn_setSockNonBlock();
static int fxn_initsock();
static int fxn_http(const char *request, const char *endmark, char **response);
static int fxn_login();
static int fxn_search();
static int fxn_sendsms();
static int fxn_logout();

//唯一的一个公共接口
int fxn_send(const char *mobile, const char *password, const char *to, const char *sms, const int tocell, const int verbose) {
	fxn_error = NULL;
	m = NULL;
	pass = NULL;
	t = NULL;
	msg = NULL;
	c = tocell;
	sock = -1;
	cookie = NULL;
	userid = NULL;
	//获取参数
	m = fxn_trim(strdup(mobile));
	if (strlen(m) != 11) {
		free(m);
		fxn_error = "Error: mobile is invalid";
		return 0;
	}
	pass = fxn_trim(strdup(password));
	if (!strlen(pass)) {
		free(pass);
		fxn_error = "Error: password should not be empty.";
		return 0;
	}
	t = fxn_trim(strdup(to));
	if (!strlen(t)) {
		free(t);
		fxn_error = "Error: to number is invalid";
		return 0;
	}
	msg = fxn_trim(strdup(sms));
	if (!strlen(msg)) {
		free(msg);
		fxn_error = "Error: sms should not be empty.";
		return 0;
	}
	if (verbose) {
		printf("\n---------------\n"
				"mobile: %s\n"
				"pass:   %s\n"
				"to:     %s\n"
				"sms:    %s\n"
				"tocell: %d\n\n", m, pass, t, msg, c);	
	}
	//建立sock
	if (!fxn_initsock()) {
		fxn_safeexit();
		return 0;
	}
	if (verbose) {
		printf("... init done!\n");
	}
	//登录获取cookie
	if (!fxn_login()) {
		fxn_safeexit();
		return 0;
	}
	if (verbose) {
		printf("... login done!\n");
	}
	//获取userid(需携带cookie)
	if (strcmp(m, t) != 0) {
		if (!fxn_search()) {
			fxn_safeexit();
			return 0;			
		}
	}
	if (verbose) {
		printf("... search done, userid = %s\n", userid);
	}
	//发送短信(需携带cookie)
	if (!fxn_sendsms()) {
		fxn_safeexit();
		return 0;
	}
	if (verbose) {
		printf("... sendsms done!\n");
	}
	//退出(需携带cookie)
	if (!fxn_logout()) {
		fxn_safeexit();
		return 0;
	}
	if (verbose) {
		printf("... logout done!\n");
	}

	fxn_safeexit();
	return 1;
}

/* private的静态函数 */

//安全退出
static void fxn_safeexit() {
	if (m) {
		free(m);
	}
	if (pass) {
		free(pass);
	}
	if (t) {
		free(t);
	}
	if (msg) {
		free(msg);
	}
	if (sock > 0) {
		close(sock);
	}
	if (cookie) {
		free(cookie);
	}
}
//trim字符串
static char *fxn_trim(char *src) {
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
//根据hostname获取IP地址
static int fxn_getip(const char *input, struct in_addr *ip) {
	struct hostent *host;
	if ((host = gethostbyname(input)) == NULL) {
		fxn_error = "Error: get ip of fetion site faield.";
		return 0;
	}
	int i;
	for (i = 0; host->h_addr_list[i] != NULL; i++) {
		if (host->h_addrtype == AF_INET) {
			memcpy(ip, (struct in_addr *)(host->h_addr_list[i]), sizeof(struct in_addr));
			break;
		}
	}
	return 1;
}
//将sock设置为non-blocking的
static int fxn_setSockNonBlock() {
	int flags;
	flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0) {
		fxn_error = "Error: fcntl(F_GETFL) failed";
		return 0;
	}
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
		fxn_error = "Error: fcntl(F_SETFL) failed";
		return 0;
	}
	return 1;
}
//创建一个sock
static int fxn_initsock() {
	struct in_addr ip;
	if (!fxn_getip(fxn_hostname, &ip)) {
		return 0;
	}
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		fxn_error = "Error: create socket failed";
		return 0;
	}
	if (!fxn_setSockNonBlock()) {
		close(sock);
		return 0;
	}
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr = ip;
	serv_addr.sin_port = htons(80);
	if ( (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) && (errno != EINPROGRESS) ) {
		close(sock);
		fxn_error = "Error: connect to fetion server faield";
		return 0;
	}
	return 1;
}
//http request和response
static int fxn_http(const char *request, const char *endmark, char **response) {
	//send request
	char *tmp = (char *)request;
	int remaining = strlen(request);
	ssize_t sent_size;
	while (remaining) {
		sent_size = send(sock, tmp, remaining, 0);
		if (sent_size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			continue;	//blocking状态，继续尝试发送
		}
		if (sent_size <= 0) {
			fxn_error = "Error: send data to fetion server failed";
			return 0;
		}
		remaining -= sent_size;
		tmp += sent_size;
	}
	//get response
	char *resp = NULL;
	int resp_size = 0;
	char buf[1024];
	ssize_t recv_size;
	while (1) {
		recv_size = recv(sock, buf, sizeof(buf), 0);
		if (recv_size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			continue;
		}
		if (recv_size <= 0) {
			break;
		}
		tmp = (char *) realloc(resp, resp_size + recv_size + 1);
		if (tmp == NULL) {
			fxn_error = "Error: realloc failed, not enought memory to save response";
			if (resp) {
				free(resp);
			}
			return 0;
		}
		resp = tmp;
		tmp = resp + resp_size;
		memcpy(tmp, buf, recv_size);
		resp_size += recv_size;
		resp[resp_size] = '\0';
		if (strstr(resp, endmark)) {
			*response = resp;
			return 1;
		}
	}
	fxn_error = "Error: receive data from fetion server failed";
	return 0;
}
//登录获取cookie
static int fxn_login() {
	char buf[1024] = {0};
	sprintf(buf, "HEAD %s?m=%s&pass=%s&loginstatus=%s HTTP/1.1\r\nHOST: %s\r\nUser-Agent: Mozilla/5.0 (Windows NT 5.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1\r\n\r\n", fxn_loginpath, m, pass, fxn_loginstatus, fxn_hostname);	
	char *response = NULL;
	if (!fxn_http(buf, "\r\n\r\n", &response)) {
		if (response) {
			free(response);
		}
		return 0;
	}
	if (!response) {
		fxn_error = "Error: get response from fetion server failed";
		return 0;
	}
	//从response中得到cookie
	char *tmp = response;
	char *start = buf;
	char *tmp2, *tmp3;
	while ((tmp2 = strstr(tmp, "Set-Cookie: "))) {
		tmp2 = tmp2 + strlen("Set-Cookie: ");
		if ((tmp3 = strstr(tmp2, "path="))) {
			tmp = tmp3;
			memcpy(start, tmp2, tmp3 - tmp2);
			start = start + (tmp3 - tmp2);
		} else {
			break;
		}
	}
	free(response);
	*start = '\0';
	if (strlen(buf) && strstr(buf, "cell")) {
		cookie = strdup(buf);
		return 1;
	} else {
		fxn_error = "Error: mobile or password is incorrect.";
		return 0;
	}
}
//获取userid
static int fxn_search() {
	char buf[1024] = {0};
	sprintf(buf, "GET %s?searchText=%s HTTP/1.1\r\nHOST: %s\r\nUser-Agent: Mozilla/5.0 (Windows NT 5.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1\r\nCookie: %s\r\n\r\n", fxn_searchpath, t, fxn_hostname, cookie);
	char *response = NULL;
	if (!fxn_http(buf, "</wml>", &response)) {
		if (response) {
			free(response);
		}
		return 0;
	}
	if (!response) {
		fxn_error = "Error: get response from fetion server failed";
		return 0;
	}
	//从response中得到userid
	char *tmp;
	if ((tmp = strstr(response, "touserid=")) == NULL) {
		free(response);
		fxn_error = "Error: the to number is not your friend";
		return 0;
	}
	tmp = tmp + strlen("touserid=");
	char *tmp2;
	if ((tmp2 = strstr(tmp, "&amp;type=all")) == NULL) {
		free(response);
		fxn_error = "Error: the to number is not your friend";
		return 0;
	}
	char *start = buf;
	memcpy(start, tmp, tmp2 - tmp);
	free(response);
	*(start + (tmp2 - tmp)) = '\0';
	if (strlen(buf)) {
		userid = strdup(buf);
		return 1;
	} else {
		fxn_error = "Error: the to number is not your friend";
		return 0;
	}
}
//发送短信
static int fxn_sendsms() {
	char *path = NULL;
	if (userid == NULL) {
		path = (char *)fxn_sendselfpath;
		userid = "";
	} else {
		if (c == 0) {
			path = (char *)fxn_send2fetionpath;
		} else {
			path = (char *)fxn_send2cellpath;
		}
	}
	char post[1024] = {0};
	sprintf(post, "backUrl=&touchTitle=&touchTextLength=&msg=%s\r\n", msg);
	char buf[1024] = {0};
	sprintf(buf, "POST %s%s HTTP/1.1\r\nHOST: %s\r\nUser-Agent: Mozilla/5.0 (Windows NT 5.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1\r\nCookie: %s\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n%s\r\n", path, userid, fxn_hostname, cookie, strlen(post), post);

	char *response = NULL;
	if (!fxn_http(buf, "</wml>", &response)) {
		if (response) {
			free(response);
		}
		return 0;
	}
	if (!response) {
		fxn_error = "Error: get response from fetion server failed";
		return 0;
	}
	//从response中得到结果
	if (strstr(response, "成功")) {
		free(response);
		return 1;
	} else {
		free(response);
		fxn_error = "Error: send sms failed, please retry later.";
		return 0;
	}
}
//退出
static int fxn_logout() {
	char buf[1024] = {0};
	sprintf(buf, "GET %s HTTP/1.1\r\nHOST: %s\r\nUser-Agent: Mozilla/5.0 (Windows NT 5.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1\r\nCookie: %s\r\n\r\n", fxn_logoutpath, fxn_hostname, cookie);
	char *response = NULL;
	if (!fxn_http(buf, "</wml>", &response)) {
		if (response) {
			free(response);
		}
		return 0;
	}
	if (!response) {
		fxn_error = "Error: get response from fetion server failed";
		return 0;
	}
	//从response中得到结果
	if (strstr(response, "成功")) {
		free(response);
		return 1;
	} else {
		free(response);
		fxn_error = "Error: logout faield, please retry later.";
		return 0;
	}
}