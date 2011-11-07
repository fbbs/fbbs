<?xml version='1.0' encoding='gb2312'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
<xsl:import href='showpost.xsl'/>
<xsl:output method='html' encoding='gb2312' indent='no' doctype-public='-//W3C//DTD HTML 4.01//EN' doctype-system='http://www.w3.org/TR/html4/strict.dtd'/>

<xsl:variable name='bbsname'>日月光华</xsl:variable>

<xsl:template name='timeconvert'>
	<xsl:param name='time'/>
	<xsl:value-of select='concat(substring($time, 1, 10), " ", substring($time, 12, 5))'/>
</xsl:template>

<xsl:template name="splitbm">
	<xsl:param name='names'/>
	<xsl:param name='isdir'/>
	<xsl:param name='isfirst'/>
	<xsl:variable name='first' select='substring-before($names," ")'/>
	<xsl:variable name='rest' select='substring-after($names," ")'/>
	<xsl:if test='$first'>
		<a href='qry?u={$first}'><xsl:value-of select='$first'/></a>
	</xsl:if>
	<xsl:if test='$rest'>
		<xsl:text>&#160;</xsl:text>
		<xsl:call-template name='splitbm'>
			<xsl:with-param name='names' select='$rest'/>
			<xsl:with-param name='isdir' select='$isdir'/>
			<xsl:with-param name='isfirst' select='0'/>
		</xsl:call-template>
	</xsl:if>
	<xsl:if test='not($rest)'>
		<xsl:if test='$names'>
			<a href='qry?u={$names}'><xsl:value-of select='$names'/></a>
		</xsl:if>
		<xsl:if test="$names=''">
			<xsl:if test="$isdir='0'">诚征版主中</xsl:if>
			<xsl:if test="$isdir!='0'">-</xsl:if>
		</xsl:if>
	</xsl:if>
</xsl:template>

<xsl:template name='navigation'>
	<xsl:param name='session'/>
	<ul id='nav'>
		<li id='navh'><a href='sec'>推荐版面</a></li>
		<xsl:if test='$bbsname="日月光华"'><li id='navb'>
			<a href='#'>分类讨论</a>
			<ul><li><a href='boa?s=0'>0 BBS系统</a></li>
			<li><a href='boa?s=1'>1 复旦大学</a></li>
			<li><a href='boa?s=2'>2 院系风采</a></li>
			<li><a href='boa?s=3'>3 电脑技术</a></li>
			<li><a href='boa?s=4'>4 休闲娱乐</a></li>
			<li><a href='boa?s=5'>5 文学艺术</a></li>
			<li><a href='boa?s=6'>6 体育健身</a></li>
			<li><a href='boa?s=7'>7 感性空间</a></li>
			<li><a href='boa?s=8'>8 新闻信息</a></li>
			<li><a href='boa?s=9'>9 学科学术</a></li>
			<li><a href='boa?s=A'>A 音乐影视</a></li>
			<li><a href='boa?s=B'>B 交易专区</a></li>
			</ul>
		</li></xsl:if>
		<li id='nava'><a href='0an'>本站精华</a></li>
		<li id='navp'><a href='all'>全部讨论</a></li>
		<li id='navt'>
			<a href='#'>统计数据</a>
			<ul>
				<li><a href='top10'>本日十大</a></li>
			</ul>
		</li>
		<xsl:if test='contains($session/p, "l")'>
			<li id='navf'>
				<a href='#'>我的收藏</a>
				<ul>
					<li><a href='fav'>查看详情</a></li>
					<xsl:for-each select='$session/f/b'><xsl:sort select='.'/>
						<li class='sf'><a href='{$session/@m}doc?board={.}'><xsl:value-of select='.'/></a></li>
					</xsl:for-each>
				</ul>
			</li>
			<li id='navc'>
				<a href='#'>鹊桥相会</a>
				<ul>
					<li><a href='ovr'>在线好友</a></li>
				</ul>
			</li>
			<li id='navm'>
				<a href='#'>处理信件</a>
				<ul>
					<li><a href='newmail'>阅览新信</a></li>
					<li><a href='mail'>所有信件</a></li>
					<li><a href='pstmail'>发送信件</a></li>
				</ul>
			</li>
			<li id='navco'>
				<a href='#'>个人设置</a>
				<ul>
					<li><a href='info'>个人资料</a></li>
					<li><a href='plan'>改说明档</a></li>
					<li><a href='sig'>改签名档</a></li>
					<li><a href='pwd'>修改密码</a></li>
					<li><a href='fall'>设定好友</a></li>
				</ul>
			</li>
		</xsl:if>
		<li id='navs'>
			<a href='#'>查找选项</a>
			<ul>
				<li><a href='qry'>查询网友</a></li>
				<li><a href='sel'>查找版面</a></li>
			</ul>
		</li>
	</ul>
</xsl:template>

<xsl:template name='header'>
	<xsl:param name='session'/>
	<xsl:variable name='user' select='string($session/u)'/>
	<div id='hd'>
		<div id='hdright'><xsl:if test='$user != ""'><a id='nave' href='logout'>注销</a></xsl:if></div>
		<xsl:if test='$user != ""'><a id='navu' href='qry?u={$user}'><xsl:value-of select='$user'/></a></xsl:if>
		<xsl:if test='$user = ""'><a id='navl' href='login'>登录</a></xsl:if>
		<a id='navnm' href='newmail'>您有[<span id='navmc'></span>]封新信件</a>
		<a id='navte' href='telnet://bbs.fudan.sh.cn:23'>终端登录</a>
		<div id='loading' class='prompt'><img src='../images/indicator.gif'/> 载入中...</div>
		<div id='error' class='prompt'></div>
		<span id='iewarn'><xsl:comment><![CDATA[[if lt IE 7]><![endif]]]></xsl:comment></span>

		<div id='login-dialog-form' title='登录进站'><form action='login' method='post'>
			<label for='id'>账号</label><input type='text' name='id' class="text ui-widget-content ui-corner-all"/>
			<label for='pw'>密码</label><input type="password" name="pw" value='' class='text ui-widget-content ui-corner-all'/>
		</form></div>
	</div>
</xsl:template>

<xsl:template name='foot'>
	<div id='ft'><a href='#'>[<img src='../images/button/up.gif'/>回页首]</a>&#160;<xsl:value-of select='$bbsname'/> &#169;1996-2011 Powered by <a href='http://code.google.com/p/fbbs/'><strong>fbbs</strong></a></div>
</xsl:template>

<xsl:template name='include-css'>
	<link rel='stylesheet' type='text/css' href='../css/redmond/jquery-ui-1.8.16.custom.css'/>
	<link rel='stylesheet' type='text/css' href='../css/common.css'/>
	<link rel='stylesheet' type='text/css' href='../css/bbs.css?v1416'/>
	<xsl:comment><![CDATA[[if lt IE 7]><link rel='stylesheet' type='text/css' href='../css/ie6fix.css?v1283'/><![endif]]]></xsl:comment>
</xsl:template>

<xsl:template name='include-js'>
	<script src='../js/persist-all-min.js'></script>
	<script src='/js/jquery-1.6.2.min.js'></script>
	<script src='../js/jquery-ui-1.8.16.custom.min.js'></script>
	<script src='../js/bbs.js?v1416' charset='gb2312' defer='defer'></script>
</xsl:template>

