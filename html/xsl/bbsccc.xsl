<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>转载文章</xsl:with-param>
			<xsl:with-param name='p'><xsl:value-of select='bbsccc/@p' /></xsl:with-param>
			<xsl:with-param name='u'><xsl:value-of select='bbsccc/@u' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsccc'>
		<xsl:choose>
			<xsl:when test='not(@bid)'>
				<p>转载成功</p>
				<p><a><xsl:attribute name='href'>doc?bid=<xsl:value-of select='@b' /></xsl:attribute>[ <img src='../images/button/back.gif' />返回原先版面 ]</a></p>
				<p><a><xsl:attribute name='href'>doc?bid=<xsl:value-of select='@t' /></xsl:attribute>[ <img src='../images/button/forward.gif' />进入目标版面 ]</a></p>
			</xsl:when>
			<xsl:otherwise>
				<form method='get' action='ccc'>
					<p>文章标题: <xsl:value-of select='.' /></p>
					<p>文章作者: <xsl:value-of select='@owner' /></p>
					<p>原始版面: <xsl:value-of select='@brd' /></p>
					<input type='hidden' name='bid'><xsl:attribute name='value'><xsl:value-of select='@bid' /></xsl:attribute></input>
					<input type='hidden' name='f'><xsl:attribute name='value'><xsl:value-of select='@fid' /></xsl:attribute></input>
					<label for='t'>转载到版面: </label><input type='text' name='t' />
					<input type='submit' value='转载' />
					<p><strong>转帖注意：未经站务委员会批准，多版面转贴相同或相似文章超过五个版的，将受到全站处罚。</strong></p>
				</form>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>