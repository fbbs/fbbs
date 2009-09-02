<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:import href='misc.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='bbssec'>
	<html>
		<head>
			<title>推荐版面<xsl:call-template name='bbsname' /></title>
			<meta http-equiv='content-type' content='text/html; charset=gb2312' />
			<link rel='stylesheet' type='text/css' href='/css/bbs.css' />
			<script type='text/javascript' src='/js/bbs.js'></script>
		</head>
		<body><div id='wrap'>
			<xsl:call-template name='navgation-bar'><xsl:with-param name='perm' select='@p' /></xsl:call-template>
			<div id='main'>
				<img src='/images/secbanner.jpg' />
				<xsl:for-each select='sec'>
					<ul class='sec'>
						<li><a><xsl:attribute name='href'>boa?s=<xsl:value-of select='@id' /></xsl:attribute><xsl:value-of select='@id' />&#160;<xsl:value-of select='@desc' /></a></li>
						<ul class='brd'>
							<xsl:for-each select='brd'>
								<li><a><xsl:attribute name='href'>doc?board=<xsl:value-of select='@name' /></xsl:attribute><xsl:value-of select='@desc' /></a></li>
							</xsl:for-each>
						</ul>
					</ul>
				</xsl:for-each>
			</div>
		</div></body>
	</html>
	</xsl:template>
</xsl:stylesheet>
