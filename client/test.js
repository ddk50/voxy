
var g_imglist = {};
var cgi_path = "";

var userid = 0;

var xpos, ypos, bf;
var nowx, nowy, nowmbf;
var kcode, kflag, keyevent;

var terminated      = 0;
var listen_interval = 100;
var mouse_interval  = 50;

var listen_xmlhttp      = null;
var keyevt_push_xmlhttp = null;
var keyevt_up_xmlhttp   = null;
var mouse_xmlhttp       = null;

function stop_listen() 
{
    terminated = 1;
    alert("stop");
}

function init() 
{
    xpos = ypos = nowx = nowy = 0;
    nowmbf = bf = 0;
    kcode = kflag = 0;
  
    if (!terminated)
	document.getElementById('logarea').value = "(setf *listen-delay-lst* \'( ";
    
    history.forward();

    keyevt_push_xmlhttp = new createXMLHttpRequest();
    keyevt_up_xmlhttp   = new createXMLHttpRequest();
    listen_xmlhttp      = new createXMLHttpRequest();
    mouse_xmlhttp       = new createXMLHttpRequest();
    head_xmlhttp        = new createXMLHttpRequest();
}

function createXMLHttpRequest() 
{
    var XMLhttpObject = null;  
    try {
	XMLhttpObject = new XMLHttpRequest();
    } catch (e) {
	try {
	    XMLhttpObject = new ActiveXObject("Msxml2.XMLHTTP");
	} catch (e) {
	    try {
		XMLhttpObject = new ActiveXObject("Microsoft.XMLHTTP");
	    } catch (e) {
		return null;
	    }
	}
    }

    if (XMLhttpObject == null)
	alert(" error! can not create XMLhttpObject");
  
    return XMLhttpObject;
}

function processReqChange(req, starttime) 
{  
    if (req.readyState == 4 && req.status == 200 
	&& req.responseXML != null) {
	
	var items = [];
	var nl = req.responseXML.getElementsByTagName('slide');
		
	for (var i = 0 ; i < nl.length ; i++) {	    
	    var nli    = nl.item(i);
	    var src    = nli.getAttribute('src').toString();
	    var id     = nli.getAttribute('id').toString();
	    var xpos   = parseInt(nli.getAttribute('xpos').toString());
	    var ypos   = parseInt(nli.getAttribute('ypos').toString());
	    var width  = parseInt(nli.getAttribute('width').toString());
	    var height = parseInt(nli.getAttribute('height').toString());

	    // { src:    'pngtest/1808047150.png',
	    //   id:     1808047150,
	    //   xpos:   0,
	    //   ypos:   0,
	    //   width:  200,
	    //   height: 200 };
		  
	    items[items.length] = { 
		src:    src, 
		id:     id,
		xpos:   xpos, 
		ypos:   ypos,
		width:  width,
		height: height 
	    };
	}
		
	if (nl.length > 0) {
	    var last_src = items[nl.length - 1].src;
	    load_slides(items, last_src, starttime);
	} else {
	    if (!terminated)
		setTimeout("javascript:listen();", listen_interval);
	}
    }
}

function load_slides(images, last_src, starttime) 
{  
    var ic = $('#lookup2');
    
    for (i = 0 ; i < images.length ; i++) {	
	var img      = images[i];
	var need_add = 0;
	var imgObj;
	
	imgObj = document.getElementById(img.id);
	if (imgObj == null || imgObj == "") {
	    imgObj = document.createElement('img');
	    need_add = 1;
	}
	
	if (last_src == img.src) {
	    if (!terminated) {
		imgObj.onload = function() {
		    end = (new Date()).getTime();
		    document.getElementById('logarea').value += ((end - starttime) + " ");
		    setTimeout("javascript:listen();", 1);
		}
	    }
	} else {
	    // very important!!!!!!!!!!!!!!!
	    imgObj.onload = null;
	}
	
	imgObj.style.position   = 'absolute';
	imgObj.style.left       = img.xpos + 'px';
	imgObj.style.top        = img.ypos + 'px';
	imgObj.style.visibility = 'visible';
	imgObj.id               = img.id;
	imgObj.src              = img.src;
	
	if (need_add)
	    ic.appendChild(imgObj);
    }
}

function checkimgdata(paths) 
{
    var xmlhttp = head_xmlhttp;

    for (i = 0 ; i < paths.length ; i++ ) {	
	var img = paths[i];
	
	xmlhttp.open("HEAD", "http://tertes.homelinux.com/qemuvnchtml/" + paths.src, true);
	xmlhttp.onreadystatechange = function() {
	    if (xmlhttp.readyState == 4 && xmlhttp.status == 200
		 && xmlhttp.responseXML != null) {
		
		var imgObj;
		var need_add = 0;

		alert(xmlhttp.responseXML);
			
		imgObj = document.getElementById(img.id);
		if (imgObj == null || imgObj == "") {
		    imgObj = document.createElement('img');
		    need_add = 1;
		}
			
		imgObj.style.position   = 'absolute';
		imgObj.style.left       = img.xpos + 'px';
		imgObj.style.top        = img.ypos + 'px';
		imgObj.style.visibility = 'visible';
		imgObj.id               = img.id;
		imgObj.src              = img.src;
			
		if (need_add)
		    ic.appendChild(imgObj);
	    }
	}
	setTimeout("javascript:test_load_slides();", 100);
	xmlhttp.send(null);
    }
}

