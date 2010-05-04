
#ifndef	_SEXPR_H
#define	_SEXPR_H

#include <string>
#include <queue>
#include <deque>
#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;

//
//                    @@@ The list of command @@@
//
// [ management-server ]                             [ httpd-frontend ]
//           |             update mosic tiles                    |
//           |       --(:UPDATATILE fname x y w h)-->            |
//           |                                                   |
//           |             update resolution                     |
//           |           --(:CHNGRESOL w h)-->                   |
//           |                                                   |
//           |         <--(:MOUSEEVENT x y btnnum)--             |
//           |          <--(:KEYEVENT string)--                  |
//           |                                                   |
//           |           make new vnc connection                 |
//           |        <--(:VNCCONNECT hostname password)--       |
//           |                                                   |
//           |         destroy current vnc connection            |
//           |            <--(:VNCDISCONNECT)--                  |

class SExpr
{  
protected:
  std::string	fst_sexpr;
  std::deque<std::string> list_que;  
  
public:
  SExpr();
  virtual ~SExpr();

  int out(std::string& val);
  int out(std::string& val, int taskcnt);

  void first(std::string& val);  
  void clear();
  virtual void add_val(std::string &val);  
};

class sparser
{
private:

  
  int read_next_delimita(string& in,						 
						 string& token,						 
						 int start,						 
						 const char delim)	
  {	
	unsigned int i = start;	
	for (; i < in.size() && in[i] != delim ; i++) {	  
	  token.push_back(in[i]);	  
	}	
	return i;	
  }  
  
public:  
  sparser() {};    
  virtual ~sparser() {};

  //  
  // This function does  not parse nested s-expression well.  
  //  
  int read_expression(string& s,
					  deque<string>& tokens,					  
					  int bracket_level = 0)	
  {
	char c = s[0];
	unsigned int n = 0;	
	
	for (; n < s.size() ; c = s[n]) {	  
	  if (std::isspace(c)) {		
		n++;		
		continue;		
	  } else if (c == '\"') {		
		string token = "";		
		n = read_next_delimita(s, token, n + 1, '\"');		
		tokens.push_back(token);		
	  } else if (c == '(') {		
		string rest = "";		
		n = read_next_delimita(s, rest, n + 1, ')');		
		read_expression(rest, tokens, bracket_level++);		
	  } else if (c == ':' ||
				 std::isalpha(c) ||
				 std::isalnum(c)) {		
		string token = "";		
		n = read_next_delimita(s, token, n, ' ');		
		tokens.push_back(token);		
	  }	  
	  n++;	  
	}
	return bracket_level;	
  }  
  
};

#endif
