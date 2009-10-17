<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>修改密码</xsl:with-param>
			<xsl:with-param name='p'><xsl:value-of select='bbspwd/@p' /></xsl:with-param>
			<xsl:with-param name='u'><xsl:value-of select='bbspwd/@u' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbspwd'>
		<xsl:choose>
			<xsl:when test='@i'><form action='pwd' method='post'>
				<p><label for='pw1'>您的旧密码: </label><input maxlength='12' size='12' type='password' name='pw1' /></p>
				<p><label for='pw2'>输入新密码: </label><input maxlength='12' size='12' type='password' name='pw2' /></p>
				<p><label for='pw3'>确认新密码: </label><input maxlength='12' size='12' type='password' name='pw3' /></p>
				<input type='submit' value='确定修改' />
			</form></xsl:when>
			<xsl:otherwise>
				<xsl:choose><xsl:when test='string-length(.)=0'>修改密码成功<br/><a href='javascript:history.go(-2)'>返回</a></xsl:when><xsl:otherwise><xsl:value-of select='.' /><br/><a href='javascript:history.go(-1)'>返回</a></xsl:otherwise></xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>
