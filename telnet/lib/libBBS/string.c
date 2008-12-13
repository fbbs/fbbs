#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Convert string src to lowercase and store it in dst.
// Caller should ensure the capacity of dst is no less than src.
char *strtolower(char *dst, char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src != '\0')
		*dst++ = tolower(*src++);
	*dst = '\0';
	return ret;	
}

// Convert string src to uppercase and store it in dst.
// Caller should ensure the capacity of dst is no less than src.
char *strtoupper(char *dst, char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src != '\0')
		*dst++ = toupper(*src++);
	*dst = '\0';
	return ret;	
}

void my_ansi_filter(char *source) {
	char result[500];
	int i, flag = 0, loc=0;

	for (i = 0; i < strlen(source) ; i++) {
		if (source[i] == '\033') {
			flag = 1;
			continue;
		} else if (flag == 1 && isalpha(source[i]) ) {
			flag = 0;
			continue;
		} else if (flag == 1) {
			continue;
		} else {
			result[loc++]=source[i];
		}
	}
	result[loc]='\0';
	strncpy(source, result, loc+1);
}

char * ansi_filter(char *source) {
	char *result, ch[3];
	int i, flag = 0, slen = strlen(source);

	result = (char *)malloc((slen+10)*sizeof(char));

	for (i = 0; i < slen; i++) {
		if (source[i] == '\033') {
			flag = 1;
			continue;
		} else if (flag == 1 && isalpha(source[i]) ) {
			flag = 0;
			continue;
		} else if (flag == 1) {
			continue;
		} else {
			sprintf(ch, "%c", source[i]);
			strcat(result, ch);
		}
	}

	return (char *)result;
}

// 将一个整数时间值轮换成 年月日时分秒周日格式,并返回
char *Cdate(time_t *clock) {
	static char foo[23];
	struct tm *mytm = localtime(clock);

	strftime(foo, 23, "%D %T %a", mytm);
	return (foo);
}
