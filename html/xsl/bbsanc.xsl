<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>精华区文章阅读</xsl:with-param>
			<xsl:with-param name='session'><xsl:value-of select='bbsanc/@s' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsanc'>
		<h3><xsl:if test='@brd'><xsl:value-of select='@brd' />版 - </xsl:if>精华区文章阅读</h3>
		<div class='post'>
			<div class='ptop'><xsl:call-template name='navbar'/></div>
			<div class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='.' /></xsl:call-template></div>
			<div class='pbot'><xsl:call-template name='navbar'/></div>
		</div>
	</xsl:template>
	
	<xsl:template name='navbar'>
		<xsl:if test='@brd'>
			<a><xsl:attribute name='href'>gdoc?board=<xsl:value-of select='@brd' /></xsl:attribute>[ 文摘区 ]</a>
			<a><xsl:attribute name='href'>doc?board=<xsl:value-of select='@brd' /></xsl:attribute>[<img src='../images/button/home.gif' />本讨论区]</a>
		</xsl:if>
	</xsl:template>
</xsl:stylesheet>
