
#include "sexpr.hpp"

using namespace std;

SExpr::SExpr() { fst_sexpr = ""; }
SExpr::~SExpr() {}

int SExpr::out(string& val)
{ 
  int	cnt = 0;  
  
  if (list_que.size() <= 0) {	
	val = "";
	return cnt;
  }

  val = "(" + fst_sexpr;  
  
  while (1) {
	if (list_que.size() <= 0) {	  
	  val += ")";
	  break;
	} else {	
	  val += list_que.front();
	  list_que.pop_front();	  
	  val += " ";
	  ++cnt;
	}
  }
  
  return cnt;
}

int SExpr::out(string& val, int taskcnt)
{    
  ofstream fout("/tmp/sexpr_code.lsp", std::ios::app);  
  int	   cnt = 0;  
  
  if (list_que.size() <= 0) {	
	val = "";
	return cnt;
  }
  
  val = "(" + fst_sexpr;  
  
  while (1) {
	if (list_que.size() <= 0 || taskcnt <= cnt) {	  
	  val += ")";
	  break;
	} else {	
	  val += list_que.front();
	  list_que.pop_front();	  
	  val += " ";
	  ++cnt;
	}
  }

  fout << val << "\n";
  fout.close();  
  
  return cnt;
}

void SExpr::add_val(string& val)
{  
  list_que.push_front(val);  
}

void SExpr::first(string& val)  
{  
  fst_sexpr = val; 
}

void SExpr::clear()
{  
  this->list_que.clear();  
}
