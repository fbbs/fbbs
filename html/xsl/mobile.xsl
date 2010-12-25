<?xml version='1.0' encoding='gb18030'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns='http://www.w3.org/1999/xhtml'>
<xsl:import href='showpost.xsl'/>
<xsl:output method='html' encoding='gb18030' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd'/>

<xsl:template name='bbsname'>日月光华</xsl:template>

<xsl:template name='timeconvert'>
	<xsl:param name='time'/>
	<xsl:value-of select='concat(substring($time, 1, 10), " ", substring($time, 12, 5))'/>
</xsl:template>

<xsl:template name='time-conv-short'>
	<xsl:param name='time'/>
	<xsl:value-of select='concat(substring($time, 6, 5), " ", substring($time, 12, 5))'/>
</xsl:template>

<xsl:template name='navigation'>
	<xsl:param name='session'/>
	<xsl:variable name='user' select='$session/u'/>
	<xsl:variable name='bbsname'><xsl:call-template name='bbsname'/></xsl:variable>
	<div id='hd'>
		<xsl:if test='$user != ""'><a href='qry?u={$user}'><xsl:value-of select='$user'/></a> <a id='fave' href='#' onclick='return expandFav();'>收藏▽</a><a href='#' id='favc' onclick='return collapseFav();'>收起△</a>|</xsl:if>
		<xsl:if test='$user = ""'><a href='login'>登录</a>|</xsl:if>
		<a href='sec'>首页</a>|<a href='top10'>十大</a><xsl:if test='contains($session/p, "l")'>|<a href='mail'>信件</a>|<a href='logout'>注销</a></xsl:if>
	</div>
	<div id='fav'><xsl:for-each select='$session/f/b'><xsl:sort select='translate(., "abcdefghijklmnopqrstuvwxyz","ABCDEFGHIJKLMNOPQRSTUVWXYZ")' order='ascending'/>
	<a><xsl:if test='@r'><xsl:attribute name='class'>rd</xsl:attribute></xsl:if><xsl:attribute name='href'><xsl:value-of select='$session/@m'/>doc?<xsl:choose><xsl:when test='@bid'>bid=<xsl:value-of select='@bid'/></xsl:when><xsl:otherwise>board=<xsl:value-of select='.'/></xsl:otherwise></xsl:choose></xsl:attribute><xsl:value-of select='.'/></a>
	</xsl:for-each></div>
</xsl:template>

<xsl:template name='foot'>
	<div id='ft'><xsl:call-template name='bbsname'/> &#169;1996-2010</div>
</xsl:template>

<xsl:template name='include-css'>
	<link rel='stylesheet' type='text/css' href='../css/mobile.css'/>
</xsl:template>

<xsl:template name='include-js'>
<script src='../js/mobile.js' defer='defer'></script>
</xsl:template>

