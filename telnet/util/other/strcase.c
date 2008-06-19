int
strncasecmp2(char *s1, char *s2,int n)
  {
      register int c1, c2,l=0;
  
      while (*s1 && *s2 && l<n) {
	
      	c1 = *s1;	
      	c2 = *s2;
	if ((c1-'a'>=0 && c1-'z'<=0) || (c1-'A'>=0 && c1-'Z'<=0)) {
		c1=tolower(c1);
		c2=tolower(c2);
	}
      	if (c1 != c2)
          return (c1 - c2);
      	s1++;
      	s2++;
      	l++;
      }
      if (l==n)
      	return (int) (0);
      else
	return -1;
  }


char *strcasestr(char *s1, char *s2) {
	int l;
	l=strlen(s2);
	while(s1[0]) {
		if(!strncasecmp2(s1, s2, l)) 
			{
				printf("strlen:%d,s1:%s\r\n,s1c1: %d,s1c1lower%d,s2c2: %d,s2c2lower%d,c1-'a':%d\n\r",l,s1,*s1,tolower(*s1),s2,tolower(*s2),*s1-'a'); 
				return s1;
			}
		s1++;
	}
	return 0;
}

int main()
{
if (strcasestr("知识产权要发展","冠军杯"))
 printf("true");
else
 printf("false");
return 0;
}