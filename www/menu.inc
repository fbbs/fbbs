static char *BEGIN_MENU="stm_bm([\"noname\",400,\"\",\"/images/blank.gif\",0,\"\",\"\",0,0,0,0,50,1,0,0]);";
static char *BEGIN_SUBMENU="stm_bp(\"submenu\",[1,4,0,0,0,2,0,7,100,\"\",-2,\"\",-2,90,0,0,\"#999999\",\"#ffffff\",\"\",3,0,0,\"#ffffff\"]);";
static char *BEGIN_POPUPMENU="stm_bp(\"popupmenu\",[1,2,-3,-3,0,3,0,7,100,\"\",-2,\"\",-2,70,2,3,\"#999999\",\"#336699\",\"\",3,1,1,\"#B1C9F1\"]);";
static char *SUBMENU_FIRST="stm_ai(\"submenu\",[1,\"%s%s\",\"\",\"\",-1,-1,0,\"%s\",\"%s\",\"%s\",\"\",\"\",\"\",-1,-1,0,\"%s\",\"%s\",-1,-1,0,0,1,\"#336699\",0,\"#6699CC\",0,\"\",\"\",3,3,1,1,\"#336699 #336699 #336699 #336699\",\"#B1C9F1 #4A7AC9 #4A7AC9 #B1C9F1\",\"#ffffff\",\"#ffffff\",\"bold 9pt 'tahoma','verdana','helvetica'\",\"bold 9pt 'tahoma','verdana','helvetica'\",0,0]);\n";
static char *SUBMENU="stm_aix(\"noname\",\"submenu\",[1,\"%s%s\",\"\",\"\",-1,-1,0,\"%s\",\"%s\",\"%s\",\"\",\"\",\"\",0,0,0,\"%s\",\"%s\",0,0,0,0,1,\"#336699\",0,\"#6699CC\",0,\"\",\"\",3,3,1,1,\"#336699\"]);";
static char *END_SUBMENU="stm_ep();";
static char *END_MENU="stm_em();";
#define END_POPUPMENU END_SUBMENU

static char *FRAME_VIEW="view";

