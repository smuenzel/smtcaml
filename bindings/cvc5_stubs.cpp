#include <cvc5/cvc5.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;
using CppCaml::T_value;

using Solver = cvc5::Solver;
using Sort = cvc5::Sort;
using Term = cvc5::Term;
using Result = cvc5::Result;
using UnknownExplanation = cvc5::UnknownExplanation;
using Kind = cvc5::Kind;
using SortKind = cvc5::SortKind;

DECL_API_TYPE(uint32_t,uint32_t);
DECL_API_TYPE(uint64_t,uint64_t);
DECL_API_TYPE(int32_t,int32_t);
DECL_API_TYPE(bool,bool);
DECL_API_TYPE(Solver*,solver);
DECL_API_TYPE(Sort,sort);
DECL_API_TYPE(Sort*,sort);
DECL_API_TYPE(Term,term);
DECL_API_TYPE(Term*,term);
DECL_API_TYPE(void,unit);
DECL_API_TYPE(std::string,string);
DECL_API_TYPE(Result,sat_result);
DECL_API_TYPE(Result*,sat_result);
DECL_API_TYPE(UnknownExplanation,unknownExplanation);
DECL_API_TYPE(Kind,kind);
DECL_API_TYPE(SortKind,sortKind);


CAML_REPRESENTATION(Solver*, ContainerSharedPointer);
CAML_REPRESENTATION(Sort,InlinedWithContext);
CAML_REPRESENTATION(Term,InlinedWithContext);
CAML_REPRESENTATION(const Term&,InlinedWithContext);
CAML_REPRESENTATION(Result,InlinedWithContext);
CAML_REPRESENTATION(UnknownExplanation,Immediate);

CAML_REPRESENTATION(int32_t,Immediate);
CAML_REPRESENTATION(uint32_t,Immediate);
CAML_REPRESENTATION(uint64_t,Immediate);

CAML_REPRESENTATION(Kind,Immediate);
CAML_REPRESENTATION(SortKind,Immediate);

/////////////////////////////////////////////////////////////////////////////////////////
///
template<> struct CppCaml::CamlConversionProperties<Sort>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerWithContext;
  typedef Solver Context;
};

template<> struct CppCaml::CamlConversionProperties<Solver*>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerSharedPointer;
};

static_assert(CppCaml::CamlOfValue<Solver*>);

value xxx(value v){
  return CppCaml::call_api_class(&Solver::getBooleanSort,v);

}
/////////////////////////////////////////////////////////////////////////////////////////

template<> struct CppCaml::ImmediateProperties<uint32_t> {
  static inline value to_value(uint32_t b) { return Val_long(b); }
  static inline uint32_t of_value(value v) { return Long_val(v); }
};

/* CR smuenzel: not full range*/
template<> struct CppCaml::ImmediateProperties<uint64_t> {
  static inline value to_value(uint64_t b) { return Val_long(b); }
  static inline uint64_t of_value(value v) { return Long_val(v); }
};

template<> struct CppCaml::ImmediateProperties<int32_t> {
  static inline value to_value(int32_t b) { return Val_long(b); }
  static inline int32_t of_value(value v) { return Long_val(v); }
};

#define ENUM_UnknownExplanation(F) \
  F(UnknownExplanation,REQUIRES_FULL_CHECK) \
  F(UnknownExplanation,INCOMPLETE) \
  F(UnknownExplanation,TIMEOUT) \
  F(UnknownExplanation,RESOURCEOUT) \
  F(UnknownExplanation,MEMOUT) \
  F(UnknownExplanation,INTERRUPTED) \
  F(UnknownExplanation,UNSUPPORTED) \
  F(UnknownExplanation,OTHER) \
  F(UnknownExplanation,REQUIRES_CHECK_AGAIN) \
  F(UnknownExplanation,UNKNOWN_REASON)

MAKE_ENUM_IMMEDIATE_PROPERTIES(UnknownExplanation,ENUM_UnknownExplanation,UNKNOWN_REASON)

