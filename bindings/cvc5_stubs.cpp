#include <cvc5/cvc5.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;
using CppCaml::T_value;

using Solver = cvc5::Solver;

DECL_API_TYPE(Solver*,solver);

CAML_REPRESENTATION(Solver*, ContainerSharedPointer);

template<> struct CppCaml::SharedPointerProperties<Solver>{
  static void delete_T(Solver*s) { delete s; }
};

using caml_cvc5_solver = CppCaml::ContainerSharedPointer<Solver>;


apireturn caml_cvc5_new_Solver(value){
  return caml_cvc5_solver::allocate(new Solver());
}
REGISTER_API_CONSTRUCTOR(Solver,caml_cvc5_new_Solver);

apireturn caml_cvc5_unit(value){
  return Val_unit;
}
