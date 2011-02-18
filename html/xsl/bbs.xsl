<?xml version='1.0' encoding='utf-8'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
	<xsl:output method='html' encoding='utf-8' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd'/>

	<xsl:template name='timeconvert'>
		<xsl:param name='time'/>
		<xsl:value-of select='concat(substring($time, 6, 5), " ", substring($time, 12, 5))'/>
	</xsl:template>

	<xsl:template match='/'>
		<html>
			<head>
				<title></title>
				<meta http-equiv="content-type" content="text/html; charset=utf-8"/>
				<link rel='stylesheet' type='text/css' href='../css/bbs.css'/>
			</head>
			<body>
				<div id='main'><xsl:apply-templates/></div>
			</body>
		</html>
	</xsl:template>

	<xsl:template match='bbs_board'>
		<h2><a href='board?bid={brd/@bid}'><xsl:value-of select='brd/@desc'/> [<xsl:value-of select='brd/@name'/>]</a></h2>
		<table>
			<tr><th>标记</th><th>作者</th><th>发表时间</th><th>标题</th></tr>
			<xsl:for-each select='po'>
				<tr>
					<td><xsl:value-of select='@m'/></td>
					<td><a href='user?name={@o}'><xsl:value-of select='@o'/></a></td>
					<td><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@t'/></xsl:call-template></td>
					<td><a href='post?id={@id}'><xsl:value-of select='.'/></a></td>
				</tr>
			</xsl:for-each>
		</table>
		<div>
			<a href='board?bid={brd/@bid}&amp;pid={brd/@begin}&amp;a=p'>前页</a>
			<a href='board?bid={brd/@bid}&amp;pid={brd/@end}&amp;a=n'>后页</a>
		</div>
	</xsl:template>

	<xsl:template match='bbs_post'>
		<a href='board?bid={@bid}'>返回版面</a>
		<a href='post?id={post/@id}&amp;a=n'>下楼</a>
		<a href='post?id={post/@id}&amp;a=p'>上楼</a>
		<xsl:if test='post/@id!=@gid'><a href='post?id={@gid}'>顶楼</a></xsl:if>
		<xsl:apply-templates select='post'/>
	</xsl:template>

	<xsl:template match='post'>
		<div class='post'>
			<div class='post_h'>
				<p><span>发信人: </span><a href='user?name={owner}'><xsl:value-of select='owner'/></a> (<xsl:value-of select='nick'/>), 信区: <a href='board?name={board}'><xsl:value-of select='board'/></a></p>
				<p><span>标  题: </span><xsl:value-of select='title'/></p>
				<p><span>发信站: </span>复旦泉 (<xsl:value-of select='date'/>), 站内信件</p>
			</div>
			<xsl:for-each select='pa'>
				<div class='post_{@m}'>
					<xsl:for-each select='p'>
						<p><xsl:apply-templates select='.'/></p>
					</xsl:for-each>
				</div>
			</xsl:for-each>
		</div>
	</xsl:template>

	<xsl:template match='c'>
		<span class='a{@h}{@f} a{@b}'><xsl:value-of select='.'/></span>
	</xsl:template>

	<xsl:template match='a'>
		<a href='{@href}'><xsl:choose><xsl:when test='@i'><img src='{@href}'/></xsl:when><xsl:otherwise><xsl:value-of select='@href'/></xsl:otherwise></xsl:choose></a>
	</xsl:template>
</xsl:stylesheet>