<xsl:template name='page-title'>
	<xsl:variable name='cgi' select='local-name(node()[2])'/>
	<xsl:choose>
		<xsl:when test='bbssec'>分区列表</xsl:when>
		<xsl:when test='bbsboa'><xsl:choose><xsl:when test='bbsboa/@dir'>版面目录</xsl:when><xsl:otherwise>分类讨论区</xsl:otherwise></xsl:choose></xsl:when>
		<xsl:when test='bbsall'>全部讨论区</xsl:when>
		<xsl:when test='bbssel'>选择讨论区</xsl:when>
		<xsl:when test='bbsdoc'><xsl:value-of select='bbsdoc/brd/@desc'/></xsl:when>
		<xsl:when test='bbscon'>文章阅读</xsl:when>
		<xsl:when test='bbstcon'>同主题文章阅读</xsl:when>
		<xsl:when test='bbsqry'>查询网友</xsl:when>
		<xsl:when test='bbspst'><xsl:choose><xsl:when test='bbspst/@edit="0"'>发表</xsl:when><xsl:otherwise>修改</xsl:otherwise></xsl:choose>文章</xsl:when>
		<xsl:when test='bbstop10'>本日十大</xsl:when>
		<xsl:when test='bbsbfind'>版内文章查询</xsl:when>
		<xsl:when test='bbsnewmail'>阅览新信件</xsl:when>
		<xsl:when test='bbsmail'>信件列表</xsl:when>
		<xsl:when test='bbsmailcon'>信件阅读</xsl:when>
		<xsl:when test='bbspstmail'>寄语信鸽</xsl:when>
		<xsl:when test='bbs0an'>精华区浏览</xsl:when>
		<xsl:when test='bbsanc'>精华区文章阅读</xsl:when>
		<xsl:when test='bbsfwd'>转寄文章</xsl:when>
		<xsl:when test='bbsccc'>转载文章</xsl:when>
		<xsl:when test='bbsfall'>设定关注名单</xsl:when>
		<xsl:when test='bbsfadd'>增加关注网友</xsl:when>
		<xsl:when test='bbsovr'>在线关注网友</xsl:when>
		<xsl:when test='bbsfav'>收藏夹</xsl:when>
		<xsl:when test='bbsmybrd'>设定收藏夹</xsl:when>
		<xsl:when test='bbseufile'><xsl:value-of select='bbseufile/@desc'/></xsl:when>
		<xsl:when test='bbsmybrd'>个人资料</xsl:when>
		<xsl:when test='bbspwd'>修改密码</xsl:when>
		<xsl:when test='bbsnot'>进版画面</xsl:when>
		<xsl:otherwise></xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match='/'>
	<html>
		<head>
			<meta http-equiv="content-type" content="text/html; charset=gb2312"/>
			<meta name="viewport" content="width=device-width"/>
			<title><xsl:call-template name='page-title'/> - <xsl:call-template name='bbsname'/>手机版</title>
			<xsl:call-template name='include-css'/>
			<xsl:call-template name='include-js'/>
		</head>
		<body>
			<a name='top'/>
			<xsl:call-template name='navigation'><xsl:with-param name='session' select='node()[2]/session'/></xsl:call-template>
			<div id='main'><xsl:apply-templates/></div>
			<xsl:call-template name='foot'/>
		</body>
	</html>
</xsl:template>

<xsl:template match='bbssec'>
	<ul class='sec'><xsl:for-each select='sec'>
		<li><a href='boa?s={@id}'><xsl:value-of select='@id'/>&#160;<xsl:value-of select='@desc'/></a></li>
	</xsl:for-each></ul>
</xsl:template>

<xsl:template name='bbsdoc-link'>
	<xsl:if test='brd/@start > 1'>
		<xsl:variable name='prev'><xsl:choose><xsl:when test='brd/@start - brd/@page &lt; 1'>1</xsl:when><xsl:otherwise><xsl:value-of select='brd/@start - brd/@page'/></xsl:otherwise></xsl:choose></xsl:variable>
		<a href='{brd/@link}doc?bid={brd/@bid}&amp;start={$prev}'>[前页]</a>
	</xsl:if>
	<xsl:if test='brd/@total > brd/@start + brd/@page - 1'>
		<xsl:variable name='next'><xsl:value-of select='brd/@start + brd/@page'/></xsl:variable>
		<a href='{brd/@link}doc?bid={brd/@bid}&amp;start={$next}'>[后页]</a>
	</xsl:if>
</xsl:template>

<xsl:template match='bbsdoc'>
	<h2><a href='{brd/@link}doc?bid={brd/@bid}'><xsl:value-of select='brd/@desc'/>[<xsl:value-of select='brd/@title'/>]</a><xsl:if test='brd/@link = "g"'>[文摘]</xsl:if><xsl:if test='brd/@link = "t"'>[主题]</xsl:if></h2>
	<div class='nav'>
		<xsl:choose>
			<xsl:when test='brd/@link="t"'><a href='doc?bid={brd/@bid}'>[传统]</a></xsl:when>
			<xsl:otherwise><a href='tdoc?bid={brd/@bid}'>[主题]</a></xsl:otherwise>
		</xsl:choose>
		<a href='pst?bid={brd/@bid}'>[发文]</a>
		<xsl:call-template name='bbsdoc-link'></xsl:call-template>
		<a href='0an?bid={brd/@bid}'>[精华]</a>
	</div>
	<ul class='po'>
		<xsl:for-each select='po'><xsl:sort select='@time' order='descending'/><xsl:if test='not(@sticky)'><li>
			<xsl:if test='contains(" gmbw", @m)'><xsl:attribute name='class'>rd</xsl:attribute></xsl:if>
			<p><a href='{../brd/@link}con?bid={../brd/@bid}&amp;f={@id}'>
				<xsl:variable name='text'><xsl:choose><xsl:when test='substring(., 1, 4) = "Re: "'><xsl:value-of select='substring(., 5)'/></xsl:when><xsl:otherwise><xsl:value-of select='.'/></xsl:otherwise></xsl:choose></xsl:variable>
				<xsl:if test='substring(., 1, 4) = "Re: "'>Re: </xsl:if>
				<xsl:call-template name='ansi-escape'>
					<xsl:with-param name='content'><xsl:value-of select='$text'/></xsl:with-param>
					<xsl:with-param name='fgcolor'>37</xsl:with-param>
					<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
					<xsl:with-param name='ishl'>0</xsl:with-param>
				</xsl:call-template>
			</a></p>
			<p><a class='owner' href='qry?u={@owner}'><xsl:value-of select='@owner'/></a><xsl:text> </xsl:text><span class='time'><xsl:call-template name='time-conv-short'><xsl:with-param name='time' select='@time'/></xsl:call-template></span></p>
		</li></xsl:if></xsl:for-each>
	</ul>
	<div class='nav'>
		<xsl:call-template name='bbsdoc-link'></xsl:call-template>
	</div>
