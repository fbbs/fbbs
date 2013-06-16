/* 七天未上站的版主统计 by soff of bitbbs.org */

#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "fbbs/time.h"

int main (int argc, char *argv[])
{
	FILE *fp, *fout, *bmfp;
	fb_time_t now = fb_time();
	char which[20];
	int n, i, j = 0;
	struct userec aman;
  	char buf[256];

  	sprintf (buf, "%s/.PASSWDS", BBSHOME);
  	if ((fp = fopen (buf, "rb")) == NULL) {
    	printf ("Can't open record data file.\n");
    	return 1;
    }
  	sprintf (buf, "%s/0Announce/bbslist/badbms", BBSHOME);
  	if ((fout = fopen (buf, "w")) == NULL){
    	fclose (fp);
      	printf ("Can't write to badbms file.\n");
      	return 1;
    }
  	//% fprintf (fout, "\n%s七天未上站的版主名单\n\n", BBSNAME);
  	fprintf (fout, "\n%s\xc6\xdf\xcc\xec\xce\xb4\xc9\xcf\xd5\xbe\xb5\xc4\xb0\xe6\xd6\xf7\xc3\xfb\xb5\xa5\n\n", BBSNAME);
  	for (i = 0;; i++)  {
    	if (fread (&aman, sizeof (struct userec), 1, fp) <= 0)
			break;
      	if (!(aman.userlevel & PERM_BOARDS)|| !strcasecmp (aman.userid, "SYSOP"))
			continue;
      	if ((now - aman.lastlogin) / 86400 >= 7){
	  		sprintf(	buf,"%s/home/%c/%s/.bmfile",BBSHOME,
						toupper(aman.userid[0]),aman.userid);
		  	if ((bmfp = fopen (buf, "rb")) == NULL) {
	    		printf ("Can't read %s bmfile file.\n", aman.userid);
	    		continue;
		    }
		  	for (n = 0; n < 3; n++){
	    		if (feof (bmfp))
					break;
		     	fscanf (bmfp, "%s\n", which);
		      	fprintf (	fout,
		    		  //% " ** [1;33m%-16s[m版主 [1;32m%-15.15s[m [ %-20.20s ]%6"PRIdFBT"天\n",
		    		  " ** [1;33m%-16s[m\xb0\xe6\xd6\xf7 [1;32m%-15.15s[m [ %-20.20s ]%6"PRIdFBT"\xcc\xec\n",
		       			which, aman.userid, aman.username, (now - aman.lastlogin) / 86400);
	   		}
	 		fclose (bmfp);
		  	j++;
		}
	}
	fclose (fp);
	if (j)  {
    	//% fprintf (fout, "\n\n总共有 %d 位七天未上站的版主。\n", j);
    	fprintf (fout, "\n\n\xd7\xdc\xb9\xb2\xd3\xd0 %d \xce\xbb\xc6\xdf\xcc\xec\xce\xb4\xc9\xcf\xd5\xbe\xb5\xc4\xb0\xe6\xd6\xf7\xa1\xa3\n", j);
      	//% fprintf (fout, "\n[1;31m    请以上版主注意。[m\n");
      	fprintf (fout, "\n[1;31m    \xc7\xeb\xd2\xd4\xc9\xcf\xb0\xe6\xd6\xf7\xd7\xa2\xd2\xe2\xa1\xa3[m\n");
      	//% fprintf (fout, "\n\n超过一个月连续不上线将取消版主资格。\n");
      	fprintf (fout, "\n\n\xb3\xac\xb9\xfd\xd2\xbb\xb8\xf6\xd4\xc2\xc1\xac\xd0\xf8\xb2\xbb\xc9\xcf\xcf\xdf\xbd\xab\xc8\xa1\xcf\xfb\xb0\xe6\xd6\xf7\xd7\xca\xb8\xf1\xa1\xa3\n");
    }  else    {
    	//% fprintf (fout, "\n\n本站目前尚无七天未上站的板主。\n");
    	fprintf (fout, "\n\n\xb1\xbe\xd5\xbe\xc4\xbf\xc7\xb0\xc9\xd0\xce\xde\xc6\xdf\xcc\xec\xce\xb4\xc9\xcf\xd5\xbe\xb5\xc4\xb0\xe5\xd6\xf7\xa1\xa3\n");
    }
  	fclose (fout);
	return 0;
}
