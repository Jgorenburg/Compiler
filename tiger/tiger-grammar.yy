// Note: to learn how to invoke bison/flex in Eclipse, see https://www.eclipse.org/forums/index.php/t/1075215/
// And refer to the example at https://www.gnu.org/software/bison/manual/html_node/A-Complete-C_002b_002b-Example.html#A-Complete-C_002b_002b-Example

%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define api.parser.class {tigerParser}

/* next two from https://www.gnu.org/software/bison/manual/html_node/Complete-Symbols.html#Complete-Symbols */
%define api.value.type variant
%define api.token.constructor

%code requires
{
class tigerParseDriver;
}
%param { tigerParseDriver& driver }

// According to the Example, this turns on "location tracking"
%locations
// Thanks to Ian Fisher HC '19 for explaining the details of the parts I'd failed to get working


%{
#include "tigerParseDriver.h"
%}

%token END  0
%token <bool> BOOL
%token <int>  INT
%token <std::string> ID STRING TYPE

// NOTE that bison complains if you have the same symbol listed as %token (above) and %type (below)
//      so if you want to add attributes to a token, remove it from the list below

%token
  COMMA COLON SEMICOLON LPAREN RPAREN L_SQUARE_BRACKET R_SQUARE_BRACKET
  L_CURLY_BRACE R_CURLY_BRACE
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END_LET OF
  BREAK NIL
  FUNCTION VAR DOT
  PLUS MINUS TIMES DIVIDE ASSIGN EQ NEQ LT LE GT GE OR AND NOT
;

/* precedence (stickiness) ... put the stickiest stuff at the bottom of the list */

%left COMMA SEMICOLON
%left WHILE FOR TO ASSIGN
%left DO
%left THEN
%left ELSE
%left IF
%left OR
%left AND
%left NOT
%left EQ NEQ LT LE GT GE
%left PLUS MINUS
%left TIMES DIVIDE
%left RPAREN LPAREN




/* Attributes types for nonterminals are next */
%type <expAttrs>  exp
%type <decAttrs>  dec
%type <expListAttrs> expList
%type <expListAttrs> seqExp
%type <decListAttrs> decList


// The line below means our grammar must not have conflicts
//  (no conflicts means it is "an LALR(1) grammar",
//   meaning it must be unambiguous and have some other properties).
%expect 0


%%

%start program;
program: exp[main]	{ EM_debug("Got the main expression of our tiger program.", $main.AST->pos());
		 			  driver.AST = new A_root_($main.AST);
		 			}
	;

