<?xml version="1.0" encoding="gb2312"?>
<xsl:stylesheet version="2.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/1999/xhtml">
	<xsl:template name="timeconvert">
		<xsl:param name='time' />
		<xsl:value-of select="concat(substring($time, 6, 5), ' ', substring($time, 12, 5))" />
	</xsl:template>
</xsl:stylesheet>