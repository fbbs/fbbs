<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>进版画面</xsl:with-param>
			<xsl:with-param name='session'><xsl:value-of select='bbsnot/@s' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsnot'>
		<h2>进版画面 - [<xsl:value-of select='@brd' />]</h2>
		<table class='post'><tr>
			<td class='pleft' rowspan='3'>
				<xsl:if test='@brd'>
					<a><xsl:attribute name='href'>gdoc?board=<xsl:value-of select='@brd' /></xsl:attribute>[ 文摘区 ]</a>
					<a><xsl:attribute name='href'>doc?board=<xsl:value-of select='@brd' /></xsl:attribute>[<img src='../images/button/home.gif' />本讨论区]</a>
				</xsl:if>
			</td>
			<td class='pmtop'><xsl:call-template name='linkbar' /></td></tr>
			<tr><td class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='.' /></xsl:call-template></td></tr>
			<tr><td class='pmbot'><xsl:call-template name='linkbar' /></td></tr>
		</table>
	</xsl:template>
	
	<xsl:template name='linkbar'>
	</xsl:template>
</xsl:stylesheet>