//  grep -v '^#' vendor/build/include/cvc5/cvc5_kind.h | sed -n 's/.*EVALUE(\([^) ]*\).*/  F(Kind,\1)\\/p'
#define ENUM_Kind(F) \
  F(Kind,INTERNAL_KIND)\
  F(Kind,UNDEFINED_KIND)\
  F(Kind,NULL_TERM)\
  F(Kind,UNINTERPRETED_SORT_VALUE)\
  F(Kind,EQUAL)\
  F(Kind,DISTINCT)\
  F(Kind,CONSTANT)\
  F(Kind,VARIABLE)\
  F(Kind,SEXPR)\
  F(Kind,LAMBDA)\
  F(Kind,WITNESS)\
  F(Kind,CONST_BOOLEAN)\
  F(Kind,NOT)\
  F(Kind,AND)\
  F(Kind,IMPLIES)\
  F(Kind,OR)\
  F(Kind,XOR)\
  F(Kind,ITE)\
  F(Kind,APPLY_UF)\
  F(Kind,CARDINALITY_CONSTRAINT)\
  F(Kind,HO_APPLY)\
  F(Kind,ADD)\
  F(Kind,MULT)\
  F(Kind,IAND)\
  F(Kind,POW2)\
  F(Kind,SUB)\
  F(Kind,NEG)\
  F(Kind,DIVISION)\
  F(Kind,INTS_DIVISION)\
  F(Kind,INTS_MODULUS)\
  F(Kind,ABS)\
  F(Kind,POW)\
  F(Kind,EXPONENTIAL)\
  F(Kind,SINE)\
  F(Kind,COSINE)\
  F(Kind,TANGENT)\
  F(Kind,COSECANT)\
  F(Kind,SECANT)\
  F(Kind,COTANGENT)\
  F(Kind,ARCSINE)\
  F(Kind,ARCCOSINE)\
  F(Kind,ARCTANGENT)\
  F(Kind,ARCCOSECANT)\
  F(Kind,ARCSECANT)\
  F(Kind,ARCCOTANGENT)\
  F(Kind,SQRT)\
  F(Kind,DIVISIBLE)\
  F(Kind,CONST_RATIONAL)\
  F(Kind,CONST_INTEGER)\
  F(Kind,LT)\
  F(Kind,LEQ)\
  F(Kind,GT)\
  F(Kind,GEQ)\
  F(Kind,IS_INTEGER)\
  F(Kind,TO_INTEGER)\
  F(Kind,TO_REAL)\
  F(Kind,PI)\
  F(Kind,CONST_BITVECTOR)\
  F(Kind,BITVECTOR_CONCAT)\
  F(Kind,BITVECTOR_AND)\
  F(Kind,BITVECTOR_OR)\
  F(Kind,BITVECTOR_XOR)\
  F(Kind,BITVECTOR_NOT)\
  F(Kind,BITVECTOR_NAND)\
  F(Kind,BITVECTOR_NOR)\
  F(Kind,BITVECTOR_XNOR)\
  F(Kind,BITVECTOR_COMP)\
  F(Kind,BITVECTOR_MULT)\
  F(Kind,BITVECTOR_ADD)\
  F(Kind,BITVECTOR_SUB)\
  F(Kind,BITVECTOR_NEG)\
  F(Kind,BITVECTOR_UDIV)\
  F(Kind,BITVECTOR_UREM)\
  F(Kind,BITVECTOR_SDIV)\
  F(Kind,BITVECTOR_SREM)\
  F(Kind,BITVECTOR_SMOD)\
  F(Kind,BITVECTOR_SHL)\
  F(Kind,BITVECTOR_LSHR)\
  F(Kind,BITVECTOR_ASHR)\
  F(Kind,BITVECTOR_ULT)\
  F(Kind,BITVECTOR_ULE)\
  F(Kind,BITVECTOR_UGT)\
  F(Kind,BITVECTOR_UGE)\
  F(Kind,BITVECTOR_SLT)\
  F(Kind,BITVECTOR_SLE)\
  F(Kind,BITVECTOR_SGT)\
  F(Kind,BITVECTOR_SGE)\
  F(Kind,BITVECTOR_ULTBV)\
  F(Kind,BITVECTOR_SLTBV)\
  F(Kind,BITVECTOR_ITE)\
  F(Kind,BITVECTOR_REDOR)\
  F(Kind,BITVECTOR_REDAND)\
  F(Kind,BITVECTOR_UADDO)\
  F(Kind,BITVECTOR_SADDO)\
  F(Kind,BITVECTOR_UMULO)\
  F(Kind,BITVECTOR_SMULO)\
  F(Kind,BITVECTOR_USUBO)\
  F(Kind,BITVECTOR_SSUBO)\
  F(Kind,BITVECTOR_SDIVO)\
  F(Kind,BITVECTOR_EXTRACT)\
  F(Kind,BITVECTOR_REPEAT)\
  F(Kind,BITVECTOR_ZERO_EXTEND)\
  F(Kind,BITVECTOR_SIGN_EXTEND)\
  F(Kind,BITVECTOR_ROTATE_LEFT)\
  F(Kind,BITVECTOR_ROTATE_RIGHT)\
  F(Kind,INT_TO_BITVECTOR)\
  F(Kind,BITVECTOR_TO_NAT)\
  F(Kind,CONST_FINITE_FIELD)\
  F(Kind,FINITE_FIELD_NEG)\
  F(Kind,FINITE_FIELD_ADD)\
  F(Kind,FINITE_FIELD_MULT)\
  F(Kind,CONST_FLOATINGPOINT)\
  F(Kind,CONST_ROUNDINGMODE)\
  F(Kind,FLOATINGPOINT_FP)\
  F(Kind,FLOATINGPOINT_EQ)\
  F(Kind,FLOATINGPOINT_ABS)\
  F(Kind,FLOATINGPOINT_NEG)\
  F(Kind,FLOATINGPOINT_ADD)\
  F(Kind,FLOATINGPOINT_SUB)\
  F(Kind,FLOATINGPOINT_MULT)\
  F(Kind,FLOATINGPOINT_DIV)\
  F(Kind,FLOATINGPOINT_FMA)\
  F(Kind,FLOATINGPOINT_SQRT)\
  F(Kind,FLOATINGPOINT_REM)\
  F(Kind,FLOATINGPOINT_RTI)\
  F(Kind,FLOATINGPOINT_MIN)\
  F(Kind,FLOATINGPOINT_MAX)\
  F(Kind,FLOATINGPOINT_LEQ)\
  F(Kind,FLOATINGPOINT_LT)\
  F(Kind,FLOATINGPOINT_GEQ)\
  F(Kind,FLOATINGPOINT_GT)\
  F(Kind,FLOATINGPOINT_IS_NORMAL)\
  F(Kind,FLOATINGPOINT_IS_SUBNORMAL)\
  F(Kind,FLOATINGPOINT_IS_ZERO)\
  F(Kind,FLOATINGPOINT_IS_INF)\
  F(Kind,FLOATINGPOINT_IS_NAN)\
  F(Kind,FLOATINGPOINT_IS_NEG)\
  F(Kind,FLOATINGPOINT_IS_POS)\
  F(Kind,FLOATINGPOINT_TO_FP_FROM_IEEE_BV)\
  F(Kind,FLOATINGPOINT_TO_FP_FROM_FP)\
  F(Kind,FLOATINGPOINT_TO_FP_FROM_REAL)\
  F(Kind,FLOATINGPOINT_TO_FP_FROM_SBV)\
  F(Kind,FLOATINGPOINT_TO_FP_FROM_UBV)\
  F(Kind,FLOATINGPOINT_TO_UBV)\
  F(Kind,FLOATINGPOINT_TO_SBV)\
  F(Kind,FLOATINGPOINT_TO_REAL)\
  F(Kind,SELECT)\
  F(Kind,STORE)\
  F(Kind,CONST_ARRAY)\
  F(Kind,EQ_RANGE)\
  F(Kind,APPLY_CONSTRUCTOR)\
  F(Kind,APPLY_SELECTOR)\
  F(Kind,APPLY_TESTER)\
  F(Kind,APPLY_UPDATER)\
  F(Kind,MATCH)\
  F(Kind,MATCH_CASE)\
  F(Kind,MATCH_BIND_CASE)\
  F(Kind,TUPLE_PROJECT)\
  F(Kind,SEP_NIL)\
  F(Kind,SEP_EMP)\
  F(Kind,SEP_PTO)\
  F(Kind,SEP_STAR)\
  F(Kind,SEP_WAND)\
  F(Kind,SET_EMPTY)\
  F(Kind,SET_UNION)\
  F(Kind,SET_INTER)\
  F(Kind,SET_MINUS)\
  F(Kind,SET_SUBSET)\
  F(Kind,SET_MEMBER)\
  F(Kind,SET_SINGLETON)\
  F(Kind,SET_INSERT)\
  F(Kind,SET_CARD)\
  F(Kind,SET_COMPLEMENT)\
  F(Kind,SET_UNIVERSE)\
  F(Kind,SET_COMPREHENSION)\
  F(Kind,SET_CHOOSE)\
  F(Kind,SET_IS_SINGLETON)\
  F(Kind,SET_MAP)\
  F(Kind,SET_FILTER)\
  F(Kind,SET_FOLD)\
  F(Kind,RELATION_JOIN)\
  F(Kind,RELATION_PRODUCT)\
  F(Kind,RELATION_TRANSPOSE)\
  F(Kind,RELATION_TCLOSURE)\
  F(Kind,RELATION_JOIN_IMAGE)\
  F(Kind,RELATION_IDEN)\
  F(Kind,RELATION_GROUP)\
  F(Kind,RELATION_AGGREGATE)\
  F(Kind,RELATION_PROJECT)\
  F(Kind,BAG_EMPTY)\
  F(Kind,BAG_UNION_MAX)\
  F(Kind,BAG_UNION_DISJOINT)\
  F(Kind,BAG_INTER_MIN)\
  F(Kind,BAG_DIFFERENCE_SUBTRACT)\
  F(Kind,BAG_DIFFERENCE_REMOVE)\
  F(Kind,BAG_SUBBAG)\
  F(Kind,BAG_COUNT)\
  F(Kind,BAG_MEMBER)\
  F(Kind,BAG_DUPLICATE_REMOVAL)\
  F(Kind,BAG_MAKE)\
  F(Kind,BAG_CARD)\
  F(Kind,BAG_CHOOSE)\
  F(Kind,BAG_IS_SINGLETON)\
  F(Kind,BAG_FROM_SET)\
  F(Kind,BAG_TO_SET)\
  F(Kind,BAG_MAP)\
  F(Kind,BAG_FILTER)\
  F(Kind,BAG_FOLD)\
  F(Kind,BAG_PARTITION)\
  F(Kind,TABLE_PRODUCT)\
  F(Kind,TABLE_PROJECT)\
  F(Kind,TABLE_AGGREGATE)\
  F(Kind,TABLE_JOIN)\
  F(Kind,TABLE_GROUP)\
  F(Kind,STRING_CONCAT)\
  F(Kind,STRING_IN_REGEXP)\
  F(Kind,STRING_LENGTH)\
  F(Kind,STRING_SUBSTR)\
  F(Kind,STRING_UPDATE)\
  F(Kind,STRING_CHARAT)\
  F(Kind,STRING_CONTAINS)\
  F(Kind,STRING_INDEXOF)\
  F(Kind,STRING_INDEXOF_RE)\
  F(Kind,STRING_REPLACE)\
  F(Kind,STRING_REPLACE_ALL)\
  F(Kind,STRING_REPLACE_RE)\
  F(Kind,STRING_REPLACE_RE_ALL)\
  F(Kind,STRING_TO_LOWER)\
  F(Kind,STRING_TO_UPPER)\
  F(Kind,STRING_REV)\
  F(Kind,STRING_TO_CODE)\
  F(Kind,STRING_FROM_CODE)\
  F(Kind,STRING_LT)\
  F(Kind,STRING_LEQ)\
  F(Kind,STRING_PREFIX)\
  F(Kind,STRING_SUFFIX)\
  F(Kind,STRING_IS_DIGIT)\
  F(Kind,STRING_FROM_INT)\
  F(Kind,STRING_TO_INT)\
  F(Kind,CONST_STRING)\
  F(Kind,STRING_TO_REGEXP)\
  F(Kind,REGEXP_CONCAT)\
  F(Kind,REGEXP_UNION)\
  F(Kind,REGEXP_INTER)\
  F(Kind,REGEXP_DIFF)\
  F(Kind,REGEXP_STAR)\
  F(Kind,REGEXP_PLUS)\
  F(Kind,REGEXP_OPT)\
  F(Kind,REGEXP_RANGE)\
  F(Kind,REGEXP_REPEAT)\
  F(Kind,REGEXP_LOOP)\
  F(Kind,REGEXP_NONE)\
  F(Kind,REGEXP_ALL)\
  F(Kind,REGEXP_ALLCHAR)\
  F(Kind,REGEXP_COMPLEMENT)\
  F(Kind,SEQ_CONCAT)\
  F(Kind,SEQ_LENGTH)\
  F(Kind,SEQ_EXTRACT)\
  F(Kind,SEQ_UPDATE)\
  F(Kind,SEQ_AT)\
  F(Kind,SEQ_CONTAINS)\
  F(Kind,SEQ_INDEXOF)\
  F(Kind,SEQ_REPLACE)\
  F(Kind,SEQ_REPLACE_ALL)\
  F(Kind,SEQ_REV)\
  F(Kind,SEQ_PREFIX)\
  F(Kind,SEQ_SUFFIX)\
  F(Kind,CONST_SEQUENCE)\
  F(Kind,SEQ_UNIT)\
  F(Kind,SEQ_NTH)\
  F(Kind,FORALL)\
  F(Kind,EXISTS)\
  F(Kind,VARIABLE_LIST)\
  F(Kind,INST_PATTERN)\
  F(Kind,INST_NO_PATTERN)\
  F(Kind,INST_POOL)\
  F(Kind,INST_ADD_TO_POOL)\
  F(Kind,SKOLEM_ADD_TO_POOL)\
  F(Kind,INST_ATTRIBUTE)\
  F(Kind,INST_PATTERN_LIST)\
  F(Kind,LAST_KIND)

