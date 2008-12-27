/* ÆßÌìÎ´ÉÏÕ¾µÄ°æÖ÷Í³¼Æ by soff of bitbbs.org */

#include <time.h>
#include <stdio.h>
#include "../../include/bbs.h"

int main (int argc, char *argv[])
{
	FILE *fp, *fout, *bmfp;
	time_t now;
	char which[20];
	int n, i, j = 0;
	struct userec aman;
  	char buf[256];

  	now = time (0);
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
  	fprintf (fout, "\n%sÆßÌìÎ´ÉÏÕ¾µÄ°æÖ÷Ãûµ¥\n\n", BBSNAME);
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
		    		  " ** [1;33m%-16s[m°æÖ÷ [1;32m%-15.15s[m [ %-20.20s ]%6dÌì\n",
		       			which, aman.userid, aman.username,(now - aman.lastlogin) / 86400);
	   		}
	 		fclose (bmfp);
		  	j++;
		}
	}
	fclose (fp);
	if (j)  {
    	fprintf (fout, "\n\n×Ü¹²ÓÐ %d Î»ÆßÌìÎ´ÉÏÕ¾µÄ°æÖ÷¡£\n", j);
      	fprintf (fout, "\n[1;31m    ÇëÒÔÉÏ°æÖ÷×¢Òâ¡£[m\n");
      	fprintf (fout, "\n\n³¬¹ýÒ»¸öÔÂÁ¬Ðø²»ÉÏÏß½«È¡Ïû°æÖ÷×Ê¸ñ¡£\n", j);
    }  else    {
    	fprintf (fout, "\n\n±¾Õ¾Ä¿Ç°ÉÐÎÞÆßÌìÎ´ÉÏÕ¾µÄ°åÖ÷¡£\n");
    }
  	fclose (fout);
}
