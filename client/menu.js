
var cMenu = new Object();

cMenu["lookup1"] =
		{ menuID: "contextMenu1",
		  tgtElem: null,
		  hrefs:[ (function () {
						alert( "select lookup1 0" );
					}),
				  (function () {
						alert( "select lookup1 1" );
					})]
		};

cMenu["lookup2"] =
		{ menuID: "contextMenu2",
		  tgtElem: null,
		  hrefs:[ (function () {
						getnewid();
					}),
				  (function () {
						execmachine();
					})]
		};

// position and display context menu
function showContextMenu(evt) {
  
  // hide any existing menu just in case
  hideContextMenus();
  evt = (evt) ? evt : ((event) ? event : null);
  if (evt) {
    var elem = (evt.target) ? evt.target : evt.srcElement;
    if (elem.nodeType == 3) {
      elem = elem.parentNode;
    }

	// find lookup table
    if (elem.className == "contextEntry") {
      var menu = document.getElementById(cMenu[elem.id].menuID);
	  
	  elem.style.border = "solid";
	  
      // turn on IE mouse capture
      if (menu.setCapture) {
        menu.setCapture();
      }

	  // position menu at mouse event location
      var left, top;
      if (evt.pageX) {
        left = evt.pageX;
        top = evt.pageY;
      } else if (evt.offsetX || evt.offsetY) {
        left = evt.offsetX;
        top = evt.offsetY;
      } else if (evt.clientX) {
        left = evt.clientX;
        top = evt.clientY;
      }
	  
      menu.style.left = left + "px";
      menu.style.top = top + "px";
      menu.style.visibility = "visible";
	  //menu.style.border = "double";
	  
      if (evt.preventDefault) {
        evt.preventDefault();
      }
      evt.returnValue = false;
    }
  }
}

// retrieve URL from cMenu object related to chosen item
function getFunc(tdElem) {
  var div = tdElem.parentNode.parentNode.parentNode.parentNode;
  var index = tdElem.parentNode.rowIndex;
  for (var i in cMenu) {
    if (cMenu[i].menuID == div.id) {
	  return cMenu[i].hrefs[index];
    }
  }
  return (function () { alert ("Can't get menu handler") });
}

// navigate to chosen menu item
function execMenu(evt) {
  
  evt = (evt) ? evt : ((event) ? event : null);
  if (evt) {
    var elem = (evt.target) ? evt.target : evt.srcElement;
    if (elem.nodeType == 3) {
      elem = elem.parentNode;
    }
    if (elem.className == "menuItemOn") {
	  var func = getFunc(elem);
	  func();
    }
    hideContextMenus();
  }
}

// hide all context menus
function hideContextMenus() {
  
  if (document.releaseCapture) {
        // turn off IE mouse event capture
    document.releaseCapture();
  }
  for (var i in cMenu) {
    var div = document.getElementById(cMenu[i].menuID)
    div.style.visibility = "hidden";
  }
}

// rollover highlights of context menu items
function toggleHighlight(evt) {
  evt = (evt) ? evt : ((event) ? event : null);
  if (evt) {
    var elem = (evt.target) ? evt.target : evt.srcElement;
    if (elem.nodeType == 3) {
      elem = elem.parentNode;
    }
    if (elem.className.indexOf("menuItem") != -1) {
      elem.className = (evt.type == "mouseover") ? "menuItemOn" : "menuItem";
    }
  }
}

// set tooltips for menu-capable and lesser browsers
function setContextTitles() {
  var cMenuReady = (document.body.addEventListener || typeof document.oncontextmenu != "undefined")
  var spans = document.body.getElementsByTagName("span");
  for (var i = 0; i < spans.length; i++) {
    if (spans[i].className == "contextEntry") {
      if (cMenuReady) {
        var menuAction = (navigator.userAgent.indexOf("Mac") != -1) ? "Click and hold " : "Right click ";
        spans[i].title = menuAction + "to view relevant links"
      } else {
        spans[i].title = "Relevant links available with other browsers (IE5+/Windows, Netscape 6+)."
        spans[i].style.cursor = "default";
      }
    }
  }
}

