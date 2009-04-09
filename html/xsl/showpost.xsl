<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:template name="showpost">
		<xsl:param name='content' />
		<xsl:variable name='line' select='substring-before($content, "&#10;")' />
		<xsl:variable name='rest' select='substring-after($content, "&#10;")' />
		<xsl:call-template name='ansi-escape'>
			<xsl:with-param name='content' select='$line' />
			<xsl:with-param name='fgcolor'>
				<xsl:choose>
					<xsl:when test='starts-with($line, ": ")'>36</xsl:when>
					<xsl:when test='starts-with($line, "> ")'>36</xsl:when>
					<xsl:otherwise>37</xsl:otherwise>
				</xsl:choose>
			</xsl:with-param>
			<xsl:with-param name='bgcolor' select='40' />
			<xsl:with-param name='ishl' select='0' />
		</xsl:call-template>
		<br />
		<xsl:if test='$rest'>
			<xsl:call-template name='showpost'>
				<xsl:with-param name='content' select='$rest' />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>
	
	<xsl:template name='ansi-escape'>
		<xsl:param name='content' />
		<xsl:param name='fgcolor' />
		<xsl:param name='bgcolor' />
		<xsl:param name='ishl' />
		<xsl:choose>
			<xsl:when test='contains($content, ">1b[")'>
				<xsl:choose>
					<xsl:when test='$fgcolor = 37'>
						<xsl:value-of select='substring-before($content, ">1b[")' />
					</xsl:when>
					<xsl:otherwise>
						<span>
							<xsl:attribute name='class'>ansi0<xsl:value-of select='$fgcolor' /></xsl:attribute>
							<xsl:value-of select='substring-before($content, ">1b[")' />
						</span>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:variable name='second' select='substring-after($content, ">1b[")' />
				<xsl:variable name='first-alpha'>
					<xsl:call-template name='index-of-first-alpha'>
						<xsl:with-param name='content' select='$second' />
						<xsl:with-param name='start' select='1' />
					</xsl:call-template>
				</xsl:variable>
				<xsl:if test='$first-alpha'>
					<xsl:if test='substring($second, $first-alpha, 1) = "m"'>
						<xsl:variable name='code' select='substring($second, 1, $first-alpha - 1)' />
						<xsl:variable name='fgc'>
							<xsl:call-template name='get-color'>
								<xsl:with-param name='code' select='$code' />
								<xsl:with-param name='param-name'>fgcolor</xsl:with-param>
								<xsl:with-param name='current-value' select='$fgcolor' />
							</xsl:call-template>
						</xsl:variable>
						<xsl:variable name='bgc'>
							<xsl:call-template name='get-color'>
								<xsl:with-param name='code' select='$code' />
								<xsl:with-param name='param-name'>bgcolor</xsl:with-param>
								<xsl:with-param name='current-value' select='$bgcolor' />
							</xsl:call-template>
						</xsl:variable>
						<xsl:variable name='hl'>
							<xsl:call-template name='get-color'>
								<xsl:with-param name='code' select='$code' />
								<xsl:with-param name='param-name'>highlight</xsl:with-param>
								<xsl:with-param name='current-value' select='$ishl' />
							</xsl:call-template>
						</xsl:variable>
						<xsl:variable name='last' select='substring($second, $first-alpha + 1)' />
						<xsl:variable name='text'>
							<xsl:choose>
								<xsl:when test='contains($last, ">1b[")'>
									<xsl:value-of select='substring-before($last, ">1b[")' />
								</xsl:when>
								<xsl:otherwise>
									<xsl:value-of select='$last' />
								</xsl:otherwise>
							</xsl:choose>
						</xsl:variable>
						<span>
							<xsl:attribute name='class'>ansi<xsl:value-of select='$hl' /><xsl:value-of select='$fgc' /><xsl:text> </xsl:text>ansi<xsl:value-of select='$bgc' /></xsl:attribute>
							<xsl:value-of select='$text' />
						</span>
						<xsl:variable name='next' select='substring($last, string-length($text) + 1)' />
						<xsl:if test='$next'>
							<xsl:call-template name='ansi-escape'>
								<xsl:with-param name='content' select='$next' />
								<xsl:with-param name='fgcolor' select='$fgc' />
								<xsl:with-param name='bgcolor' select='$bgc' />
								<xsl:with-param name='ishl' select='$hl' />
							</xsl:call-template>
						</xsl:if>
					</xsl:if>
				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test='$fgcolor = 37'>
						<xsl:value-of select='$content' />
					</xsl:when>
					<xsl:otherwise>
						<span>
							<xsl:attribute name='class'>ansi0<xsl:value-of select='$fgcolor' /></xsl:attribute>
							<xsl:value-of select='$content' />
						</span>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template name='index-of-first-alpha'>
		<xsl:param name='content' />
		<xsl:param name='start' />
		<xsl:variable name='char' select='substring($content, $start, 1)' />
		<xsl:if test='$char'>
			<xsl:choose>
				<xsl:when test='not(contains("0123456789;", $char))'>
					<xsl:value-of select='$start' />
				</xsl:when>
				<xsl:otherwise>
					<xsl:call-template name='index-of-first-alpha'>
						<xsl:with-param name='content' select='$content' />
						<xsl:with-param name='start' select='$start + 1' />
					</xsl:call-template>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:template>

	<xsl:template name='get-color'>
		<xsl:param name='code' />
		<xsl:param name='param-name' />
		<xsl:param name='current-value' />
		<xsl:variable name='first'>
			<xsl:choose>
				<xsl:when test='contains($code, ";")'>
					<xsl:value-of select='substring-before($code, ";")' />
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select='$code' />
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name='next' select='substring-after($code, ";")' />
		<xsl:variable name='new-value'>
			<xsl:if test='$first'>
				<xsl:choose>
					<xsl:when test='$param-name = "fgcolor" and $first >= 30 and $first &lt;= 37'>
						<xsl:value-of select='$first' />
					</xsl:when>
					<xsl:when test='$param-name = "bgcolor" and $first >= 40 and $first &lt;= 47'>
						<xsl:value-of select='$first' />
					</xsl:when>
					<xsl:when test='$param-name = "highlight" and ($first = 0 or $first = 1)'>
						<xsl:value-of select='$first' />
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select='$current-value' />
					</xsl:otherwise>
				</xsl:choose>
			</xsl:if>
		</xsl:variable>
		<xsl:choose>
			<xsl:when test='$next'>
				<xsl:call-template name='get-color'>
					<xsl:with-param name='code' select='$next' />
					<xsl:with-param name='param-name' select='$param-name' />
					<xsl:with-param name='current-value' select='$new-value' />
				</xsl:call-template>
			</xsl:when>
			<xsl:when test='not($next) and not($first)'>
				<xsl:choose>
					<xsl:when test='$param-name = "fgcolor"'>37</xsl:when>
					<xsl:when test='$param-name = "bgcolor"'>40</xsl:when>
					<xsl:when test='$param-name = "highlight"'>0</xsl:when>
					<xsl:otherwise></xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select='$new-value' />
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>