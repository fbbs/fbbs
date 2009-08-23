<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsccc">
		<html>
			<head>
				<title>转载文章 - 日月光华BBS</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
			</head>
			<body>
				<xsl:choose>
					<xsl:when test='not(bid)'><p>转载成功</p><a href='javascript:history.go(-1)'>快速返回</a></xsl:when>
					<xsl:otherwise>
						<form method='get' action='ccc'>
							<p><strong>文章标题: </strong><xsl:value-of select='title' /></p>
							<p><strong>文章作者: </strong><xsl:value-of select='author' /></p>
							<p><strong>原始版面: </strong><xsl:value-of select='board' /></p>
							<input type='hidden' name='bid'><xsl:attribute name='value'><xsl:value-of select='bid' /></xsl:attribute></input>
							<input type='hidden' name='f'><xsl:attribute name='value'><xsl:value-of select='f' /></xsl:attribute></input>
							<label for='t'>转载到版面: </label><input type='text' name='t' />
							<input type='submit' value='转载' />
							<p><strong>转帖注意：未经站务委员会批准，多版面转贴相同或相似文章超过五个版的，将受到全站处罚。</strong></p>
						</form>
					</xsl:otherwise>
				</xsl:choose>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>
