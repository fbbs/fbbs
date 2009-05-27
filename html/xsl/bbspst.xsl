<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbspst">
		<html>
			<head>
				<title>发表文章 - 日月光华</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
				<script type='text/javascript' src='/js/bbspst.js'></script>
			</head>
			<body>
				<span>帐号：<strong><xsl:value-of select='user' /></strong></span><br />
				<span>版面：<strong id='board'><xsl:value-of select='board' /></strong></span><br />
				<form name='postform' method='post'>
					<xsl:attribute name='action'>bbssnd?bid=<xsl:value-of select='bid' />&amp;f=<xsl:value-of select='f' /></xsl:attribute>
					<span>标题：</span>
					<input class='thinborder' type='text' name='title' size='60' maxlength='50'>
						<xsl:variable name='retitle'>
							<xsl:choose>
								<xsl:when test='substring(title, 1, 4) = "Re: "'>
									<xsl:value-of select='title' />
								</xsl:when>
								<xsl:otherwise>
									<xsl:value-of select='concat("Re: ", title)' />
								</xsl:otherwise>
							</xsl:choose>
						</xsl:variable>
						<xsl:attribute name='value'>
							<xsl:call-template name='remove-ansi'>
								<xsl:with-param name='str' select='$retitle' />
							</xsl:call-template>
						</xsl:attribute>
					</input><br />
					<textarea class='thinborder' name='text' rows='20' cols='90' wrap='virtual'>
						<xsl:text>&#x0d;&#x0a;</xsl:text>
						<xsl:call-template name='show-quoted'>
							<xsl:with-param name='content' select='post' />
						</xsl:call-template>
					</textarea><br />
					<input type='submit' value='发表' id='btnPost' size='10'/>
					<input type='button' name='attach' value='上传附件' onclick='return opnewwin() ' />
				</form>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>