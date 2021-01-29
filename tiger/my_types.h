#include "types.h"
#include "ST.h"



struct bi_sym_table {
public:
  bi_sym_table(Ty_ty the_type, int the_reg = -1);

  Ty_ty _type;
  int _reg;

};

typedef ST<bi_sym_table> ST_bi;



/*
struct funct_sym_table {
public:
  funct_sym_table(Ty_ty return_type, Ty_tyList param_types = 0);

  //std::string _funct;
  Ty_ty _return_type;
  Ty_tyList _param_types;
//  Ty_tyList _param_type;


};

typedef ST<funct_sym_table> ST_Funct;
*/
