<?xml version="1.0" encoding="GB2312"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
<HTML> 
<HEAD> 
<TITLE><xsl:value-of select="rss/channel/title"></xsl:value-of></TITLE> 
<STYLE>
	BODY {
		font-family: Tahoma, Verdana, "MS Sans Serif", "SimSun";
		font-size: 12px;
		background-color: #FFFFFF;
	}
	A {font-size: 12px; TEXT-DECORATION: none;} 
	A:link {COLOR: #666} 
	A:visited {COLOR: #000000} 
	A:active {COLOR: #000000} 
	A:hover {COLOR: #000000} 

	.title {
		font-family: SimHei;
		font-size: 28px; 
		padding-bottom: 10px; 
		text-align: left;
	}
	.pubdate {
		font-family: Tahoma, Verdana;
		font-size: 9pt;
		text-align: right;
	}
	.itemtitle {
		font-size: 15px; 
		font-family: Verdana;
	}
	.itemdate {
		font-size: 12px; 
		color: #000;
		text-align: right;
	}
	.content_text {
		font-size: 12px;
		background-color: #DDD;
	}
	.footer {
		font-size: 12px;
		color:#aaa;
	}
</STYLE> 
</HEAD> 

<BODY> 
<CENTER> 
        <DIV class="title">
          <xsl:value-of select="rss/channel/title"></xsl:value-of>
        </DIV> 
        <DIV class="pubdate">
          <xsl:value-of select="rss/channel/pubDate"></xsl:value-of>
        </DIV> 
	    <hr color="000" />
        <xsl:apply-templates select="rss/channel/item"/> 
        <P class="footer"> 
          <xsl:value-of select="rss/channel/description"></xsl:value-of>
		  <BR/> 
          <xsl:value-of select="rss/channel/link"></xsl:value-of> 
        </P> 
</CENTER>
</BODY></HTML>
</xsl:template>

  <!--Item 总框架-->
  <xsl:template match="item">
	  <BR/>
      <TABLE width="85%" border="1" cellpadding="8" cellspacing="0">
	  <TR>
		  <TD height="25" class="itemtitle">
			  <a href="{link}" target="_blank" style="TEXT-DECORATION: none">
				<xsl:value-of select="title"/>
			  </a>
		  <div class="itemdate">
				<xsl:value-of select="substring(pubDate,5)"/>
		  </div>
		  </TD>	
	  </TR>
	  <xsl:apply-templates select="description"/>
      </TABLE>
      <BR/>
  </xsl:template>

  <!--Item内容模板-->
  <xsl:template match="description">
      <TR>
        <TD class="content_text">
		  <div id="content">
		  <xsl:value-of select="." disable-output-escaping="yes"/>
		  <!-- disable-output-escaping is IE only -->
		  </div>
		  <xsl:if test="system-property('xsl:vendor')='Transformiix'">
		  <!-- these javascript code is used for non-IE -->
			  <script language="javascript">
				var el = document.getElementById("content");
				el.innerHTML = el.textContent;
			  </script>
		  </xsl:if>
		</TD>
	  </TR>
    </xsl:template>
  </xsl:stylesheet>