<xsl:template name='page-title'>
	<xsl:variable name='cgi' select='local-name(node()[2])'/>
	<xsl:choose>
		<xsl:when test='bbssec'>推荐版面</xsl:when>
		<xsl:when test='bbsboa'><xsl:choose><xsl:when test='bbsboa/@dir'>版面目录</xsl:when><xsl:otherwise>分类讨论区</xsl:otherwise></xsl:choose></xsl:when>
		<xsl:when test='bbsall'>全部讨论区</xsl:when>
		<xsl:when test='bbssel'>选择讨论区</xsl:when>
		<xsl:when test='bbsdoc'><xsl:value-of select='bbsdoc/brd/@desc'/></xsl:when>
		<xsl:when test='forum'><xsl:value-of select='forum/@desc'/></xsl:when>
		<xsl:when test='bbscon'>文章阅读</xsl:when>
		<xsl:when test='bbstcon'>同主题文章阅读</xsl:when>
		<xsl:when test='bbsqry'>查询网友</xsl:when>
		<xsl:when test='bbspst'><xsl:choose><xsl:when test='bbspst/@edit="0"'>发表</xsl:when><xsl:otherwise>修改</xsl:otherwise></xsl:choose>文章</xsl:when>
		<xsl:when test='bbstop10'>本日十大</xsl:when>
		<xsl:when test='bbsbfind'>版内文章查询</xsl:when>
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
		<xsl:when test='bbsinfo'>个人资料</xsl:when>
		<xsl:when test='bbspwd'>修改密码</xsl:when>
		<xsl:when test='bbsnot'>进版画面</xsl:when>
		<xsl:when test='bbsreg'>注册帐号</xsl:when>
		<xsl:otherwise></xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match='/'>
	<html>
		<head>
			<title><xsl:call-template name='page-title'/> - <xsl:value-of select='$bbsname'/></title>
			<meta http-equiv="content-type" content="text/html; charset=gb2312"/>
			<xsl:call-template name='include-css'/>
<xsl:comment><![CDATA[[if lt IE 7]><style>
#hd{position:absolute;top:0;left:144px;margin:0.5em;}
#main{position:absolute;top:32px;left:144px;margin:0.5em 0 0.5em 0}
#ft{display:none;margin:0.5em}
#main td{font-size:12px;height:1}
table.content{width:100%}
th.ptitle,td.ptitle{width:auto}
.content tr{line-height:24px}
table.post{width:100%}
</style><![endif]]]></xsl:comment>
			<xsl:call-template name='include-js'/>
		</head>
		<body>
			<a name='top'/>
			<xsl:call-template name='navigation'><xsl:with-param name='session' select='node()[2]/session'/></xsl:call-template>
				<xsl:call-template name='header'><xsl:with-param name='session' select='node()[2]/session'/></xsl:call-template>
			<div id='main'><xsl:apply-templates/></div>
			<xsl:call-template name='foot'/>
		</body>
		<xsl:comment><![CDATA[[if lt IE 7]><script defer='defer'>$(document).ready(function(){ie6fix();$(window).resize(function(){ie6fix();});});</script><![endif]]]></xsl:comment>
	</html>
</xsl:template>

<xsl:template match='bbssec'>
	<img src='../images/secbanner.jpg'/>
	<xsl:for-each select='sec'>
		<ul class='sec'>
			<li><a href='boa?s={@id}'><xsl:value-of select='@id'/>&#160;<xsl:value-of select='@desc'/></a></li>
			<ul class='brd'>
				<xsl:for-each select='brd'>
					<li><a href='doc?board={@name}'><xsl:value-of select='@desc'/></a></li>
				</xsl:for-each>
			</ul>
		</ul>
	</xsl:for-each>
</xsl:template>

<xsl:template match='bbsboa'>
	<h2><xsl:if test='@icon'><img src='{icon}'/></xsl:if><xsl:value-of select='@title'/></h2>
	<table class='content'>
		<tr><th class='no'>序号</th><th class='read'>未读</th><th class='no'>文章数</th><th class='title'>讨论区名称</th><th class='cate'>类别</th><th class='desc'>中文描述</th><th class='bm'>版主</th></tr>
		<xsl:for-each select='brd'><xsl:sort select='@title'/><tr>
			<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
			<td class='no'><xsl:value-of select='position()'/></td>
			<td class='read'><xsl:choose><xsl:when test='@read="0"'>◇</xsl:when><xsl:otherwise>◆</xsl:otherwise></xsl:choose></td>
			<td class='no'><xsl:value-of select='@count'/></td>
			<td class='title'><a class='title'><xsl:choose>
				<xsl:when test='@dir="1"'><xsl:attribute name='href'>boa?board=<xsl:value-of select='@title'/></xsl:attribute>[ <xsl:value-of select='@title'/> ]</xsl:when>
				<xsl:otherwise><xsl:attribute name='href'>doc?board=<xsl:value-of select='@title'/></xsl:attribute><xsl:value-of select='@title'/></xsl:otherwise>
			</xsl:choose></a></td>
			<td class='cate'><xsl:choose><xsl:when test='@dir="1"'>[目录]</xsl:when><xsl:otherwise><xsl:value-of select='@cate'/></xsl:otherwise></xsl:choose></td>
			<td class='desc'><a class='desc'><xsl:attribute name='href'><xsl:choose><xsl:when test='@dir="1"'>boa</xsl:when><xsl:otherwise>doc</xsl:otherwise></xsl:choose>?board=<xsl:value-of select='@title'/></xsl:attribute><xsl:value-of select='@desc'/></a></td>
			<td class='bm'><xsl:call-template name='splitbm'><xsl:with-param name='names' select='@bm'/><xsl:with-param name='isdir' select='@dir'/><xsl:with-param name='isfirst' select='1'/></xsl:call-template></td>
		</tr></xsl:for-each>
	</table>
</xsl:template>

<xsl:template match='bbsall'>
	<h2>全部讨论区</h2>
	<p>[讨论区数: <xsl:value-of select="count(brd)"/>]</p>
	<table class='content'>
		<tr><th class='no'>序号</th><th class='title'>讨论区名称</th><th class='cate'>类别</th><th class='desc'>中文描述</th><th class='bm'>版主</th></tr>
		<xsl:for-each select='brd'>
			<xsl:sort select="@title"/>
			<tr>
				<xsl:attribute name='class'>
					<xsl:if test='position() mod 2 = 1'>light</xsl:if>
					<xsl:if test='position() mod 2 = 0'>dark</xsl:if>
				</xsl:attribute>
				<td class='no'><xsl:value-of select='position()'/></td>
				<td class='title'><a class='title'><xsl:choose>
					<xsl:when test='@dir="1"'><xsl:attribute name='href'>boa?board=<xsl:value-of select='@title'/></xsl:attribute>[ <xsl:value-of select='@title'/> ]</xsl:when>
					<xsl:otherwise><xsl:attribute name='href'>doc?board=<xsl:value-of select='@title'/></xsl:attribute><xsl:value-of select='@title'/></xsl:otherwise>
				</xsl:choose></a></td>
				<td class='cate'><xsl:choose>
					<xsl:when test='@dir="1"'>[目录]</xsl:when>
					<xsl:otherwise><xsl:value-of select='@cate'/></xsl:otherwise>
				</xsl:choose></td>
				<td class='desc'><a class='desc'><xsl:choose>
					<xsl:when test='@dir="1"'><xsl:attribute name='href'>boa?board=<xsl:value-of select='@title'/></xsl:attribute><xsl:value-of select='@desc'/></xsl:when>
					<xsl:otherwise><xsl:attribute name='href'>doc?board=<xsl:value-of select='@title'/></xsl:attribute><xsl:value-of select='@desc'/></xsl:otherwise>
				</xsl:choose></a></td>
				<td class='bm'>
					<xsl:call-template name='splitbm'>
						<xsl:with-param name='names' select='@bm'/>
						<xsl:with-param name='isdir' select='@dir'/>
						<xsl:with-param name='isfirst' select='1'/>
					</xsl:call-template>
				</td>
			</tr>
		</xsl:for-each>
	</table>
</xsl:template>

