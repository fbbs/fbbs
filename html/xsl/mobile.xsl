<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
<xsl:import href='showpost.xsl'/>
<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd'/>

<xsl:template name='bbsname'>日月光华</xsl:template>

<xsl:template name='timeconvert'>
	<xsl:param name='time'/>
	<xsl:value-of select='concat(substring($time, 1, 10), " ", substring($time, 12, 5))'/>
</xsl:template>

<xsl:template name='time-conv-short'>
	<xsl:param name='time'/>
	<xsl:value-of select='concat(substring($time, 6, 5), " ", substring($time, 12, 5))'/>
</xsl:template>

<xsl:template name='show-fav'>
	<xsl:param name='fav'/>
	<xsl:variable name='brd' select='substring-before($fav, " ")'/>
	<xsl:if test='$brd!=""'><a><xsl:attribute name='href'>tdoc?board=<xsl:value-of select='$brd'/></xsl:attribute><xsl:value-of select='$brd'/></a></xsl:if>
	<xsl:variable name='rest' select='substring-after($fav, " ")'/>
	<xsl:if test='$rest!=""'>|<xsl:call-template name='show-fav'><xsl:with-param name='fav' select='$rest'/></xsl:call-template></xsl:if>
</xsl:template>

<xsl:template name='navigation'>
	<xsl:param name='perm'/><xsl:param name='user'/><xsl:param name='fav'/>
	<xsl:variable name='bbsname'><xsl:call-template name='bbsname'/></xsl:variable>
	<div id='hd'>
		<xsl:if test='$user != ""'><a><xsl:attribute name='href'>qry?u=<xsl:value-of select='$user'/></xsl:attribute><xsl:value-of select='$user'/></a>|</xsl:if>
		<xsl:if test='$user = ""'><a href='login'>登录</a>|</xsl:if>
		<a href='0an'>精华</a>|<a href='top10'>十大</a><xsl:if test='contains($perm, "l")'>|<a href='mail'>信件</a>|<a href='logout'>注销</a></xsl:if>
	</div>
	<div id='fav'>收藏 <xsl:call-template name='show-fav'><xsl:with-param name='fav' select='$fav'/></xsl:call-template></div>
</xsl:template>

<xsl:template name='foot'>
	<div id='ft'><xsl:call-template name='bbsname'/> &#169;1996-2010</div>
</xsl:template>

<xsl:template name='include-css'>
	<link rel='stylesheet' type='text/css' href='../css/mobile.css'/>
</xsl:template>

<xsl:template name='include-js'>
</xsl:template>

