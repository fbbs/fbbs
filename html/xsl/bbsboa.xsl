<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='bbsboa'>
	<html>
		<head>
			<title><xsl:choose><xsl:when test='@dir'>版面目录</xsl:when><xsl:otherwise>分类讨论区</xsl:otherwise></xsl:choose> - <xsl:call-template name='bbsname' /></title>
			<meta http-equiv='content-type' content='text/html; charset=gb2312' />
			<link rel='stylesheet' type='text/css' href='/css/bbs.css' />
		</head>
		<body><div id='wrap'>
			<xsl:call-template name='header'><xsl:with-param name='perm' select='@p' /><xsl:with-param name='user' select='@u' /></xsl:call-template>
			<xsl:call-template name='navigation'><xsl:with-param name='perm' select='@p' /></xsl:call-template>
			<div id='main'>
				<h2><xsl:if test='@icon'><img><xsl:attribute name='src'><xsl:value-of select='icon' /></xsl:attribute></img></xsl:if><xsl:value-of select='@title' /></h2>
				<table class='content'>
					<tr><th class='no'>序号</th><th class='read'>未读</th><th class='no'>文章数</th><th class='title'>讨论区名称</th><th class='cate'>类别</th><th class='desc'>中文描述</th><th class='bm'>版主</th></tr>
					<xsl:for-each select='brd'><xsl:sort select='@title' /><tr>
						<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
						<td class='no'><xsl:value-of select='position()' /></td>
						<td class='read'><xsl:choose><xsl:when test='@read="0"'>◇</xsl:when><xsl:otherwise>◆</xsl:otherwise></xsl:choose></td>
						<td class='no'><xsl:value-of select='@count' /></td>
						<td class='title'><a class='title'><xsl:choose>
							<xsl:when test='@dir="1"'><xsl:attribute name='href'>boa?board=<xsl:value-of select='@title' /></xsl:attribute>[ <xsl:value-of select='@title' /> ]</xsl:when>
							<xsl:otherwise><xsl:attribute name='href'>doc?board=<xsl:value-of select='@title' /></xsl:attribute><xsl:value-of select='@title' /></xsl:otherwise>
						</xsl:choose></a></td>
						<td class='cate'><xsl:choose><xsl:when test='@dir="1"'>[目录]</xsl:when><xsl:otherwise><xsl:value-of select='@cate' /></xsl:otherwise></xsl:choose></td>
						<td class='desc'><a class='desc'><xsl:attribute name='href'><xsl:choose><xsl:when test='@dir="1"'>boa</xsl:when><xsl:otherwise>doc</xsl:otherwise></xsl:choose>?board=<xsl:value-of select='@title' /></xsl:attribute><xsl:value-of select='@desc' /></a></td>
						<td class='bm'><xsl:call-template name='splitbm'><xsl:with-param name='names' select='@bm' /><xsl:with-param name='isdir' select='@dir' /><xsl:with-param name='isfirst' select='1' /></xsl:call-template></td>
					</tr></xsl:for-each>
				</table>
			</div>
			<xsl:call-template name='foot' />
		</div></body>
	</html>
	</xsl:template>
</xsl:stylesheet>
