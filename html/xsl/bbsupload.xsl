<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsupload">
		<html>
			<head>
				<title>文件上传成功 - 日月光华</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
				<script type='text/javascript' src='/js/bbsupload.js'></script>
			</head>
			<body>
				<p>文件上传成功, 详细信息如下:</p>
				<p>上传帐号：<strong><xsl:value-of select='user' /></strong></p>
				<p>文件地址：<strong id='url'><xsl:value-of select='url' /></strong></p>
				<p>上传文件将自动在文章中添加http://转义，请保持自动添加部分原样，在web界面下转义部分将自动转换为对应的链接/图片</p>
				<p><a href='#' onclick='return closewin()'>返回</a></p>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>