</xsl:template>

<xsl:template match='bbscon'>
<div class='post'>
	<div class='pmain'><xsl:call-template name='simple-post'><xsl:with-param name='content' select='po'/></xsl:call-template></div>
	<div class='plink'>
		<xsl:variable name='param'>bid=<xsl:value-of select='@bid'/>&amp;f=<xsl:value-of select='po/@fid'/></xsl:variable>
		<xsl:if test='@link = "con"'><a href='pst?{$param}'>[回复]</a></xsl:if>
		<a href='edit?{$param}'>[修改]</a>
	</div>
	<div class='nav'>
		<a href='doc?bid={@bid}'>[版面]</a>
		<xsl:variable name='baseurl'>con?bid=<xsl:value-of select='@bid'/>&amp;f=<xsl:value-of select='po/@fid'/>&amp;a=</xsl:variable>
		<xsl:if test='not(po/@sticky)'><a href='{$baseurl}p'>[上篇]</a>
		<a href='{$baseurl}n'>[下篇]</a>
		<xsl:if test='po/@reid != f'><a href='{$baseurl}b'>[上楼]</a></xsl:if>
		<a href='{$baseurl}a'>[下楼]</a>
		<xsl:if test='po/@gid'><a href='con?bid={@bid}&amp;f={po/@gid}'>[顶楼]</a></xsl:if>
		<xsl:variable name='gid'><xsl:choose><xsl:when test='po/@gid'><xsl:value-of select='po/@gid'/></xsl:when><xsl:otherwise><xsl:value-of select='po/@fid'/></xsl:otherwise></xsl:choose></xsl:variable>
		<a href='tcon?bid={@bid}&amp;f={$gid}'>[展开]</a>
		<a href='tcon?bid={@bid}&amp;g={$gid}&amp;f={po/@fid}&amp;a=n'>[后展]</a></xsl:if>
	</div>
</div>
</xsl:template>

<xsl:template match='bbstcon'>
	<xsl:for-each select='po'>
		<div class='post'>
			<div class='pmain'><xsl:call-template name='simple-post'><xsl:with-param name='content' select='.'/></xsl:call-template></div>
			<div class='nav'>
				<xsl:variable name='first'><xsl:value-of select='../po[1]/@fid'/></xsl:variable>
				<xsl:variable name='last'><xsl:value-of select='../po[last()]/@fid'/></xsl:variable>
				<xsl:if test='count(../po) = ../@page'><a href='tcon?bid={../@bid}&amp;g={../@gid}&amp;f={$last}&amp;a=n'>[下一页]</a></xsl:if>
				<xsl:if test='$first != ../@gid'><a href='tcon?bid={../@bid}&amp;g={../@gid}&amp;f={$first}&amp;a=p'>[上一页]</a></xsl:if>
				<a href='pst?bid={../@bid}&amp;f={@fid}'>[回复]</a>
				<a href='ccc?bid={../@bid}&amp;f={@fid}'>[转载]</a>
				<a href='tdoc?bid={../@bid}'>[版面]</a>
				<a href='con?bid={../@bid}&amp;f={@fid}'>[链接]</a>
				<a href='gdoc?bid={../@bid}'>[文摘]</a>
				<a href='qry?u={@owner}'>[作者: <xsl:value-of select='@owner'/>]</a>
			</div>
		</div>
	</xsl:for-each>
</xsl:template>

