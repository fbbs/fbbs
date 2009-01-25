var imgCollapse=new Image();
imgCollapse.src='/images/collapse.gif';

var imgExpand=new Image();
imgExpand.src='/images/expand.gif';

var imgNow=imgCollapse;

function switch_bar()
{
	var mybar=document.getElementById('mainbar');
	if(mybar.style.display!='none')
	{
		mybar.style.display='none';
		imgNow=imgExpand;
		parent.document.getElementById('bar').cols='6, *';
	}
	else
		{
			mybar.style.display='block';
			imgNow=imgCollapse;
			parent.document.getElementById('bar').cols='130, *';
		}
	document.getElementById('switchbar').style.backgroundImage='url('+imgNow.src+')';
}

var lastitem='undefined';

function SwitchPanel(itemid)
{
	if(lastitem!='undefined' && lastitem!=itemid) document.getElementById(lastitem).style.display='none';
	var item=document.getElementById(itemid).style;
	if(item.display=="block"){
		item.display="none";
		lastitem="undefined";
	}
	else{
		item.display="block";
		lastitem=itemid;
	}
	return false;
}

