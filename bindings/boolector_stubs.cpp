#include <boolector/boolector.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/threads.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include "caml/domain_state.h"

#include <memory>
#include <typeinfo>
#include <functional>


static void abort_callback(const char* msg){
  if(Caml_state == NULL) caml_acquire_runtime_system();
  caml_failwith(msg);
}

void delete_btor(Btor*b){
  boolector_delete(b);
}

struct caml_boolector_btor {
  std::shared_ptr<Btor> btor;

  caml_boolector_btor(Btor* b) : btor(b,delete_btor){}
  caml_boolector_btor(std::shared_ptr<Btor>&btor) : btor(btor) {}
};

template<typename T>
struct always_false : std::false_type {};

template<typename T>
struct remove_const_pointer { typedef T type; };

template<typename T>
struct remove_const_pointer<const T*> { typedef T* type; };

template<typename t_bt> void release(Btor*, t_bt){
  static_assert(always_false<t_bt>::value , "You must specialize release<> for your type");
}

template<> void release<BoolectorNode*>(Btor*btor, BoolectorNode*node){
  boolector_release(btor,node);
}

template<> void release<BoolectorSort>(Btor*btor, BoolectorSort sort){
  boolector_release_sort(btor,sort);
}

template<typename t_bt>
struct caml_boolector_wrap {
  std::shared_ptr<Btor> btor;
  t_bt dep;

  caml_boolector_wrap(std::shared_ptr<Btor>&btor, t_bt bt)
    : btor(btor), dep(bt) { }

  ~caml_boolector_wrap(){
    release(this->btor.get(), this->dep);
  }
};

typedef caml_boolector_wrap<BoolectorNode*> caml_boolector_node;
typedef caml_boolector_wrap<BoolectorSort> caml_boolector_sort;

template<typename T> static inline T& Custom_value(value v){
  return (*((T*)Data_custom_val(v)));
}


static inline Btor * Btor_value(value v){
  auto& s_btor = Custom_value<caml_boolector_btor>(v);
  return s_btor.btor.get();
}

static inline BoolectorNode *& Node_value(value v){
  return Custom_value<caml_boolector_node>(v).dep;
}

static inline caml_boolector_node& Node_s_value(value v){
  return Custom_value<caml_boolector_node>(v);
}

static inline caml_boolector_sort& Sort_s_value(value v){
  return Custom_value<caml_boolector_sort>(v);
}

static inline BoolectorSort& Sort_value(value v){
  return Custom_value<caml_boolector_sort>(v).dep;
}

template<typename T> void finalize_custom(value v_custom){
  Custom_value<T>(v_custom).~T();
}

template<typename Container> struct container_ops {
  static struct custom_operations value;
};

template<typename Container>
  struct custom_operations container_ops<Container>::value =
  { typeid(Container).name()
  , &finalize_custom<Container>
  , custom_compare_default
  , custom_hash_default
  , custom_serialize_default
  , custom_deserialize_default
  , custom_compare_ext_default
  , custom_fixed_length_default
  };

#define apireturn extern "C" CAMLprim value

apireturn caml_boolector_new(value){
  boolector_set_abort(&abort_callback);
  value v_btor = caml_alloc_custom(&container_ops<caml_boolector_btor>::value,sizeof(caml_boolector_btor),1,10);
  new(&Custom_value<caml_boolector_btor>(v_btor)) caml_boolector_btor(boolector_new());
  return v_btor;
}

template<typename t_dep> struct t_dep_container { typedef void type; };
template<> struct t_dep_container<BoolectorNode*> { typedef caml_boolector_node type; };
template<> struct t_dep_container<BoolectorSort> { typedef caml_boolector_sort type; };

template<typename t_dep> 
static inline value alloc_dependent_internal(std::shared_ptr<Btor>& btor, t_dep dep){
  typedef typename t_dep_container<t_dep>::type Container;
  value v_container = caml_alloc_custom(&container_ops<Container>::value,sizeof(Container),1,500000);
  new(&Custom_value<Container>(v_container)) Container(btor, dep);
  return v_container;
}

template<typename t_dep>
static inline value alloc_dependent(value v_btor, t_dep dep){
  typedef typename t_dep_container<t_dep>::type Container;
  auto& s_btor = Custom_value<Container>(v_btor);
  return alloc_dependent_internal<t_dep>(s_btor.btor, dep);
}

apireturn caml_boolector_get_btor(value v_node){
  auto node = Custom_value<caml_boolector_node>(v_node);
  auto btor = node.btor;
  value v_btor = caml_alloc_custom(&container_ops<caml_boolector_btor>::value,sizeof(caml_boolector_btor),1,10);
  new(&Custom_value<caml_boolector_btor>(v_btor)) caml_boolector_btor(btor);
  return v_btor;
}

template<typename F> inline value boolector_api0(F mkdep, value v_btor){
  auto btor = Btor_value(v_btor);
  auto dep = mkdep(btor);
  value v_dep = alloc_dependent<decltype(dep)>(v_btor, dep);
  return v_dep;
}

