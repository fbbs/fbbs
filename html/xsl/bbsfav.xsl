<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='bbsfav'>
	<html>
		<head>
			<title>收藏夹 - <xsl:call-template name='bbsname' /></title>
			<meta http-equiv='content-type' content='text/html; charset=gb2312' />
			<xsl:call-template name='include-css' />
			<xsl:call-template name='include-js' />
		</head>
		<body><div id='wrap'>
			<xsl:call-template name='header'><xsl:with-param name='perm' select='@p' /><xsl:with-param name='user' select='@u' /></xsl:call-template>
			<xsl:call-template name='navigation'><xsl:with-param name='perm' select='@p' /></xsl:call-template>
			<div id='main'>
				<h2>我的收藏夹</h2>
				<table class='content'>
					<tr><th class='no'>序号</th><th class='title'>讨论区名称</th><th class='desc'>中文描述</th></tr>
					<xsl:for-each select='brd'><tr>
						<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
						<td class='no'><xsl:value-of select='position()' /></td>
						<td class='title'><a class='title'><xsl:attribute name='href'>doc?bid=<xsl:value-of select='@bid' /></xsl:attribute><xsl:value-of select='@brd' /></a></td>
						<td class='desc'><a class='desc'><xsl:attribute name='href'>doc?bid=<xsl:value-of select='@bid' /></xsl:attribute><xsl:value-of select='.' /></a></td>
					</tr></xsl:for-each>
				</table>
			</div>
			<xsl:call-template name='foot' />
		</div></body>
	</html>
	</xsl:template>
</xsl:stylesheet>
