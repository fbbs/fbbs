<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:import href='misc.xsl' />
	<xsl:import href='showpost.xsl' />
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsmail">
		<html>
			<head>
				<title>信件列表 - 日月光华BBS</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<script type='text/javascript' src='/js/bbsmail.js'></script>
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
			</head>
			<body>
				<strong>使用者 [<xsl:value-of select='user' />]&#160;&#160;&#160;信件总数 [<xsl:value-of select='total' />]封</strong>
				<form name="list" method="post" action="bbsmailman">
					<table width="100%" bgcolor="#ffffff">
						<tr class="pt9h">
							<th>序号</th><th>管理</th><th>状态</th><th>发信人</th><th>日期</th><th>信件标题</th>
						</tr>
						<xsl:for-each select='mail'>
							<tr>
								<xsl:attribute name='class'>
									<xsl:if test='position() mod 2 = 1'>pt9lc</xsl:if>
									<xsl:if test='position() mod 2 = 0'>pt9dc</xsl:if>
								</xsl:attribute>
								<!-- No. -->
								<td align='right'><xsl:value-of select='position() - 1 + /bbsmail/start' /></td>
								<!-- Checkbox -->
								<td><input type="checkbox">
									<xsl:attribute name='name'>box<xsl:value-of select='name' /></xsl:attribute>
								</input></td>
								<!-- Mark -->
								<td align='center'><xsl:value-of select='mark' /></td>
								<!-- Author -->
								<td><strong><a>
									<xsl:attribute name='href'>bbsqry?u=<xsl:value-of select='sender' /></xsl:attribute><xsl:value-of select='sender' />
								</a></strong></td>
								<!-- Time -->
								<td>
									<xsl:call-template name='timeconvert'>
										<xsl:with-param name='time' select='date' />
									</xsl:call-template>
								</td>
								<!-- Title -->
								<td width='100%'>
									<xsl:if test='substring(title, 1, 4) != "Re: "'>★</xsl:if>
									<a>
										<xsl:attribute name='href'>bbsmailcon?f=<xsl:value-of select='name' /></xsl:attribute>
										<xsl:call-template name='ansi-escape'>
											<xsl:with-param name='content'><xsl:value-of select='title' /></xsl:with-param>
											<xsl:with-param name='fgcolor'>37</xsl:with-param>
											<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
											<xsl:with-param name='ishl'>0</xsl:with-param>
										</xsl:call-template>
									</a>
								</td>
							</tr>
						</xsl:for-each>
					</table>
					<input name='mode' value='' type='hidden' />
				</form>
				<div>
					[<a href="#"  onclick="checkAll();">本页全选</a>]
					[<a href="#"  onclick="checkReverse();">本页反选</a>]
					[<a href="#" onclick="delSelected()">删除所选信件</a>]
				</div>
				<div>
					<xsl:if test='start > 1'>
						<xsl:variable name='prev'>
							<xsl:choose>
								<xsl:when test='start - page &lt; 1'>1</xsl:when>
								<xsl:otherwise><xsl:value-of select='start - page' /></xsl:otherwise>
							</xsl:choose>
						</xsl:variable>
						<a><xsl:attribute name='href'>bbsmail?start=<xsl:value-of select='$prev' /></xsl:attribute>[ <img src='/images/button/up.gif' />上一页 ]</a>
					</xsl:if>
					<xsl:if test='total > start + page - 1'>
						<xsl:variable name='next'><xsl:value-of select='start + page' /></xsl:variable>
						<a><xsl:attribute name='href'>bbsmail?start=<xsl:value-of select='$next' /></xsl:attribute>[ <img src='/images/button/down.gif' />下一页 ]</a>
					</xsl:if>
					<form><input value='跳转到' type='submit' />第<input name="start" size="4" type="text" />封</form>
				</div>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>