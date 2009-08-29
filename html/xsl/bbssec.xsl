<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns='http://www.w3.org/1999/xhtml'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match='bbssec'>
		<html>
			<head>
				<title>推荐版面 - 日月光华</title>
				<meta http-equiv='content-type' content='text/html; charset=gb2312' />
				<link rel='stylesheet' type='text/css' href='/css/bbs.css' />
			</head>
		</html>
		<body><div class='main'>
			<xsl:call-template name='navgation-bar'><xsl:with-param name='perm' select='@p' /></xsl:call-template>
			<img src='/images/secbanner.jpg' />
			<xsl:for-each select='sec'>
				<div>
					<h2><a><xsl:attribute name='href'>boa?<xsl:value-of select='@id' /></xsl:attribute><xsl:value-of select='@id' />&#160;<xsl:value-of select='@desc' /></a></h2>
					<ul class='brd'>
						<xsl:for-each select='brd'>
							<li><a><xsl:attribute name='href'>doc?board=<xsl:value-of select='@name' /></xsl:attribute><xsl:value-of select='@desc' /></a></li>
						</xsl:for-each>
					</ul>
				</div>
			</xsl:for-each>
		</div></body>
	</xsl:template>
</xsl:stylesheet>
