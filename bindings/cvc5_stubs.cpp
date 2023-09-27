#include <cvc5/cvc5.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;
using CppCaml::T_value;

using Solver = cvc5::Solver;

DECL_API_TYPE(Solver*,solver);
DECL_API_TYPE(void,unit);
DECL_API_TYPE(std::string,string);

CAML_REPRESENTATION(Solver*, ContainerSharedPointer);

template<> struct CppCaml::SharedPointerProperties<Solver>{
  static void delete_T(Solver*s) { delete s; }
};

using caml_cvc5_solver = CppCaml::ContainerSharedPointer<Solver>;


apireturn caml_cvc5__Solver__operator_new(value){
  return caml_cvc5_solver::allocate(new Solver());
}
REGISTER_API_CONSTRUCTOR(Solver,caml_cvc5__Solver__operator_new);

#define APIM2(CLASS,APIF) \
  REGISTER_API_MEMBER(CLASS,APIF, caml_cvc5__##CLASS ## __##APIF); \
  apireturn caml_cvc5__##CLASS ## __##APIF(value v_c, value v_p0, value v_p1){ \
    return CppCaml::apiN_class(&CLASS :: APIF, v_c, v_p0, v_p1); \
  }


APIM2(Solver,setOption)

apireturn caml_cvc5_unit(value){
  return Val_unit;
}
