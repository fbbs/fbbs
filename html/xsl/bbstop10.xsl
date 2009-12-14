<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl'/>
	<xsl:import href='showpost.xsl'/>
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>本日十大</xsl:with-param>
			<xsl:with-param name='session'><xsl:value-of select='bbstop10/@s' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbstop10'>
		<h2>24小时十大热门话题</h2>
		<table class='content'>
			<tr><th class='no'>排名</th><th>作者</th><th class='ptitle'>标题</th><th class='no'>篇数</th></tr>
			<xsl:for-each select='top'><tr>
				<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
				<td class='no'><xsl:value-of select='position()'/></td>
				<td><a class='owner'><xsl:attribute name='href'>qry?u=<xsl:value-of select='@owner' /></xsl:attribute><xsl:value-of select='@owner' /></a></td>
				<td class='ptitle'><a class='ptitle'><xsl:attribute name='href'>tcon?board=<xsl:value-of select='@board' />&amp;gid=<xsl:value-of select='@gid' /></xsl:attribute><xsl:call-template name='ansi-escape'>	<xsl:with-param name='content' select='.'/><xsl:with-param name='fgcolor'>37</xsl:with-param><xsl:with-param name='bgcolor'>ignore</xsl:with-param><xsl:with-param name='ishl'>0</xsl:with-param></xsl:call-template></a></td>
				<td class='no'><xsl:value-of select='@count'/></td>
			</tr></xsl:for-each>
		</table>
	</xsl:template>
</xsl:stylesheet>
