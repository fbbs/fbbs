<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='bbs.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	
	<xsl:template match='/'>
		<html>
			<head>
				<title><xsl:value-of select='rss/channel/title'/> - <xsl:call-template name='bbsname' /></title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<xsl:call-template name='include-css' />
			</head>
			<body><div id='wrap'>
				<div id='main'><xsl:apply-templates /></div>
			</div></body>
		</html>
	</xsl:template>
	
	<xsl:template match='rss/channel'>
		<h2><a><xsl:attribute name='href'><xsl:value-of select='link'/></xsl:attribute><xsl:value-of select='description'/>[<xsl:value-of select='title'/>]</a></h2>
		<xsl:for-each select='item'>
			<div class='post'>
				<h3><a><xsl:attribute name='href'><xsl:value-of select='link'/></xsl:attribute><xsl:value-of select='title'/></a></h3>
				<span class='time'><xsl:value-of select='pubDate'/></span><hr/>
				<xsl:call-template name='showpost'><xsl:with-param name='content' select='description'/></xsl:call-template>
			</div>
		</xsl:for-each>
	</xsl:template>
</xsl:stylesheet>