<xsl:template match='bbssel'>
	<fieldset><legend>选择讨论区</legend><form action='sel' method='get'>
		<label for='brd'>讨论区名称：</label>
		<input type='text' name='brd' size='20' maxlength='20'/><br/>
		<input type='submit' value='提交查询'/>
	</form></fieldset>
	<xsl:if test='count(brd)!=0'>
		<p>搜索到: <xsl:value-of select='count(brd)'/>个版面</p>
		<table class='content'>
			<tr><th class='title'>讨论区名称</th><th class='desc'>中文描述</th></tr>
			<xsl:for-each select='brd'><xsl:sort select="@title"/><tr>
				<xsl:attribute name='class'>
					<xsl:if test='position() mod 2 = 1'>light</xsl:if>
					<xsl:if test='position() mod 2 = 0'>dark</xsl:if>
				</xsl:attribute>
				<td class='title'><a class='title'><xsl:choose>
					<xsl:when test='@dir="1"'><xsl:attribute name='href'>boa?board=<xsl:value-of select='@title'/></xsl:attribute>[ <xsl:value-of select='@title'/> ]</xsl:when>
					<xsl:otherwise><xsl:attribute name='href'>doc?board=<xsl:value-of select='@title'/></xsl:attribute><xsl:value-of select='@title'/></xsl:otherwise>
				</xsl:choose></a></td>
				<td class='desc'><a class='desc'><xsl:choose>
					<xsl:when test='@dir="1"'><xsl:attribute name='href'>boa?board=<xsl:value-of select='@title'/></xsl:attribute><xsl:value-of select='@desc'/></xsl:when>
					<xsl:otherwise><xsl:attribute name='href'>doc?board=<xsl:value-of select='@title'/></xsl:attribute><xsl:value-of select='@desc'/></xsl:otherwise>
				</xsl:choose></a></td>
			</tr></xsl:for-each>
		</table>
	</xsl:if>
	<xsl:if test='notfound'>没有找到符合条件的版面</xsl:if>
</xsl:template>

<xsl:template match='forum'>
	<div class='heading'>
		<xsl:choose>
			<xsl:when test='@banner'><img src='{@banner}'/></xsl:when>
			<xsl:otherwise><div class='h2'><xsl:if test='@icon'><img src='{@icon}'/></xsl:if><a href='fdoc?bid={@bid}'><xsl:value-of select='@desc'/> [<xsl:value-of select='@title'/>] - 论坛</a></div></xsl:otherwise>
		</xsl:choose>
		<div class='modelist'><span>切换模式</span>
			<a href='tdoc?bid={@bid}'>主题</a>
			<a href='doc?bid={@bid}'>传统</a>
		</div>
		<div class='bmlist'><span>版主</span><xsl:call-template name='splitbm'><xsl:with-param name='names' select='@bm'/><xsl:with-param name='isdir'>0</xsl:with-param><xsl:with-param name='isfirst' select='1'/></xsl:call-template></div>
	</div>
	<div class='btop'>
		<a class='newpost' href='pst?bid={@bid}'>发表文章</a>
		<a href='brdadd?bid={@bid}'>收藏本版</a>
		<a href='gdoc?bid={@bid}'>文摘区</a>
		<a href='0an?bid={@bid}'><img src='../images/announce.gif'/>精华区</a>
		<a href='bfind?bid={@bid}'><img src='../images/search.gif'/>版内搜索</a>
		<a href='not?board={@title}'>进版画面</a>
		<a href='rss?bid={@bid}'>RSS</a>
	</div>
	<xsl:call-template name='forum-nav'/>
	<table class='content' id='forum'>
		<tr><th class='mark'>标记</th><th class='replies'>回帖</th><th class='owner'>作者</th><th class='owner'>最新回复</th><th class='ptitle'>标题</th></tr>
		<xsl:for-each select='po'><tr>
			<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
			<td class='mark'><xsl:value-of select='@m'/></td>
			<td class='replies'><xsl:choose><xsl:when test='@posts=1'>-</xsl:when><xsl:otherwise><xsl:value-of select='@posts - 1'/></xsl:otherwise></xsl:choose></td>
			<td class='owner'><xsl:if test='@owner'><a class='owner' href='qry?u={@owner}'><xsl:value-of select='@owner'/></a><div class='time'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@potime'/></xsl:call-template></div></xsl:if></td>
			<td class='owner'><xsl:choose><xsl:when test='@upuser'><a class='owner' href='qry?u={@upuser}'><xsl:value-of select='@upuser'/></a><div class='uptime'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@uptime'/></xsl:call-template></div></xsl:when><xsl:otherwise>---</xsl:otherwise></xsl:choose></td>
			<td class='ptitle'><a class='ptitle'>
				<xsl:attribute name='href'>tcon?new=1&amp;bid=<xsl:value-of select='../@bid'/>&amp;f=<xsl:value-of select='@gid'/><xsl:if test='@sticky'>&amp;s=1</xsl:if></xsl:attribute>
				<xsl:call-template name='ansi-escape'>
					<xsl:with-param name='content'><xsl:value-of select='.'/></xsl:with-param>
					<xsl:with-param name='fgcolor'>37</xsl:with-param>
					<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
					<xsl:with-param name='ishl'>0</xsl:with-param>
				</xsl:call-template>
			</a><xsl:if test='@lastpage'><a class='lastpage' href='tcon?new=1&amp;bid={../@bid}&amp;g={@gid}&amp;f={@lastpage}&amp;a=n'>[最新页]</a></xsl:if></td>
		</tr></xsl:for-each>
	</table>
	<xsl:call-template name='forum-nav'/>
</xsl:template>

<xsl:template name='forum-nav'>
	<div class='bnav'>
		<xsl:if test='@next!=0'><a href='fdoc?bid={@bid}&amp;start={@next}'><img src='../images/button/down.gif'/>下一页</a></xsl:if>
		<a href='clear?board={@title}'>清除未读</a>
	</div>
</xsl:template>

<xsl:template match='bbsdoc'>
	<div class='heading'>
	<xsl:choose>
		<xsl:when test='brd/@banner'><img src='{brd/@banner}'/></xsl:when>
		<xsl:otherwise><div class='h2'>
		<xsl:if test='brd/@icon'><img src='{brd/@icon}'/></xsl:if><a href='{brd/@link}doc?bid={brd/@bid}'><xsl:value-of select='brd/@desc'/> [<xsl:value-of select='brd/@title'/>]<xsl:if test='brd/@link = "g"'> - 文摘</xsl:if><xsl:if test='brd/@link = "t"'> - 主题</xsl:if></a></div></xsl:otherwise>
	</xsl:choose>
	<div class='modelist'><span>切换模式</span>
		<a href='fdoc?bid={brd/@bid}'>论坛</a>
		<xsl:if test='brd/@link != "t"'><a href='tdoc?bid={brd/@bid}'>主题</a></xsl:if>
		<xsl:if test='brd/@link != ""'><a href='doc?bid={brd/@bid}'>传统</a></xsl:if>
	</div>
	<div class='bmlist'><span>版主</span><xsl:call-template name='splitbm'><xsl:with-param name='names' select='brd/@bm'/><xsl:with-param name='isdir'>0</xsl:with-param><xsl:with-param name='isfirst' select='1'/></xsl:call-template></div>
	</div>
	
	<div class='btop'>
		<a class='newpost' href='pst?bid={brd/@bid}'>发表文章</a>
		<a href='brdadd?bid={brd/@bid}'>收藏本版</a>
		<xsl:if test='brd/@link != "g"'><a href='gdoc?bid={brd/@bid}'>文摘区</a></xsl:if>
		<a href='0an?bid={brd/@bid}'>精华区</a>
		<a href='bfind?bid={brd/@bid}'>版内搜索</a>
		<a href='not?board={brd/@title}'>进版画面</a>
		<a href='rss?bid={brd/@bid}'>RSS</a>
	</div>

	<xsl:call-template name='bbsdoc-nav'/>
	<table class='content' id='postlist'>
		<tr><th class='no'>序号</th><th class='mark'>标记</th><th>作者</th><th class='time'>发表时间</th><th class='ptitle'>标题</th></tr>
		<xsl:for-each select='po'><tr>
			<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
			<td class='no'><xsl:choose><xsl:when test='@sticky'>【∞】</xsl:when><xsl:otherwise><xsl:value-of select='position() - 1 + ../brd/@start'/></xsl:otherwise></xsl:choose></td>
			<td class='mark'><xsl:value-of select='@m'/></td>
			<td class='owner'><a class='owner' href='qry?u={@owner}'><xsl:value-of select='@owner'/></a></td>
			<td class='time'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@time'/></xsl:call-template></td>
			<td class='ptitle'><a class='ptitle'>
				<xsl:attribute name='href'><xsl:value-of select='../brd/@link'/>con?new=1&amp;bid=<xsl:value-of select='../brd/@bid'/>&amp;f=<xsl:value-of select='@id'/><xsl:if test='@sticky'>&amp;s=1</xsl:if></xsl:attribute>
				<xsl:if test='substring(., 1, 4) != "Re: "'><img src='../images/types/text.gif'/></xsl:if>
				<xsl:call-template name='ansi-escape'>
					<xsl:with-param name='content'><xsl:value-of select='.'/></xsl:with-param>
					<xsl:with-param name='fgcolor'>37</xsl:with-param>
					<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
					<xsl:with-param name='ishl'>0</xsl:with-param>
				</xsl:call-template>
			</a></td>
		</tr></xsl:for-each>
	</table>
	<xsl:call-template name='bbsdoc-nav'/>
