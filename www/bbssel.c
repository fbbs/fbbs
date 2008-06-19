#include "BBSLIB.inc"

int main() {
	char *board, buf[80], *board1, *title;
	int i, total=0;
	init_all(); 
	board=nohtml(getparm("board"));
	if(board[0]==0) {
		printf("<b>%s -- 选择讨论区</b>\n", BBSNAME);
		printpretable_lite();
		printf("<form action=bbssel>\n");
		printf("讨论区名称: <input type=text name=board>");
		printf(" <input type=submit value=确定>");
		printf("</form>\n");
		printposttable_lite();
		http_quit();
	} else {
		for(i=0; i<MAXBOARD; i++) {
			board1=bcache[i].filename;
			if(!has_read_perm(&currentuser, board1)) continue;
			if(!strcasecmp(board, board1)) {
				if (bcache[i].flag & BOARD_DIR_FLAG) {
					sprintf(buf, "bbsboa?board=%s", board1);
				} else {
					sprintf(buf, "bbsdoc?board=%s", board1);
				}
				//sprintf(buf, "bbsdoc?board=%s", board1);
				redirect(buf);
				http_quit();
			}
		}
		printf("<b>%s -- 选择讨论区</b>\n", BBSNAME);
		printf("找不到这个讨论区, ", board);
		printf("标题中含有'%s'的讨论区有: <br><br>\n", board);
		printpretable();
		printf("<table>");
		for(i=0; i<MAXBOARD; i++) {
			board1=bcache[i].filename;
			title=bcache[i].title;
			if(!has_read_perm(&currentuser, board1)) continue;
			if(strcasestr(board1, board) || strcasestr(title, board)) {
				total++;
				printf("<tr><td>%d", total);
				if (bcache[i].flag & BOARD_DIR_FLAG) {
					printf("<td><a href=bbsboa?board=%s>{ %s }</a><td>%s<br>\n",
					       board1, board1, title+7);
				} else {
					printf("<td><a href=bbsdoc?board=%s>%s</a><td>%s<br>\n",
					       board1, board1, title+7);
				}
				//printf("<td><a href=bbsdoc?board=%s>%s</a><td>%s<br>\n",
				//	board1, board1, title+7);
			}
		}
		printf("</table>\n");
		printposttable();
		printf("<br>共找到%d个符合条件的讨论区.\n", total);

		http_quit();

	}
}