template<typename R, typename A0, typename A1> inline value
boolector_api1_implied(R (*mknod)(A0, A1), value v_p0){
  auto p0_s = Custom_value<caml_boolector_wrap<typename remove_const_pointer<A1>::type>>(v_p0);
  auto p0 = p0_s.dep;
  auto btor = p0_s.btor.get();
  // we retrieve all the inner values before allocation, so we don't need to register
  // roots
  auto dep = mknod(btor,p0);
  value v_dep = alloc_dependent_internal(p0_s.btor, dep);
  return v_dep;
}

template<typename R, typename A0, typename A1, typename A2> inline value
boolector_api2_implied(R (*mknod)(A0,A1,A2), value v_p0, value v_p1){
  auto p0_s = Custom_value<caml_boolector_wrap<A1>>(v_p0);
  auto p0 = p0_s.dep;
  auto p1 = Custom_value<caml_boolector_wrap<A2>>(v_p1).dep;
  auto btor = p0_s.btor.get();
  // we retrieve all the inner values before allocation, so we don't need to register
  // roots
  auto node = mknod(btor,p0,p1);
  value v_node = alloc_dependent_internal(p0_s.btor, node);
  return v_node;
}

template<typename R, typename A0, typename A1, typename A2, typename A3> inline value
boolector_api3_implied(R (*mknod)(A0,A1,A2,A3), value v_p0, value v_p1, value v_p2){
  auto p0_s = Custom_value<caml_boolector_wrap<A1>>(v_p0);
  auto p0 = p0_s.dep;
  auto p1 = Custom_value<caml_boolector_wrap<A2>>(v_p1).dep;
  auto p2 = Custom_value<caml_boolector_wrap<A3>>(v_p2).dep;
  auto btor = p0_s.btor.get();
  // we retrieve all the inner values before allocation, so we don't need to register
  // roots
  auto node = mknod(btor,p0,p1,p2);
  value v_node = alloc_dependent_internal(p0_s.btor, node);
  return v_node;
}

#define API0(APIF) \
  apireturn caml_##APIF (value v_btor){\
    return boolector_api0(APIF,v_btor);\
  }

#define API1(APIF) \
  apireturn caml_##APIF (value v_p0){\
    return boolector_api1_implied(APIF,v_p0);\
  }

#define API2(APIF) \
  apireturn caml_##APIF (value v_p0, value v_p1){\
    return boolector_api2_implied(APIF,v_p0, v_p1);\
  }

#define API3(APIF) \
  apireturn caml_##APIF (value v_p0, value v_p1, value v_p2){\
    return boolector_api3_implied(APIF,v_p0, v_p1, v_p2);\
  }

API1(boolector_get_sort);

API0(boolector_false);
API0(boolector_true);
API2(boolector_implies);
API2(boolector_iff);
API2(boolector_eq);
API2(boolector_ne);
API2(boolector_xor);
API2(boolector_and);
API2(boolector_or);
API2(boolector_nand);
API2(boolector_nor);
API2(boolector_add);
API2(boolector_uaddo);
API2(boolector_saddo);
API2(boolector_mul);
API2(boolector_umulo);
API2(boolector_smulo);
API2(boolector_ult);
API2(boolector_ulte);
API2(boolector_slte);
API2(boolector_slt);
API2(boolector_ugt);
API2(boolector_sgt);
API2(boolector_ugte);
API2(boolector_sgte);
API2(boolector_sll);
API2(boolector_srl);
API2(boolector_sra);
API2(boolector_rol);
API2(boolector_ror);
API2(boolector_sub);
API2(boolector_usubo);
API2(boolector_ssubo);
API2(boolector_udiv);
API2(boolector_sdiv);
API2(boolector_sdivo);
API2(boolector_urem);
API2(boolector_srem);
API2(boolector_smod);
API2(boolector_concat);
API2(boolector_read);
API3(boolector_write);
API3(boolector_cond);
API1(boolector_inc);
API1(boolector_dec);


apireturn caml_boolector_var(value v_sort, value v_symbol){
  auto sort_s = Sort_s_value(v_sort);
  auto symbol = String_val(v_symbol);
  auto node = boolector_var(sort_s.btor.get(), sort_s.dep, symbol);
  return alloc_dependent_internal(sort_s.btor, node);
}

API0(boolector_bool_sort);

apireturn caml_boolector_bitvec_sort(value v_btor, value v_width){
  auto btor = Btor_value(v_btor);
  auto width = Int_val(v_width);
  auto dep = boolector_bitvec_sort(btor, width);
  value v_dep = alloc_dependent<decltype(dep)>(v_btor, dep);
  return v_dep;
}

apireturn caml_boolector_array_sort(value v_index, value v_element){
  auto index_s = Sort_s_value(v_index);
  auto element = Sort_value(v_element);
  auto sort = boolector_array_sort(index_s.btor.get(), index_s.dep, element);
  return alloc_dependent_internal(index_s.btor, sort);
}

apireturn caml_boolector_print_stats(value v_btor){
  auto btor = Btor_value(v_btor);
  boolector_print_stats(btor);
  return Val_unit;
}

apireturn caml_boolector_assert(value v_node){
  auto node = Custom_value<caml_boolector_node>(v_node);
  boolector_assert(node.btor.get(),node.dep);
  return Val_unit;
}

apireturn caml_boolector_sat(value v_btor){
  /* acquire ownership before blocking section */
  auto s_btor = Custom_value<caml_boolector_btor>(v_btor).btor;
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
}

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