<xsl:template match='bbsboa'>
	<h2><xsl:value-of select='@title'/></h2>
	<ul><xsl:for-each select='brd'><xsl:sort select='@title'/><li class='brd'>
		<a><xsl:choose>
			<xsl:when test='@dir="1"'><xsl:attribute name='href'>boa?board=<xsl:value-of select='@title'/></xsl:attribute></xsl:when>
			<xsl:otherwise><xsl:attribute name='href'><xsl:value-of select='../session/@m'/>doc?board=<xsl:value-of select='@title'/></xsl:attribute></xsl:otherwise>
		</xsl:choose>
		<xsl:choose><xsl:when test='@read="0"'>◇</xsl:when><xsl:otherwise>◆</xsl:otherwise></xsl:choose>
		<xsl:choose><xsl:when test='@dir="1"'>[目录]</xsl:when><xsl:otherwise><xsl:value-of select='@cate'/></xsl:otherwise></xsl:choose>
		<xsl:value-of select='@desc'/></a><span class='tt'>[<xsl:value-of select='@title'/>]</span>
	</li></xsl:for-each></ul>
</xsl:template>

<xsl:template match='bbspst'>
	<p>版面：<xsl:value-of select='@brd'/></p>
	<form id='postform' name='postform' method='post' action='snd?bid={@bid}&amp;f={po/@f}&amp;e={@edit}'>
		<input type='hidden' id='brd' value='{@brd}'></input>
		<p>标题：<xsl:choose>
		<xsl:when test='@edit=0'><input class='binput' type='text' name='title' size='27' maxlength='50'>
			<xsl:variable name='retitle'>
				<xsl:choose>
					<xsl:when test='substring(t, 1, 4) = "Re: "'><xsl:value-of select='t'/></xsl:when>
					<xsl:when test='not(t)'></xsl:when>
					<xsl:otherwise><xsl:value-of select='concat("Re: ", t)'/></xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:attribute name='value'>
				<xsl:call-template name='remove-ansi'>
					<xsl:with-param name='str' select='$retitle'/>
				</xsl:call-template>
			</xsl:attribute>
		</input></xsl:when>
		<xsl:otherwise><xsl:call-template name='remove-ansi'><xsl:with-param name='str' select='t'/></xsl:call-template></xsl:otherwise>
		</xsl:choose></p>
		<p>签名档: <input type='radio' name='sig' value='1' checked='checked'/>1 <input type='radio' name='sig' value='2'/>2 <input type='radio' name='sig' value='3'/>3 <input type='radio' name='sig' value='4'/>4 <input type='radio' name='sig' value='5'/>5 <input type='radio' name='sig' value='6'/>6</p>
		<p><textarea class='binput' name='text' rows='10' cols='27' wrap='virtual'>
			<xsl:if test='@edit=0'><xsl:text> &#x0d;&#x0a;</xsl:text></xsl:if>
			<xsl:call-template name='show-quoted'>
				<xsl:with-param name='content' select='po'/>
			</xsl:call-template>
		</textarea></p>
		<input type='submit' value='发表' id='btnPost' size='10'/>
		<input type='reset' value='复原' size='10'/>
		<xsl:if test='@edit="0" and @att!=0'><input type='button' name='attach' value='上传附件' onclick='return preUpload() '/></xsl:if>
	</form>
	<xsl:choose>
		<xsl:when test='not(t)'><script type='text/javascript' defer='defer'>addLoadEvent(function(){document.postform.title.focus();})</script></xsl:when>
		<xsl:otherwise><script type='text/javascript' defer='defer'>addLoadEvent(function() {var text = document.postform.text; text.selectionStart = 0; text.selectionEnd = 1; text.focus();})</script></xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match='bbsqry'>
	<form action='qry' method='get'><label for='u'>帐号：</label><input type='text' name='u' maxlength='12' size='12'/><input type='submit' value='查询'/></form>
	<xsl:choose><xsl:when test='@login'><div class='uqry'>
		<div class='nav'><xsl:call-template name='qry-linkbar'/></div>
		<div class='umain' id='uinfo'>
		<p><strong><xsl:value-of select='@id'/></strong> （<strong><xsl:value-of select='nick'/></strong>） <xsl:call-template name='show-horo'/></p>
		<p>最近上站:【<span class='a132'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@lastlogin'/></xsl:call-template></span>】</p>
		<p>地址:【<span class='a132'><xsl:value-of select='ip'/></span>】</p>
		<xsl:if test='logout'><p>离站于:【<span class='a132'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='logout'/></xsl:call-template></span>】</p></xsl:if>
		<p>文章数:【<span class='a132'><xsl:value-of select='@post'/></span>】 生命力:【<span class='a132'><xsl:value-of select='@hp'/></span>】</p> 
		<p>表现值:【<span class='a133'><xsl:value-of select='@perf'/></span>】</p>
		<p>经验值:【<xsl:value-of select='@level * 10 + @repeat'/>/60】</p>
		<p>身份: <xsl:call-template name='ansi-escape'><xsl:with-param name='content'><xsl:value-of select='ident'/></xsl:with-param><xsl:with-param name='fgcolor'>37</xsl:with-param><xsl:with-param name='bgcolor'>ignore</xsl:with-param><xsl:with-param name='ishl'>0</xsl:with-param></xsl:call-template></p></div>
		<xsl:if test='st'><div class='nav'>目前状态</div>
		<div class='umain'><xsl:for-each select='st'><p><xsl:value-of select='@desc'/><xsl:if test='@idle!=0'>[发呆<xsl:value-of select='@idle'/>分钟]</xsl:if><xsl:if test='@web=1'>（web）</xsl:if><xsl:if test='@vis=0'>（隐）</xsl:if></p></xsl:for-each></div></xsl:if>
		<div class='nav'>个人说明档如下</div>
		<div class='usmd'><xsl:call-template name='simple-post'><xsl:with-param name='content' select='smd'/></xsl:call-template></div>
	</div></xsl:when><xsl:otherwise><xsl:if test='@id!=""'><p>没有找到用户【<xsl:value-of select='@id'/>】</p></xsl:if></xsl:otherwise></xsl:choose>