</xsl:template>

<xsl:template name='bbsdoc-nav'>
	<div class='bnav'>
		<a href='javascript:location=location'><img src='../images/button/reload.gif'/>刷新</a>
		<xsl:if test='brd/@start > 1'>
			<xsl:variable name='prev'><xsl:choose><xsl:when test='brd/@start - brd/@page &lt; 1'>1</xsl:when><xsl:otherwise><xsl:value-of select='brd/@start - brd/@page'/></xsl:otherwise></xsl:choose></xsl:variable>
			<a href='{brd/@link}doc?bid={brd/@bid}&amp;start={$prev}'><img src='../images/button/up.gif'/>上一页</a>
		</xsl:if>
		<xsl:if test='brd/@total > brd/@start + brd/@page - 1'>
			<xsl:variable name='next'><xsl:value-of select='brd/@start + brd/@page'/></xsl:variable>
			<a href='{brd/@link}doc?bid={brd/@bid}&amp;start={$next}'><img src='../images/button/down.gif'/>下一页</a>
		</xsl:if>
		<a href='clear?board={brd/@title}&amp;start={brd/@start}'>清除未读</a>
		<form class='jump' method='get' action='{brd/@link}doc'><input type='hidden' name='bid' value='{brd/@bid}'></input><img src='../images/button/forward.gif'/>跳转到<input type='text' name='start' size='6'/>篇</form>
	</div>
</xsl:template>

<xsl:template match='bbscon'>
	<div class='post'>
		<div class='ptop'>
			<xsl:if test='@link != "con"'><a href='gdoc?bid={@bid}'>文摘区</a></xsl:if>
			<a href='doc?bid={@bid}'><img src='../images/button/home.gif'/>本讨论区</a>
			<xsl:variable name='baseurl'>con?new=1&amp;bid=<xsl:value-of select='@bid'/>&amp;f=<xsl:value-of select='po/@fid'/>&amp;a=</xsl:variable>
			<xsl:if test='not(po/@sticky)'>
				<xsl:if test='not(po/@first)'><a href='{$baseurl}p'><img src='../images/button/up.gif'/>上篇</a></xsl:if>
				<xsl:if test='not(po/@last)'><a href='{$baseurl}n'><img src='../images/button/down.gif'/>下篇</a></xsl:if>
				<xsl:if test='po/@reid != f'><a href='{$baseurl}b'>上楼</a></xsl:if>
				<xsl:if test='not(po/@tlast)'><a href='{$baseurl}a'>下楼</a></xsl:if>
				<xsl:if test='po/@gid'><a href='con?new=1&amp;bid={@bid}&amp;f={po/@gid}'>顶楼</a></xsl:if>
				<xsl:variable name='gid'><xsl:choose><xsl:when test='po/@gid'><xsl:value-of select='po/@gid'/></xsl:when><xsl:otherwise><xsl:value-of select='po/@fid'/></xsl:otherwise></xsl:choose></xsl:variable>
				<xsl:if test='po/@fid != po/@gid or not(po/@tlast)'><a href='tcon?new=1&amp;bid={@bid}&amp;f={$gid}'>展开主题</a></xsl:if>
				<xsl:if test='not(po/@tlast)'><a href='tcon?new=1&amp;bid={@bid}&amp;g={$gid}&amp;f={po/@fid}&amp;a=n'>向后展开</a></xsl:if>
			</xsl:if>
			<a><xsl:attribute name='href'>con?new=1&amp;bid=<xsl:value-of select='@bid'/>&amp;f=<xsl:value-of select='po/@fid'/><xsl:if test='po/@sticky'>&amp;s=1</xsl:if></xsl:attribute>本文链接</a>
			<a><xsl:attribute name='href'>../static/con?new=1&amp;bid=<xsl:value-of select='@bid'/>&amp;f=<xsl:value-of select='po/@fid'/><xsl:if test='po/@sticky'>&amp;s=1</xsl:if></xsl:attribute>保存/打印</a>
			<xsl:call-template name='sigature-options'/>
		</div>

		<div class='pmain'><xsl:apply-templates select='po'/></div>
		<div class='plink'><xsl:call-template name='con-linkbar'/></div>
	</div>
	<xsl:call-template name='quick-reply-form'/>
	<xsl:call-template name='quick-cp-form'/>
</xsl:template>

<xsl:template name='quick-cp-form'>
<div id='quick-cp' title='转载文章'>
	<div class='ui-state-highlight'>未经站务委员会批准，多版面转贴相同或相似文章超过五个版的，将受到全站处罚。</div>
	<form class='quick-cp'><label for='t'>转载到版面</label><input type='text' name='t'/></form>
</div>
</xsl:template>

<xsl:template name='quick-reply-form'>
<xsl:if test='@attach&gt;0'><iframe name='quick-upload-ifrm' class='quick-upload-ifrm'></iframe></xsl:if>
<div id='quick-reply' title='回复文章'>
<div id='quick-reply-error' class='ui-state-error'></div>
<form id='quick-reply-form' method='post'>
<label for='title'>标题 </label><input class='text ui-widget-content ui-corner-all' type='text' name='title' size='60' maxlength='50'></input>
<xsl:if test='@anony=1'><input type='checkbox' id='quick-reply-anony' name='anony' value='1' checked='checked'/><label for='quick-reply-anony'>匿名</label></xsl:if>
<label for='sig' id='quick-reply-sig-label'>签名档 </label><select name='sig'><option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option><option value="5">5</option><option value="6">6</option></select>
<textarea class='binput' name='text' rows='15' cols='85' wrap='virtual'></textarea>
</form>
<xsl:if test='@attach&gt;0'>
<form id='quick-upload' method='post' enctype='multipart/form-data' target='quick-upload-ifrm' action='upload?bid={@bid}'><label for='up'>上传文件</label><input type='file' name='up'/></form>
</xsl:if>
</div>
</xsl:template>

<xsl:template match='po'>
<xsl:if test='owner'><div class='post_h'>
	<p>发信人: <a class='powner' href='qry?u={owner}'><xsl:value-of select='owner'/></a> (<xsl:value-of select='nick'/>), 信区: <a href='doc?board={board}'><xsl:value-of select='board'/></a></p>
	<p>标&#160;&#160;题: <span class='ptitle'><xsl:value-of select='title'/></span></p>
	<p>发信站: <xsl:value-of select='$bbsname'/> (<xsl:value-of select='date'/>), 站内信件</p>
</div></xsl:if>
<xsl:for-each select='pa'>
	<div class='post_{@m}'>
		<xsl:for-each select='p'><p><xsl:apply-templates select='.'/></p></xsl:for-each>
	</div>
</xsl:for-each>
</xsl:template>

<xsl:template match='br'><br/></xsl:template>

<xsl:template match='c'>
<span class='a{@h}{@f} a{@b}'><xsl:value-of select='.'/></span>
</xsl:template>

<xsl:template match='a'>
<a href='{@href}'><xsl:choose><xsl:when test='@i'><img src='{@href}'/></xsl:when><xsl:otherwise><xsl:value-of select='@href'/></xsl:otherwise></xsl:choose></a>
</xsl:template>