exp:  INT[i]	  {
                  $$.AST = A_IntExp(Position::fromLex(@i), $i);
								  EM_debug("Got int " + str($i), $$.AST->pos());
								}
	| STRING[s]  	{
                  $$.AST = A_StringExp(Position::fromLex(@s), $s);
								  EM_debug("Got string " + $s, $$.AST->pos());
                }
  | BOOL[b]     {
                  $$.AST = A_BoolExp(Position::fromLex(@b), $b);
                  EM_debug("Got bool " + $b, $$.AST->pos());
								}
  | ID[i]       {
                  $$.AST = A_VarExp(Position::fromLex(@i), A_SimpleVar(Position::fromLex(@i), to_Symbol($i)));
                  EM_debug("Got var " + $i, $$.AST->pos());
                }
  | LPAREN RPAREN {
                  $$.AST = A_SeqExp(Position::undefined(), 0);
                  EM_debug("Got a null expression");
                }
	| exp[exp1] PLUS exp[exp2]	{
								  $$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
												   A_plusOp,  $exp1.AST,$exp2.AST);
								  EM_debug("Got plus expression.", $$.AST->pos());

								}
	| exp[exp1] TIMES exp[exp2]	{
                  $$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
												   A_timesOp, $exp1.AST,$exp2.AST);
								  EM_debug("Got times expression.", $$.AST->pos());
								}
	| exp[exp1] MINUS exp[exp2]	{
                  $$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
												   A_minusOp,  $exp1.AST,$exp2.AST);
								  EM_debug("Got minus expression.", $$.AST->pos());
			  					}
	| exp[exp1] DIVIDE exp[exp2]{
                  $$.AST = A_CallExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
										        to_Symbol("div"), A_ExpList($exp1.AST, A_ExpList($exp2.AST, 0)));
								  EM_debug("Got divide expression.", $$.AST->pos());
								}
  | exp[exp1] EQ exp[exp2]	{
                  $$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
                           A_eqOp,  $exp1.AST,$exp2.AST);
                  EM_debug("Got equal expression.", $$.AST->pos());
                  }
  | exp[exp1] NEQ exp[exp2]	{
								  $$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
												   A_neqOp,  $exp1.AST,$exp2.AST);
								  EM_debug("Got plus expression.", $$.AST->pos());

								}
	| exp[exp1] LT exp[exp2]	{
                  $$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
												   A_ltOp, $exp1.AST,$exp2.AST);
								  EM_debug("Got times expression.", $$.AST->pos());
								}
	| exp[exp1] LE exp[exp2]	{
                  $$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
												   A_leOp,  $exp1.AST,$exp2.AST);
								  EM_debug("Got minus expression.", $$.AST->pos());
			  				}
  | exp[exp1] GT exp[exp2]	{
                  $$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
                           A_gtOp, $exp1.AST,$exp2.AST);
                  EM_debug("Got times expression.", $$.AST->pos());
                }
  | exp[exp1] GE exp[exp2]	{
                  $$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
                           A_geOp,  $exp1.AST,$exp2.AST);
                  EM_debug("Got minus expression.", $$.AST->pos());
                }
  | MINUS exp[exp1]          {
                  $$.AST = A_OpExp(Position::fromLex(@exp1),
                            A_timesOp, A_IntExp(Position::fromLex(@exp1), -1), $exp1.AST);
                  EM_debug("Got negation expression.", $$.AST->pos());
                }
  | NOT exp[exp1]            {
                  $$.AST = A_CallExp(Position::fromLex(@exp1), to_Symbol("not"), A_ExpList($exp1.AST, 0));
                  EM_debug("Got NOT expression", $$.AST->pos());
                }
  | exp[exp1] AND exp[exp2]  {
                  $$.AST = A_IfExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
                                    $exp1.AST, $exp2.AST, A_BoolExp(Position::undefined(), false));
                  EM_debug("Got AND expression", $$.AST->pos());
                }
  | exp[exp1] OR exp[exp2]   {
                  $$.AST = A_IfExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
                                    $exp1.AST, A_BoolExp(Position::undefined(), true), $exp2.AST);
                  EM_debug("Got OR expression", $$.AST->pos());
                }
	| ID[i] LPAREN expList[el1] RPAREN {
                $$.AST = A_CallExp(Position::fromLex(@i), to_Symbol($i), $el1.AST);
								EM_debug("Got function call.", $$.AST->pos());
								}
	| ID[i] LPAREN RPAREN 	    	{
                $$.AST = A_CallExp(Position::fromLex(@i), to_Symbol($i), 0);
							  EM_debug("Got function call.", $$.AST->pos());
							  }
  | IF exp[exp1] THEN exp[exp2] ELSE exp[exp3] {
                $$.AST = A_IfExp(Position::range($exp1.AST->pos(), $exp3.AST->pos()),
                                  $exp1.AST, $exp2.AST, $exp3.AST);
                EM_debug("Got if statement.", $$.AST->pos());
                }
  | IF exp[exp1] THEN exp[exp2] {
                $$.AST = A_IfExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
                                  $exp1.AST, $exp2.AST, 0);
                EM_debug("Got if statement.", $$.AST->pos());
                }
  | LPAREN seqExp[se1] RPAREN   {
                $$.AST = A_SeqExp(Position::fromLex(@se1), $se1.AST);
                EM_debug("Got seq exp.", $$.AST->pos());
                }
  | WHILE exp[exp1] DO exp[exp2]{
                $$.AST = A_WhileExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
                                      $exp1.AST, $exp2.AST);
                EM_debug("Got while exp", $$.AST->pos());
                }
  | FOR ID[i] ASSIGN exp[exp1] TO exp[exp2] DO exp[exp3] {
                $$.AST = A_ForExp(Position::range($exp1.AST->pos(), $exp3.AST->pos()),
                                      to_Symbol($i), $exp1.AST, $exp2.AST, $exp3.AST);
                EM_debug("Got for exp", $$.AST->pos());
                }
  | BREAK       {
                $$.AST = A_BreakExp(Position::undefined());
                EM_debug("Got break");
                }
  | LET decList[dl1] IN seqExp[se1] END_LET {
                $$.AST = A_LetExp(Position::undefined(),
                  $dl1.AST,
                  A_SeqExp(Position::fromLex(@se1), $se1.AST));
                EM_debug("Got let exp", $$.AST->pos());
                }
  | ID[i] ASSIGN exp[exp1] {
                $$.AST = A_AssignExp(Position::fromLex(@exp1), A_SimpleVar(Position::fromLex(@i), to_Symbol($i)), $exp1.AST);
                EM_debug("Got an assign exp", $$.AST->pos());
                }
	;


expList: exp[exp1]	{
                      $$.AST = A_ExpList($exp1.AST, 0);
										}
		| exp[exp1] COMMA expList[el1]  {
                      $$.AST = A_ExpList($exp1.AST, $el1.AST);
										  EM_debug("Got param , ...", $$.AST->pos());
										}

	;


seqExp: exp[exp1]   {
                      $$.AST = A_ExpList($exp1.AST, 0);
                    }
    | exp[exp1] SEMICOLON seqExp[el1]  {
                      $$.AST = A_ExpList($exp1.AST, $el1.AST);
                      EM_debug("Got param , ...", $$.AST->pos());
                    }

  ;


decList: dec[dec1]  {
                      $$.AST = A_DecList($dec1.AST, 0);
                      EM_debug("Got DecList", $$.AST->pos());
                    }
    | dec[dec1] decList[dl1]  {
                      $$.AST = A_DecList($dec1.AST, $dl1.AST);
                      EM_debug("Got DecList", $$.AST->pos());
                    }

  ;

dec:  VAR ID[i] COLON TYPE[t] ASSIGN exp[exp1] {
                $$.AST = A_VarDec(Position::fromLex(@exp1), to_Symbol($i), to_Symbol($t), $exp1.AST);
                EM_debug("Got new vardec", $$.AST->pos());
                }
  | VAR ID[i] ASSIGN exp[exp1] {
                $$.AST = A_VarDec(Position::fromLex(@exp1), to_Symbol($i), 0, $exp1.AST);
                EM_debug("Got new vardec", $$.AST->pos());
                }

  ;







%%

void
yy::tigerParser::error(const location_type& l,
          	       const std::string& m)
  {
	  EM_debug("In yy::tigerParser::error");
	  EM_error(m, true, Position::fromLex(l));
  }
