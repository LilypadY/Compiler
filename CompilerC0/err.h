#ifndef ERR_H_INCLUDED
#define ERR_H_INCLUDED
#include"compile.h"
#define LEX_ERR 0
#define STX_ERR 1
#define SMT_ERR 2
#define MEMORY_ERR 3

#define NOMORE ""
#define LEADINGZERO "Leading Zero"
#define QMISMATCH "' mismatching"
#define INVALIDCHAR "invalid character content"
#define INVALIDSTR "invalid string content"
#define STRTOOLONG "string too long"
#define DQMISMATCH "\" expected"
#define INVALID "invalid character"
#define NEQERR "!=:= expected"
#define TYPEERR "invalid type"
#define IDSYERR "invalid identifier"
#define ASNERR "assignment: = expected"
#define CONSTUNINIT "constant uninitialized"
#define CONSTASNMISMATCH "const assignment mismatch"
#define SMCLERR "; expected"
#define TYPESYERR "type symbol not found"
#define RBKERR "] expected"
#define ARRLENERR "array length error"
#define RPERR ") expected"
#define LPERR "( expected"
#define LBRCERR "{ expected"
#define RBRCERR "} expected"
#define MSMAIN "function main expected"
#define UINTMS "unsigned integer expected"
#define GLBSTM "unexpected statements"
#define FCTINSTM "unexpected function declaration in compound_statement"
#define CMPSPT "comparing separator expected"
#define CASEERR "case expected"
#define CSCONSTERR "integer or character constant expected"
#define COLONERR ": expected"
#define COMMAERR ", expected"
#define PRINTFERR "invalid print content"
#define WHILEERR "while expected"
#define FACTORERR "invalid factor"
#define SZERO "+-0 is unexpected"
#define STMERR "invalid statement"
#define SYMTABOVERFLOW "symbol table overflow"
#define BTABOVERFLOW "block table overflow"
#define RENAMEERR "identifier redeclared"
#define UNDEFINE "identifier undefined"
#define NOTARR "identifier not an array"
#define ARRIDXERR "index out of bounds"
#define FUNCCALL "a function with return value expected"
#define TOOMANYPARAM "too many parameters"
#define TOOLITPARAM "parameters missing"
#define NOTVAR "not a variable"
#define NOTFUNC "not a function"
#define NORET "return statement expected"
#define NOMORETMP "no more temporary register and memory"
#define CHARTRANSERR "cannot transform to char "
#define TYPEMISMATCH "types mismatch"
#define NOTPUSH "not push order"
#define SURPLUSERR "surplus content"
#define REPCASEERR "repeated case constant"
#define INVALIDRET "invalid return statement"

extern int errs;
extern int skipblk_flag,skipline_flag;
int skip(int target);
void skip2(int target);
void skipblk();
void skipline();
void skippart(int target);
void error(int type,char* info);

#endif // ERR_H_INCLUDED