#define ENUM_SortKind(F) \
  F(SortKind,INTERNAL_SORT_KIND)\
  F(SortKind,UNDEFINED_SORT_KIND)\
  F(SortKind,NULL_SORT)\
  F(SortKind,ABSTRACT_SORT)\
  F(SortKind,ARRAY_SORT)\
  F(SortKind,BAG_SORT)\
  F(SortKind,BOOLEAN_SORT)\
  F(SortKind,BITVECTOR_SORT)\
  F(SortKind,DATATYPE_SORT)\
  F(SortKind,FINITE_FIELD_SORT)\
  F(SortKind,FLOATINGPOINT_SORT)\
  F(SortKind,FUNCTION_SORT)\
  F(SortKind,INTEGER_SORT)\
  F(SortKind,REAL_SORT)\
  F(SortKind,REGLAN_SORT)\
  F(SortKind,ROUNDINGMODE_SORT)\
  F(SortKind,SEQUENCE_SORT)\
  F(SortKind,SET_SORT)\
  F(SortKind,STRING_SORT)\
  F(SortKind,TUPLE_SORT)\
  F(SortKind,UNINTERPRETED_SORT)\
  F(SortKind,LAST_SORT_KIND)

MAKE_ENUM_IMMEDIATE_PROPERTIES(Kind,ENUM_Kind,UNDEFINED_KIND)
MAKE_ENUM_IMMEDIATE_PROPERTIES(SortKind,ENUM_SortKind,UNDEFINED_SORT_KIND)

