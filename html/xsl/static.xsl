<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
<xsl:import href='bbs.xsl'/>
<xsl:import href='showpost.xsl'/>
<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd'/>
<xsl:template match='/'>
<html>
<head>
	<title>文章阅读 - <xsl:call-template name='bbsname'/></title>
	<meta http-equiv="content-type" content="text/html; charset=gb2312"/>
	<xsl:call-template name='include-css'/>
</head>
<body>
	<div class='post'>
		<div class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='bbscon/po'/></xsl:call-template></div>
	</div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>
