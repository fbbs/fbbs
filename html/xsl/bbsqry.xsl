<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='misc.xsl'/>
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsqry">
		<html>
			<head>
				<title>查询网友 - 日月光华BBS</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
			</head>
			<body>
				<span><xsl:value-of select='id' /> (<xsl:value-of select='nick' />) 共上站 </span><span class='ansi132'><xsl:value-of select='login' /></span><span> 次  </span>
				<xsl:call-template name='show-horo'></xsl:call-template><br />
				<span>上次在:[</span><span class='ansi132'><xsl:value-of select='lastlogin' /></span>] 从 [<span class='ansi132'><xsl:value-of select='ip' /></span>] 到本站一游。<br />
				<span>表现值:[</span><span class='ansi133'>匆匆过客</span>]<br />
				<span>文章数:[</span><span class='ansi132'><xsl:value-of select='post' /></span>] 经验值:[<xsl:call-template name="showexp"><xsl:with-param name="count" select="repeat" /></xsl:call-template>] 生命力:[<span class='ansi132'><xsl:value-of select='hp' /></span>]<br />
			</body>
		</html>
	</xsl:template>
	
	<xsl:template name='show-horo'>
		<xsl:if test='horo'>
			<xsl:variable name='color'>
				<xsl:choose>
					<xsl:when test='gender = "M"'>ansi136</xsl:when>
					<xsl:when test='gender = "F"'>ansi135</xsl:when>
					<xsl:otherwise>ansi132</xsl:otherwise>			
				</xsl:choose>
			</xsl:variable>
			<span>[</span>
			<span>
				<xsl:attribute name='class'><xsl:value-of select='$color' /></xsl:attribute><xsl:value-of select='horo' />
			</span>
			<span>]</span>
		</xsl:if>
	</xsl:template>
</xsl:stylesheet>