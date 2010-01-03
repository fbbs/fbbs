<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
<xsl:import href='bbs.xsl' />
<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd'/>

<xsl:template match='/'>
	<xsl:choose>
		<xsl:when test='count(bbssel/brd)=1'><html><head><meta http-equiv="refresh"><xsl:attribute name='content'>0; url=<xsl:choose><xsl:when test='bbssel/brd/@dir="1"'>boa?board=<xsl:value-of select='bbssel/brd/@title'/></xsl:when><xsl:otherwise>doc?board=<xsl:value-of select='bbssel/brd/@title'/></xsl:otherwise></xsl:choose></xsl:attribute></meta></head><body></body></html></xsl:when>
		<xsl:otherwise><xsl:apply-imports/></xsl:otherwise>
	</xsl:choose>
</xsl:template>

</xsl:stylesheet>
