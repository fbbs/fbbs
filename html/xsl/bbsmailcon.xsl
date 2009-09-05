<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>信件阅读</xsl:with-param>
			<xsl:with-param name='p'><xsl:value-of select='/@p' /></xsl:with-param>
			<xsl:with-param name='u'><xsl:value-of select='/@u' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsmailcon'>
		<div class='post'>
			<div class='pleft'>
				<a href='mail'>[ <img src='../images/button/back.gif' />信件列表 ]</a>
				<xsl:if test='@prev'><a><xsl:attribute name='href'>mailcon?f=<xsl:value-of select='@prev' /></xsl:attribute>[ <img src='../images/button/up.gif' />上一封 ]</a></xsl:if>
				<xsl:if test='@next'><a><xsl:attribute name='href'>mailcon?f=<xsl:value-of select='@next' /></xsl:attribute>[ <img src='../images/button/down.gif' />下一封 ]</a></xsl:if>
			</div>
			<div class='pright'>
				<div class='pmtop'><xsl:call-template name='linkbar' /></div>
				<div class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='mail' /></xsl:call-template></div>
				<div class='pmbot'><xsl:call-template name='linkbar' /></div>
			</div>
			<div class='pclear'></div>
		</div>
	</xsl:template>
	
	<xsl:template name='linkbar'>
		<a onclick='return confirm("您真的要删除这封信吗？")'><xsl:attribute name='href'>delmail?f=<xsl:value-of select='mail/@f' /></xsl:attribute>[ 删除此信 ]</a>
	</xsl:template>
</xsl:stylesheet>