function fail()
{
    $("#failed").show();
}

function getnewid() 
{  
    $.getJSON(cgi_path + "/getuser", function(data) {
        user_id = parseInt(data.id);
                
        // imgObj = document.createElement('h1');

        // imgObj.style.position = "absolute";
        // imgObj.style.left	  = "40%";
        // imgObj.style.top	  = "40%";
        // imgObj.id		  = newid;

        // txt = document.createTextNode(newid);
        // imgObj.appendChild(txt);

        // ic.appendChild(imgObj);
        // ic.style.background = "#000000";
        // ic.style.color      = "#FFFFFF";
    });
}

function start_UIListener() 
{
    setTimeout("javascript:listen();", 1000);
    setTimeout("javascript:do_sendmouse();", 1000);
}

function execmachine() 
{  
    $.getJSON(cgi_path + "/execemu/" + userid, function(data) {
        if(data.status != "ok") return; // FIXME: handle error

        
        $(document).oneTime(1000, listen);
        $(document).oneTime(1000, do_sendmouse);
    });
}

function listen() 
{
    var start = (new Date()).getTime();
    if (userid == 0) return;
  
    $.getJSON(cgi_path + "/listen/" + userid, function(data) {
        processReqChange( xmlhttp, start );
    });
}

function release() 
{
    var start      = (new Date()).getTime();
    var xmlhttp    = createXMLHttpRequest();
    var submit_cmd = "(release+" + userid + ")";
  
    if (xmlhttp != null) {
	xmlhttp.open("GET", cgi_path + "?" + submit_cmd);
	xmlhttp.setRequestHeader("content-type",
				 "application/x-www-form-urlencoded;charset=UTF-8");
	xmlhttp.setRequestHeader("If-Modified-Since", "Thu, 01 Jun 1970 00:00:00 GMT");
	xmlhttp.onreadystatechange = function() {
	    if (xmlhttp.readyState == 4 && xmlhttp.status == 200
		 && xmlhttp.responseXML != null) {
		userid = 0; terminated = 1;
	    }
	}
	xmlhttp.send(null);
    }
}

function changeblocksize() 
{
    var start      = (new Date()).getTime();
    var xmlhttp    = createXMLHttpRequest();
    var blocklen   = parseInt(document.getElementById('blocklen').value);
    var submit_cmd = "(syscmd+" + userid + "+chgblocklen+" + blocklen + ")";    

    if (xmlhttp != null) {
	xmlhttp.open("GET", cgi_path + "?" + submit_cmd);
	xmlhttp.setRequestHeader("content-type",
				 "application/x-www-form-urlencoded;charset=UTF-8");
	xmlhttp.setRequestHeader("If-Modified-Since", "Thu, 01 Jun 1970 00:00:00 GMT");
	xmlhttp.onreadystatechange = function() {
	    if ( xmlhttp.readyState == 4 && xmlhttp.status == 200
		 && xmlhttp.responseXML != null ) {
		//userid = 0; terminated = 1;
		//start_UIListener();
	    }
	}
	xmlhttp.send(null);
    }
}

function change_intervals() 
{
    if (userid == 0)
	return;
  
    release();
  
    listen_interval = parseInt(document.getElementById('interval').value);
    mouse_interval  = parseInt(document.getElementById('mouseinterval').value);
  
    //setTimeout( "javascript:getnewid();", 3000);
    getnewid();
    setTimeout("javascript:execmachine();", 5000);
}

function do_sendmouse() 
{
    if (userid == 0)
	return;

    sendmouseevt(nowx, nowy, nowmbf);
    setTimeout("javascript:do_sendmouse();", mouse_interval);  
}

function do_sendkey() 
{
    //sendkeyevt(kcode, kflag);
    //keyevent = 0;
    //setTimeout("javascript:do_sendkey();", mouse_interval);
}

function sendmouseevt(x, y, mbf) 
{
    if (xpos != x || ypos != y || bf != mbf) {	
	if (x < 0 || y < 0 || y == "undefined" || x == "undefined"
	    || mbf == "undefined") {
	    return;
	}
	
	var start = (new Date()).getTime();
	var xmlhttp = mouse_xmlhttp;
	var submit_cmd = "(mouse+" + userid + "+" + x + "+" + y + "+" + mbf + "+)";
	
	if (xmlhttp != null) {	 
	    xmlhttp.open("GET", cgi_path + "?" + submit_cmd);
	    xmlhttp.setRequestHeader("content-type",
				     "application/x-www-form-urlencoded;charset=UTF-8");
	    xmlhttp.onreadystatechange = function() {
		//processReqChange(xmlhttp, start);
	    }
	  
	    xmlhttp.send(null);
	    xpos = x; ypos = y; bf = mbf;
	}
    }
}

function sendkeyevt(keycode, flag) 
{
    //if (keyevent) {
    var start      = (new Date()).getTime();
    var xmlhttp    = flag ? keyevt_push_xmlhttp : keyevt_up_xmlhttp;
    var submit_cmd = "(key+" + userid + "+" + keycode + "+" + flag + "+)";

    if (xmlhttp != null) {
	xmlhttp.open("GET", cgi_path + "?" + submit_cmd);
	xmlhttp.setRequestHeader("content-type",
				 "application/x-www-form-urlencoded;charset=UTF-8");
	xmlhttp.onreadystatechange = function() {
	    //processReqChange( xmlhttp, start );
	}
	xmlhttp.send( null );	
    }
    //}
}