<xsl:template name='page-title'>
	<xsl:variable name='cgi' select='local-name(node()[2])'/>
	<xsl:choose>
		<xsl:when test='bbssec'>推荐版面</xsl:when>
		<xsl:when test='bbsboa'><xsl:choose><xsl:when test='bbsboa/@dir'>版面目录</xsl:when><xsl:otherwise>分类讨论区</xsl:otherwise></xsl:choose></xsl:when>
		<xsl:when test='bbsall'>全部讨论区</xsl:when>
		<xsl:when test='bbssel'>选择讨论区</xsl:when>
		<xsl:when test='bbsdoc'><xsl:value-of select='bbsdoc/brd/@desc'/></xsl:when>
		<xsl:when test='bbscon'>文章阅读</xsl:when>
		<xsl:when test='bbstcon'>同主题文章阅读</xsl:when>
		<xsl:when test='bbsqry'>查询网友</xsl:when>
		<xsl:when test='bbspst'><xsl:choose><xsl:when test='bbspst/@edit="0"'>发表</xsl:when><xsl:otherwise>修改</xsl:otherwise></xsl:choose>文章</xsl:when>
		<xsl:when test='bbstop10'>本日十大</xsl:when>
		<xsl:when test='bbsbfind'>版内文章查询</xsl:when>
		<xsl:when test='bbsnewmail'>阅览新信件</xsl:when>
		<xsl:when test='bbsmail'>信件列表</xsl:when>
		<xsl:when test='bbsmailcon'>信件阅读</xsl:when>
		<xsl:when test='bbspstmail'>寄语信鸽</xsl:when>
		<xsl:when test='bbs0an'>精华区浏览</xsl:when>
		<xsl:when test='bbsanc'>精华区文章阅读</xsl:when>
		<xsl:when test='bbsfwd'>转寄文章</xsl:when>
		<xsl:when test='bbsccc'>转载文章</xsl:when>
		<xsl:when test='bbsfall'>设定关注名单</xsl:when>
		<xsl:when test='bbsfadd'>增加关注网友</xsl:when>
		<xsl:when test='bbsovr'>在线关注网友</xsl:when>
		<xsl:when test='bbsfav'>收藏夹</xsl:when>
		<xsl:when test='bbsmybrd'>设定收藏夹</xsl:when>
		<xsl:when test='bbseufile'><xsl:value-of select='bbseufile/@desc'/></xsl:when>
		<xsl:when test='bbsmybrd'>个人资料</xsl:when>
		<xsl:when test='bbspwd'>修改密码</xsl:when>
		<xsl:when test='bbsnot'>进版画面</xsl:when>
		<xsl:otherwise></xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match='/'>
	<xsl:variable name='session' select='node()[2]/@s'/>
	<xsl:variable name='p' select='substring-before($session,";")'/>
	<xsl:variable name='rest' select='substring-after($session,";")'/>
	<xsl:variable name='u' select='substring-before($rest,";")'/>
	<xsl:variable name='fav' select='substring-after($rest,";")'/>
	<html>
		<head>
			<title><xsl:call-template name='page-title'/> - <xsl:call-template name='bbsname'/></title>
			<meta http-equiv="content-type" content="text/html; charset=gb2312"/>
			<xsl:call-template name='include-css'/>
			<xsl:call-template name='include-js'/>
		</head>
		<body>
			<a name='top'/>
			<xsl:call-template name='navigation'><xsl:with-param name='perm' select='$p'/><xsl:with-param name='user' select='$u'/><xsl:with-param name='fav' select='$fav'/></xsl:call-template>
			<div id='main'><xsl:apply-templates/></div>
			<xsl:call-template name='foot'/>
		</body>
	</html>
</xsl:template>

<xsl:template match='bbssec'>
	<xsl:for-each select='sec'>
		<ul class='sec'>
			<li><a><xsl:attribute name='href'>boa?s=<xsl:value-of select='@id'/></xsl:attribute><xsl:value-of select='@id'/>&#160;<xsl:value-of select='@desc'/></a></li>
			<ul class='brd'>
				<xsl:for-each select='brd'>
					<li><a><xsl:attribute name='href'>doc?board=<xsl:value-of select='@name'/></xsl:attribute><xsl:value-of select='@desc'/></a></li>
				</xsl:for-each>
			</ul>
		</ul>
	</xsl:for-each>
</xsl:template>

