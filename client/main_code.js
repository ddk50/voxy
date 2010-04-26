
var canvas_margin_x = 8;
var canvas_margin_y = 220;
var host_root = "http://192.168.24.53/";
var cgi_path = "cgi-bin/qemu_vnc/vnc2.cgi?";
var user_id = 0;
var mouseX, mouseY;
var postX, postY;

var tokenize = function (str) {
  //return str.match(/\"(?:[^"])*\"|\|[^\|]*\||(\)|\(|\s+|[^\(\)\s]+)/gm).filter(/^[^\s]/);
};

function ctoken_get_token() {
  if (this.token_list.length <= this.lex_pos) {
	return "DONE";
  }
  return this.token_list[this.lex_pos++];
}

function ctoken(xmlhttp) {
  var tmpstr = new String (tokenize(xmlhttp.responseText));
  this.token_list = tmpstr.split(",");
  this.lex_pos = 0;
  this.get_token = ctoken_get_token;
}

function emuexec_comet() {
  
  var xmlhttp = createXMLHttpRequest();
  var submit_cmd = "(execemu+" + user_id + ")";
  
  if(xmlhttp != null){
	xmlhttp.open("GET", cgi_path + submit_cmd, true);
	xmlhttp.setRequestHeader("content-type",
							 "application/x-www-form-urlencoded;charset=UTF-8");
	xmlhttp.onreadystatechange = function() {
	  if(xmlhttp.readyState == 4 && xmlhttp.status == 200){
		if(xmlhttp.responseText == "500 ok\n"){
		  document.getElementById("testForm").value = submit_cmd;
		}else{
		  document.getElementById("testForm").value = submit_cmd;
		}
	  }
	}
	xmlhttp.send(null);
  }
}

function mousemov_coment(x, y) {
  
  var xmlhttp	= createXMLHttpRequest();
  var down_flag	= 1;
  var submit_cmd
	= "(mousemove+" + user_id + "+" + down_flag + "+" + x + "+" + y + ")";
  
  if(xmlhttp != null){
	xmlhttp.open("GET", cgi_path + submit_cmd, true);
	xmlhttp.setRequestHeader("content-type",
							 "application/x-www-form-urlencoded;charset=UTF-8");
//	xmlhttp.onreadystatechange = function(){
//	  if(xmlhttp.readyState == 4 && xmlhttp.status == 200){
//		if(xmlhttp.responseText == "500 ok\n"){
//		  document.getElementById("testForm").value = "executed emulator";
//		}else{
//		  document.getElementById("testForm").value = "can not startup emulator";
//		}
//	  }
//	}
	xmlhttp.send(null);
  }
  
}

function getuser_comet() {

  var xmlhttp = createXMLHttpRequest();
  var submit_cmd = "(getuser)";

  if(xmlhttp != null){
	this.hashed = 0;
	xmlhttp.open("GET", cgi_path + submit_cmd, true);
	xmlhttp.setRequestHeader("content-type",
							 "application/x-www-form-urlencoded;charset=UTF-8");
	xmlhttp.onreadystatechange = function(){
	  
	  if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
		
		var token = new ctoken(xmlhttp);
		var next_token = token.get_token();
		
		if (next_token == "(" ){
		  next_token = token.get_token();
		  
		  if (next_token == "SETUSER") {
			next_token = token.get_token();
			user_id = parseInt(next_token);
			document.title = "user id: " + user_id;
		  }
		} else {
		  document.title = "error " + token;
		}
	  }
	}
	xmlhttp.send(null);
  }
}

function load() {
  setTimeout('javascript:reload();', 1000);
  //setTimeout('javascript:updateMouse();', 250);
}

function read_list(ctoken) {
  
  while (1) {
	var next_token = ctoken.get_token();
	if (next_token == ")") {
	  break;
	} else if (next_token == "DONE") {
	  throw "illegal S expr";
	}
	read_s_next_expr(ctoken, next_token);
  }
}

function read_s_next_expr(ctoken, next_token) {
  
  if (next_token == "(") {
	read_list(ctoken);
  } else if (next_token == "RFBQUE") {
	return;
  } else if (next_token == "RECTDATA") {
	draw_rectdata(ctoken);
  } else if (next_token == "BEEP") {
	beep(ctoken);
  } else if (next_token == "DONE") {
	return;
  } else if (next_token == "ERROR") {
	throw "server error";
  } else {
	throw ("s expr error. illegal symbol: " + next_token);
	return;
  }
}

function read_s_expr(ctoken) {

  try {
	while (1) {
	  var next_token = ctoken.get_token();
	  if (next_token == "DONE") {
		break;
	  }
	  read_s_next_expr(ctoken, next_token);
	}
  } catch (err) {
	document.getElementById("testForm").value = err;
  }
}

function reload() {

  var xmlhttp = createXMLHttpRequest();
  var submit_cmd = "(listen+" + user_id + ")";
  var ind_cmd_lst;
  if(xmlhttp != null){
	xmlhttp.open("GET", cgi_path + submit_cmd, true);
	xmlhttp.setRequestHeader("content-type",
							 "application/x-www-form-urlencoded;charset=UTF-8");
	this.hashed = 0;
	xmlhttp.onreadystatechange = function() {
	  
	  if (xmlhttp.readyState == 4) {
		if (xmlhttp.status == 200) {
		  var token = new ctoken(xmlhttp);
		  read_s_expr(token);
		}
		reload();
		//setTimeout('javascript:reload();', 500);
	  }
	}
	xmlhttp.send(null);
  }
}

