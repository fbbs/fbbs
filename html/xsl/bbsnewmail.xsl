<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>阅览新信件</xsl:with-param>
			<xsl:with-param name='session'><xsl:value-of select='bbsnewmail/@s' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsnewmail'>
		<h2>阅览新信件</h2>
		<p><a href='mail'><xsl:choose><xsl:when test='count(mail)=0'>您没有30天内的未读信件</xsl:when><xsl:otherwise>本页仅显示30天内未读信件</xsl:otherwise></xsl:choose>，查看全部信件请点此处</a></p>
		<xsl:if test='count(mail)!=0'><form name='list' method='post' action='mailman'>
			<table class='content'>
				<tr><th class='no'>序号</th><th>管理</th><th class='owner'>发信人</th><th>日期</th><th class='ptitle'>信件标题</th></tr>
				<xsl:for-each select='mail'><tr>
					<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
					<td class='no'><xsl:value-of select='@n'/></td>
					<td><input type="checkbox"><xsl:attribute name='name'>box<xsl:value-of select='@name' /></xsl:attribute></input></td>
					<td><a class='owner'><xsl:attribute name='href'>qry?u=<xsl:value-of select='@from' /></xsl:attribute><xsl:value-of select='@from' /></a></td>
					<td class='time'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@date' /></xsl:call-template></td>
					<td class='ptitle'><a class='ptitle'>
						<xsl:attribute name='href'>mailcon?f=<xsl:value-of select='@name' />&amp;n=<xsl:value-of select='@n' /></xsl:attribute>
						<xsl:call-template name='ansi-escape'>
							<xsl:with-param name='content'><xsl:value-of select='.' /></xsl:with-param>
							<xsl:with-param name='fgcolor'>37</xsl:with-param>
							<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
							<xsl:with-param name='ishl'>0</xsl:with-param>
						</xsl:call-template>
					</a></td>
				</tr></xsl:for-each>
			</table>
			<input name='mode' value='' type='hidden' />
		</form>
		<div>[<a href="#"  onclick="checkAll();">全选</a>] [<a href="#"  onclick="checkReverse();">反选</a>] [<a href="#" onclick="delSelected()">删除所选信件</a>]</div></xsl:if>
	</xsl:template>
</xsl:stylesheet>
