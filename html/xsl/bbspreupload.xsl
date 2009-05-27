<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbspreupload">
		<html>
			<head>
				<title>上传文件 - 日月光华</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
				<script type='text/javascript' src='/js/bbspreupload.js'></script>
			</head>
			<body>
				<p>上传文件到[<strong><xsl:value-of select='board' /></strong>]讨论区</p>
				<p>帐号：[<strong><xsl:value-of select='user' /></strong>]</p>
				<p>目前本版单个上传文件大小限制为[<strong><xsl:value-of select='floor(max div 1024)' /></strong>]KB</p>
				<p>服务器资源有限，请勿上传与版面无关的文件。</p>
				<p>请遵守国家法律，严禁上传非法资料和可能导致纠纷的资料。</p>
				<p>允许的文件类型：jpeg/jpg/gif/png/pdf</p>
				<form method='post' name='upform' enctype='multipart/form-data'>
					<xsl:attribute name='action'>bbsupload?b=<xsl:value-of select='board' /></xsl:attribute>
					<span>上传文件: </span><input type='file' name='up'></input>
					<input type='button' value='上传' onclick='return clickup()' />
				</form>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>