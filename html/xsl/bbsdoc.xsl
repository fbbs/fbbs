<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='bbsdoc'>
		<html>
			<head>
				<title><xsl:value-of select='brd/@desc' /><xsl:call-template name='bbsname' /></title>
				<meta http-equiv='content-type' content='text/html; charset=gb2312' />
				<link rel='stylesheet' type='text/css' href='/css/bbs.css' />
			</head>
			<body><div id='wrap'>
				<xsl:call-template name='navgation-bar'><xsl:with-param name='perm' select='@p' /></xsl:call-template>
				<xsl:if test='brd/@icon'><img><xsl:attribute name='src'><xsl:value-of select='brd/@icon' /></xsl:attribute></img></xsl:if>
				<div id='main'>
					<xsl:choose>
						<xsl:when test='brd/@banner'><img><xsl:attribute name='src'><xsl:value-of select='brd/@banner' /></xsl:attribute></img></xsl:when>
						<xsl:otherwise><h2><a><xsl:attribute name='href'><xsl:value-of select='brd/@link' />doc?bid=<xsl:value-of select='brd/@bid' /></xsl:attribute><xsl:value-of select='brd/@desc' /> [<xsl:value-of select='brd/@title' />]<xsl:if test='brd/@link = "g"'> - 文摘区</xsl:if><xsl:if test='brd/@link = "t"'> - 主题模式</xsl:if></a></h2></xsl:otherwise>
					</xsl:choose>
					<p>版主 [ <xsl:call-template name='splitbm'><xsl:with-param name='names' select='brd/@bm' /><xsl:with-param name='isdir'>0</xsl:with-param><xsl:with-param name='isfirst' select='1' /></xsl:call-template> ]  文章数 [ <xsl:choose><xsl:when test='brd/@total &gt; 0'><xsl:value-of select='brd/@total' /></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose> ]</p>
					<table class='content'>
						<tr><th class='no'>序号</th><th class='mark'>标记</th><th class='owner'>作者</th><th class='time'>发表时间</th><th class='ptitle'>标题</th></tr>
						<xsl:for-each select='po'><tr>
							<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
							<td class='no'><xsl:value-of select='position() - 1 + ../brd/@start' /></td>
							<td class='mark'><xsl:value-of select='@m' /></td>
							<td class='owner'><a class='owner'><xsl:attribute name='href'>qry?u=<xsl:value-of select='@owner' /></xsl:attribute><xsl:value-of select='@owner' /></a></td>
							<td class='time'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@time' /></xsl:call-template></td>
							<xsl:variable name='imgsrc'>/images/types/<xsl:choose><xsl:when test='substring(., 1, 4) = "Re: "'>reply</xsl:when><xsl:otherwise>text</xsl:otherwise></xsl:choose>.gif</xsl:variable>
							<xsl:variable name='text'><xsl:choose><xsl:when test='substring(., 1, 4) = "Re: "'><xsl:value-of select='substring(., 5)' /></xsl:when><xsl:otherwise><xsl:value-of select='.' /></xsl:otherwise></xsl:choose></xsl:variable>
							<td class='ptitle'><a class='ptitle'>
								<xsl:attribute name='href'><xsl:value-of select='../brd/@link' />con?bid=<xsl:value-of select='../brd/@bid' />&amp;f=<xsl:value-of select='@id' /></xsl:attribute>
								<img><xsl:attribute name='src'><xsl:value-of select='$imgsrc' /></xsl:attribute></img>
								<xsl:call-template name='ansi-escape'>
									<xsl:with-param name='content'><xsl:value-of select='$text' /></xsl:with-param>
									<xsl:with-param name='fgcolor'>37</xsl:with-param>
									<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
									<xsl:with-param name='ishl'>0</xsl:with-param>
								</xsl:call-template>
							</a></td>
						</tr></xsl:for-each>
					</table>
					<xsl:apply-templates select='brd' />
				</div>
			</div></body>
		</html>
	</xsl:template>
	
	<xsl:template match='brd'>
		<xsl:if test='@start > 1'>
			<xsl:variable name='prev'><xsl:choose><xsl:when test='@start - @page &lt; 1'>1</xsl:when><xsl:otherwise><xsl:value-of select='@start - @page' /></xsl:otherwise></xsl:choose></xsl:variable>
			<a><xsl:attribute name='href'><xsl:value-of select='@link' />doc?bid=<xsl:value-of select='@bid' />&amp;start=<xsl:value-of select='$prev' /></xsl:attribute>[ <img src='/images/button/up.gif' />上一页 ]</a>
		</xsl:if>
		<xsl:if test='@total > @start + @page - 1'>
			<xsl:variable name='next'><xsl:value-of select='@start + @page' /></xsl:variable>
			<a><xsl:attribute name='href'><xsl:value-of select='@link' />doc?bid=<xsl:value-of select='@bid' />&amp;start=<xsl:value-of select='$next' /></xsl:attribute>[ <img src='/images/button/down.gif' />下一页 ]</a>
		</xsl:if>
		<a><xsl:attribute name='href'>clear?board=<xsl:value-of select='@title' />&amp;start=<xsl:value-of select='@start' /></xsl:attribute>[清除未读]</a>
		<xsl:if test='@link != ""'><a><xsl:attribute name='href'>doc?bid=<xsl:value-of select='@bid' /></xsl:attribute>[<img src='/images/button/home.gif' />一般模式]</a></xsl:if>
		<xsl:if test='@link != "t"'><a><xsl:attribute name='href'>tdoc?bid=<xsl:value-of select='@bid' /></xsl:attribute>[<img src='/images/button/content.gif' />主题模式]</a></xsl:if>
		<xsl:if test='@link != "g"'><a><xsl:attribute name='href'>gdoc?bid=<xsl:value-of select='@bid' /></xsl:attribute>[文摘区]</a></xsl:if>
		<a><xsl:attribute name='href'>not?board=<xsl:value-of select='@title' /></xsl:attribute>[进版画面]</a>
		<a><xsl:attribute name='href'>brdadd?bid=<xsl:value-of select='@bid' /></xsl:attribute>[收藏本版]</a>
	</xsl:template>
</xsl:stylesheet>
