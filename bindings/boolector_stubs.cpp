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

void delete_btor(Btor*b){
  boolector_delete(b);
}

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

CAML_REPRESENTATION(Btor, ContainerSharedPointer);
CAML_REPRESENTATION(BoolectorNode, ContainerWithContext);
CAML_REPRESENTATION(BoolectorSortRaw, ContainerWithContext);

using caml_boolector_btor = CppCaml::ContainerSharedPointer<Btor, delete_btor>;

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

template<typename T> requires is_dep_container<T>
T T_value(value v){
  return Custom_value<caml_boolector_wrap<T>>(v).t;
}

template<> uint32_t CppCaml::T_value<uint32_t>(value v){
  return Long_val(v);
}

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
  auto node = Custom_value<caml_boolector_node>(v_node);
  auto btor = node.pContext;
  value v_btor = caml_alloc_custom(&ContainerOps<caml_boolector_btor>::value,sizeof(caml_boolector_btor),1,10);
  new(&Custom_value<caml_boolector_btor>(v_btor)) caml_boolector_btor(btor);
  return v_btor;
}
REGISTER_API(boolector_get_btor, caml_boolector_get_btor);

template<typename F> inline value boolector_api0(F mkdep, value v_btor){
  auto btor = Btor_value(v_btor);
  auto dep = mkdep(btor);
  value v_dep = alloc_dependent<decltype(dep)>(v_btor, dep);
  return v_dep;
}

template<typename R, typename A0, typename A1> inline value
boolector_api1_implied(R (*mknod)(A0, A1), value v_p0){
  auto p0_s = Custom_value<caml_boolector_wrap<typename remove_const_pointer<A1>::type>>(v_p0);
  auto p0 = p0_s.t;
  auto btor = p0_s.pContext.get();
  // we retrieve all the inner values before allocation, so we don't need to register
  // roots
  auto dep = mknod(btor,p0);
  value v_dep = alloc_dependent_internal(p0_s.pContext, dep);
  return v_dep;
}

template<typename R, typename A0, typename A1, typename A2> inline value
boolector_api2_implied(R (*mknod)(A0,A1,A2), value v_p0, value v_p1){
  auto p0_s = Custom_value<caml_boolector_wrap<A1>>(v_p0);
  auto p0 = p0_s.t;
  auto p1 = T_value<A2>(v_p1);
  auto btor = p0_s.pContext.get();
  // we retrieve all the inner values before allocation, so we don't need to register
  // roots
  auto node = mknod(btor,p0,p1);
  value v_node = alloc_dependent_internal(p0_s.pContext, node);
  return v_node;
}

template<typename R, typename A0, typename A1, typename A2, typename A3> inline value
boolector_api3_implied(R (*mknod)(A0,A1,A2,A3), value v_p0, value v_p1, value v_p2){
  auto p0_s = Custom_value<caml_boolector_wrap<A1>>(v_p0);
  auto p0 = p0_s.t;
  auto p1 = T_value<A2>(v_p1);
  auto p2 = T_value<A3>(v_p2);
  auto btor = p0_s.pContext.get();
  // we retrieve all the inner values before allocation, so we don't need to register
  // roots
  auto node = mknod(btor,p0,p1,p2);
  value v_node = alloc_dependent_internal(p0_s.pContext, node);
  return v_node;
}

#define API0(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_btor){\
    return boolector_api0(boolector_##APIF,v_btor);\
  }

#define API1(APIF) \
  REGISTER_API_IMPLIED_FIRST(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0){\
    return boolector_api1_implied(boolector_##APIF,v_p0);\
  }

#define API2(APIF) \
  REGISTER_API_IMPLIED_FIRST(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0, value v_p1){\
    return boolector_api2_implied(boolector_##APIF,v_p0, v_p1);\
  }

#define API3(APIF) \
  REGISTER_API_IMPLIED_FIRST(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0, value v_p1, value v_p2){\
    return boolector_api3_implied(boolector_##APIF,v_p0, v_p1, v_p2);\
  }

API1(get_sort)
API0(false)
API0(true)
API2(implies)
API2(iff)
API2(eq)
API2(ne)
API1(not)
API1(neg)
API1(redor)
API1(redxor)
API1(redand)
API3(slice)
API2(uext)
API2(sext)
API2(xor)
API2(and)
API2(or)
API2(nand)
API2(nor)
API2(add)
API2(uaddo)
API2(saddo)
API2(mul)
API2(umulo)
API2(smulo)
API2(ult)
API2(ulte)
API2(slte)
API2(slt)
API2(ugt)
API2(sgt)
API2(ugte)
API2(sgte)
API2(sll)
API2(srl)
API2(sra)
API2(rol)
API2(ror)
API2(roli)
API2(rori)
API2(sub)
API2(usubo)
API2(ssubo)
API2(udiv)
API2(sdiv)
API2(sdivo)
API2(urem)
API2(srem)
API2(smod)
API2(concat)
API2(read)
API3(write)
API3(cond)
API1(inc)
API1(dec)
API0(bool_sort)

apireturn caml_boolector_var(value v_sort, value v_symbol){
  auto sort_s = Sort_s_value(v_sort);
  auto symbol = String_val(v_symbol);
  auto node = boolector_var(sort_s.pContext.get(), sort_s.t, symbol);
  return alloc_dependent_internal(sort_s.pContext, node);
}
REGISTER_API_IMPLIED_FIRST(boolector_var,caml_boolector_var);

apireturn caml_boolector_bitvec_sort(value v_btor, value v_width){
  auto btor = Btor_value(v_btor);
  auto width = Int_val(v_width);
  auto dep = boolector_bitvec_sort(btor, width);
  value v_dep = alloc_dependent<decltype(dep)>(v_btor, dep);
  return v_dep;
}
REGISTER_API(boolector_bitvec_sort,caml_boolector_bitvec_sort);

apireturn caml_boolector_array_sort(value v_index, value v_element){
  auto index_s = Sort_s_value(v_index);
  auto element = Sort_value(v_element);
  auto sort = boolector_array_sort(index_s.pContext.get(), index_s.t, element);
  return alloc_dependent_internal(index_s.pContext, sort);
}

apireturn caml_boolector_print_stats(value v_btor){
  auto btor = Btor_value(v_btor);
  boolector_print_stats(btor);
  return Val_unit;
}

apireturn caml_boolector_assert(value v_node){
  auto node = Custom_value<caml_boolector_node>(v_node);
  boolector_assert(node.pContext.get(),node.t);
  return Val_unit;
}

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
