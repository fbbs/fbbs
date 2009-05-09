<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='splitbm.xsl'/>
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsall">
		<html>
			<head>
				<title>全部讨论区</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
			</head>
			<body>
				<img src="/info/all/banner.jpg" align="absmiddle" border="0" />
				<strong>日月光华 [讨论区数: <xsl:value-of select="count(board)" />]</strong>
				<table width="100%" bgcolor="#ffffff">
					<tr class="pt9h">
						<th>序号</th><th>讨论区名称</th><th>类别</th><th>中文描述</th><th>版主</th>
					</tr>
					<xsl:for-each select='board'>
						<xsl:sort select="title" />
						<tr>
							<xsl:attribute name='class'>
								<xsl:if test='position() mod 2 = 1'>pt9lc</xsl:if>
								<xsl:if test='position() mod 2 = 0'>pt9dc</xsl:if>
							</xsl:attribute>
							<!-- No. -->
							<td align="right"><xsl:value-of select='position()' /></td>
							<!-- English title -->
							<td><strong><a>
								<xsl:if test='@dir="1"'>
									<xsl:attribute name='href'>bbsboa?board=<xsl:value-of select='title' /></xsl:attribute>[ <xsl:value-of select='title' /> ]
								</xsl:if>
								<xsl:if test='@dir="0"'>
									<xsl:attribute name='href'>bbsdoc?board=<xsl:value-of select='title' /></xsl:attribute><xsl:value-of select='title' />
								</xsl:if>
							</a></strong></td>
							<!-- Category -->
							<td align="center">
								<xsl:if test='@dir="1"'><strong>[目录]</strong></xsl:if>
								<xsl:if test='@dir="0"'><xsl:value-of select='cate' /></xsl:if>
							</td>
							<!-- Chinese description -->
							<td width="100%"><strong><a>
								<xsl:if test='@dir="1"'>
									<xsl:attribute name='href'>bbsboa?board=<xsl:value-of select='title' /></xsl:attribute><xsl:value-of select='desc' />
								</xsl:if>
								<xsl:if test='@dir="0"'>
									<xsl:attribute name='href'>bbsdoc?board=<xsl:value-of select='title' /></xsl:attribute><xsl:value-of select='desc' />
								</xsl:if>
							</a></strong></td>
							<!-- Board Masters -->
							<td><strong>
								<xsl:call-template name='splitbm'>
									<xsl:with-param name='names' select='bm' />
									<xsl:with-param name='isdir' select='@dir' />
									<xsl:with-param name='isfirst' select='1' />
								</xsl:call-template>
							</strong></td>
						</tr>
					</xsl:for-each>
				</table>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>