<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>推荐版面</xsl:with-param>
			<xsl:with-param name='session'><xsl:value-of select='bbssec/@s' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbssec'>
		<img src='../images/secbanner.jpg' />
		<xsl:for-each select='sec'>
			<ul class='sec'>
				<li><a><xsl:attribute name='href'>boa?s=<xsl:value-of select='@id' /></xsl:attribute><xsl:value-of select='@id' />&#160;<xsl:value-of select='@desc' /></a></li>
				<ul class='brd'>
					<xsl:for-each select='brd'>
						<li><a><xsl:attribute name='href'>doc?board=<xsl:value-of select='@name' /></xsl:attribute><xsl:value-of select='@desc' /></a></li>
					</xsl:for-each>
				</ul>
			</ul>
		</xsl:for-each>
	</xsl:template>
</xsl:stylesheet>
