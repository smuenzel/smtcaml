#include <cvc5/cvc5.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;

using Solver = cvc5::Solver;
using Sort = cvc5::Sort;
using Term = cvc5::Term;
using Op = cvc5::Op;
using Result = cvc5::Result;
using Datatype = cvc5::Datatype;
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
DECL_API_TYPE(Op,op);
DECL_API_TYPE(Op*,op);
DECL_API_TYPE(void,unit);
DECL_API_TYPE(std::string,string);
DECL_API_TYPE(Result,sat_result);
DECL_API_TYPE(Result*,sat_result);
DECL_API_TYPE(Datatype,datatype);
DECL_API_TYPE(Datatype*,datatype);

/////////////////////////////////////////////////////////////////////////////////////////
///
template<> struct CppCaml::CamlConversionProperties<Sort>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerWithContext;
  typedef Solver Context;

  static void delete_T(Context*context, Sort&t){
    delete &t;
  }
};

template<> struct CppCaml::CamlConversionProperties<Term>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerWithContext;
  typedef Solver Context;

  static void delete_T(Context*context, Term&t){
    delete &t;
  }
};

template<> struct CppCaml::CamlConversionProperties<Op>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerWithContext;
  typedef Solver Context;

  static void delete_T(Context*context, Op&t){
    delete &t;
  }
};

template<> struct CppCaml::CamlConversionProperties<Result>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerWithContext;
  typedef Solver Context;

  static void delete_T(Context*context, Result&t){
    delete &t;
  }
};

template<> struct CppCaml::CamlConversionProperties<Datatype>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerWithContext;
  typedef Solver Context;

  static void delete_T(Context*context, Datatype&t){
    delete &t;
  }
};

template<> struct CppCaml::CamlConversionProperties<Solver*>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerSharedPointer;

  static void delete_T(Solver*&s){
    delete s;
  };
};

static_assert(CppCaml::CamlOfValue<Solver*>);
static_assert(CppCaml::CamlBidirectional<Solver*>);
static_assert(CppCaml::CamlOfValue<CppCaml::NormalizeArgument<const std::vector<Sort>&>>);

/////////////////////////////////////////////////////////////////////////////////////////

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

Solver* new_Solver(){
  return new Solver();
}

API0__(cvc5,new_Solver)

#define APIM0(A,B) APIM0__(cvc5,A,B)
#define APIM1(A,B) APIM1__(cvc5,A,B)
#define APIM2(A,B) APIM2__(cvc5,A,B)
#define APIM1_IMPLIED(A,B) APIM1_IMPLIED__(cvc5,A,B)
#define APIM2_IMPLIED(A,B) APIM2_IMPLIED__(cvc5,A,B)
#define APIM1_OVERLOAD(...) APIM1_OVERLOAD_(cvc5,__VA_ARGS__)
#define APIM1_OVERLOAD_IMPLIED(...) APIM1_OVERLOAD_IMPLIED_(cvc5,__VA_ARGS__)
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
APIM2_OVERLOAD(Solver,mkTerm,op,Term,const Op&,const std::vector<Term>&)

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

APIM2_OVERLOAD(Solver,mkOp,kv,Op,Kind,const std::vector<uint32_t>&)

APIM0(Sort,getKind)
APIM0(Sort,hasSymbol)
APIM0(Sort,getSymbol)
APIM0(Sort,isNull)
APIM0(Sort,isBoolean)
APIM0(Sort,isInteger)
APIM0(Sort,isReal)
APIM0(Sort,isRegExp)
APIM0(Sort,isRoundingMode)
APIM0(Sort,isBitVector)
APIM0(Sort,isFloatingPoint)
APIM0(Sort,isDatatype)
APIM0(Sort,isDatatypeConstructor)
APIM0(Sort,isDatatypeSelector)
APIM0(Sort,isDatatypeUpdater)
APIM0(Sort,isFunction)
APIM0(Sort,isPredicate)
APIM0(Sort,isTuple)
APIM0(Sort,isRecord)
APIM0(Sort,isArray)
APIM0(Sort,isFiniteField)
APIM0(Sort,isSet)
APIM0(Sort,isBag)
APIM0(Sort,isSequence)
APIM0(Sort,isAbstract)
APIM0(Sort,isUninterpretedSort)
APIM0(Sort,isUninterpretedSortConstructor)
APIM0(Sort,getUninterpretedSortConstructor)
APIM0(Sort,getDatatype)
APIM0(Sort,isInstantiated)
APIM1(Sort,instantiate)
APIM0(Sort,getInstantiatedParameters)
APIM2_OVERLOAD(Sort,substitute,ss, Sort, const Sort&, const Sort&)
APIM2_OVERLOAD(Sort,substitute,svsv, Sort, const std::vector<Sort>&, const std::vector<Sort>&)
APIM0(Sort,toString)
APIM0(Sort,getDatatypeConstructorArity)
APIM0(Sort,getDatatypeConstructorDomainSorts)
APIM0(Sort,getDatatypeConstructorCodomainSort)
APIM0(Sort,getDatatypeSelectorDomainSort)
APIM0(Sort,getDatatypeSelectorCodomainSort)
APIM0(Sort,getDatatypeTesterDomainSort)
APIM0(Sort,getDatatypeTesterCodomainSort)
APIM0(Sort,getFunctionArity)
APIM0(Sort,getFunctionDomainSorts)
APIM0(Sort,getFunctionCodomainSort)
APIM0(Sort,getArrayIndexSort)
APIM0(Sort,getArrayElementSort)
APIM0(Sort,getSetElementSort)
APIM0(Sort,getBagElementSort)
APIM0(Sort,getSequenceElementSort)
APIM0(Sort,getAbstractedKind)
APIM0(Sort,getUninterpretedSortConstructorArity)
APIM0(Sort,getBitVectorSize)
APIM0(Sort,getFiniteFieldSize)
APIM0(Sort,getFloatingPointExponentSize)
APIM0(Sort,getFloatingPointSignificandSize)
APIM0(Sort,getDatatypeArity)
APIM0(Sort,getTupleLength)
APIM0(Sort,getTupleSorts)

APIM0(Result,isNull)
APIM0(Result,isSat)
APIM0(Result,isUnsat)
APIM0(Result,isUnknown)
APIM0(Result,toString)
APIM0(Result,getUnknownExplanation)

APIM0(Term,getNumChildren)
APIM0(Term,getId)
APIM0(Term,getKind)
APIM0(Term,getSort)
APIM0(Term,toString)
APIM2_OVERLOAD(Term,substitute,tt, Term, const Term&, const Term&)
APIM2_OVERLOAD(Term,substitute,tvtv, Term, const std::vector<Term>&, const std::vector<Term>&)
APIM0(Term,hasOp)
APIM0(Term,getOp)
APIM0(Term,hasSymbol)
APIM0(Term,getSymbol)
APIM0(Term,isNull)
APIM0(Term,notTerm)
APIM1(Term,andTerm)
APIM1(Term,orTerm)
APIM1(Term,xorTerm)
APIM1(Term,eqTerm)
APIM1(Term,impTerm)
APIM2(Term,iteTerm)
APIM0(Term,getRealOrIntegerValueSign)
APIM0(Term,isInt32Value)
APIM0(Term,getInt32Value)
APIM0(Term,isUInt32Value)
APIM0(Term,getUInt32Value)
//....
APIM0(Term,isBooleanValue)
APIM0(Term,getBooleanValue)
APIM0(Term,isBitVectorValue)
APIM1(Term,getBitVectorValue)
//...



apireturn caml_cvc5_unit(value){
  return Val_unit;
}
