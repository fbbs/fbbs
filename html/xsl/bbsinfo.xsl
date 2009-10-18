<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='/'>
		<xsl:call-template name='layout'>
			<xsl:with-param name='title'>个人资料</xsl:with-param>
			<xsl:with-param name='session'><xsl:value-of select='bbsinfo/@s' /></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template match='bbsinfo'>
		<xsl:choose><xsl:when test='@gender'>
			<fieldset><legend>修改个人资料</legend><form action='info?type=1' method='post'>
				<p>您的昵称: <input type='text' name='nick' maxlength='30'><xsl:attribute name='value'><xsl:value-of select='nick' /></xsl:attribute></input></p>
				<p>出生日期: <input type='text' name='year' size='4' maxlength='4'><xsl:attribute name='value'><xsl:value-of select='@year + 1900' /></xsl:attribute></input> 年 <input type='text' name='month' size='2' maxlength='2'><xsl:attribute name='value'><xsl:value-of select='@month' /></xsl:attribute></input> 月 <input type='text' name='day' size='2' maxlength='2'><xsl:attribute name='value'><xsl:value-of select='@day' /></xsl:attribute></input> 日</p>
				<p>用户性别: <input type='radio' value='M' name='gender'><xsl:if test='@gender = "M"'><xsl:attribute name='checked'>checked</xsl:attribute></xsl:if></input> 男 <input type='radio' value='F' name='gender'><xsl:if test='@gender = "F"'><xsl:attribute name='checked'>checked</xsl:attribute></xsl:if></input> 女</p>
				<input type='submit' value='确定' /> <input type='reset' value='复原' />
			</form></fieldset>
			<p>登录本站: <xsl:value-of select='@login' /> 次</p>
			<p>上站时间: <xsl:value-of select='floor(@stay div 60)' /> 小时 <xsl:value-of select='@stay mod 60' /> 分钟</p>
			<p>发表大作: <xsl:value-of select='@post' /> 篇</p>
			<p>帐号建立: <xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@since' /></xsl:call-template></p>
			<p>最近光临: <xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@last' /></xsl:call-template></p>
			<p>来源地址: <xsl:value-of select='@host' /></p>
		</xsl:when>
		<xsl:otherwise><xsl:choose><xsl:when test='string-length(.) = 0'>修改个人资料成功<br/><a href='javascript:history.go(-2)'>返回</a></xsl:when><xsl:otherwise><xsl:value-of select='.' /></xsl:otherwise></xsl:choose></xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>