<xsl:template name='con-linkbar'>
	<xsl:variable name='param'>bid=<xsl:value-of select='@bid'/>&amp;f=<xsl:value-of select='po/@fid'/></xsl:variable>
	<xsl:if test='po/@nore'><span class='disabled'>本文不可回复</span></xsl:if>
	<xsl:if test='not(po/@nore) and @link="con"'><a class='reply'><xsl:attribute name='href'>pst?<xsl:value-of select='$param'/></xsl:attribute>回复本文</a></xsl:if>
	<xsl:if test='po/@edit'>
		<a href='edit?{$param}'>修改</a>
		<a href='del?{$param}'>删除</a>
	</xsl:if>
	<a href='ccc?{$param}' class='crosspost'>转载</a>
	<a href='fwd?{$param}'>转寄</a>
</xsl:template>

<xsl:template match='bbstcon'>
	<div class='pnav'>
		<xsl:call-template name='tcon-navbar'/>
		<xsl:call-template name='sigature-options'/>
	</div>
	<xsl:for-each select='po'>
		<div class='post'>
			<div class='pmain'><xsl:apply-templates select='.'/></div>
			<div class='plink'>
				<xsl:if test='@nore'><span class='disabled'>本文不可回复</span></xsl:if>
				<xsl:if test='not(@nore)'><a class='reply' href='pst?bid={../@bid}&amp;f={@fid}'>回复本文</a></xsl:if>
				<a href='ccc?bid={../@bid}&amp;f={@fid}' class='crosspost'>转载</a>
				<a href='con?new=1&amp;bid={../@bid}&amp;f={@fid}'><img src='../images/button/content.gif'/>本文链接</a>
			</div>
		</div>
	</xsl:for-each>
	<div class='pnav'><xsl:call-template name='tcon-navbar'/></div>
	<xsl:call-template name='quick-reply-form'/>
	<xsl:call-template name='quick-cp-form'/>
</xsl:template>

<xsl:template name='tcon-navbar'>
		<a href='{/bbstcon/session/@m}doc?bid={@bid}'><img src='../images/button/home.gif'/>本讨论区</a>
		<xsl:if test='count(po) = @page'><a href='tcon?new=1&amp;bid={@bid}&amp;g={@gid}&amp;f={po[last()]/@fid}&amp;a=n'><img src='../images/button/down.gif'/>下页</a></xsl:if>
		<xsl:if test='po[1]/@fid != @gid'><a href='tcon?new=1&amp;bid={@bid}&amp;g={@gid}&amp;f={po[1]/@fid}&amp;a=p'><img src='../images/button/up.gif'/>上页</a></xsl:if>
		<xsl:if test='not(@tlast)'><a href='tcon?new=1&amp;bid={@bid}&amp;f={@gid}&amp;a=a'>下一主题</a></xsl:if>
		<xsl:if test='not(@tfirst)'><a href='tcon?new=1&amp;bid={@bid}&amp;f={@gid}&amp;a=b'>上一主题</a></xsl:if>
</xsl:template>

<xsl:template name='sigature-options'>
	<a href='#' class='sig_option'>签名档选项</a>
	<form class='sig_option' action='sigopt'>
		<input type='checkbox' name='hidesig'><xsl:if test='@nosig'><xsl:attribute name='checked'>checked</xsl:attribute></xsl:if></input>隐藏签名档<input type='checkbox' name='hideimg'><xsl:if test='@nosigimg'><xsl:attribute name='checked'>checked</xsl:attribute></xsl:if></input>隐藏签名档图片<input type='submit' value='保存'/><input type='button' class='cancel' value='取消'/>
	</form>
</xsl:template>

<xsl:template match='bbsqry'>
	<form action='qry' method='get'><label for='u'>请输入欲查询的帐号：</label><input type='text' name='u' maxlength='12' size='12'/><input type='submit' value='查询'/></form>
	<xsl:choose><xsl:when test='@login'><div class='post'>
		<div class='ptop'><a href='pstmail?recv={@id}'>发送信件</a></div>
		<div class='umain' id='uinfo'>
		<p><strong><xsl:value-of select='@id'/></strong> （<strong><xsl:value-of select='nick'/></strong>） <xsl:call-template name='show-horo'/></p>
		<p>上次在:【<span class='a132'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@lastlogin'/></xsl:call-template></span>】从【<span class='a132'><xsl:value-of select='ip'/></span>】到本站一游。</p>
		<xsl:if test='logout'><p>离站于:【<span class='a132'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='logout'/></xsl:call-template></span>】</p></xsl:if>
		<p>文章数:【<span class='a132'><xsl:value-of select='@post'/></span>】<xsl:if test='@money'>光华币 【<span class='a132'><xsl:value-of select='@money'/></span>】</xsl:if> 生命力:【<span class='a132'><xsl:value-of select='@hp'/></span>】</p>
		<p>表现值:【<span class='a133'><xsl:value-of select='@perf'/></span>】</p>
		<p>经验值:【<xsl:call-template name="show-exp"/>】 (<xsl:value-of select='@level * 10 + @repeat'/>/60)</p>
		<p>身份: <xsl:call-template name='ansi-escape'><xsl:with-param name='content'><xsl:value-of select='ident'/></xsl:with-param><xsl:with-param name='fgcolor'>37</xsl:with-param><xsl:with-param name='bgcolor'>ignore</xsl:with-param><xsl:with-param name='ishl'>0</xsl:with-param></xsl:call-template></p></div>
		<xsl:if test='st'><div class='usplit'>目前状态</div>
		<div class='umain'><xsl:for-each select='st'><p><strong><xsl:value-of select='@desc'/></strong><xsl:if test='@idle!=0'>[发呆<xsl:value-of select='@idle'/>分钟]</xsl:if><xsl:if test='@web=1'>（web登录）</xsl:if><xsl:if test='@vis=0'>（隐身）</xsl:if></p></xsl:for-each></div></xsl:if>
		<div class='usplit'>个人说明档如下</div>
		<div class='usmd'><xsl:call-template name='showpost'><xsl:with-param name='content' select='smd'/></xsl:call-template></div>
	</div></xsl:when><xsl:otherwise><xsl:if test='@id!=""'><p>没有找到用户【<xsl:value-of select='@id'/>】</p></xsl:if></xsl:otherwise></xsl:choose>
</xsl:template>

<xsl:template name='show-horo'>
	<xsl:if test='@horo'>
		<xsl:variable name='color'><xsl:choose><xsl:when test='@gender = "M"'>a136</xsl:when><xsl:when test='@gender = "F"'>a135</xsl:when><xsl:otherwise>a132</xsl:otherwise></xsl:choose></xsl:variable>
		<span>【</span><span class='{$color}'><xsl:value-of select='@horo'/></span><span>】</span>
	</xsl:if>
</xsl:template>

<xsl:template name='show-exp'>
	<span class='lev{@level}'>
		<span class='lev{@level}' style='width:{@repeat * 10}%;'></span>
	</span>
</xsl:template>

<xsl:template match='bbspst'>
	<p>版面：<xsl:value-of select='@brd'/></p>
	<form id='postform' name='postform' method='post' action='snd?bid={@bid}&amp;f={po/@f}&amp;e={@edit}'>
		<input type='hidden' id='brd' value='{@brd}'></input>
		<p>标题：<xsl:choose>
		<xsl:when test='@edit=0'><input class='binput' type='text' name='title' size='60' maxlength='50'>
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
		<p><xsl:if test='@anony=1'><input type="checkbox" name="anony" value="1" checked="checked"/>匿名 </xsl:if>签名档: <input type='radio' name='sig' value='1' checked='checked'/>1 <input type='radio' name='sig' value='2'/>2 <input type='radio' name='sig' value='3'/>3 <input type='radio' name='sig' value='4'/>4 <input type='radio' name='sig' value='5'/>5 <input type='radio' name='sig' value='6'/>6</p>
		<p><textarea class='binput' name='text' rows='20' cols='85' wrap='virtual'>
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

