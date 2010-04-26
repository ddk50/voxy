
#ifndef	_SEXPR_H
#define	_SEXPR_H

#include <string>
#include <queue>
#include <iostream>
#include <fstream>

class SExpr {

protected:
  int	flag;
  std::string	fst_sexpr;
  std::deque<std::string> list_que;
  
public:
  SExpr();
  virtual ~SExpr();

  int out(std::string& val);
  int out(std::string& val, int taskcnt);

  void first(char *sexpr);
  void clear();
  virtual void add_val(std::string val);
  
};

#endif
