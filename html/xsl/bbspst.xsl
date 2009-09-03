<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns='http://www.w3.org/1999/xhtml'>
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd' />
	<xsl:template match='bbspst'>
		<html>
			<head>
				<title>发表文章 - 日月光华</title>
				<meta http-equiv='content-type' content='text/html; charset=gb2312' />
				<link rel="stylesheet" type="text/css" href="/css/bbs.css" />
				<script type='text/javascript' src='/js/bbs.js'></script>
			</head>
			<body><div id='wrap'>
				<xsl:call-template name='navgation-bar'><xsl:with-param name='perm' select='@p' /></xsl:call-template>
				<div id='main'>
					<p>帐号：<xsl:value-of select='@u' /></p>
					<p>版面：<xsl:value-of select='@brd' /></p>
					<form id='postform' name='postform' method='post'>
						<xsl:attribute name='action'>snd?bid=<xsl:value-of select='@bid' />&amp;f=<xsl:value-of select='po/@f' /></xsl:attribute>
						<input type='hidden' id='brd'><xsl:attribute name='value'><xsl:value-of select='@brd' /></xsl:attribute></input>
						<p><label for='title'>标题：</label>
						<input class='binput' type='text' name='title' size='60' maxlength='50'>
							<xsl:variable name='retitle'>
								<xsl:choose>
									<xsl:when test='substring(t, 1, 4) = "Re: "'><xsl:value-of select='t' /></xsl:when>
									<xsl:when test='not(t)'></xsl:when>
									<xsl:otherwise><xsl:value-of select='concat("Re: ", t)' /></xsl:otherwise>
								</xsl:choose>
							</xsl:variable>
							<xsl:attribute name='value'>
								<xsl:call-template name='remove-ansi'>
									<xsl:with-param name='str' select='$retitle' />
								</xsl:call-template>
							</xsl:attribute>
						</input></p>
						<p><textarea class='binput' name='text' rows='20' cols='90' wrap='virtual'>
							<xsl:text>&#x0d;&#x0a;</xsl:text>
							<xsl:call-template name='show-quoted'>
								<xsl:with-param name='content' select='po' />
							</xsl:call-template>
						</textarea></p>
						<input type='submit' value='发表' id='btnPost' size='10'/>
						<input type='button' name='attach' value='上传附件' onclick='return preUpload() ' />
					</form>
				</div>
			</div></body>
		</html>
	</xsl:template>
</xsl:stylesheet>