<xsl:template match='bbstop10'>
	<h2>24小时十大热门话题</h2>
	<table class='content'>
		<tr><th class='no'>排名</th><th class='owner'>作者</th><th class='title'>版面</th><th class='no'>篇数</th><th class='ptitle'>标题</th></tr>
		<xsl:for-each select='top'><tr>
			<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
			<td class='no'><xsl:value-of select='position()'/></td>
			<td class='owner'><a class='owner' href='qry?u={@owner}'><xsl:value-of select='@owner'/></a></td>
			<td class='title'><a class='title' href='doc?board={@board}'><xsl:value-of select='@board'/></a></td>
			<td class='no'><xsl:value-of select='@count'/></td>
			<td class='ptitle'><a class='ptitle' href='tcon?new=1&amp;board={@board}&amp;f={@gid}'><xsl:call-template name='ansi-escape'><xsl:with-param name='content' select='.'/><xsl:with-param name='fgcolor'>37</xsl:with-param><xsl:with-param name='bgcolor'>ignore</xsl:with-param><xsl:with-param name='ishl'>0</xsl:with-param></xsl:call-template></a></td>
		</tr></xsl:for-each>
	</table>
</xsl:template>

<xsl:template match='bbsbfind'>
	<h2>版内文章搜索</h2>
	<xsl:variable name='count' select='count(po)'/>
	<xsl:choose><xsl:when test='@result'><p>共找到 <xsl:value-of select='$count'/> 篇文章 <xsl:if test='$count&gt;=100'>（100篇以上部分省略）</xsl:if></p><xsl:if test='$count!=0'><p>最新文章靠前</p><table class='content'>
		<tr><th class='no'>序号</th><th class='mark'>标记</th><th class='owner'>作者</th><th class='time'>发表时间</th><th class='ptitle'>标题</th></tr>
		<xsl:for-each select='po'><tr>
			<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
			<td class='no'><xsl:value-of select='position()'/></td>
			<td class='mark'><xsl:value-of select='@m'/></td>
			<td class='owner'><a class='owner' href='qry?u={@owner}'><xsl:value-of select='@owner'/></a></td>
			<td class='time'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@time'/></xsl:call-template></td>
			<xsl:variable name='imgsrc'>../images/types/<xsl:choose><xsl:when test='substring(., 1, 4) = "Re: "'>reply</xsl:when><xsl:otherwise>text</xsl:otherwise></xsl:choose>.gif</xsl:variable>
			<xsl:variable name='text'><xsl:choose><xsl:when test='substring(., 1, 4) = "Re: "'><xsl:value-of select='substring(., 5)'/></xsl:when><xsl:otherwise><xsl:value-of select='.'/></xsl:otherwise></xsl:choose></xsl:variable>
			<td class='ptitle'><a class='ptitle' href='{../brd/@link}con?new=1&amp;bid={../@bid}&amp;f={@id}'>
				<img src='{$imgsrc}'/>
				<xsl:call-template name='ansi-escape'>
					<xsl:with-param name='content'><xsl:value-of select='$text'/></xsl:with-param>
					<xsl:with-param name='fgcolor'>37</xsl:with-param>
					<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
					<xsl:with-param name='ishl'>0</xsl:with-param>
				</xsl:call-template>
			</a></td>
		</tr></xsl:for-each>
	</table></xsl:if></xsl:when>
	<xsl:otherwise><form action='bfind' method='get'><fieldset><legend>至少填写一项</legend>
		<input name='bid' type='hidden' value='{@bid}'></input>
		<p>标题含有: <input name='t1' type='text' maxlength='50' size='20'/> 和 <input name='t2' type='text' maxlength='50' size='20'/></p>
		<p>标题不含: <input name='t3' type='text' maxlength='50' size='20'/></p>
		<p>作者帐号: <input name='user' type='text' maxlength='12' size='16'/></p></fieldset>
		<fieldset><legend>选填</legend><p>时间范围: <input name='limit' type='text' maxlength='4' size='4' value='7'/>天以内 (至多30天)</p>
		<p>文章标记: <input name='mark' type='checkbox'/></p>
		<p>不含跟帖: <input name='nore' type='checkbox'/></p></fieldset>
		<p><input type='submit' value='给我搜！'/></p>
	</form></xsl:otherwise></xsl:choose>
</xsl:template>

<xsl:template match='bbsmail'>
	<h2>信件列表</h2>
	<xsl:if test='@new'><p><a href='mail'><xsl:choose><xsl:when test='count(mail)=0'>您没有30天内的未读信件</xsl:when><xsl:otherwise>本页仅显示30天内未读信件</xsl:otherwise></xsl:choose>，查看全部信件请点此处</a></p></xsl:if>
	<div class='mnav'><xsl:call-template name='bbsmail-nav'/></div>
	<form name='maillist' method='post' action='mailman'>
		<table class='content' id='maillist'>
			<tr><th class='chkbox'></th><th class='mark'>状态</th><th class='owner'>发信人</th><th class='time'>日期</th><th class='ptitle'>信件标题</th></tr>
			<xsl:for-each select='mail'><tr>
				<xsl:if test='@r=0 or ../@new'><xsl:attribute name='class'>light</xsl:attribute></xsl:if>
				<td class='chkbox'><input type="checkbox" name='box{@name}'></input></td>
				<td class='mark'><xsl:value-of select='@m'/></td>
				<td><a class='owner' href='qry?u={@from}'><xsl:value-of select='@from'/></a></td>
				<td class='time'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@date'/></xsl:call-template></td>
				<td class='ptitle'>
					<a class='ptitle'>
					<xsl:attribute name='href'>mailcon?f=<xsl:value-of select='@name'/>&amp;n=<xsl:choose><xsl:when test='../@new'><xsl:value-of select='@n'/></xsl:when><xsl:otherwise><xsl:value-of select='../@start + count(../mail) - position()'/></xsl:otherwise></xsl:choose></xsl:attribute>
					<xsl:call-template name='ansi-escape'>
						<xsl:with-param name='content'><xsl:value-of select='.'/></xsl:with-param>
						<xsl:with-param name='fgcolor'>37</xsl:with-param>
						<xsl:with-param name='bgcolor'>ignore</xsl:with-param>
						<xsl:with-param name='ishl'>0</xsl:with-param>
					</xsl:call-template>
				</a></td>
			</tr></xsl:for-each>
		</table>
		<input name='mode' value='' type='hidden'/>
	</form>
	<div class='mnav'><xsl:call-template name='bbsmail-nav'/></div>
</xsl:template>

<xsl:template name='bbsmail-nav'>
<a href="#" class='check_all'>全选</a>
<a href="#" class='check_rev'>反选</a>
<input type='button' class='del_mail' value='删除'/>
<div class='mailsum'>
<xsl:if test='@total &gt; @start + @page - 1'>
<xsl:variable name='next'><xsl:value-of select='@start + @page'/></xsl:variable>
<a href='mail?start={$next}'>&lt; 新信件</a>
</xsl:if>
<xsl:if test='not(@new)'>第 <xsl:value-of select='@total - @start - count(mail) + 2'/> - <xsl:value-of select='@total - @start + 1'/> 封 共 <xsl:value-of select='@total'/> 封</xsl:if>
<xsl:if test='@start &gt; 1'>
<xsl:variable name='prev'><xsl:choose><xsl:when test='@start - @dpage &lt; 1'>1&amp;page=<xsl:value-of select='@start - 1'/></xsl:when><xsl:otherwise><xsl:value-of select='@start - @dpage'/></xsl:otherwise></xsl:choose></xsl:variable>
<a href='mail?start={$prev}'>旧信件 &gt;</a>
</xsl:if>
</div>
</xsl:template>

<xsl:template match='bbsmailcon'>
	<div class='mnav'><xsl:call-template name='mailcon-navbar'/></div>
	<div class='post'>
		<div class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='mail'/></xsl:call-template></div>
	</div>
	<div class='mnav'><xsl:call-template name='mailcon-navbar'/></div>
	<xsl:if test='@new'><script type='text/javascript' defer='defer'>bbs.init();bbs.store.set('last', 0);</script></xsl:if>