</xsl:template>

<xsl:template name='show-horo'>
	<xsl:if test='@horo'>
		<xsl:variable name='color'><xsl:choose><xsl:when test='@gender = "M"'>a136</xsl:when><xsl:when test='@gender = "F"'>a135</xsl:when><xsl:otherwise>a132</xsl:otherwise></xsl:choose></xsl:variable>
		<span>【</span><span class='{$color}'><xsl:value-of select='@horo'/></span><span>】</span>
	</xsl:if>
</xsl:template>

<xsl:template name='qry-linkbar'>
	<a href='pstmail?recv={@id}'>[发信]</a>
</xsl:template>

<xsl:template match='bbstop10'>
	<h2>24小时十大热门话题</h2>
	<ol class='po'><xsl:for-each select='top'><li>
		<p><a href='tcon?board={@board}&amp;f={@gid}'><xsl:call-template name='ansi-escape'><xsl:with-param name='content' select='.'/><xsl:with-param name='fgcolor'>37</xsl:with-param><xsl:with-param name='bgcolor'>ignore</xsl:with-param><xsl:with-param name='ishl'>0</xsl:with-param></xsl:call-template></a></p>
		<p class='ainfo'><a href='{../session/@m}doc?board={@board}'><xsl:value-of select='@board'/>版</a> - <a href='qry?u={@owner}'><xsl:value-of select='@owner'/></a> - <xsl:value-of select='@count'/>篇</p>
	</li></xsl:for-each></ol>
</xsl:template>

<xsl:template match='bbs0an'>
	<xsl:call-template name='anc-navbar'/>
	<ol><xsl:for-each select='ent'><li>
		<p><xsl:choose>
			<xsl:when test='@t = "d"'>[目录]<a class='ptitle' href='0an?path={../@path}{@path}'><xsl:value-of select='.'/></a></xsl:when>
			<xsl:when test='@t = "f"'><a class='ptitle' href='anc?path={../@path}{@path}'><xsl:value-of select='.'/></a></xsl:when>
			<xsl:otherwise>[错误]<xsl:value-of select='@t'/></xsl:otherwise>
		</xsl:choose></p>
		<p><span class='time'><xsl:if test='@t != "e"'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@time'/></xsl:call-template></xsl:if></span></p>
	</li></xsl:for-each></ol>
	<xsl:if test='not(ent)'><p>&lt;&lt;目前没有文章&gt;&gt;</p></xsl:if>
	<xsl:call-template name='anc-navbar'/>
</xsl:template>

<xsl:template match='bbsanc'>
	<h3><xsl:if test='@brd'><xsl:value-of select='@brd'/>版 - </xsl:if>精华区文章阅读</h3>
	<xsl:call-template name='anc-navbar'/>
	<div class='po'><xsl:call-template name='simple-post'><xsl:with-param name='content' select='po'/></xsl:call-template></div>
	<xsl:call-template name='anc-navbar'/>
