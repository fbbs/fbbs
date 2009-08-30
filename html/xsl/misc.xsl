<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns='http://www.w3.org/1999/xhtml'>
	<xsl:template name='timeconvert'>
		<xsl:param name='time' />
		<xsl:value-of select='concat(substring($time, 6, 5), " ", substring($time, 12, 5))' />
	</xsl:template>

	<xsl:template name="splitbm">
		<xsl:param name='names' />
		<xsl:param name='isdir' />
		<xsl:param name='isfirst' />
        <xsl:variable name='first' select='substring-before($names," ")' />
        <xsl:variable name='rest' select='substring-after($names," ")' />
        <xsl:if test='$first'>
			<a><xsl:attribute name='href'>qry?u=<xsl:value-of select='$first' /></xsl:attribute><xsl:value-of select='$first' /></a>
		</xsl:if>
		<xsl:if test='$rest'>
			<span>&#160;</span>
			<xsl:call-template name='splitbm'>
				<xsl:with-param name='names' select='$rest'/>
				<xsl:with-param name='isdir' select='$isdir'/>
				<xsl:with-param name='isfirst' select='0'/>
			</xsl:call-template>
        </xsl:if>
		<xsl:if test='not($rest)'>
			<xsl:if test='$names'>
				<a><xsl:attribute name='href'>qry?u=<xsl:value-of select='$names' /></xsl:attribute><xsl:value-of select='$names' /></a>
			</xsl:if>
			<xsl:if test="$names=''">
				<xsl:if test="$isdir='0'">诚征版主中</xsl:if>
				<xsl:if test="$isdir!='0'">-</xsl:if>
			</xsl:if>
		</xsl:if>
	</xsl:template>
	
	<xsl:template name="showexp">
		<xsl:param name="count" />
		<xsl:if test="$count > 0">
			<img>
				<xsl:attribute name='src'>/images/level/<xsl:value-of select="level" />.gif</xsl:attribute>
			</img>
			<xsl:call-template name="showexp">
				<xsl:with-param name="count" select="$count - 1"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

		
	<xsl:template name='navgation-bar'>
		<xsl:param name='perm' />
		<div>
			<ul class='nav'>
				<li id='navh'><a href='sec'>首页</a></li>
				<xsl:if test='not(contains($perm, "l"))'><li id='navl'><a href='login'>登录</a></li></xsl:if>
				<xsl:if test='contains($perm, "l")'>
					<li id='navf'><a href='fav'>收藏夹</a></li>
					<li id='navc'>
						<a href='#'>鹊桥</a>
						<ul>
							<li><a href='friend'>在线好友</a></li>
							<li><a href='usr'>环顾四方</a></li>
							<li><a href='sendmsg'>发送讯息</a></li>
							<li><a href='msg'>查看所有讯息</a></li>
						</ul>
					</li>
					<li id='navm'>
						<a href='#'>信件</a>
						<ul>
							<li><a href='newmail'>阅览新信件</a></li>
							<li><a href='mail'>所有信件</a></li>
							<li><a href='pstmail'>发送信件</a></li>
							<li></li>
						</ul>
					</li>
					<li id='navco'>
						<a href='#'>设置</a>
						<ul>
							<li><a href='info'>个人资料</a></li>
							<li><a href='plan'>改说明档</a></li>
							<li><a href='sig'>改签名档</a></li>
							<li><a href='mywww'>Web定制</a></li>
							<li><a href='pwd'>修改密码</a></li>
							<li><a href='nick'>临时改昵称</a></li>
							<li><a href='fall'>设定好友</a></li>
							<li><a href='cloak'>切换隐身</a></li>
						</ul>
					</li>
					<li id='nave'><a href='logout'>退出</a></li>
				</xsl:if>
				<li id='nava'><a href='0an'>本站精华</a></li>
				<li id='navp'><a href='all'>全部讨论</a></li>
				<li id='navs'>
					<a href='#'>查找</a>
					<ul>
						<li><a href='qry'>查找文章</a></li>
						<li><a href='qry'>查询网友</a></li>
						<li><a href='sel'>查找讨论区</a></li>
					</ul>
				</li>
				<li id='navt'>
					<a href='#'>统计</a>
					<ul>
						<li><a href='top10'>本日十大</a></li>
						<li><a href='topb10'>热门讨论</a></li>
						<li><a href='userinfo'>在线统计</a></li>
					</ul>
				</li>
				<li id='navte'><a href='telnet://bbs.fudan.sh.cn:23'>终端登录</a></li>
			</ul>
		</div>
	</xsl:template>
	
	<xsl:template name='bbsname'>&#160;- 日月光华</xsl:template>
</xsl:stylesheet>
