#include <boolector/boolector.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;
using CppCaml::T_value;

struct solver_result;

DECL_API_TYPE(uint32_t,uint32_t);
DECL_API_TYPE(bool,bool);
DECL_API_TYPE(BoolectorNode*,node);
DECL_API_TYPE(BoolectorSort,sort);
DECL_API_TYPE(Btor*,btor);
DECL_API_TYPE(const char*,string);
DECL_API_TYPE(BtorOption,btor_option);
DECL_API_TYPE(void,unit);
DECL_API_TYPE(solver_result,solver_result);

static void abort_callback(const char* msg){
  if(Caml_state == NULL) caml_acquire_runtime_system();
  caml_failwith(msg);
}

template<> struct CppCaml::SharedPointerProperties<Btor>{
  static void delete_T(Btor*b) { boolector_delete(b); }
};

template<> struct CppCaml::ValueWithContextProperties<BoolectorNode>{
  typedef Btor Context;
  static void delete_T(Context*context, BoolectorNode*t){
    boolector_release(context,t);
  }
};

typedef std::remove_pointer<BoolectorSort>::type BoolectorSortRaw;

template<> struct CppCaml::ValueWithContextProperties<BoolectorSortRaw>{
  typedef Btor Context;
  static void delete_T(Context*context, BoolectorSortRaw*t){
    boolector_release_sort(context,t);
  }
};

CAML_REPRESENTATION(Btor*, ContainerSharedPointer);
CAML_REPRESENTATION(BoolectorNode*, ContainerWithContext);
CAML_REPRESENTATION(BoolectorSortRaw*, ContainerWithContext);
CAML_REPRESENTATION(BtorOption,Immediate);
CAML_REPRESENTATION(uint32_t,Immediate);

using caml_boolector_btor = CppCaml::ContainerSharedPointer<Btor>;

using caml_boolector_node =
  CppCaml::ContainerWithContext<BoolectorNode>;
using caml_boolector_sort =
  CppCaml::ContainerWithContext<std::remove_pointer<BoolectorSort>::type>;

template<typename t_bt>
struct caml_boolector_wrap {
  static_assert(CppCaml::always_false<t_bt>::value , "You must specialize caml_boolector_wrap for your type");
};

template<> struct caml_boolector_wrap<BoolectorNode*> : caml_boolector_node { };
template<> struct caml_boolector_wrap<BoolectorSort> : caml_boolector_sort { };


static inline Btor * Btor_value(value v){
  auto& s_btor = Custom_value<caml_boolector_btor>(v);
  return s_btor.get();
}

static inline BoolectorNode *& Node_value(value v){
  return Custom_value<caml_boolector_node>(v).t;
}

static inline caml_boolector_node& Node_s_value(value v){
  return Custom_value<caml_boolector_node>(v);
}

static inline caml_boolector_sort& Sort_s_value(value v){
  return Custom_value<caml_boolector_sort>(v);
}

static inline BoolectorSort& Sort_value(value v){
  return Custom_value<caml_boolector_sort>(v).t;
}

template<> struct CppCaml::T_value_wrapper<const char*>{
  static inline const char * get(value v) { return String_val(v); }
};

template<> struct CppCaml::ImmediateProperties<BtorOption> {
  static inline value to_value(BtorOption b) { return Val_long(b); }
  static inline BtorOption of_value(value v) { return (BtorOption)Long_val(v); }
};

template<> struct CppCaml::ImmediateProperties<uint32_t> {
  static inline value to_value(uint32_t b) { return Val_long(b); }
  static inline uint32_t of_value(value v) { return Long_val(v); }
};


#define API1(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_btor){\
    return CppCaml::apiN(boolector_##APIF,v_btor);\
  }

#define API2(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0, value v_p1){\
    return CppCaml::apiN(boolector_##APIF,v_p0,v_p1);\
  }

#define API3(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0, value v_p1, value v_p2){\
    return CppCaml::apiN(boolector_##APIF,v_p0,v_p1,v_p2);\
  }

#define API1I(APIF) \
  REGISTER_API_IMPLIED_FIRST(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0){\
    return CppCaml::apiN_implied_context(boolector_##APIF,v_p0);\
  }

#define API2I(APIF) \
  REGISTER_API_IMPLIED_FIRST(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0, value v_p1){\
    return CppCaml::apiN_implied_context(boolector_##APIF,v_p0, v_p1);\
  }

#define API3I(APIF) \
  REGISTER_API_IMPLIED_FIRST(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0, value v_p1, value v_p2){\
    return CppCaml::apiN_implied_context(boolector_##APIF,v_p0, v_p1, v_p2);\
  }

API1I(get_sort)
API1(false)
API1(true)
API2I(implies)
API2I(iff)
API2I(eq)
API2I(ne)
API1I(not)
API1I(neg)
API1I(redor)
API1I(redxor)
API1I(redand)
API3I(slice)
API2I(uext)
API2I(sext)
API2I(xor)
API2I(and)
API2I(or)
API2I(nand)
API2I(nor)
API2I(add)
API2I(uaddo)
API2I(saddo)
API2I(mul)
API2I(umulo)
API2I(smulo)
API2I(ult)
API2I(ulte)
API2I(slte)
API2I(slt)
API2I(ugt)
API2I(sgt)
API2I(ugte)
API2I(sgte)
API2I(sll)
API2I(srl)
API2I(sra)
API2I(rol)
API2I(ror)
API2I(roli)
API2I(rori)
API2I(sub)
API2I(usubo)
API2I(ssubo)
API2I(udiv)
API2I(sdiv)
API2I(sdivo)
API2I(urem)
API2I(srem)
API2I(smod)
API2I(concat)
API2I(read)
API3I(write)
API3I(cond)
API1I(inc)
API1I(dec)
API1(bool_sort)

API2I(var);

API2(bitvec_sort);

API2I(array_sort);

API1(print_stats);
API1I(assert);

API2(set_sat_solver);

API1(first_opt);

API2(has_opt);
API2(next_opt);
API2(get_opt_lng);
API2(get_opt_desc);
API2(get_opt_min);
API2(get_opt_max);
API2(get_opt_dflt);
API2(get_opt);
API3(set_opt);


apireturn caml_boolector_new(value){
  boolector_set_abort(&abort_callback);
  return caml_boolector_btor::allocate(boolector_new());
}
REGISTER_API(boolector_new,caml_boolector_new);

apireturn caml_boolector_get_btor(value v_node){
  auto&node = Custom_value<caml_boolector_node>(v_node);
  auto btor = node.pContext;
  value v_btor = caml_alloc_custom(&ContainerOps<caml_boolector_btor>::value,sizeof(caml_boolector_btor),1,10);
  new(&Custom_value<caml_boolector_btor>(v_btor)) caml_boolector_btor(btor);
  return v_btor;
}
REGISTER_API(boolector_get_btor, caml_boolector_get_btor);

apireturn caml_boolector_sat(value v_btor){
  /* acquire ownership before blocking section */
  auto s_btor = Custom_value<caml_boolector_btor>(v_btor).pT;
  caml_enter_blocking_section();
  auto sat = boolector_sat(s_btor.get());
  caml_leave_blocking_section();
  switch(sat){
    case BTOR_RESULT_SAT: return Val_int(1);
    case BTOR_RESULT_UNSAT: return Val_int(2);
    default: return Val_int(0);
  }
}
REGISTER_API_CUSTOM(boolector_sat,caml_boolector_sat,solver_result,Btor*);

