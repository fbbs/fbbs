<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'><xsl:value-of select='bbseufile/@desc' /></xsl:with-param>
			<xsl:with-param name='p'><xsl:value-of select='bbseufile/@p' /></xsl:with-param>
			<xsl:with-param name='u'><xsl:value-of select='bbseufile/@u' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbseufile'>
		<h2><xsl:value-of select='@desc' /></h2>
		<xsl:choose>
			<xsl:when test='@submit'>
				<form name='postform' method='post'>
					<xsl:attribute name='action'><xsl:value-of select='@submit' /></xsl:attribute>
					<p><textarea class='binput' name='text' rows='20' cols='85' wrap='virtual'><xsl:call-template name='show-quoted'><xsl:with-param name='content' select='.' /></xsl:call-template></textarea></p>
					<p><input type='submit' value='保存' id='btnPost' size='10'/></p>
				</form>
			</xsl:when>
			<xsl:otherwise><p>保存成功</p><a href='javascript:history.go(-2)'>返回</a></xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>