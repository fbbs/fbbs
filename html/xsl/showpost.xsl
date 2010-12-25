<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">
	<xsl:template name='showpost'>
		<xsl:param name='content'/>
		<xsl:call-template name='_showpost'>
			<xsl:with-param name='content' select='$content'/>
			<xsl:with-param name='mode'>normal</xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template name='simple-post'>
		<xsl:param name='content'/>
		<xsl:call-template name='_showpost'>
			<xsl:with-param name='content' select='$content'/>
			<xsl:with-param name='mode'>simple</xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<xsl:template name="_showpost">
		<xsl:param name='content'/>
		<xsl:param name='mode'/>
		<xsl:variable name='show-image' select='$mode = "normal"'/>
		<xsl:variable name='before'><xsl:if test='not(contains($content, "&#10;"))'><xsl:value-of select='$content'/></xsl:if><xsl:value-of select='substring-before($content, "&#10;")'/></xsl:variable>
		<xsl:variable name='rest' select='substring-after($content, "&#10;")'/>
		<xsl:variable name='line'>
			<xsl:call-template name='replace-space'>
				<xsl:with-param name='str' select='$before'/>
			</xsl:call-template>
		</xsl:variable>
		<xsl:call-template name='showline'>
			<xsl:with-param name='content' select='$line'/>
			<xsl:with-param name='isquote' select='starts-with($line, ":&#160;") or starts-with($line, ">&#160;")'/>
			<xsl:with-param name='show-image' select='$show-image'/>
		</xsl:call-template>
		<br/>
		<xsl:if test='$rest'>
			<xsl:call-template name='_showpost'>
				<xsl:with-param name='content' select='$rest'/>
				<xsl:with-param name='mode'><xsl:value-of select='$mode'/></xsl:with-param>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<xsl:template name='replace-space'>
		<xsl:param name='str'/>
		<xsl:choose>
			<xsl:when test='contains($str, " ")'>
				<xsl:value-of select='substring-before($str, " ")'/><xsl:text>&#160;</xsl:text>
				<xsl:call-template name='replace-space'>
					<xsl:with-param name='str' select='substring-after($str, " ")'/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select='$str'/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name='showline'>
		<xsl:param name='content'/>
		<xsl:param name='isquote'/>
		<xsl:param name='show-image'/>
		<xsl:choose>
			<xsl:when test='$isquote'>
				<xsl:call-template name='ansi-escape'>
					<xsl:with-param name='content' select='$content'/>
					<xsl:with-param name='fgcolor'>36</xsl:with-param>
					<xsl:with-param name='bgcolor'>40</xsl:with-param>
					<xsl:with-param name='ishl'>0</xsl:with-param>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:variable name='before'>
					<xsl:choose>
						<xsl:when test='contains($content, "http://")'>
							<xsl:value-of select='substring-before($content, "http://")'/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:value-of select='$content'/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:variable>
				<xsl:if test='$before'>
					<xsl:call-template name='ansi-escape'>
						<xsl:with-param name='content' select='$before'/>
						<xsl:with-param name='fgcolor'>37</xsl:with-param>
						<xsl:with-param name='bgcolor'>40</xsl:with-param>
						<xsl:with-param name='ishl'>0</xsl:with-param>
					</xsl:call-template>
				</xsl:if>
				<xsl:variable name='after' select='substring-after($content, "http://")'/>
				<xsl:variable name='index'>
					<xsl:if test='$after'>
						<xsl:call-template name='not-str-token'>
							<xsl:with-param name='str' select='$after'/>
							<xsl:with-param name='delim'>0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~$-_.+!*')(,/:;=?@%#&amp;</xsl:with-param>
							<xsl:with-param name='start' select='1'/>
						</xsl:call-template>
					</xsl:if>
				</xsl:variable>
				<xsl:variable name='url'>
					<xsl:if test='$after'>
						<xsl:choose>
							<xsl:when test='string-length($index)'>
								<xsl:value-of select='substring($after, 1, $index - 1)'/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select='$after'/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:if>
				</xsl:variable>
				<xsl:if test='contains($content, "http://")'>
					<xsl:call-template name='show-url'>
						<xsl:with-param name='url' select='concat("http://", $url)'/>
						<xsl:with-param name='show-image' select='$show-image'/>
					</xsl:call-template>
				</xsl:if>
				<xsl:if test='$after and $index'>
					<xsl:call-template name='showline'>
						<xsl:with-param name='content' select='substring($after, $index)'/>
						<xsl:with-param name='isquote' select='0'/>
						<xsl:with-param name='show-image' select='$show-image'/>
					</xsl:call-template>
				</xsl:if>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name='show-url'>
		<xsl:param name='url'/>
		<xsl:param name='show-image'/>
		<xsl:variable name='length' select='string-length($url)'/>
		<xsl:variable name='lurl' select='translate($url, "JPEGNIF", "jpegnif")'/>
		<xsl:choose>
			<xsl:when test='boolean($show-image) and ((substring($lurl, $length - 4) = ".jpeg") or (substring($lurl, $length - 3) = ".jpg") or (substring($lurl, $length - 3) = ".png") or (substring($lurl, $length - 3) = ".gif"))'>
				<img src='{$url}' alt='{$url}'/>
			</xsl:when>
			<xsl:otherwise>
				<a href='{$url}'><xsl:value-of select='$url'/></a>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template name='ansi-escape'>
		<xsl:param name='content'/>
		<xsl:param name='fgcolor'/>
		<xsl:param name='bgcolor'/>
		<xsl:param name='ishl'/>
		<xsl:choose>
			<xsl:when test='contains($content, ">1b")'>
				<xsl:choose>
					<xsl:when test='$fgcolor = 37'>
						<xsl:value-of select='substring-before($content, ">1b")'/>
					</xsl:when>
					<xsl:otherwise>
						<span class='a0{$fgcolor}'>
							<xsl:value-of select='substring-before($content, ">1b")'/>
						</span>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:variable name='second' select='substring-after($content, ">1b")'/>
				<xsl:variable name='first-alpha'>
					<xsl:call-template name='not-str-token'>
						<xsl:with-param name='str' select='$second'/>
						<xsl:with-param name='delim'>0123456789;[</xsl:with-param>
						<xsl:with-param name='start' select='1'/>
					</xsl:call-template>
				</xsl:variable>
				<xsl:if test='string-length($first-alpha)'>
					<xsl:variable name='code'>
						<xsl:choose>
							<xsl:when test='substring($second, 1, 1) = "[" and substring($second, $first-alpha, 1) = "m"'>
								<xsl:value-of select='substring($second, 2, $first-alpha - 2)'/>
							</xsl:when>
							<xsl:otherwise></xsl:otherwise>
						</xsl:choose>
					</xsl:variable>
					<xsl:variable name='fgc'>
						<xsl:call-template name='get-color'>
							<xsl:with-param name='code' select='$code'/>
							<xsl:with-param name='param-name'>fgcolor</xsl:with-param>
							<xsl:with-param name='current-value' select='$fgcolor'/>
						</xsl:call-template>
					</xsl:variable>
					<xsl:variable name='bgc'>
						<xsl:choose>
							<xsl:when test='$bgcolor = "ignore"'>ignore</xsl:when>
							<xsl:otherwise>
								<xsl:call-template name='get-color'>
									<xsl:with-param name='code' select='$code'/>
									<xsl:with-param name='param-name'>bgcolor</xsl:with-param>
									<xsl:with-param name='current-value' select='$bgcolor'/>
								</xsl:call-template>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:variable>
					<xsl:variable name='hl'>
						<xsl:call-template name='get-color'>
							<xsl:with-param name='code' select='$code'/>
							<xsl:with-param name='param-name'>highlight</xsl:with-param>
							<xsl:with-param name='current-value' select='$ishl'/>
						</xsl:call-template>
					</xsl:variable>
					<xsl:variable name='last' select='substring($second, $first-alpha + 1)'/>
					<xsl:variable name='text'>
						<xsl:choose>
							<xsl:when test='contains($last, ">1b")'>
								<xsl:value-of select='substring-before($last, ">1b")'/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select='$last'/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:variable>
					<span>
						<xsl:attribute name='class'>a<xsl:value-of select='$hl'/><xsl:value-of select='$fgc'/><xsl:if test='not($bgc = "ignore")'><xsl:text> </xsl:text>a<xsl:value-of select='$bgc'/></xsl:if></xsl:attribute>
						<xsl:value-of select='$text'/>
					</span>
					<xsl:variable name='next' select='substring($last, string-length($text) + 1)'/>
					<xsl:if test='$next'>
						<xsl:call-template name='ansi-escape'>
							<xsl:with-param name='content' select='$next'/>
							<xsl:with-param name='fgcolor' select='$fgc'/>
							<xsl:with-param name='bgcolor' select='$bgc'/>
							<xsl:with-param name='ishl' select='$hl'/>
						</xsl:call-template>
					</xsl:if>
				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test='$fgcolor = 37'>
						<xsl:value-of select='$content'/>
					</xsl:when>
					<xsl:otherwise>
						<span class='a0{$fgcolor}'>
							<xsl:value-of select='$content'/>
						</span>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template name='not-str-token'>
		<xsl:param name='str'/>
		<xsl:param name='delim'/>
		<xsl:param name='start'/>
		<xsl:variable name='char' select='substring($str, $start, 1)'/>
		<xsl:if test='$char'>
			<xsl:choose>
				<xsl:when test='not(contains($delim, $char))'>
					<xsl:value-of select='$start'/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:call-template name='not-str-token'>
						<xsl:with-param name='str' select='$str'/>
						<xsl:with-param name='delim' select='$delim'/>
						<xsl:with-param name='start' select='$start + 1'/>
					</xsl:call-template>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:template>

	<xsl:template name='get-color'>
		<xsl:param name='code'/>
		<xsl:param name='param-name'/>
		<xsl:param name='current-value'/>
		<xsl:variable name='first'>
			<xsl:choose>
				<xsl:when test='contains($code, ";")'>
					<xsl:value-of select='substring-before($code, ";")'/>
				</xsl:when>
				<xsl:otherwise><xsl:value-of select='$code'/></xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name='next' select='substring-after($code, ";")'/>
		<xsl:variable name='new-value'>
			<xsl:if test='$first'>
				<xsl:choose>
					<xsl:when test='$param-name = "fgcolor" and $first >= 30 and $first &lt;= 37'>
						<xsl:value-of select='$first'/>
					</xsl:when>
					<xsl:when test='$param-name = "bgcolor" and $first >= 40 and $first &lt;= 47'>
						<xsl:value-of select='$first'/>
					</xsl:when>
					<xsl:when test='$param-name = "highlight" and (($first = 0) or ($first = 1))'>
						<xsl:value-of select='$first'/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select='$current-value'/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:if>
		</xsl:variable>
		<xsl:choose>
			<xsl:when test='$next'>
				<xsl:call-template name='get-color'>
					<xsl:with-param name='code' select='$next'/>
					<xsl:with-param name='param-name' select='$param-name'/>
					<xsl:with-param name='current-value' select='$new-value'/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test='not($next) and string-length($first) = 0'>
				<xsl:choose>
					<xsl:when test='$param-name = "fgcolor"'>37</xsl:when>
					<xsl:when test='$param-name = "bgcolor"'>40</xsl:when>
					<xsl:when test='$param-name = "highlight"'>0</xsl:when>
					<xsl:otherwise></xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select='$new-value'/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template name="show-quoted">
		<xsl:param name='content'/>
		<xsl:choose>
			<xsl:when test='contains($content, "&#10;")'>
				<xsl:variable name='before' select='substring-before($content, "&#10;")'/>
				<xsl:variable name='rest' select='substring-after($content, "&#10;")'/>
				<xsl:call-template name='remove-ansi'>
					<xsl:with-param name='str' select='$before'/>
				</xsl:call-template>
				<xsl:text>&#x0d;&#x0a;</xsl:text>
				<xsl:if test='$rest'>
					<xsl:call-template name='show-quoted'>
						<xsl:with-param name='content' select='$rest'/>
					</xsl:call-template>
				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name='remove-ansi'>
					<xsl:with-param name='str' select='$content'/>
				</xsl:call-template>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name='remove-ansi'>
		<xsl:param name='str'/>
		<xsl:choose>
			<xsl:when test='contains($str, ">1b[")'>
				<xsl:value-of select='substring-before($str, ">1b[")'/>
				<xsl:variable name='after' select='substring-after($str, ">1b[")'/>
				<xsl:if test='$after'>
					<xsl:variable name='first-alpha'>
						<xsl:call-template name='not-str-token'>
							<xsl:with-param name='str' select='$after'/>
							<xsl:with-param name='delim'>0123456789;</xsl:with-param>
							<xsl:with-param name='start' select='1'/>
						</xsl:call-template>
					</xsl:variable>
					<xsl:if test='string-length($first-alpha)'>
						<xsl:call-template name='remove-ansi'>
							<xsl:with-param name='str' select='substring($after, $first-alpha + 1)'/>
						</xsl:call-template>
					</xsl:if>
				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select='$str'/>
			</xsl:otherwise>		
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>
