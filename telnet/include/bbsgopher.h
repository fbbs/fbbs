
#ifndef _BBSGOPHER_H
	#define _BBSGOPHER_H
	
	typedef struct _gopher GOPHER;
	struct _gopher {
    	char        file[81],title[71],server[41];
	    int         port;
    	int         position;
	    GOPHER      *next;
	};
#endif
