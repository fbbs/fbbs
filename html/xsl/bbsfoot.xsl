<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:output method='html' encoding='gb2312' />
	<xsl:template match="bbsfoot">
		<html>
			<head>
				<title>底部状态条</title>
				<meta http-equiv="content-type" content="text/html; charset=gb2312" />
				<link rel="stylesheet" type="text/css" href="/css/bbs0.css" />
				<link rel="stylesheet" type="text/css" href="/css/bbsfoot.css" />
			</head>
			<body>
				<xsl:if test="mail = '1'"><bgsound src="/mail.wav" /></xsl:if>
				<div id='left'>
					<xsl:choose>
						<xsl:when test="user">
							<img>
								<xsl:attribute name='src'>/images/user_<xsl:value-of select="gender" />.gif</xsl:attribute>
							</img>
							<a target='view'>
								<xsl:attribute name='href'>bbsqry?userid=<xsl:value-of select="user" /></xsl:attribute>
								<strong><xsl:value-of select="user" /></strong>
							</a>
							<xsl:call-template name="showexp">
								<xsl:with-param name="count" select="repeat" />
							</xsl:call-template>
							<xsl:if test="bind = '0'">
								<span class='notice'>您尚未绑定邮箱，请尽快用telnet登录本站进行邮箱认证</span>
							</xsl:if>
						</xsl:when>
						<xsl:otherwise>
							<form action="bbslogin" method="post" target="_top">
								用户<input name="id" type="text" maxlength="12" size="12" class="thinblue" />
								密码<input name='pw' type='password' maxlength='12' size='12' class='thinblue' />
								<input src='/images/login.gif' type='image' border='0' align='absmiddle' />
							</form>
						</xsl:otherwise>
					</xsl:choose>
				</div>
				<div id='right'>
					<img src='/images/users.gif' />
					<span>[<a href='bbsusr' target='view'><xsl:value-of select="online" /></a>]</span>
					<xsl:if test="mail = '1'">
						<img src='/images/mailw.gif' />
						<span class='notice'>有新信</span>
					</xsl:if>
					<img src='/images/water.gif' />
					<span>
						<xsl:choose>
							<xsl:when test="hour">
								[<xsl:value-of select="hour" />小时<xsl:value-of select="min" />分]
							</xsl:when>
							<xsl:otherwise>
								[0小时0分]
							</xsl:otherwise>
						</xsl:choose>
					</span>
				</div>
				<script>setTimeout('self.location=self.location', 240000);</script>
			</body>
		</html>
	</xsl:template>
	
	  <xsl:template name="showexp">
		<xsl:param name="count" select="1"/>
		<xsl:if test="$count > 0">
			<img>
				<xsl:attribute name='src'>/images/level/<xsl:value-of select="level" />.gif</xsl:attribute>
			</img>
			<xsl:call-template name="showexp">
				<xsl:with-param name="count" select="$count - 1"/>
			</xsl:call-template>
      </xsl:if>
      
  </xsl:template>
</xsl:stylesheet>