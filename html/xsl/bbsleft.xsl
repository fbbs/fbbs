<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:template match="/">
		<html>
			<head>
				<title>Main Menu</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbsleft.css" />
				<script type="text/javascript" src="/js/bbsleft.js"></script>
			</head>
		</html>
		<body>
			<div style="height:100%">
				<a href="#" onclick="switch_bar()" id="switchbar"></a>
				<div id="mainbar">
					<!-- 本站精华 -->
					<a href="/fcgi/bbs0an"><img src="/images/announce.gif" />本站精华</a>
					<!-- 全部讨论 -->
					<a href="/fcgi/bbsall"><img src="/images/penguin.gif" />全部讨论</a>
					<!-- 统计数据 -->
					<a href="#" onclick="return SwitchPanel('Stat')"><img src="/images/top10.gif" />统计数据</a>
					<div id="Stat">
						<a href="/fcgi/bbstop10"><img src="/images/blankblock.gif" />本日十大</a>
						<a href="/fcgi/bbstopb10"><img src="/images/blankblock.gif" />热门讨论</a>
						<a href="/fcgi/bbsuserinfo"><img src="/images/blankblock.gif" />在线统计</a>
					</div>
					<!-- 我的收藏 -->
					<xsl:if test="/bbsleft/favbrd">
						<a href="#" onclick="return SwitchPanel('Favorite')"><img src="/images/favorite.gif" />我的收藏</a>
						<div id="Favorite">
							<a href="/fcgi/bbsmybrd"><img src="/images/blankblock.gif" />预定管理</a>
							<xsl:apply-templates select="bbsleft/favbrd" />
						</div>
					</xsl:if>
					<!-- 鹊桥相会 -->
					<a href="#" onclick="return SwitchPanel('QueQiao')"><img src="/images/chat.gif" />鹊桥相会</a>
					<div id="QueQiao">
						<xsl:if test="/bbsleft/login='1'">
							<a href="/fcgi/bbsfriend"><img src="/images/blankblock.gif" />在线好友</a>
						</xsl:if>
						<a href="/fcgi/bbsfriend"><img src="/images/blankblock.gif" />环顾四方</a>
						<xsl:if test="/bbsleft/talk='1'">
							<a href="/fcgi/bbssendmsg"><img src="/images/blankblock.gif" />发送讯息</a>
							<a href="/fcgi/bbsmsg"><img src="/images/blankblock.gif" />查看所有讯息</a>
						</xsl:if>
					</div>
					<xsl:if test="/bbsleft/login='1'">
						<!-- 个人设置 -->
						<a href="#" onclick="return SwitchPanel('Config')"><img src="/images/config.gif" />个人设置</a>
						<div id="Config">
							<a href="/fcgi/bbsinfo"><img src="/images/blankblock.gif" />个人资料</a>
							<a href="/fcgi/bbsplan"><img src="/images/blankblock.gif" />改说明档</a>
							<a href="/fcgi/bbssig"><img src="/images/blankblock.gif" />改签名档</a>
							<a href="/fcgi/bbsmywww"><img src="/images/blankblock.gif" />WWW定制</a>
							<a href="/fcgi/bbspwd"><img src="/images/blankblock.gif" />修改密码</a>
							<a href="/fcgi/bbsnick"><img src="/images/blankblock.gif" />临时改昵称</a>
							<a href="/fcgi/bbsfall"><img src="/images/blankblock.gif" />设定好友</a>
							<xsl:if test="/bbsleft/cloak='1'">
								<a href="/fcgi/bbscloak"><img src="/images/blankblock.gif" />切换隐身</a>
							</xsl:if>
						</div>
						<!-- 处理信件 -->
						<a href="#" onclick="return SwitchPanel('Mail')"><img src="/images/mail.gif" />个人设置</a>
						<div id="Mail">
							<a href="/fcgi/bbsnewmail"><img src="/images/blankblock.gif" />阅览新信件</a>
							<a href="/fcgi/bbsmail"><img src="/images/blankblock.gif" />所有信件</a>
							<a href="/fcgi/bbsmaildown"><img src="/images/blankblock.gif" />下载信件</a>
							<a href="/fcgi/bbspstmail"><img src="/images/blankblock.gif" />发送信件</a>
						</div>
					</xsl:if>
					<!-- 查找选项 -->
					<a href="#" onclick="return SwitchPanel('Search')"><img src="/images/search.gif" />查找选项</a>
					<div id="Search">
						<xsl:if test="/bbsleft/find='1'">
							<a href="/fcgi/bbsfind"><img src="/images/blankblock.gif" />查找文章</a>
						</xsl:if>
						<xsl:if test="/bbsleft/login='1'">
							<a href="/fcgi/bbsqry"><img src="/images/blankblock.gif" />查询网友</a>
						</xsl:if>
						<a href="/fcgi/bbssel"><img src="/images/blankblock.gif" />查找讨论区</a>
					</div>
					<!-- 终端登录 -->
					<a href="telnet://bbs.fudan.sh.cn:23"><img src="/images/telnet.gif" />终端登录</a>
					<!-- 注销登录 -->
					<xsl:if test="/bbsleft/login='1'">
							<a href="/fcgi/bbslogout"><img src="/images/exit.gif" />注销登录</a>
					</xsl:if>
				</div>
			</div>
		</body>
	</xsl:template>

	<xsl:template match="bbsleft/favbrd">
		<xsl:for-each select="./*">
			<xsl:if test="name()='dir'">
				<a><xsl:attribute name="href">/fcgi/bbsboa?board=<xsl:value-of select="." /></xsl:attribute><xsl:value-of select="." /></a>
			</xsl:if>
			<xsl:if test="name()='board'">
				<a><xsl:attribute name="href">/fcgi/<xsl:value-of select="/bbsleft/favurl" />?board=<xsl:value-of select="." /></xsl:attribute><xsl:value-of select="." /></a>
			</xsl:if>
		</xsl:for-each>
	</xsl:template>
</xsl:stylesheet>