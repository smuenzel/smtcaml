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


CAML_REPRESENTATION(Solver*, ContainerSharedPointer);
CAML_REPRESENTATION(Sort,InlinedWithContext);
CAML_REPRESENTATION(Term,InlinedWithContext);
CAML_REPRESENTATION(Result,InlinedWithContext);
CAML_REPRESENTATION(UnknownExplanation,Immediate);

CAML_REPRESENTATION(int32_t,Immediate);
CAML_REPRESENTATION(uint32_t,Immediate);
CAML_REPRESENTATION(uint64_t,Immediate);

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

#define APIM0(CLASS,APIF) \
  REGISTER_API_MEMBER(CLASS,APIF, caml_cvc5__##CLASS ## __##APIF); \
  apireturn caml_cvc5__##CLASS ## __##APIF(value v_c){ \
    return CppCaml::apiN_class(&CLASS :: APIF, v_c); \
  }

#define APIM1(CLASS,APIF) \
  REGISTER_API_MEMBER(CLASS,APIF, caml_cvc5__##CLASS ## __##APIF); \
  apireturn caml_cvc5__##CLASS ## __##APIF(value v_c, value v_p0){ \
    return CppCaml::apiN_class(&CLASS :: APIF, v_c, v_p0); \
  }

#define APIM2(CLASS,APIF) \
  REGISTER_API_MEMBER(CLASS,APIF, caml_cvc5__##CLASS ## __##APIF); \
  apireturn caml_cvc5__##CLASS ## __##APIF(value v_c, value v_p0, value v_p1){ \
    return CppCaml::apiN_class(&CLASS :: APIF, v_c, v_p0, v_p1); \
  }

#define APIM2_OVERLOAD(CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(CLASS,APIF,SUFFIX, caml_cvc5__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_cvc5__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_c, value v_p0, value v_p1){ \
    return CppCaml::apiN_class(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_c, v_p0, v_p1); \
  }

#define APIM3_OVERLOAD(CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(CLASS,APIF,SUFFIX, caml_cvc5__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_cvc5__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_c, value v_p0, value v_p1, value v_p2){ \
    return CppCaml::apiN_class(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_c, v_p0, v_p1, v_p2); \
  }


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

APIM2(Solver,mkConstArray)

APIM2(Solver,mkConst)
APIM2(Solver,mkVar)

APIM1(Solver,simplify)

APIM1(Solver,assertFormula)
APIM0(Solver,checkSat)

APIM2(Solver,setOption)

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

apireturn caml_cvc5_unit(value){
  return Val_unit;
}
