
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
	document.getElementById("testForm").value
	  = elem.id + "=" + "(" + evt.clientX + ", " + evt.clientY + ")";
  }
}

