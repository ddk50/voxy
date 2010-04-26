
#include		<iostream>

class CVncErr {

private:
  std::string	err_reason;
  
public:

  CVncErr(std::string err_val);
  ~CVncErr();
  
  std::string what();
  void set_errval(std::string res);
  
};

