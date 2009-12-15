<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>设定关注名单</xsl:with-param>
			<xsl:with-param name='session'><xsl:value-of select='bbsfall/@s' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsfall'>
		<h2>设定关注名单</h2>
		<table class='content'>
			<tr><th class='owner'>帐号</th><th class='chkbox'>操作</th><th class='idesc'>说明</th></tr>
			<xsl:for-each select='ov'><tr>
				<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
				<td class='owner'><a class='owner'><xsl:attribute name='href'>qry?u=<xsl:value-of select='@id' /></xsl:attribute><xsl:value-of select='@id' /></a></td>
				<td class='chkbox'><a><xsl:attribute name='href'>fdel?u=<xsl:value-of select='@id' /></xsl:attribute>删除</a></td>
				<td class='idesc'><xsl:value-of select='.' /></td>
			</tr></xsl:for-each>
		</table>
		<a href='fadd'>[增加关注网友]</a>
	</xsl:template>
	
	<xsl:template match='bbsfadd'>
		<h2>增加关注网友</h2>
		<form name='add' method='get' action='fadd'>
			<p><label for='id'>帐号: </label><input class='binput' type='text' name='id' size='15' maxlength='15'></input></p>
			<p><label for='id'>说明: </label><input class='binput' type='text' name='desc' size='50' maxlength='50'></input></p>
			<p><input type='submit' value='提交' size='10' /></p>
		</form>
	</xsl:template>
</xsl:stylesheet>
