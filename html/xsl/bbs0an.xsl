<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='misc.xsl'/>
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbs0an">
		<html>
			<head>
				<title>精华区浏览 - 日月光华BBS</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
			</head>
			<body>
				<p>本目录web浏览次数：[<xsl:value-of select='visit' />]</p>
				<table>
					<tr class="pt9h">
						<th>序号</th><th>标题</th><th>整理者</th><th>日期</th>
					</tr>
					<xsl:for-each select='entry'>
						<tr>
							<xsl:attribute name='class'>
								<xsl:if test='position() mod 2 = 1'>pt9lc</xsl:if>
								<xsl:if test='position() mod 2 = 0'>pt9dc</xsl:if>
							</xsl:attribute>
							<!-- No. -->
							<td align='right'><xsl:value-of select='position()' /></td>
							<!-- Title -->
							<td width='80%'>
								<xsl:choose>
									<xsl:when test='type = "dir"'>
										<img src='/images/types/folder.gif' /><a><xsl:attribute name='href'>bbs0an?path=<xsl:value-of select='/bbs0an/path' /><xsl:value-of select='path' /></xsl:attribute><xsl:value-of select='title' /></a>
									</xsl:when>
									<xsl:when test='type = "file"'>
										<img src='/images/types/text.gif' /><a><xsl:attribute name='href'>bbsanc?path=<xsl:value-of select='/bbs0an/path' /><xsl:value-of select='path' /></xsl:attribute><xsl:value-of select='title' /></a>
									</xsl:when>
									<xsl:otherwise><img src='/images/types/error.gif' /><xsl:value-of select='title' /></xsl:otherwise>
								</xsl:choose>
							</td>
							<!-- Arranger -->
							<td>
								<xsl:if test='id'>
									<xsl:call-template name='splitbm'>
										<xsl:with-param name='names' select='id' />
										<xsl:with-param name='isdir' select='0' />
										<xsl:with-param name='isfirst' select='1' />
									</xsl:call-template>
								</xsl:if>
							</td>
							<!-- Time -->
							<td>
								<xsl:if test='type != "err"'>
									<xsl:call-template name='timeconvert'>
										<xsl:with-param name='time' select='time' />
									</xsl:call-template>
								</xsl:if>
							</td>
						</tr>
					</xsl:for-each>
					<xsl:if test='not(entry)'>
						<td /><td width='80%'>&lt;&lt;目前没有文章&gt;&gt;</td>
					</xsl:if>
				</table>
				<a href='javascript:history.go(-1)'>[<img src='/images/button/back.gif' />返回上一页]</a>
				<xsl:if test='board'><a><xsl:attribute name='href'>bbsdoc?board=<xsl:value-of select='board' /></xsl:attribute>[<img src='/images/button/home.gif' />本讨论区]</a></xsl:if>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>