<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>查询网友</xsl:with-param>
			<xsl:with-param name='p'><xsl:value-of select='bbsqry/@p' /></xsl:with-param>
			<xsl:with-param name='u'><xsl:value-of select='bbsqry/@u' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsqry'>
		<form action='qry' method='get'><label for='u'>请输入欲查询的帐号：</label><input type='text' name='u' maxlength='12' size='12'/><input type='submit' value='查询'/></form>
		<xsl:if test='@id'><div id='user'>
			<p><strong><xsl:value-of select='@id' /></strong> （<strong><xsl:value-of select='nick' /></strong>） <xsl:call-template name='show-horo'/></p>
			<p>上次在:【<span class='a132'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@lastlogin' /></xsl:call-template></span>】从【<span class='a132'><xsl:value-of select='ip' /></span>】到本站一游。</p>
			<p>文章数:【<span class='a132'><xsl:value-of select='@post' /></span>】 生命力:【<span class='a132'><xsl:value-of select='@hp' /></span>】</p> 
			<p>表现值:【<span class='a133'><xsl:value-of select='@perf' /></span>】</p>
			<div>经验值:【<xsl:call-template name="show-exp" />】 (<xsl:value-of select='@level * 10 + @repeat' />/120)</div>
		</div></xsl:if>
	</xsl:template>
	
	<xsl:template name='show-horo'>
		<xsl:if test='@horo'>
			<xsl:variable name='color'><xsl:choose><xsl:when test='gender = "M"'>a136</xsl:when><xsl:when test='gender = "F"'>a135</xsl:when><xsl:otherwise>a132</xsl:otherwise></xsl:choose></xsl:variable>
			<span>【</span><span><xsl:attribute name='class'><xsl:value-of select='$color' /></xsl:attribute><xsl:value-of select='@horo' /></span><span>】</span>
		</xsl:if>
	</xsl:template>
	
	<xsl:template name='show-exp'>
		<div>
			<xsl:attribute name='class'>lev<xsl:value-of select='@level' /></xsl:attribute>
			<div><xsl:attribute name='class'>lev<xsl:value-of select='@level' /></xsl:attribute><xsl:attribute name='style'>width:<xsl:value-of select='@repeat * 10' />%;</xsl:attribute></div>
		</div>
	</xsl:template>
</xsl:stylesheet>
