<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbstcon">
		<html>
			<head>
				<title>同主题文章阅读 - 日月光华</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
			</head>
			<body>
				<a name='top' />
				<xsl:for-each select='post'>
					<div>
						<a><xsl:attribute name='href'>pst?bid=<xsl:value-of select='/bbstcon/bid' />&amp;f=<xsl:value-of select='fid' /></xsl:attribute>[ <img src='/images/button/edit.gif' />回复本文 ]</a>
						<a><xsl:attribute name='href'>qry?u=<xsl:value-of select='author' /></xsl:attribute>[ 本篇作者: <xsl:value-of select='author' /> ]</a>
						<a><xsl:attribute name='href'>con?bid=<xsl:value-of select='/bbstcon/bid' />&amp;f=<xsl:value-of select='fid' /></xsl:attribute>[ <img src='/images/button/content.gif' />单篇阅读 ]</a>
						<a href='#top'>[ <img src='/images/button/up.gif' />回页首 ]</a>
					</div>
					<div class='article'>
						<xsl:call-template name='showpost'>
							<xsl:with-param name='content' select='content' />
						</xsl:call-template>
					</div>
				</xsl:for-each>
				<xsl:variable name='first'><xsl:for-each select='post'><xsl:if test='position() = 1'><xsl:value-of select='fid' /></xsl:if></xsl:for-each></xsl:variable>
				<xsl:variable name='last'><xsl:for-each select='post'><xsl:if test='position() = last()'><xsl:value-of select='fid' /></xsl:if></xsl:for-each></xsl:variable>
				<div>
					<xsl:if test='count(post) = page'><a><xsl:attribute name='href'>tcon?bid=<xsl:value-of select='bid' />&amp;g=<xsl:value-of select='gid' />&amp;f=<xsl:value-of select='$last' />&amp;a=n</xsl:attribute>[<img src='/images/button/down.gif' />下一页 ]</a></xsl:if>
					<xsl:if test='$first != gid'><a><xsl:attribute name='href'>tcon?bid=<xsl:value-of select='bid' />&amp;g=<xsl:value-of select='gid' />&amp;f=<xsl:value-of select='$first' />&amp;a=p</xsl:attribute>[<img src='/images/button/up.gif' />上一页 ]</a></xsl:if>
					<a href='javascript:history.go(-1)'>[ <img src='/images/button/back.gif' />返回 ]</a>
					<a><xsl:attribute name='href'>doc?bid=<xsl:value-of select='bid' /></xsl:attribute>[ <img src='/images/button/home.gif' />本讨论区 ]</a>
				</div>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>