</xsl:template>

<xsl:template name='anc-navbar'>
	<xsl:if test='@brd'><div class='nav'><a href='{session/@m}doc?board={@brd}'>[版面]</a></div></xsl:if>
</xsl:template>

<xsl:template match='bbsmail'>
	<h2>信件列表</h2>
	<ol class='po'><xsl:for-each select='mail'><xsl:sort select='@date' order='descending'/><li>
		<p><a href='mailcon?f={@name}&amp;n={../@start+count(../mail)-position()}'><xsl:call-template name='ansi-escape'><xsl:with-param name='content'><xsl:value-of select='.'/></xsl:with-param><xsl:with-param name='fgcolor'>37</xsl:with-param><xsl:with-param name='bgcolor'>ignore</xsl:with-param><xsl:with-param name='ishl'>0</xsl:with-param></xsl:call-template></a></p>
		<p><a class='owner' href='qry?u={@from}'><xsl:value-of select='@from'/></a><xsl:text> </xsl:text><span class='time'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@date'/></xsl:call-template></span></p>
	</li></xsl:for-each></ol>
	<div class='nav'>
		<xsl:if test='@start &gt; 1'>
			<xsl:variable name='prev'><xsl:choose><xsl:when test='@start - @page &lt; 1'>1</xsl:when><xsl:otherwise><xsl:value-of select='@start - @page'/></xsl:otherwise></xsl:choose></xsl:variable>
			<a href='mail?start={$prev}'>[上页]</a>
		</xsl:if>
		<xsl:if test='@total &gt; @start + @page - 1'>
			<xsl:variable name='next'><xsl:value-of select='@start + @page'/></xsl:variable>
			<a href='mail?start={$next}'>[下页]</a>
		</xsl:if>
	</div>
</xsl:template>

<xsl:template match='bbsmailcon'>
	<div class='nav'><xsl:call-template name='mailcon-navbar'/></div>
	<div class='po'><xsl:call-template name='simple-post'><xsl:with-param name='content' select='mail'/></xsl:call-template></div>
	<div class='nav'><xsl:call-template name='mailcon-navbar'/></div>
</xsl:template>

<xsl:template name='mailcon-navbar'>
	<a href='pstmail?n={mail/@n}'>[回信]</a>
	<a onclick='return confirm("您真的要删除这封信吗？")' href='delmail?f={mail/@f}'>[删除]</a>
	<xsl:if test='@prev'><a href='mailcon?f={@prev}&amp;n={mail/@n-1}'>[上封]</a></xsl:if>
	<xsl:if test='@next'><a href='mailcon?f={@next}&amp;n={mail/@n+1}'>[下封]</a></xsl:if>
	<a href='mail'>[列表]</a>
</xsl:template>

<xsl:template match='bbspstmail'>
	<form id='postform' name='postform' method='post' action='sndmail'>
		<input type='hidden' name='ref' value='{@ref}'></input>
		<p><label for='recv'>收信人: </label><input class='binput' type='text' name='recv' size='15' maxlength='15' value='{@recv}'></input></p>
		<p><label for='title'>标题: </label>
		<input class='binput' type='text' name='title' size='25' maxlength='40'>
			<xsl:variable name='retitle'>
				<xsl:choose>
					<xsl:when test='substring(t, 1, 4) = "Re: "'><xsl:value-of select='t'/></xsl:when>
					<xsl:when test='not(t)'></xsl:when>
					<xsl:otherwise><xsl:value-of select='concat("Re: ", t)'/></xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:attribute name='value'>
				<xsl:call-template name='remove-ansi'>
					<xsl:with-param name='str' select='$retitle'/>
				</xsl:call-template>
			</xsl:attribute>
		</input></p>
		<p>备份给自己 <input type='checkbox' name='backup' value='backup'/></p>
		<p><textarea class='binput' name='text' rows='10' cols='30' wrap='virtual'>
			<xsl:text>&#x0d;&#x0a;</xsl:text>
			<xsl:call-template name='show-quoted'>
				<xsl:with-param name='content' select='m'/>
			</xsl:call-template>
		</textarea></p>
		<input type='submit' value='寄出' id='btnPost' size='10'/>
		<input type='reset' value='重置'/>
	</form>
</xsl:template>

</xsl:stylesheet>
