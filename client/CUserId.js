
function CUserId( id, htmlObj, txtObj ) {

  this.id = id;

  this.txtObj = txtObj;
  this.htmlObj = htmlObj;

  this.htmlObj.style.position = 'absolute';
  this.htmlObj.style.left = '50%';
  this.htmlObj.style.top = '50%';
  this.htmlObj.id = 'userid';
  
}

CUserId.prototype.get_txt = function() {
  return this.txtObj;
}

CUserId.prototype.get_h1 = function() {
  return this.htmlObj;
}