<xsl:template match='bbsdoc'>
	<h2><a><xsl:attribute name='href'><xsl:value-of select='brd/@link'/>doc?bid=<xsl:value-of select='brd/@bid'/></xsl:attribute><xsl:value-of select='brd/@desc'/>[<xsl:value-of select='brd/@title'/>]<xsl:if test='brd/@link = "g"'>[文摘]</xsl:if><xsl:if test='brd/@link = "t"'>[主题]</xsl:if></a></h2>
	<div class='nav'>
		<a><xsl:attribute name='href'>pst?bid=<xsl:value-of select='brd/@bid'/></xsl:attribute>[发文]</a>
		<a href='javascript:location=location'>[刷新]</a>
		<xsl:if test='brd/@start > 1'>
			<xsl:variable name='prev'><xsl:choose><xsl:when test='brd/@start - brd/@page &lt; 1'>1</xsl:when><xsl:otherwise><xsl:value-of select='brd/@start - brd/@page'/></xsl:otherwise></xsl:choose></xsl:variable>
			<a><xsl:attribute name='href'><xsl:value-of select='brd/@link'/>doc?bid=<xsl:value-of select='brd/@bid'/>&amp;start=<xsl:value-of select='$prev'/></xsl:attribute>[上页]</a>
		</xsl:if>
		<xsl:if test='brd/@total > brd/@start + brd/@page - 1'>
			<xsl:variable name='next'><xsl:value-of select='brd/@start + brd/@page'/></xsl:variable>
			<a><xsl:attribute name='href'><xsl:value-of select='brd/@link'/>doc?bid=<xsl:value-of select='brd/@bid'/>&amp;start=<xsl:value-of select='$next'/></xsl:attribute>[下页]</a>
		</xsl:if>
		<a><xsl:attribute name='href'>clear?board=<xsl:value-of select='brd/@title'/>&amp;start=<xsl:value-of select='brd/@start'/></xsl:attribute>[清除未读]</a>
		<a><xsl:attribute name='href'>0an?bid=<xsl:value-of select='brd/@bid'/></xsl:attribute>[精华]</a>
	</div>
	<div>
		<xsl:for-each select='po'><xsl:sort select='@time' order='descending'/><div class='po'>
			<xsl:if test='@sticky'>[置底]</xsl:if>
			<a>
				<xsl:attribute name='href'><xsl:value-of select='../brd/@link'/>con?bid=<xsl:value-of select='../brd/@bid'/>&amp;f=<xsl:value-of select='@id'/><xsl:if test='@sticky'>&amp;s=1</xsl:if></xsl:attribute>
				<xsl:variable name='text'><xsl:choose><xsl:when test='substring(., 1, 4) = "Re: "'><xsl:value-of select='substring(., 5)'/></xsl:when><xsl:otherwise><xsl:value-of select='.'/></xsl:otherwise></xsl:choose></xsl:variable>
				<xsl:if test='substring(., 1, 4) = "Re: "'>Re: </xsl:if>
				<xsl:call-template name='ansi-escape'>
					<xsl:with-param name='content'><xsl:value-of select='$text'/></xsl:with-param>
					<xsl:with-param name='fgcolor'>37</xsl:with-param>
					<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
					<xsl:with-param name='ishl'>0</xsl:with-param>
				</xsl:call-template>
			</a> [<a class='owner'><xsl:attribute name='href'>qry?u=<xsl:value-of select='@owner'/></xsl:attribute><xsl:value-of select='@owner'/></a><xsl:text> </xsl:text><span class='time'><xsl:call-template name='time-conv-short'><xsl:with-param name='time' select='@time'/></xsl:call-template></span>]
		</div></xsl:for-each>
	</div>
</xsl:template>

<xsl:template match='brd'>

</xsl:template>

<xsl:template match='bbstcon'>
	<xsl:for-each select='po'>
		<div class='post'>
			<div class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='.'/></xsl:call-template></div>
			<div class='nav'>
				<xsl:variable name='first'><xsl:value-of select='../po[1]/@fid'/></xsl:variable>
				<xsl:variable name='last'><xsl:value-of select='../po[last()]/@fid'/></xsl:variable>
				<xsl:if test='count(../po) = ../@page'><a><xsl:attribute name='href'>tcon?bid=<xsl:value-of select='../@bid'/>&amp;g=<xsl:value-of select='../@gid'/>&amp;f=<xsl:value-of select='$last'/>&amp;a=n</xsl:attribute>[<img src='../images/button/down.gif'/>下一页]</a></xsl:if>
				<xsl:if test='$first != ../@gid'><a><xsl:attribute name='href'>tcon?bid=<xsl:value-of select='../@bid'/>&amp;g=<xsl:value-of select='../@gid'/>&amp;f=<xsl:value-of select='$first'/>&amp;a=p</xsl:attribute>[<img src='../images/button/up.gif'/>上一页]</a></xsl:if>
				<a><xsl:attribute name='href'>pst?bid=<xsl:value-of select='../@bid'/>&amp;f=<xsl:value-of select='@fid'/></xsl:attribute>[回复]</a>
				<a><xsl:attribute name='href'>ccc?bid=<xsl:value-of select='../@bid'/>&amp;f=<xsl:value-of select='@fid'/></xsl:attribute>[转载]</a>
				<a><xsl:attribute name='href'>tdoc?bid=<xsl:value-of select='../@bid'/></xsl:attribute>[版面]</a>
				<a><xsl:attribute name='href'>con?bid=<xsl:value-of select='../@bid'/>&amp;f=<xsl:value-of select='@fid'/></xsl:attribute>[链接]</a>
				<a><xsl:attribute name='href'>gdoc?bid=<xsl:value-of select='../@bid'/></xsl:attribute>[文摘]</a>
				<a><xsl:attribute name='href'>qry?u=<xsl:value-of select='@owner'/></xsl:attribute>[作者: <xsl:value-of select='@owner'/>]</a>
			</div>
		</div>
	</xsl:for-each>
</xsl:template>

</xsl:stylesheet>