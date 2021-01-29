#include "AST.h"


ST_bi A_root_::init_ST()
{
return  merge(ST_bi(to_Symbol("size"), bi_sym_table(Ty_Function(Ty_Int(), Ty_FieldList(Ty_Field(to_Symbol("s"), Ty_String()), 0)))),
          merge(ST_bi(to_Symbol("concat"), bi_sym_table(Ty_Function(Ty_String(), Ty_FieldList(Ty_Field(to_Symbol("s1"), Ty_String()),
                                                              Ty_FieldList(Ty_Field(to_Symbol("s2"), Ty_String()), 0))))),
          merge(ST_bi(to_Symbol("tstrcmp"), bi_sym_table(Ty_Function(Ty_Int(), Ty_FieldList(Ty_Field(to_Symbol("s1"), Ty_String()),
                                                              Ty_FieldList(Ty_Field(to_Symbol("s2"), Ty_String()), 0))))),
          merge(ST_bi(to_Symbol("div"), bi_sym_table(Ty_Function(Ty_Int(), Ty_FieldList(Ty_Field(to_Symbol("num"), Ty_Int()),
                                                              Ty_FieldList(Ty_Field(to_Symbol("den"), Ty_Int()), 0))))),
        //  merge(ST_bi(to_Symbol("mod"), bi_sym_table(Ty_Function(Ty_Int(), Ty_FieldList(Ty_Int(), Ty_FieldList(Ty_Int(), 0))))),
          merge(ST_bi(to_Symbol("not"), bi_sym_table(Ty_Function(Ty_Bool(), Ty_FieldList(Ty_Field(to_Symbol("b"), Ty_Bool()), 0)))),
          //skipped a few here
          merge(ST_bi(to_Symbol("printint"), bi_sym_table(Ty_Function(Ty_Void(),  Ty_FieldList(Ty_Field(to_Symbol("i"), Ty_Int()), 0)))),
          merge(ST_bi(to_Symbol("printbool"), bi_sym_table(Ty_Function(Ty_Void(),  Ty_FieldList(Ty_Field(to_Symbol("b"), Ty_Bool()), 0)))),
          merge(ST_bi(to_Symbol("print"), bi_sym_table(Ty_Function(Ty_Void(),  Ty_FieldList(Ty_Field(to_Symbol("s"), Ty_String()), 0)))),
          //skipped a few here
          merge(ST_bi(to_Symbol("getint"), bi_sym_table(Ty_Function(Ty_Int(), 0))),

          merge(ST_bi(to_Symbol("exit"), bi_sym_table(Ty_Function(Ty_Void(), Ty_FieldList(Ty_Field(to_Symbol("i"), Ty_Int()), 0)))),
          merge(ST_bi(to_Symbol("malloc"), bi_sym_table(Ty_Function(Ty_Int(), Ty_FieldList(Ty_Field(to_Symbol("n_cells"), Ty_Int()), 0)))),
                ST_bi(to_Symbol("free"), bi_sym_table(Ty_Function(Ty_Void(), Ty_FieldList(Ty_Field(to_Symbol("address"), Ty_Int()), 0))))


          ))))))))))
        );
}


void AST_node_::set_ST() {
  this->bi_ST = this->parent()->get_ST();
}

void AST_node_::set_ST_to(ST_bi new_ST)
{
  this->bi_ST = new_ST;
}

void A_root_::set_ST() {
  this->bi_ST = this->init_ST();
}

ST_bi AST_node_::get_ST() {
  if (!is_name_there(to_Symbol("getint"), this->bi_ST)) { this->set_ST(); }
  return bi_ST;
}

void AST_node_::merge_ST(ST_bi to_add) {
  bi_ST = merge(to_add, this->get_ST());
}

// code to set let's ST
void A_letExp_::send_or_recieve_ST(ST_bi new_ST)
{
  this->set_ST_to(new_ST);
}

void A_decList_::send_or_recieve_ST(ST_bi new_ST)
{
  this->set_ST_to(new_ST);
  this->parent()->send_or_recieve_ST(new_ST);
}


void A_varDec_::send_or_recieve_ST(ST_bi new_ST)
{
  this->parent()->send_or_recieve_ST(new_ST);
}
