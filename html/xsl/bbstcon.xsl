<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='bbstcon'>
	<html>
		<head>
			<title>同主题文章阅读 - <xsl:call-template name='bbsname' /></title>
			<meta http-equiv='content-type' content='text/html; charset=gb2312' />
			<link rel='stylesheet' type='text/css' href='/css/bbs.css' />
		</head>
		<body><a name='top' /><div id='wrap'>
			<xsl:call-template name='header'><xsl:with-param name='perm' select='@p' /><xsl:with-param name='user' select='@u' /></xsl:call-template>
			<xsl:call-template name='navigation'><xsl:with-param name='perm' select='@p' /></xsl:call-template>
			<div id='main'>
				<xsl:for-each select='po'>
					<div class='post'>
						<div class='pleft'>
							<a href='#top'>[ <img src='/images/button/up.gif' />回页首 ]</a>
							<a><xsl:attribute name='href'>gdoc?bid=<xsl:value-of select='../@bid' /></xsl:attribute>[ 文摘区 ]</a>
							<a><xsl:attribute name='href'>tdoc?bid=<xsl:value-of select='../@bid' /></xsl:attribute>[ <img src='/images/button/home.gif' />本讨论区 ]</a>
							<a><xsl:attribute name='href'>con?bid=<xsl:value-of select='../@bid' />&amp;f=<xsl:value-of select='@fid' /></xsl:attribute>[ <img src='/images/button/content.gif' />本文链接 ]</a>
							<xsl:variable name='first'><xsl:value-of select='../po[1]/@fid' /></xsl:variable>
							<xsl:variable name='last'><xsl:value-of select='../po[last()]/@fid' /></xsl:variable>
							<xsl:if test='count(../po) = ../@page'><a><xsl:attribute name='href'>tcon?bid=<xsl:value-of select='../@bid' />&amp;g=<xsl:value-of select='../@gid' />&amp;f=<xsl:value-of select='$last' />&amp;a=n</xsl:attribute>[<img src='/images/button/down.gif' />下一页 ]</a></xsl:if>
							<xsl:if test='$first != ../@gid'><a><xsl:attribute name='href'>tcon?bid=<xsl:value-of select='../@bid' />&amp;g=<xsl:value-of select='../@gid' />&amp;f=<xsl:value-of select='$first' />&amp;a=p</xsl:attribute>[<img src='/images/button/up.gif' />上一页 ]</a></xsl:if>
						</div>
						<div class='pright'>
							<div class='pmtop'><xsl:call-template name='linkbar' /></div>
							<div class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='.' /></xsl:call-template></div>
							<div class='pmbot'><xsl:call-template name='linkbar' /></div>
						</div>
					</div>
				</xsl:for-each>
			</div>
			<xsl:call-template name='foot' />
		</div></body>
	</html>
	</xsl:template>
	
	<xsl:template name='linkbar'>
		<a><xsl:attribute name='href'>pst?bid=<xsl:value-of select='../@bid' />&amp;f=<xsl:value-of select='@fid' /></xsl:attribute>[ <img src='/images/button/edit.gif' />回复本文 ]</a>
		<a><xsl:attribute name='href'>ccc?bid=<xsl:value-of select='../@bid' />&amp;f=<xsl:value-of select='@fid' /></xsl:attribute>[ 转载 ]</a>
		<a><xsl:attribute name='href'>qry?u=<xsl:value-of select='@owner' /></xsl:attribute>[ 本篇作者: <xsl:value-of select='@owner' /> ]</a>
	</xsl:template>
</xsl:stylesheet>