
#include "SExpr.h"
#include "cmutex.h"

namespace { static_mutex m = STATIC_MUTEX_INIT; }

SExpr::SExpr() {
  this->fst_sexpr = "";
  this->flag = 0;
}

SExpr::~SExpr() {
}

int SExpr::out(std::string& val) {
  
  static_mutex::scoped_lock		sync(m);
  int	cnt = 0;
  
  if (this->list_que.size() <= 0) {
	val = "";
	return cnt;
  }

  val = "(" + this->fst_sexpr;
  
  while (1) {
	if (this->list_que.size() <= 0) {
	  val += ")";
	  break;
	} else {	
	  val += this->list_que.front();
	  this->list_que.pop_front();
	  val += " ";
	  ++cnt;
	}
  }
  
  return cnt;
}

int SExpr::out(std::string& val, int taskcnt) {
  
  static_mutex::scoped_lock		sync(m);
  std::ofstream					fout( "/tmp/sexpr_code.lsp", std::ios::app );
  int							cnt = 0;
  
  if (this->list_que.size() <= 0) {
	val = "";
	return cnt;
  }
  
  val = "(" + this->fst_sexpr;
  
  while (1) {
	if (this->list_que.size() <= 0 || taskcnt <= cnt) {
	  val += ")";
	  break;
	} else {	
	  val += this->list_que.front();
	  this->list_que.pop_front();
	  val += " ";
	  ++cnt;
	}
  }

  fout << val << "\n";
  fout.close();
  
  return cnt;
}

void SExpr::add_val(std::string val) {
  static_mutex::scoped_lock sync(m);
  this->list_que.push_front(val);
}

void SExpr::first(char *sexpr) {
  this->fst_sexpr = sexpr;
}

void SExpr::clear() {
  static_mutex::scoped_lock sync(m);
  this->list_que.clear();
}
