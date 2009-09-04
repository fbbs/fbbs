<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='root'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>进版画面</xsl:with-param>
			<xsl:with-param name='p'><xsl:value-of select='bbsnot/@p' /></xsl:with-param>
			<xsl:with-param name='u'><xsl:value-of select='bbsnot/@u' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsnot'>
		<h2>进版画面 - [<xsl:value-of select='@brd' />]</h2>
		<div class='post'>
			<div class='pleft'>
				<xsl:if test='@brd'>
					<a><xsl:attribute name='href'>gdoc?board=<xsl:value-of select='@brd' /></xsl:attribute>[ 文摘区 ]</a>
					<a><xsl:attribute name='href'>doc?board=<xsl:value-of select='@brd' /></xsl:attribute>[<img src='../images/button/home.gif' />本讨论区]</a>
				</xsl:if>
			</div>
			<div class='pright'>
				<div class='pmtop'><xsl:call-template name='linkbar' /></div>
				<div class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='.' /></xsl:call-template></div>
				<div class='pmbot'><xsl:call-template name='linkbar' /></div>
			</div>
			<div class='pclear'></div>
		</div>
	</xsl:template>
	
	<xsl:template name='linkbar'>
	</xsl:template>
</xsl:stylesheet>