</xsl:template>

<xsl:template name='mailcon-navbar'>
	<a><xsl:attribute name='href'>mail<xsl:choose><xsl:when test='mail/@n &gt; @total - @dpage'></xsl:when><xsl:when test='mail/@n &lt;= @total mod @dpage'>?start=1&amp;page=<xsl:value-of select='@total mod @dpage'/></xsl:when><xsl:otherwise>?start=<xsl:value-of select='floor((mail/@n - 1 - @total mod @dpage) div @dpage) * @dpage + @total mod @dpage + 1'/></xsl:otherwise></xsl:choose></xsl:attribute>&lt;&lt; 回信件列表</a>
	<form method='post' action='pstmail?n={mail/@n}'><input type='submit' value='回复'/></form>
	<form method='post' action='delmail?f={mail/@f}'><input type='submit' onclick='return confirm("您真的要删除这封信吗？")' value='删除'/></form>
	<div class='mailsum'>
		<xsl:if test='@next'><a href='mailcon?f={@next}&amp;n={mail/@n + 1}'>&lt; 新一封</a></xsl:if>
		第 <xsl:value-of select='mail/@n'/> 封 共 <xsl:value-of select='@total'/> 封 
		<xsl:if test='@prev'><a href='mailcon?f={@prev}&amp;n={mail/@n - 1}'>旧一封 &gt;</a></xsl:if>
	</div>
</xsl:template>

<xsl:template match='bbspstmail'>
	<form id='postform' name='postform' method='post' action='sndmail'>
		<input type='hidden' name='ref' value='{@ref}'></input>
		<p><label for='recv'>收信人:&#160;&#160;&#160;</label><input class='binput' type='text' name='recv' size='15' maxlength='15' value='{@recv}'></input></p>
		<p><label for='title'>信件标题 </label>
		<input class='binput' type='text' name='title' size='60' maxlength='50'>
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
		<p><textarea class='binput' name='text' rows='20' cols='85' wrap='virtual'>
			<xsl:text>&#x0d;&#x0a;</xsl:text>
			<xsl:call-template name='show-quoted'>
				<xsl:with-param name='content' select='m'/>
			</xsl:call-template>
		</textarea></p>
		<input type='submit' value='寄出' id='btnPost' size='10'/>
		<input type='reset' value='重置'/>
	</form>
</xsl:template>

<xsl:template match='bbs0an'>
	<p>本目录web浏览次数：[<xsl:value-of select='@v'/>]</p>
	<table class='content'>
		<tr><th class='no'>序号</th><th class='ptitle'>标题</th><th class='bm'>整理者</th><th class='time'>日期</th></tr>
		<xsl:for-each select='ent'><tr>
				<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
				<td class='no'><xsl:value-of select='position()'/></td>
				<td class='ptitle'><xsl:choose>
					<xsl:when test='@t = "d"'><a class='ptitle' href='0an?path={../@path}{@path}'><img src='../images/types/folder.gif'/><xsl:value-of select='.'/></a></xsl:when>
					<xsl:when test='@t = "f"'><a class='ptitle' href='anc?path={../@path}{@path}'><img src='../images/types/text.gif'/><xsl:value-of select='.'/></a></xsl:when>
					<xsl:otherwise><img src='../images/types/error.gif'/><xsl:value-of select='@t'/></xsl:otherwise>
				</xsl:choose></td>
				<td class='bm'><xsl:if test='@id'>
					<xsl:call-template name='splitbm'>
						<xsl:with-param name='names' select='@id'/>
						<xsl:with-param name='isdir' select='0'/>
						<xsl:with-param name='isfirst' select='1'/>
					</xsl:call-template>
				</xsl:if></td>
				<td class='time'><xsl:if test='@t != "e"'><xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@time'/></xsl:call-template></xsl:if></td>
		</tr></xsl:for-each>
		<xsl:if test='not(ent)'>
			<td/><td width='80%'>&lt;&lt;目前没有文章&gt;&gt;</td>
		</xsl:if>
	</table>
	<xsl:if test='@brd'><a href='doc?board={@brd}'>[<img src='../images/button/home.gif'/>本讨论区]</a></xsl:if>
</xsl:template>

<xsl:template match='bbsanc'>
	<h3><xsl:if test='@brd'><xsl:value-of select='@brd'/>版 - </xsl:if>精华区文章阅读</h3>
	<div class='post'>
		<div class='ptop'><xsl:call-template name='anc-navbar'/></div>
		<div class='pmain'><xsl:call-template name='showpost'><xsl:with-param name='content' select='po'/></xsl:call-template></div>
		<div class='pbot'><xsl:call-template name='anc-navbar'/></div>
	</div>
</xsl:template>

<xsl:template name='anc-navbar'>
	<xsl:if test='@brd'>
		<a href='gdoc?board={@brd}'>[ 文摘区 ]</a>
		<a href='doc?board={@brd}'>[<img src='../images/button/home.gif'/>本讨论区]</a>
	</xsl:if>
</xsl:template>

<xsl:template match='bbsfwd'>
	<form action='fwd' method='post'>
		<input type='hidden' name='bid' value='{@bid}'></input>
		<input type='hidden' name='f' value='{@f}'></input>
		<label for='u'>收信人:&#160;</label><input type='text' name='u' size='16'></input><br/>
		<input value='转寄' type='submit'/>
	</form>
</xsl:template>

<xsl:template match='bbsccc'>
	<xsl:choose>
		<xsl:when test='not(@bid)'>
			<p>转载成功</p>
			<p><a id='url' href='con?new=1&amp;bid={@b}&amp;f={@f}'>进入转载文章</a></p>
			<p><a href='doc?bid={@b}'>[ <img src='../images/button/back.gif'/>返回原先版面 ]</a></p>
			<p><a href='doc?bid={@t}'>[ <img src='../images/button/forward.gif'/>进入目标版面 ]</a></p>
		</xsl:when>
		<xsl:otherwise>
			<form method='get' action='ccc'>
				<p>文章标题: <xsl:value-of select='node()[1]'/></p>
				<p>文章作者: <xsl:value-of select='@owner'/></p>
				<p>原始版面: <xsl:value-of select='@brd'/></p>
				<input type='hidden' name='bid' value='{@bid}'></input>
				<input type='hidden' name='f' value='{@fid}'></input>
				<label for='t'>转载到版面: </label><input type='text' name='t'/>
				<input type='submit' value='转载'/>
				<p><strong>转帖注意：未经站务委员会批准，多版面转贴相同或相似文章超过五个版的，将受到全站处罚。</strong></p>
			</form>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match='bbsfall'>
	<h2>设定关注名单</h2>
	<table class='content'>
		<tr><th class='owner'>帐号</th><th class='chkbox'>操作</th><th class='idesc'>说明</th></tr>
		<xsl:for-each select='ov'><tr>
			<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
			<td class='owner'><a class='owner' href='qry?u={@id}'><xsl:value-of select='@id'/></a></td>
			<td class='chkbox'><a href='fdel?u={@id}'>删除</a></td>
			<td class='idesc'><xsl:value-of select='.'/></td>
		</tr></xsl:for-each>
	</table>
	<a href='fadd'>[增加关注网友]</a>
</xsl:template>

<xsl:template match='bbsfadd'>
	<h2>增加关注网友</h2>
	<form name='add' method='get' action='fadd'>
		<p><label for='id'>帐号: </label><input class='binput' type='text' name='id' size='15' maxlength='15'></input></p>
		<p><label for='id'>说明: </label><input class='binput' type='text' name='desc' size='50' maxlength='50'></input></p>
		<p><input type='submit' value='提交' size='10'/></p>
	</form>
</xsl:template>

