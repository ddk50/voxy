function createXMLHttpRequest(cbFunc){
	var XMLhttpObject = null;

	try{
		XMLhttpObject = new XMLHttpRequest();
	}catch(e){
		try{
			XMLhttpObject = new ActiveXObject("Msxml2.XMLHTTP");
		}catch(e){
			try{
				XMLhttpObject = new ActiveXObject("Microsoft.XMLHTTP");
			}catch(e){
				return null;
			}
		}
	}
	
	if(XMLhttpObject) {
		XMLhttpObject.onreadystatechange = cbFunc;
	}

	return XMLhttpObject;
}
