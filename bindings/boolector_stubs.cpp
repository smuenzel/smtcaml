#include <boolector/boolector.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;
using CppCaml::T_value;

DECL_API_TYPE(uint32_t,uint32_t);
DECL_API_TYPE(bool,bool);
DECL_API_TYPE(BoolectorNode*,node);
DECL_API_TYPE(BoolectorSort,sort);
DECL_API_TYPE(Btor*,btor);
DECL_API_TYPE(const char*,string);
DECL_API_TYPE(BtorOption,btor_option);
DECL_API_TYPE(void,unit);

static void abort_callback(const char* msg){
  if(Caml_state == NULL) caml_acquire_runtime_system();
  caml_failwith(msg);
}

template<> struct CppCaml::SharedPointerProperties<Btor>{
  static void delete_T(Btor*b) { boolector_delete(b); }
};

template<typename T>
struct remove_const_pointer { typedef T type; };

template<typename T>
struct remove_const_pointer<const T*> { typedef T* type; };

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

apireturn caml_boolector_new(value){
  boolector_set_abort(&abort_callback);
  return caml_boolector_btor::allocate(boolector_new());
}

REGISTER_API(boolector_new,caml_boolector_new);

template<typename t_dep> struct t_dep_container { };
template<> struct t_dep_container<BoolectorNode*> { typedef caml_boolector_node type; };
template<> struct t_dep_container<BoolectorSort> { typedef caml_boolector_sort type; };

template<typename T> concept is_dep_container = requires {
  typename t_dep_container<T>::type;
};

template<> struct CppCaml::T_value_wrapper<uint32_t>{
  static inline uint32_t get(value v) { return Long_val(v); }
};

template<> struct CppCaml::T_value_wrapper<const char*>{
  static inline const char * get(value v) { return String_val(v); }
};

template<typename t_dep> 
static inline value alloc_dependent_internal(std::shared_ptr<Btor>& btor, t_dep dep){
  typedef typename t_dep_container<t_dep>::type Container;
  return Container::allocate(btor, dep);
}

template<typename t_dep>
static inline value alloc_dependent(value v_btor, t_dep dep){
  typedef typename t_dep_container<t_dep>::type Container;
  auto& s_btor = Custom_value<caml_boolector_btor>(v_btor);
  return Container::allocate(s_btor.pT, dep);
}

apireturn caml_boolector_get_btor(value v_node){
  auto&node = Custom_value<caml_boolector_node>(v_node);
  auto btor = node.pContext;
  value v_btor = caml_alloc_custom(&ContainerOps<caml_boolector_btor>::value,sizeof(caml_boolector_btor),1,10);
  new(&Custom_value<caml_boolector_btor>(v_btor)) caml_boolector_btor(btor);
  return v_btor;
}
REGISTER_API(boolector_get_btor, caml_boolector_get_btor);

#define API0(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_btor){\
    return CppCaml::apiN(boolector_##APIF,v_btor);\
  }

#define API2(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0, value v_p1){\
    return CppCaml::apiN(boolector_##APIF,v_p0,v_p1);\
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
API0(false)
API0(true)
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
API0(bool_sort)

API2I(var);

API2(bitvec_sort);

API2I(array_sort);

API0(print_stats);
API1I(assert);

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

apireturn caml_boolector_set_solver(value v_btor, value v_solver){
  auto btor = Btor_value(v_btor);
  auto s = String_val(v_solver);
  boolector_set_sat_solver(btor,s);
  return Val_unit;
}

apireturn caml_boolector_first_opt(value v_btor){
  auto btor = Btor_value(v_btor);
  return Val_long(boolector_first_opt(btor));
}

#define OPTFUN(name, wrap) \
apireturn caml_boolector_##name (value v_btor, value v_opt){\
  auto btor = Btor_value(v_btor); \
  auto opt = (BtorOption)Long_val(v_opt); \
  auto out = boolector_##name (btor, opt); \
  return wrap(out); \
} \
REGISTER_API(boolector_##name, caml_boolector_##name)

inline value caml_copy_string_safe(const char * s) { return caml_copy_string(s?s:""); }

OPTFUN(has_opt, Val_bool);
OPTFUN(next_opt, Val_long);
OPTFUN(get_opt_lng, caml_copy_string);
OPTFUN(get_opt_desc, caml_copy_string_safe);
OPTFUN(get_opt_min, Val_long);
OPTFUN(get_opt_max, Val_long);
OPTFUN(get_opt_dflt, Val_long);
OPTFUN(get_opt, Val_long);

apireturn caml_boolector_set_opt (value v_btor, value v_opt, value v_val){
  auto btor = Btor_value(v_btor);
  auto opt = (BtorOption)Long_val(v_opt);
  boolector_set_opt (btor, opt, Long_val(v_val));
  return Val_unit;
}
REGISTER_API(boolector_set_opt,caml_boolector_set_opt);
