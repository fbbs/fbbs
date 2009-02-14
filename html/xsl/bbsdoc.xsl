<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='splitbm.xsl'/>
	<xsl:import href='date-time.xsl'/>
	<xsl:template match="bbsdoc">
		<html>
			<head>
				<title><xsl:value-of select='desc' /> - 日月光华BBS</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
			</head>
			<body>
				<xsl:if test='icon'>
					<img align='absmiddle'><xsl:attribute name='src'><xsl:value-of select='icon' /></xsl:attribute></img>
				</xsl:if>
				<xsl:choose>
					<xsl:when test='banner'>
						<img align='absmiddle'><xsl:attribute name='src'><xsl:value-of select='icon' /></xsl:attribute></img>
					</xsl:when>
					<xsl:otherwise>
						<h1><xsl:value-of select='desc' /> [<xsl:value-of select='title' />]</h1>
					</xsl:otherwise>
				</xsl:choose>
				<strong>版主 [ 
					<xsl:call-template name='splitbm'>
						<xsl:with-param name='names' select='bm' />
						<xsl:with-param name='isdir' select='@dir' />
						<xsl:with-param name='isfirst' select='1' />
					</xsl:call-template> ]  文章数 [ <xsl:value-of select='total' /> ]
				</strong>
				<table width="100%" bgcolor="#ffffff">
					<tr class="pt9h">
						<th>序号</th><th>作者</th><th>发表时间</th><th>标题</th>
					</tr>
					<xsl:for-each select='post'>
						<tr>
							<xsl:attribute name='class'>
								<xsl:if test='position() mod 2 = 1'>pt9lc</xsl:if>
								<xsl:if test='position() mod 2 = 0'>pt9dc</xsl:if>
							</xsl:attribute>
							<!-- No. -->
							<td align='right'><xsl:value-of select='position() - 1 + /bbsdoc/start' /></td>
							<!-- Author -->
							<td><strong><a>
								<xsl:attribute name='href'>bbsqry?userid=<xsl:value-of select='author' /></xsl:attribute><xsl:value-of select='author' />
							</a></strong></td>
							<!-- Time -->
							<td>
								<xsl:call-template name='timeconvert'>
									<xsl:with-param name='time' select='time' />
								</xsl:call-template>
							</td>
							<!-- Title -->
							<td width='100%'>
								<xsl:choose>
									<xsl:when test='substring(title, 1, 4) = "Re: "'>
										<img align='absmiddle' border='0' src='/images/types/reply.gif' />
										<a>
											<xsl:attribute name='href'>bbscon?b=<xsl:value-of select='/bbsdoc/title' />f=<xsl:value-of select='id' /></xsl:attribute>
											<xsl:value-of select='substring(title, 5)' />
										</a>
									</xsl:when>
									<xsl:otherwise>
										<img align='absmiddle' border='0' src='/images/types/text.gif' />
										<a>
											<xsl:attribute name='href'>bbscon?b=<xsl:value-of select='/bbsdoc/title' />f=<xsl:value-of select='id' /></xsl:attribute>
											<xsl:value-of select='title' />
										</a>
									</xsl:otherwise>
								</xsl:choose>
							</td>
						</tr>
					</xsl:for-each>
				</table>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>