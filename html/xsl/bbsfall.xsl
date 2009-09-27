<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>设定关注名单</xsl:with-param>
			<xsl:with-param name='p'><xsl:value-of select='bbsfall/@p' /></xsl:with-param>
			<xsl:with-param name='u'><xsl:value-of select='bbsfall/@u' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsfall'>
		<h2>设定关注名单</h2>
		<table class='content'>
			<tr><th class='owner'>帐号</th><th class='idesc'>说明</th></tr>
			<xsl:for-each select='ov'><tr>
				<td class='owner'><a><xsl:attribute name='href'>qry?u=<xsl:value-of select='@id' /></xsl:attribute><xsl:value-of select='@id' /></a></td><td class='idesc'><xsl:value-of select='.' /></td>
			</tr></xsl:for-each>
		</table>
	</xsl:template>
</xsl:stylesheet>