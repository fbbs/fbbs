<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsnot">
		<html>
			<head>
				<title>进版画面 - 日月光华BBS</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
			</head>
			<body>
				<h3>日月光华 - <xsl:value-of select='board' />版 - 进版画面</h3>
				<div class='article'>
					<xsl:call-template name='showpost'>
						<xsl:with-param name='content' select='content' />
					</xsl:call-template>
				</div>
				<a href='javascript:history.go(-1)'>[<img src='/images/button/back.gif' />快速返回]</a>
				<xsl:if test='board'><a><xsl:attribute name='href'>bbsdoc?board=<xsl:value-of select='board' /></xsl:attribute>[<img src='/images/button/home.gif' />本讨论区]</a></xsl:if>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>