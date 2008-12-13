#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

char * substr(char *string, int from, int to) {
	char *result;
	int i, j;

	result = (char *)malloc(strlen(string)+1);

	j = 0;
	for (i = from; i < to+1; i++) {
		if (string[i] == '\0'|| i >= strlen(string) )
			break;
		result[j] = string[i];
		j++;
	}

	return ((char *)result);

}

char * stringtoken(char *string, char tag, int *log) {
	int i, j;
	char *result;

	result = (char *)malloc(strlen(string)+1);

	j = 0;
	for (i = *log;; i++) {
		if (i == strlen(string) || i >= strlen(string) )
			break;
		if (string[i] == 0)
			break;
		if (string[i] == tag)
			break;
		result[j] = string[i];
		j++;
	}

	*log = i+1;
	result[j] = '\0';
	return ((char *)result);
}

/* deliverd from bbs source .. (stuff.c) */
/* Case Independent strncmp */
//大小写无关的strncmp()
int ci_strncmp(register char *s1, register char *s2, register int n) {
	char c1, c2;

	while (n-- > 0) {
		c1 = *s1++;
		c2 = *s2++;
		if (c1 >= 'a' && c1 <= 'z')
			c1 += 'A' - 'a';
		if (c2 >= 'a' && c2 <= 'z')
			c2 += 'A' - 'a';
		if (c1 != c2)
			return (c1 - c2);
		if (c1 == 0)
			return 0;
	}
	return 0;
}

int
ci_strcmp( s1, s2 )
register char *s1, *s2;
{
	char c1, c2;

	while( 1 ) {
		c1 = *s1++;
		c2 = *s2++;
		if( c1 >= 'a' && c1 <= 'z' )
		c1 += 'A' - 'a';
		if( c2 >= 'a' && c2 <= 'z' )
		c2 += 'A' - 'a';
		if( c1 != c2 )
		return (c1 - c2);
		if( c1 == 0 )
		return 0;
	}
}

// Convert string src to lowercase and store it in dst.
// Caller should ensure the capacity of dst is no less than src.
char *strtolower(char *dst, char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src++ != '\0')
		*dst++ = tolower(*src);
	*dst = '\0';
	return ret;	
}

// Convert string src to uppercase and store it in dst.
// Caller should ensure the capacity of dst is no less than src.
char *strtoupper(char *dst, char *src) {
	char *ret = dst;

	if (dst == NULL || src == NULL)
		return NULL;
	while (*src++ != '\0')
		*dst++ = toupper(*src);
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
