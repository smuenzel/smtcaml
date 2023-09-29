#include <boolector/boolector.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;
using CppCaml::T_value;

struct solver_result;

DECL_API_TYPE(uint32_t,uint32_t);
DECL_API_TYPE(int32_t,int32_t);
DECL_API_TYPE(bool,bool);
DECL_API_TYPE(BoolectorNode*,node);
DECL_API_TYPE(BoolectorNode**,node array);
DECL_API_TYPE(BoolectorSort,sort);
DECL_API_TYPE(Btor*,btor);
DECL_API_TYPE(const char*,string);
DECL_API_TYPE(BtorOption,btor_option);
DECL_API_TYPE(void,unit);
DECL_API_TYPE(solver_result,solver_result);

typedef std::remove_pointer<BoolectorSort>::type BoolectorSortRaw;

CAML_REPRESENTATION(Btor*, ContainerSharedPointer);
CAML_REPRESENTATION(BoolectorNode*, ContainerWithContext);
CAML_REPRESENTATION(BoolectorSortRaw*, ContainerWithContext);
CAML_REPRESENTATION(BtorOption,Immediate);
CAML_REPRESENTATION(int32_t,Immediate);
CAML_REPRESENTATION(uint32_t,Immediate);
CAML_REPRESENTATION(BoolectorNode**, CustomWithContext);


////////////////////////////////////////////////////////////////////////////////////////

template<> struct CppCaml::CamlConversionProperties<BoolectorNode*>{
  static const auto representation_kind = CppCaml::CamlRepresentationKind::ContainerWithContext;
  typedef Btor Context;
  static void delete_T(Context*context, BoolectorNode*t){
    boolector_release(context,t);
  }
};

static_assert(CppCaml::CamlBidirectional<BoolectorNode*>);

////////////////////////////////////////////////////////////////////////////////////////


template<> struct CppCaml::SharedPointerProperties<Btor>{
  static void delete_T(Btor*b) { boolector_delete(b); }
};

template<> struct CppCaml::ValueWithContextProperties<BoolectorNode>{
  typedef Btor Context;
  static void delete_T(Context*context, BoolectorNode*t){
    boolector_release(context,t);
  }
};

template<> struct CppCaml::ValueWithContextProperties<BoolectorSortRaw>{
  typedef Btor Context;
  static void delete_T(Context*context, BoolectorSortRaw*t){
    boolector_release_sort(context,t);
  }
};

using caml_boolector_btor = CppCaml::ContainerSharedPointer<Btor>;

using caml_boolector_node =
  CppCaml::ContainerWithContext<BoolectorNode>;

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

template<> struct CppCaml::ImmediateProperties<int32_t> {
  static inline value to_value(int32_t b) { return Val_long(b); }
  static inline int32_t of_value(value v) { return Long_val(v); }
};

template<> struct CppCaml::CustomWithContextProperties<BoolectorNode**> {
  typedef Btor Context;
  static inline value to_value(std::shared_ptr<Context>&ctx, BoolectorNode** b) {
    // Assume boolector manages memory
    CAMLparam0();
    CAMLlocal2(v_ar,v_elt);
    size_t size = 0;
    auto i = b;
    while(*i){
      size++;
      i++;
    };
    v_ar = caml_alloc(size,0);
    for(size_t i = 0; i < size; i++){
      Store_field(v_ar,i, caml_boolector_node::allocate(ctx, b[i]));
    }
    CAMLreturn(v_ar);
  }
  static inline BoolectorNode** of_value(value v) { assert(false); }
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

API2(push);
API2(pop);
API1I(assert);
API1I(assume);
API1I(failed);

API1(get_failed_assumptions);
API1(fixate_assumptions);
API1(reset_assumptions);

API2(set_sat_solver);

API1(first_opt);

API2(has_opt);
API2(next_opt);
API2(get_opt_lng);
API2(get_opt_shrt);
API2(get_opt_desc);
API2(get_opt_min);
API2(get_opt_max);
API2(get_opt_dflt);
API2(get_opt);
API3(set_opt);

static void abort_callback(const char* msg){
  if(Caml_state == NULL) caml_acquire_runtime_system();
  caml_failwith(msg);
}

apireturn caml_boolector_new(value){
  boolector_set_abort(&abort_callback);
  return caml_boolector_btor::allocate(boolector_new());
}
REGISTER_API(boolector_new,caml_boolector_new);

apireturn caml_boolector_get_btor(value v_node){
  return caml_boolector_btor::allocate(Custom_value<caml_boolector_node>(v_node).pContext);
}
REGISTER_API(boolector_get_btor, caml_boolector_get_btor);

template<typename... Ps, size_t... Is>
static inline int32_t call_sat_generic(int32_t (*inner_sat)(Btor*,Ps...), Btor*btor, std::tuple<Ps...> args, std::index_sequence<Is...> is){
  return inner_sat(btor,std::get<Is>(args)...);
}

template<typename... Ps>
static value sat_generic(int32_t (*inner_sat)(Btor*,Ps...), value v_btor, typename CppCaml::first_type<value,Ps>::type... v_ps){
  /* acquire ownership before blocking section */
  auto s_btor = Custom_value<caml_boolector_btor>(v_btor).pT;
  auto index_sequence = std::index_sequence_for<Ps...>();
  std::tuple p_ar{ T_value<Ps>(v_ps)... };
  caml_enter_blocking_section();
  auto sat = call_sat_generic(inner_sat, s_btor.get(), p_ar, index_sequence);
  caml_leave_blocking_section();
  switch(sat){
    case BTOR_RESULT_SAT: return Val_int(1);
    case BTOR_RESULT_UNSAT: return Val_int(2);
    default: return Val_int(0);
  }
}

apireturn caml_boolector_sat(value v_btor){
  return sat_generic(boolector_sat, v_btor);
}
REGISTER_API_CUSTOM(boolector_sat,caml_boolector_sat,solver_result,Btor*);

apireturn caml_boolector_limited_sat(value v_btor, value lod_limit, value sat_limit){
  return sat_generic(boolector_limited_sat,v_btor,lod_limit,sat_limit);
}
REGISTER_API_CUSTOM(boolector_limited_sat,caml_boolector_limited_sat,solver_result,Btor*,int32_t,int32_t);

apireturn caml_boolector_bv_assignment(value v_p0){
  return CppCaml::apiN_implied_context_free(boolector_bv_assignment,boolector_free_bv_assignment,v_p0);
}
REGISTER_API_CUSTOM(boolector_bv_assignment,caml_boolector_bv_assignment,const char *, BoolectorNode * );

apireturn caml_boolector_unit(value){
  return Val_unit;
}
