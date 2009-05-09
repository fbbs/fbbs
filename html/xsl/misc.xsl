<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:template name="timeconvert">
		<xsl:param name='time' />
		<xsl:value-of select="concat(substring($time, 6, 5), ' ', substring($time, 12, 5))" />
	</xsl:template>

	<xsl:template name="splitbm">
		<xsl:param name='names' />
		<xsl:param name='isdir' />
		<xsl:param name='isfirst' />
        <xsl:variable name='first' select='substring-before($names," ")' />
        <xsl:variable name='rest' select='substring-after($names," ")' />
        <xsl:if test='$first'>
			<a><xsl:attribute name='href'>bbsqry?u=<xsl:value-of select='$first' /></xsl:attribute><xsl:value-of select='$first' /></a>
		</xsl:if>
		<xsl:if test='$rest'>
			<span>&#160;</span>
			<xsl:call-template name='splitbm'>
				<xsl:with-param name='names' select='$rest'/>
				<xsl:with-param name='isdir' select='$isdir'/>
				<xsl:with-param name='isfirst' select='0'/>
			</xsl:call-template>
        </xsl:if>
		<xsl:if test='not($rest)'>
			<xsl:if test='$names'>
				<a><xsl:attribute name='href'>bbsqry?u=<xsl:value-of select='$names' /></xsl:attribute><xsl:value-of select='$names' /></a>
			</xsl:if>
			<xsl:if test="$names=''">
				<xsl:if test="$isdir='0'">诚征版主中</xsl:if>
				<xsl:if test="$isdir!='0'">-</xsl:if>
			</xsl:if>
		</xsl:if>
	</xsl:template>
	
	<xsl:template name="showexp">
		<xsl:param name="count" />
		<xsl:if test="$count > 0">
			<img>
				<xsl:attribute name='src'>/images/level/<xsl:value-of select="level" />.gif</xsl:attribute>
			</img>
			<xsl:call-template name="showexp">
				<xsl:with-param name="count" select="$count - 1"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>
</xsl:stylesheet>