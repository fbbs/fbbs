<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>寄语信鸽</xsl:with-param>
			<xsl:with-param name='session'><xsl:value-of select='bbspstmail/@s' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbspstmail'>
		<form id='postform' name='postform' method='post' action='sndmail'>
			<input type='hidden' name='ref'><xsl:attribute name='value'><xsl:value-of select='@ref' /></xsl:attribute></input>
			<p><label for='recv'>收信人:&#160;&#160;&#160;</label><input class='binput' type='text' name='recv' size='15' maxlength='15'><xsl:attribute name='value'><xsl:value-of select='o' /></xsl:attribute></input></p>
			<p><label for='title'>信件标题 </label>
			<input class='binput' type='text' name='title' size='60' maxlength='50'>
				<xsl:variable name='retitle'>
					<xsl:choose>
						<xsl:when test='substring(t, 1, 4) = "Re: "'><xsl:value-of select='t' /></xsl:when>
						<xsl:when test='not(t)'></xsl:when>
						<xsl:otherwise><xsl:value-of select='concat("Re: ", t)' /></xsl:otherwise>
					</xsl:choose>
				</xsl:variable>
				<xsl:attribute name='value'>
					<xsl:call-template name='remove-ansi'>
						<xsl:with-param name='str' select='$retitle' />
					</xsl:call-template>
				</xsl:attribute>
			</input></p>
			<p><textarea class='binput' name='text' rows='20' cols='85' wrap='virtual'>
				<xsl:text>&#x0d;&#x0a;</xsl:text>
				<xsl:call-template name='show-quoted'>
					<xsl:with-param name='content' select='m' />
				</xsl:call-template>
			</textarea></p>
			<input type='submit' value='寄出' id='btnPost' size='10' />
			<input type='reset' value='重置' />
		</form>
	</xsl:template>
</xsl:stylesheet>