<xsl:template match='bbsovr'>
	<table class='content'>
		<tr><th class='no'>编号</th><th class='owner'>使用者代号</th><th class='idesc'>昵称</th><th>上站位置</th><th>动态</th><th>发呆</th></tr>
		<xsl:for-each select='ov'><xsl:sort select='@id'/><tr>
			<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
			<td class='no'><xsl:value-of select='position()'/></td>
			<td class='owner'><a class='owner' href='qry?u={@id}'><xsl:value-of select='@id'/></a></td>
			<td class='idesc'><xsl:value-of select='.'/></td>
			<td><xsl:value-of select='@ip'/></td>
			<td><xsl:value-of select='@action'/></td>
			<td><xsl:if test='@idle &gt; 0'><xsl:value-of select='@idle'/></xsl:if></td>
		</tr></xsl:for-each>
	</table>
</xsl:template>

<xsl:template match='bbsbrdadd'>
	<h2>添加收藏版面</h2>
	<p>成功添加 <a href='doc?bid={bid}'><xsl:value-of select='brd'/></a> 版到收藏夹</p>
</xsl:template>

<xsl:template match='bbsfav'>
	<h2>我的收藏夹</h2>
	<p><a href='mybrd'>自定义</a></p>
	<table class='content'>
		<tr><th class='no'>序号</th><th class='title'>讨论区名称</th><th class='desc'>中文描述</th></tr>
		<xsl:for-each select='brd'><tr>
			<xsl:attribute name='class'><xsl:choose><xsl:when test='position() mod 2 = 1'>light</xsl:when><xsl:otherwise>dark</xsl:otherwise></xsl:choose></xsl:attribute>
			<td class='no'><xsl:value-of select='position()'/></td>
			<td class='title'><a class='title' href='doc?bid={@bid}'><xsl:value-of select='@brd'/></a></td>
			<td class='desc'><a class='desc' href='doc?bid={@bid}'><xsl:value-of select='.'/></a></td>
		</tr></xsl:for-each>
	</table>
</xsl:template>

<xsl:template match='bbsmybrd'>
	<h2>收藏夹设定</h2>
	<xsl:choose>
		<xsl:when test='@selected'>
			<div>修改预定讨论区成功，您现在一共预定了 <xsl:value-of select='@selected'/> 个讨论区</div>
		</xsl:when>
		<xsl:otherwise>
			<form action='mybrd?type=1' method='post'>
				<div class='column-3'><ul>
					<xsl:apply-templates select='mbrd'/>
				</ul></div>
				<input type='submit' value='确认预定'/><input type='reset' value='复原'/>
			</form>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match='mbrd'>
	<xsl:variable name='check'><xsl:call-template name='is-mybrd'><xsl:with-param name='bid' select='@bid'/></xsl:call-template></xsl:variable>
	<li>
		<input type='checkbox' name='{@bid}'>
			<xsl:if test='$check = 1'><xsl:attribute name='checked'>checked</xsl:attribute></xsl:if>
		</input>
		<a class='idesc' href='doc?bid={@bid}'><xsl:value-of select='@name'/> - <xsl:value-of select='@desc'/></a>
	</li>
</xsl:template>

<xsl:template name='is-mybrd'>
	<xsl:param name='bid'/>
	<xsl:for-each select='../my'><xsl:if test='@bid = $bid'>1</xsl:if></xsl:for-each>
</xsl:template>

<xsl:template match='bbseufile'>
	<h2><xsl:value-of select='@desc'/></h2>
	<xsl:choose>
		<xsl:when test='@submit'>
			<form name='postform' method='post' action='{@submit}'>
				<p><textarea class='binput' name='text' rows='20' cols='85' wrap='virtual'><xsl:call-template name='show-quoted'><xsl:with-param name='content' select='text'/></xsl:call-template></textarea></p>
				<p><input type='submit' value='保存' id='btnPost' size='10'/></p>
			</form>
		</xsl:when>
		<xsl:otherwise><p>保存成功</p><a href='javascript:history.go(-2)'>返回</a></xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match='bbsinfo'>
	<xsl:choose><xsl:when test='@gender'>
		<fieldset><legend>修改个人资料</legend><form action='info?type=1' method='post'>
			<p>您的昵称: <input type='text' name='nick' maxlength='30' value='{nick}'></input></p>
			<p>出生日期: <input type='text' name='year' size='4' maxlength='4' value='{@year+1900}'></input> 年 <input type='text' name='month' size='2' maxlength='2' value='{@month}'></input> 月 <input type='text' name='day' size='2' maxlength='2' value='{@day}'></input> 日</p>
			<p>用户性别: <input type='radio' value='M' name='gender'><xsl:if test='@gender = "M"'><xsl:attribute name='checked'>checked</xsl:attribute></xsl:if></input> 男 <input type='radio' value='F' name='gender'><xsl:if test='@gender = "F"'><xsl:attribute name='checked'>checked</xsl:attribute></xsl:if></input> 女</p>
			<input type='submit' value='确定'/> <input type='reset' value='复原'/>
		</form></fieldset>
		<p>登录本站: <xsl:value-of select='@login'/> 次</p>
		<p>上站时间: <xsl:value-of select='floor(@stay div 60)'/> 小时 <xsl:value-of select='@stay mod 60'/> 分钟</p>
		<p>发表大作: <xsl:value-of select='@post'/> 篇</p>
		<p>帐号建立: <xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@since'/></xsl:call-template></p>
		<p>最近光临: <xsl:call-template name='timeconvert'><xsl:with-param name='time' select='@last'/></xsl:call-template></p>
		<p>来源地址: <xsl:value-of select='@host'/></p>
	</xsl:when>
	<xsl:otherwise><xsl:choose><xsl:when test='string-length(.) = 0'>修改个人资料成功<br/><a href='javascript:history.go(-2)'>返回</a></xsl:when><xsl:otherwise><xsl:value-of select='.'/></xsl:otherwise></xsl:choose></xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match='bbspwd'>
	<xsl:choose>
		<xsl:when test='@i'><form action='pwd' method='post'>
			<p><label for='pw1'>您的旧密码: </label><input maxlength='12' size='12' type='password' name='pw1'/></p>
			<p><label for='pw2'>输入新密码: </label><input maxlength='12' size='12' type='password' name='pw2'/></p>
			<p><label for='pw3'>确认新密码: </label><input maxlength='12' size='12' type='password' name='pw3'/></p>
			<input type='submit' value='确定修改'/>
		</form></xsl:when>
		<xsl:otherwise>
			<xsl:choose><xsl:when test='string-length(.)=0'>修改密码成功<br/><a href='javascript:history.go(-2)'>返回</a></xsl:when><xsl:otherwise><xsl:value-of select='.'/><br/><a href='javascript:history.go(-1)'>返回</a></xsl:otherwise></xsl:choose>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match='bbsnot'>
	<h2>进版画面 - [<xsl:value-of select='@brd'/>]</h2>
	<div class='post'>
		<div class='ptop'>
			<xsl:if test='@brd'>
				<a href='gdoc?board={@brd}'>文摘区</a>
				<a href='doc?board={@brd}'><img src='../images/button/home.gif'/>本讨论区</a>
			</xsl:if>
		</div>
		<div class='usmd'><xsl:call-template name='showpost'><xsl:with-param name='content' select='node()[1]'/></xsl:call-template></div>
	</div>
</xsl:template>

<xsl:template name='not-navbar'>
</xsl:template>

<xsl:template match='bbsreg'>
	<xsl:if test='@error=1'>
		<p>注册帐号出错：<xsl:value-of select='.'/></p>
		<p><a href='javascript:history.go(-1)'>快速返回</a></p>
	</xsl:if>
	<xsl:if test='@error=0'>
		<p>注册成功！</p>
		<p><a href='http://mail.fudan.edu.cn/'>点击此处登录复旦邮箱，查收激活链接</a></p>
	</xsl:if>
</xsl:template>

<xsl:template match='bbsactivate'>
	<xsl:if test='@success=1'>
		<p>帐号成功激活！</p>
		<p><a href='login?next=sec'>现在登录</a></p>
	</xsl:if>
	<xsl:if test='@success=0'>
		<p>帐号激活失败 :( 请检查激活链接</p>
	</xsl:if>
</xsl:template>

</xsl:stylesheet>
