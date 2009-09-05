<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>收藏夹</xsl:with-param>
			<xsl:with-param name='p'><xsl:value-of select='/@p' /></xsl:with-param>
			<xsl:with-param name='u'><xsl:value-of select='/@u' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsmybrd'>
		<h2>收藏夹设定</h2>
		<xsl:choose>
			<xsl:when test='@selected'>
				<div>修改预定讨论区成功，您现在一共预定了 <xsl:value-of select='@selected'/> 个讨论区</div>
			</xsl:when>
			<xsl:otherwise>
				<form action='mybrd?type=1' method='post'>
					<table class='content'>
						<xsl:for-each select='brd'>
							<xsl:if test='position() mod 3 = 1'><tr>
								<xsl:apply-templates select='.'/><xsl:apply-templates select='following-sibling::brd[1]'/><xsl:apply-templates select='following-sibling::brd[2]'/>
							</tr></xsl:if>
						</xsl:for-each>
					</table>
					<input type='submit' value='确认预定'/><input type='reset' value='复原'/>
				</form>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match='brd'>
		<xsl:variable name='check'><xsl:call-template name='is-mybrd'><xsl:with-param name='bid' select='@bid'/></xsl:call-template></xsl:variable>
		<td class='idesc'>
			<input type='checkbox'>
				<xsl:attribute name='name'><xsl:value-of select='@bid'/></xsl:attribute>
				<xsl:if test='$check = 1'><xsl:attribute name='checked'>checked</xsl:attribute></xsl:if>
			</input>
			<a class='idesc'><xsl:attribute name='href'>doc?bid=<xsl:value-of select='@bid'/></xsl:attribute><xsl:value-of select='@desc'/></a>
		</td>
	</xsl:template>
	
	<xsl:template name='is-mybrd'>
		<xsl:param name='bid'/>
		<xsl:for-each select='../my'><xsl:if test='@bid = $bid'>1</xsl:if></xsl:for-each>
	</xsl:template>
</xsl:stylesheet>