function draw_rectdata(ctoken) {
  
  var xpos, ypos;
  var width, height;
  var path;
  var canvas = document.getElementById('c1');
  
  if(! canvas || ! canvas.getContext){
	throw "Canvas was not init";
  }

  path = ctoken.get_token();
  xpos = parseInt(ctoken.get_token());
  ypos = parseInt(ctoken.get_token());
  width = parseInt(ctoken.get_token());
  height = parseInt(ctoken.get_token());
  
  var ctx = canvas.getContext('2d');
  var img = new Image();

  img.src = host_root + path;
  img.onload = function(){
	//ctx.fillStyle = "rgb(0,0,0)";
	//ctx.fillRect(xpos, ypos, width, height);
	ctx.clearRect(xpos, ypos, width, height);
	ctx.drawImage(img, xpos, ypos);
  };
}

function beep(xmlhttp) {
  alert('beep');
}

function createXMLHttpRequest() {

  try {
	XMLhttpObject = new XMLHttpRequest();
  } catch (trymicrosoft) {
	try {
	  XMLhttpObject = new ActiveXObject("Msxml2.XMLHTTP");
	} catch (othermicrosoft) {
	  try {
		XMLhttpObject = new ActiveXObject("Microsoft.XMLHTTP");
	  } catch (failed) {
		XMLhttpObject = null;
	  }
	}
  }

  if (XMLhttpObject == null) {
	alert(" error! can not create XMLhttpObject");
  }

  return XMLhttpObject;
}

function updatePage() {

  var response_data;
  
  if(XMLhttpObject.readyState == 4){
	response_data = XMLhttpObject.responseText;
	document.getElementById("orderForm").value = response_data;
  }
}

function updateMouse() {
  
  if(postX != mouseX || postY != mouseY){
	postX = mouseX;
	postY = mouseY;
	document.getElementById("testForm").innerHTML = "(x,y) = " + postX + ", " + postY;

	//mousemov_coment(postX, postY);
  }
  setTimeout('javascript:updateMouse();', 250);
}

function get_cgi_info() {

  if(XMLhttpObject){
	XMLhttpObject.open("GET", "cgi-bin/index.cgi?request=testreq", true);
	XMLhttpObject.onreadystatechange = updatePage;
	XMLhttpObject.send(null);
  }
}

function getMouseXY_ie() {

  x = event.pageX;
  y = event.pageY;

  x -= canvas_margin_x;
  y -= canvas_margin_y;

  if(x < 0){
	x = 0;
  }

  if(y < 0){
	y = 0;
  }

  //document.getElementById("testForm").innerHTML = "(x,y) = "+x + ", "+y;
  mouseX = x; mouseY = y;
  
}

function getMouseXY(evt) {
  
  x = evt.pageX;
  y = evt.pageY;
  
  x -= canvas_margin_x;
  y -= canvas_margin_y;

  if(x < 0){
	x = 0;
  }

  if(y < 0){
	y = 0;
  }

  mouseX = x; mouseY = y;
  
}

function getKeyevent_ie() { /* IE */
  kcode = event.keyCode;
  kstr = String.fromCharCode(kcode);

  document.getElementById("testForm").innerHTML = "キーコード:"
	+ kcode + "\nキー：" + kstr;
}

function getKeyevent(event) {
  
  kmd = event.modifiers;
  kcode = event.which;
  kstr = String.fromCharCode(kcode);

  //document.getElementById("testForm").innerHTML = "キーコード:"
	//+ kcode + "\nキー：" + kstr;

  //alert("キーコード:" + kcode + "\nキー：" + kstr);
}

function getStyleValue(selector, property, sheetindex ){
  
  selector = selector.toLowerCase();
  
  if( sheetindex == undefined ){
	sheetindex = 0;
  }
  
  if(property.indexOf( "-" ) != -1){
	property = property.camelize();
  }
	
  var rules = document.styleSheets[ sheetindex ].rules //IE
      || document.styleSheets[ sheetindex ].cssRules; //Mozilla

  for( var i = rules.length - 1; i >= 0; i-- ) {
      var rule = rules[i];
      if( rule.selectorText.toLowerCase( ) != selector
      || rule.style[ property ] == "" ) continue;
      return rule.style[ property ];
  }
  
  return null;
}

function canvas_init(width, height, Color){
  
  Dot = document.createElement("span", "Dot");
  Dot.style.width		=  width + "px";
  Dot.style.height		= height + "px";
  Dot.style.background	= Color;
  Dot.style.border		= "0px";
  Dot.style.position	= "absolute";
  Dot.style.zIndex		= "10000";
  
}

function drawDot(x, y){
  
  var dot = Dot.cloneNode(true);

  dot.style.left = x + "px";
  dot.style.top = y + "px";

  document.body.appendChild(dot);
  
}

function drawShape(){
  
  var canvas = document.getElementById('tutorial');
  
  if (canvas.getContext){
    var ctx = canvas.getContext('2d');
	
  }
}

document.onkeydown = (document.all) ? getKeyevent_ie : getKeyevent;
document.onmousemove = (document.all) ? getMouseXY_ie : getMouseXY;
