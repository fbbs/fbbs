#include "bbs.h"

#ifdef SMS
/**
 检查参数字符串mobile是否为13位的手机号码,  是则返回1,否则返回0
 **/
int check_mobile(char *mobile)
{
	int i;

	if (strlen(mobile) != 13)
	return(0);

	for(i = 0; i < 13; i++) {
		if (*(mobile+i) < 0x30)
		return(0);
		if (*(mobile+i)> 0x39)
		return(0);
	}
	return(1);
}

//	显示手机短消息选择菜单
void sms_menu()
{
	char ans[3];
	char user[18], mobile[16], message[142], regcode[12];
	int flag = 0;
	SMS_HANDLE *hsms;

	move(1, 0);
	if (currentuser.bet & 0x80000000) { /*	若贷款数为负值,即没有负债	*/
		// 此处,可能把此位设置成是否注册了手机
		prints("[1] 发送短消息\n");
		prints("[2] 注销手机\n");
		prints("[Q] 退出\n");
		getdata(t_lines - 1, 0, "请选择：", ans, 2, DOECHO, YEA);

		clear();
		refresh();

		switch(ans[0]) {
			case '1':
			move(1, 0);
			pressreturn();
			break;
			case '2':
			pressreturn();
			break;
			default:
			break;
		}
		return;
	} else {
		prints("您尚未注册使用短信服务\n");
		prints("[1] 注册手机\n");
		prints("[Q] 退出\n");
		getdata(t_lines - 1, 0, "请选择：", ans, 2, DOECHO, YEA);

		clear();
		refresh();

		if (ans[0] != '1')
		return;

		memset(mobile,0,14);

		while (1) {
			getdata( t_lines - 1, 0, "请输入您的手机号码[按q退出]：",
					mobile, 14, DOECHO, YEA);
			if ((mobile[0] == 'q')||(mobile[0] == 'Q'))
			return;
			if (check_mobile(mobile))
			break;
		}
		prints("your mobile: %s\n", mobile);
		/*sprintf(user, "%s@bbs", currentuser.userid);
		 hsms = smsmsg_open("sms.funo.net", 5919, user, pass, mobile);
		 if (hsms == NULL) {
		 prints("网络服务器连不上\n");
		 pressreturn();
		 return;
		 }
		 if (smsmsg_reg_request(hsms, mobile, NULL) != 0) {
		 prints("注册请求发送失败\n");
		 pressreturn();
		 return;
		 }*/
		//系统记录
		//错误处理

		memset(regcode,0,12);
		getdata(t_lines - 1, 0, "请输入你收到的注册码[按q退出]：",
				regcode, 12, DOECHO, YEA);
		if ((regcode[0] == 'q') || (regcode[0] == 'Q'))
		return;

		/*if (smsmsg_reg_confirm(hsms, mobile, regcode) != 0) {
		 prints("注册码不正确或者服务器不可用\n");
		 pressreturn();
		 return;
		 }*/
		pressreturn();
		prints("您已成功注册，请重新进入本菜单使用短信服务\n");
		//文件操作
		//用户权限位变更
		//系统记录
		return;
	}
}
#endif
