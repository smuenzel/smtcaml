#include <cvc5/cvc5.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;
using CppCaml::T_value;

using Solver = cvc5::Solver;
using Sort = cvc5::Sort;

DECL_API_TYPE(uint32_t,uint32_t);
DECL_API_TYPE(int32_t,int32_t);
DECL_API_TYPE(bool,bool);
DECL_API_TYPE(Solver*,solver);
DECL_API_TYPE(Sort,sort);
DECL_API_TYPE(Sort*,sort);
DECL_API_TYPE(void,unit);
DECL_API_TYPE(std::string,string);

CAML_REPRESENTATION(Solver*, ContainerSharedPointer);
CAML_REPRESENTATION(Sort,InlinedWithContext);

CAML_REPRESENTATION(int32_t,Immediate);
CAML_REPRESENTATION(uint32_t,Immediate);

template<> struct CppCaml::ImmediateProperties<uint32_t> {
  static inline value to_value(uint32_t b) { return Val_long(b); }
  static inline uint32_t of_value(value v) { return Long_val(v); }
};

template<> struct CppCaml::ImmediateProperties<int32_t> {
  static inline value to_value(int32_t b) { return Val_long(b); }
  static inline int32_t of_value(value v) { return Long_val(v); }
};

template<> struct CppCaml::ValueWithContextProperties<Sort>{
  typedef Solver Context;
  static void delete_T(Context*context, Sort*t){
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
APIM2(Solver,setOption)

APIM0(Sort,hasSymbol)
APIM0(Sort,getSymbol)
APIM0(Sort,isNull)
APIM0(Sort,isBoolean)
APIM0(Sort,isInteger)
APIM0(Sort,isReal)
APIM0(Sort,isInstantiated)

apireturn caml_cvc5_unit(value){
  return Val_unit;
}
