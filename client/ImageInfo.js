
function ImageInfo( src, xpos, ypos, width, height, htmlObj ){
  this.src = src;
  this.xpos = xpos;
  this.ypos = ypos;
  this.width = width;
  this.height = height;
  this.current_width = width;
  this.current_height = height;

  this.htmlObj = htmlObj;
  this.htmlObj.src = this.src;
  this.htmlObj.width = this.current_width;
  this.htmlObj.height = this.current_height;
}

ImageInfo.prototype.set_opacity = function( opacity ){
  
  this.htmlObj.style.MozOpacity = opacity / 100;
  var f = 'progid:DXImageTransform.Microsoft.Alpha(opacity='+opacity+')';
  this.htmlObj.style.filter = f;
  
}

ImageInfo.prototype.set_position = function( x, y ){
  
  this.htmlObj.style.left = x+'px';
  this.htmlObj.style.top = y+'px';
}

ImageInfo.prototype.set_size = function( w, h ){
  
  this.current_width = w;
  this.current_height = h;

  this.htmlObj.width = this.current_width;
  this.htmlObj.height = this.current_height;
}

ImageInfo.prototype.get_image = function(){
  return this.htmlObj;
}

ImageInfo.prototype.hide = function(){
  this.htmlObj.style.visibility = 'hidden';
}

ImageInfo.prototype.show = function(){
  this.htmlObj.style.visibility = 'visible';
}
