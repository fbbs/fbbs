<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsgetmsg">
		<html>
			<head>
				<title>查看讯息 - 日月光华BBS</title>
				<meta http-equiv='content-type' content='text/html; charset=gb2312' />
				<meta http-equiv='pragma' content='no-cache' />
				<meta http-equiv='Refresh' content='60; url=bbsgetmsg' />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
			</head>
			<body>
				<xsl:if test='msg'>
					<strong><xsl:value-of select='id' /></strong>
					<span>: <xsl:call-template name='showpost'><xsl:with-param name='content' select='msg' /></xsl:call-template></span>
					<a target='view'><xsl:attribute name='href'>bbssendmsg?id=<xsl:value-of select='id' />&amp;pid=<xsl:value-of select='pid' /></xsl:attribute>[回复]</a>
					<a href='bbsgetmsg'>[忽略]</a>
				</xsl:if>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>