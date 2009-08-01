<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsdelmail">
		<html>
			<head>
				<title>删除信件 - 日月光华BBS</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
			</head>
			<body>
				<p>信件已删除</p>
				<a href='mail'>回到信件列表</a>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>