function MouseMove(evt) {
  evt = (evt) ? evt : ((event) ? event : null);
  if (evt) {
	var elem = (evt.target) ? evt.target : evt.srcElement;
	if (elem.nodeType == 3) {
	  elem = elem.parentNode;
	}
	
	var left, top;
    if (evt.pageX) {
      left = evt.pageX;
      top = evt.pageY;
    } else if (evt.offsetX || evt.offsetY) {
      left = evt.offsetX;
      top = evt.offsetY;
    } else if (evt.clientX) {
      left = evt.clientX;
      top = evt.clientY;
    }

	left = evt.clientX - 7;
	top = evt.clientY - 40;
	
	//document.getElementById("testForm").value
	// = elem.id + "=" + "(" + left + ", " + top + ") " + nowmbf;
	
	nowx = left; nowy = top;
  }
}

function OnMouseDown(evt) {

  evt = (evt) ? evt : ((event) ? event : null);
  if (evt) {
	nowmbf |= evt.button;
  }

  evt.cancelBubble = true;
  evt.returnValue = false;
  
  if ( evt.stopPropagation ) {
	evt.stopPropagation();
	evt.preventDefault();
  }

  sendmouseevt( nowx, nowy, nowmbf );
  
  return false
}

function OnMouseUp(evt) {

  evt = (evt) ? evt : ((event) ? event : null);
  if (evt) {
	nowmbf &= (~evt.button);
  }

  evt.cancelBubble = true;
  evt.returnValue = false;

  if ( evt.stopPropagation ) {
	evt.stopPropagation();
	evt.preventDefault();
  }
  
  sendmouseevt( nowx, nowy, nowmbf );
  
  return false;
}

function KeyDown(evt) {
  
  evt = (evt) ? evt : ((event) ? event : null);
  if (evt) {
	
	var kstr;
	var key;
	
	key = (document.all) ? (evt.keyCode) : (evt.which);
	kstr = String.fromCharCode(key);
	
	if ((keysymtable[ key ] == null) | (keysymtable[ key ] == "")) {
	  key = key;
	} else {
	  key = keysymtable[ key ];
	}
	
	sendkeyevt( key, 1 );
  }
  return false;
}

function KeyUp(evt) {
  evt = (evt) ? evt : ((event) ? event : null);
  if (evt) {
	var kstr;
	var key;
	
	key = (document.all) ? (evt.keyCode) : (evt.which);
	kstr = String.fromCharCode(key);

	if ((keysymtable[ key ] == null) | (keysymtable[ key ] == "")) {
	  key = key;
	} else {
	  key = keysymtable[ key ];
	}
	
	sendkeyevt( key, 0 );
  }
  return false;
}

function isSpecialKey() {
  
}

// bind events and initialize tooltips
function initContextMenus() {
  
  if (document.body.addEventListener) {
	// W3C DOM event model
    //document.body.addEventListener("contextmenu", showContextMenu, true);
    //document.body.addEventListener("click", hideContextMenus, true);
	document.body.addEventListener("onmousedown", OnMouseDown, true);
	document.body.addEventListener("onmouseup", OnMouseUp, true);
	document.body.addEventListener("onmousemove", MouseMove, true);
	document.body.addEventListener("keydown", KeyDown, true);
	document.body.addEventListener("keyup", KeyUp, true);

	window.onmousedown = OnMouseDown;
	window.onmouseup   = OnMouseUp;
	window.onmousemove = MouseMove;
	//window.contextmenu = showContextMenu;
	//window.click	   = hideContextMenus;
	window.keydown	   = KeyDown;
	window.keyup	   = KeyUp;
	
  } else {
	
	// IE event model
    document.body.oncontextmenu = showContextMenu;
	document.body.onmousemove	= MouseMove;
	document.body.onmousedown	= OnMouseDown;
	document.body.onmouseup     = OnMouseUp;
	document.body.onkeydown		= KeyDown;
	document.body.onkeyup		= KeyUp;
  }

  // set intelligent tooltips
  //setContextTitles();

  // init test.js;
  init();
}
