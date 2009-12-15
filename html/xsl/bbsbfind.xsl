<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>版面文章搜索</xsl:with-param>
			<xsl:with-param name='session'><xsl:value-of select='bbsbfind/@s' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsbfind'>
		<h2>版内文章搜索</h2>
		<xsl:variable name='count' select='count(po)'/>
		<xsl:choose><xsl:when test='@result'><p>共找到 <xsl:value-of select='$count'/> 篇文章 <xsl:if test='$count&gt;=100'>（100篇以上部分省略）</xsl:if></p><xsl:if test='$count!=0'><p>最新文章靠前</p><table class='content'>
			<tr><th class='no'>序号</th><th class='mark'>标记</th><th class='owner'>作者</th><th class='time'>发表时间</th><th class='ptitle'>标题</th></tr>
			<xsl:for-each select='po'><tr>
				<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
				<td class='no'><xsl:value-of select='position()' /></td>
				<td class='mark'><xsl:value-of select='@m' /></td>
				<td class='owner'><a class='owner'><xsl:attribute name='href'>qry?u=<xsl:value-of select='@owner' /></xsl:attribute><xsl:value-of select='@owner' /></a></td>
				<td class='time'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@time' /></xsl:call-template></td>
				<xsl:variable name='imgsrc'>../images/types/<xsl:choose><xsl:when test='substring(., 1, 4) = "Re: "'>reply</xsl:when><xsl:otherwise>text</xsl:otherwise></xsl:choose>.gif</xsl:variable>
				<xsl:variable name='text'><xsl:choose><xsl:when test='substring(., 1, 4) = "Re: "'><xsl:value-of select='substring(., 5)' /></xsl:when><xsl:otherwise><xsl:value-of select='.' /></xsl:otherwise></xsl:choose></xsl:variable>
				<td class='ptitle'><a class='ptitle'>
					<xsl:attribute name='href'><xsl:value-of select='../brd/@link' />con?bid=<xsl:value-of select='../@bid' />&amp;f=<xsl:value-of select='@id' /></xsl:attribute>
					<img><xsl:attribute name='src'><xsl:value-of select='$imgsrc' /></xsl:attribute></img>
					<xsl:call-template name='ansi-escape'>
						<xsl:with-param name='content'><xsl:value-of select='$text' /></xsl:with-param>
						<xsl:with-param name='fgcolor'>37</xsl:with-param>
						<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
						<xsl:with-param name='ishl'>0</xsl:with-param>
					</xsl:call-template>
				</a></td>
			</tr></xsl:for-each>
		</table></xsl:if></xsl:when>
		<xsl:otherwise><form action='bfind' method='get'><fieldset><legend>至少填写一项</legend>
			<input name='bid' type='hidden'><xsl:attribute name='value'><xsl:value-of select='@bid'/></xsl:attribute></input>
			<p>标题含有: <input name='t1' type='text' maxlength='50' size='20'/> 和 <input name='t2' type='text' maxlength='50' size='20'/></p>
			<p>标题不含: <input name='t3' type='text' maxlength='50' size='20'/></p>
			<p>作者帐号: <input name='user' type='text' maxlength='12' size='16'/></p></fieldset>
			<fieldset><legend>选填</legend><p>时间范围: <input name='limit' type='text' maxlength='4' size='4' value='7'/>天以内 (至多30天)</p>
			<p>文章标记: <input name='mark' type='checkbox'/></p>
			<p>不含跟帖: <input name='nore' type='checkbox'/></p></fieldset>
			<p><input type='submit' value='给我搜！'/></p>
		</form></xsl:otherwise></xsl:choose>
	</xsl:template>
</xsl:stylesheet>