template<> struct CppCaml::ValueWithContextProperties<Sort>{
  typedef Solver Context;
  static void delete_T(Context*context, Sort*t){
    delete t;
  }
};

template<> struct CppCaml::ValueWithContextProperties<Term>{
  typedef Solver Context;
  static void delete_T(Context*context, Term*t){
    delete t;
  }
};

template<> struct CppCaml::ValueWithContextProperties<Result>{
  typedef Solver Context;
  static void delete_T(Context*context, Result*t){
    delete t;
  }
};

template<> struct CppCaml::SharedPointerProperties<Solver>{
  static void delete_T(Solver*s) { delete s; }
};

using caml_cvc5_solver = CppCaml::ContainerSharedPointer<Solver>;


apireturn caml_cvc5__Solver__operator_new(value){
  return caml_cvc5_solver::allocate(new Solver());
}
REGISTER_API_CONSTRUCTOR(Solver,caml_cvc5__Solver__operator_new);

#define APIM0(A,B) APIM0_(cvc5,A,B)
#define APIM1(A,B) APIM1_(cvc5,A,B)
#define APIM2(A,B) APIM2_(cvc5,A,B)
#define APIM1_IMPLIED(A,B) APIM1_IMPLIED_(cvc5,A,B)
#define APIM2_IMPLIED(A,B) APIM2_IMPLIED_(cvc5,A,B)
#define APIM1_OVERLOAD(...) APIM1_OVERLOAD_(cvc5,__VA_ARGS__)
#define APIM2_OVERLOAD(...) APIM2_OVERLOAD_(cvc5,__VA_ARGS__)
#define APIM2_OVERLOAD_IMPLIED(...) APIM2_OVERLOAD_IMPLIED_(cvc5,__VA_ARGS__)
#define APIM3_OVERLOAD(...) APIM3_OVERLOAD_(cvc5,__VA_ARGS__)

