<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='bbscon'>
	<html>
		<head>
			<title>文章阅读<xsl:call-template name='bbsname' /></title>
			<meta http-equiv='content-type' content='text/html; charset=gb2312' />
			<link rel='stylesheet' type='text/css' href='/css/bbs.css' />
		</head>
		<body><div class='main'>
			<xsl:call-template name='navgation-bar'><xsl:with-param name='perm' select='@p' /></xsl:call-template>
			<div class='post'>
				<div class='pleft'>
					<xsl:if test='@link != "con"'><a><xsl:attribute name='href'>gdoc?bid=<xsl:value-of select='@bid' /></xsl:attribute>[ 文摘区 ]</a></xsl:if>
					<a><xsl:attribute name='href'>doc?bid=<xsl:value-of select='@bid' /></xsl:attribute>[ <img src='/images/button/home.gif' />本讨论区 ]</a>
					<a><xsl:attribute name='href'>con?bid=<xsl:value-of select='@bid' />&amp;f=<xsl:value-of select='po/@fid' /></xsl:attribute>[ 本文链接 ]</a>
					<xsl:variable name='baseurl'>con?bid=<xsl:value-of select='@bid' />&amp;f=<xsl:value-of select='po/@fid' />&amp;a=</xsl:variable>
					<a><xsl:attribute name='href'><xsl:value-of select='$baseurl' />p</xsl:attribute>[ <img src='/images/button/up.gif' />上一篇 ]</a>
					<a><xsl:attribute name='href'><xsl:value-of select='$baseurl' />n</xsl:attribute>[ <img src='/images/button/down.gif' />下一篇 ]</a>
					<xsl:if test='po/@reid != f'><a><xsl:attribute name='href'><xsl:value-of select='$baseurl' />b</xsl:attribute>[ 同主题上篇 ]</a></xsl:if>
					<a><xsl:attribute name='href'><xsl:value-of select='$baseurl' />a</xsl:attribute>[ 同主题下篇 ]</a>
					<xsl:if test='po/@gid'><a><xsl:attribute name='href'>con?bid=<xsl:value-of select='@bid' />&amp;f=<xsl:value-of select='po/@gid' /></xsl:attribute>[ 同主题第一篇 ]</a></xsl:if>
				</div>
				<div class='pright'>
					<div class='pmtop'><xsl:call-template name='linkbar' /></div>
					<div class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='po' /></xsl:call-template></div>
					<div class='pmbot'><xsl:call-template name='linkbar' /></div>
				</div>
			</div>
		</div></body>
	</html>
	</xsl:template>
	
	<xsl:template name='linkbar'>
		<xsl:if test='@link = "con"'><a><xsl:attribute name='href'>pst?bid=<xsl:value-of select='@bid' />&amp;f=<xsl:value-of select='po/@fid' /></xsl:attribute>[ <img src='/images/button/edit.gif' />回复本文 ]</a></xsl:if>
		<a><xsl:attribute name='href'>ccc?bid=<xsl:value-of select='@bid' />&amp;f=<xsl:value-of select='po/@fid' /></xsl:attribute>[ 转载 ]</a>
	</xsl:template>
</xsl:stylesheet>