<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='showpost.xsl' />
	<xsl:template match="bbscon">
		<html>
			<head>
				<title>文章阅读 - 日月光华</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
			</head>
			<body>
				<div>
					<xsl:call-template name='showpost'>
						<xsl:with-param name='content' select='post' />
					</xsl:call-template>
				</div>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>