APIM0(Solver,getBooleanSort)
APIM0(Solver,getIntegerSort)
APIM0(Solver,getRealSort)
APIM0(Solver,getRegExpSort)
APIM0(Solver,getRoundingModeSort)
APIM0(Solver,getStringSort)
APIM2(Solver,mkArraySort)
APIM1(Solver,mkBitVectorSort)
APIM2(Solver,mkFloatingPointSort)
APIM1(Solver,mkFiniteFieldSort)
APIM2(Solver,mkFunctionSort)
APIM1(Solver,mkPredicateSort)
APIM1(Solver,mkRecordSort)
APIM1(Solver,mkSetSort)
APIM1(Solver,mkBagSort)
APIM1(Solver,mkSequenceSort)
APIM1(Solver,mkUninterpretedSort)
APIM1(Solver,mkTupleSort)

APIM2_OVERLOAD(Solver,mkBitVector,u32_u64,Term,uint32_t,uint64_t)
APIM3_OVERLOAD(Solver,mkBitVector,u32_s_u32,Term,uint32_t,const std::string&, uint32_t)

APIM0(Solver,mkTrue)
APIM0(Solver,mkFalse)
APIM1(Solver,mkBoolean)

APIM2_OVERLOAD(Solver,mkTerm,kind,Term,Kind,const std::vector<Term>&)

APIM2(Solver,mkConstArray)

APIM2_IMPLIED(Solver,mkConst)
APIM2(Solver,mkVar)

APIM1(Solver,simplify)

APIM1_IMPLIED(Solver,assertFormula)
APIM0(Solver,checkSat)

APIM1_OVERLOAD(Solver,getValue,t,Term,const Term&)
APIM1_OVERLOAD(Solver,getValue,vt,std::vector<Term>,const std::vector<Term>&)

APIM2(Solver,setOption)
APIM1(Solver,getOption)

APIM0(Sort,getKind)
APIM0(Sort,hasSymbol)
APIM0(Sort,getSymbol)
APIM0(Sort,isNull)
APIM0(Sort,isBoolean)
APIM0(Sort,isInteger)
APIM0(Sort,isReal)
APIM0(Sort,isInstantiated)

APIM0(Result,isNull)
APIM0(Result,isSat)
APIM0(Result,isUnsat)
APIM0(Result,isUnknown)
APIM0(Result,toString)
APIM0(Result,getUnknownExplanation)

API_GET_CONTEXT(cvc5,Term,Term)
APIM0(Term,toString)

apireturn caml_cvc5_unit(value){
  return Val_unit;
}
