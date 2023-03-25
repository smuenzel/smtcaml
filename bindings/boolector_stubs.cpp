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
#include <cassert>
#include <cstring>

#define apireturn extern "C" CAMLprim value

value caml_alloc_m(int line, mlsize_t s, tag_t t){
  fprintf(stderr, "line=%i, size = %lu\n", line, s); 
  fflush(stderr);
  return caml_alloc(s,t);
}

#define caml_alloc(S,T) caml_alloc_m(__LINE__,S,T)

template<typename T>
struct always_false : std::false_type {};

struct string_literal_v {
  size_t size;
  char value[];

  constexpr string_literal_v(const char (&str)[]) {
    auto size = strlen(str);
    std::copy(str, str + size, value);
  }
};

template <auto Size>
struct string_literal {
  size_t size = Size;
  char value[Size];
  constexpr string_literal(const char (&str)[Size]) {
    std::copy(str, str + Size, value);
  }
};

template<typename T>
struct CamlApiTypename{
  static_assert(always_false<T>::value , "You must specialize CamlApiTypename<> for your type");
};

template<typename T> struct CamlApiTypename<const T> : CamlApiTypename<T> {};
template<typename T> struct CamlApiTypename<const T*> : CamlApiTypename<T*> {};

#define DECL_API_TYPE(c_type, caml_type) \
  template<> \
  struct CamlApiTypename<c_type>{ \
    static constexpr const char * name = #caml_type; \
  } 

DECL_API_TYPE(uint32_t,uint32_t);
DECL_API_TYPE(bool,bool);
DECL_API_TYPE(BoolectorNode*,node);
DECL_API_TYPE(BoolectorSort,sort);
DECL_API_TYPE(Btor*,btor);

typedef const char* cstring;

template<typename T>
struct CamlLinkedList{
  const T data;
  const CamlLinkedList<T>*next;

  constexpr CamlLinkedList(T data, const CamlLinkedList<T>*next=nullptr) 
    : data{data}, next{next} {}
};

template<typename... Ps> struct Params;

template<> struct Params<>{
  static constexpr const CamlLinkedList<cstring>* p = nullptr;
};

template<typename P, typename... Ps> struct Params<P, Ps...>{
  static inline constexpr const CamlLinkedList<cstring> pp =
    CamlLinkedList(CamlApiTypename<P>::name,Params<Ps...>::p);
  static inline constexpr const CamlLinkedList<cstring>* p = &pp;
};

struct CamlApiFunctionDescription {
  cstring return_type;
  const size_t parameter_count;
  const CamlLinkedList<cstring>* parameters;

  template<typename R, typename... Ps>
    constexpr CamlApiFunctionDescription(R (*fun)(Ps...))
    : return_type(CamlApiTypename<R>::name), parameter_count(sizeof...(Ps))
      , parameters(Params<Ps...>::p)
    {}

  value to_value();
};

static constexpr const uint64_t marker = 0xe1176dafdeadbeefl;

struct CamlApiRegistryEntry {
  const size_t q1;
  const char * wrapper_name;
  const char * name;
  CamlApiFunctionDescription description;

  template<typename R, typename... Ps>
  constexpr CamlApiRegistryEntry(const char * name, const char*wrapper_name, R (*fun)(Ps...))
    : q1(marker), wrapper_name(wrapper_name), name(name), description(fun)
  { }

  value to_value();
};

value list_to_caml(const CamlLinkedList<cstring>* l){
  if(l == nullptr){
    return Val_long(0);
  } else {
    CAMLparam0();
    CAMLlocal1(v_l);
    v_l = caml_alloc(2,0);
    Store_field(v_l, 0, caml_copy_string(l->data));
    Store_field(v_l, 1, list_to_caml(l->next));
    CAMLreturn(v_l);
  }
}

value CamlApiFunctionDescription::to_value(){
  CAMLparam0();
  CAMLlocal5(v_ret, v_return_type, v_parameter_count, v_parameters, v_it);
  CAMLlocal1(v_next);
  v_return_type = caml_copy_string(this->return_type);
  v_parameter_count = Val_long(this->parameter_count);
  v_parameters = list_to_caml(this->parameters);
  v_ret = caml_alloc_small(3,0);
  Field(v_ret,0) = v_return_type;
  Field(v_ret,1) = v_parameter_count;
  Field(v_ret,2) = v_parameters;
  CAMLreturn(v_ret);
}

value CamlApiRegistryEntry::to_value(){
  CAMLparam0();
  CAMLlocal4(v_ret,v_wrapper_name, v_name, v_description);
  v_wrapper_name = caml_copy_string(this->wrapper_name);
  v_name = caml_copy_string(this->name);
  v_description = this->description.to_value();
  v_ret = caml_alloc_small(3, 0);
  Field(v_ret,0) = v_wrapper_name;
  Field(v_ret,1) = v_name;
  Field(v_ret,2) = v_description;
  CAMLreturn(v_ret);
}

value api_registry_entry_to_list(CamlApiRegistryEntry*start, CamlApiRegistryEntry*stop){
  uint64_t* idx = (uint64_t*)start;
  uint64_t* sstop = (uint64_t*)stop;
  fprintf(stderr, "start=%p, stop=%p\n", start,stop);
  fflush(stderr);
  while(idx < sstop && *idx != marker){
    idx++;
  };
  if(idx >= sstop) return Val_long(0);
  else{
    auto entry = (CamlApiRegistryEntry*)idx;
    CAMLparam0();
    CAMLlocal1(v_ret);
    v_ret = caml_alloc(2, 0);
    Store_field(v_ret,0,entry->to_value());
    Store_field(v_ret,1,api_registry_entry_to_list(entry+1, stop));
    CAMLreturn(v_ret);
  }
}

apireturn caml_get_api_registry(value){
  extern CamlApiRegistryEntry __start_caml_api_registry;
  extern CamlApiRegistryEntry __stop_caml_api_registry;
  CamlApiRegistryEntry * start = &__start_caml_api_registry;
  CamlApiRegistryEntry * stop = &__stop_caml_api_registry;
  return api_registry_entry_to_list(start,stop);
}

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

apireturn caml_boolector_new(value){
  boolector_set_abort(&abort_callback);
  value v_btor = caml_alloc_custom(&container_ops<caml_boolector_btor>::value,sizeof(caml_boolector_btor),1,10);
  new(&Custom_value<caml_boolector_btor>(v_btor)) caml_boolector_btor(boolector_new());
  return v_btor;
}

template<typename t_dep> struct t_dep_container { };
template<> struct t_dep_container<BoolectorNode*> { typedef caml_boolector_node type; };
template<> struct t_dep_container<BoolectorSort> { typedef caml_boolector_sort type; };

template<typename T> concept is_dep_container = requires {
  typename t_dep_container<T>::type;
};

template<typename T> T T_value(value v){
  static_assert(always_false<T>::value , "You must specialize T_value<> for your type");
}

template<typename T> requires is_dep_container<T>
T T_value(value v){
  return Custom_value<caml_boolector_wrap<T>>(v).dep;
}

template<> uint32_t T_value<uint32_t>(value v){
  return Long_val(v);
}

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
  auto p1 = T_value<A2>(v_p1);
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
  auto p1 = T_value<A2>(v_p1);
  auto p2 = T_value<A3>(v_p2);
  auto btor = p0_s.btor.get();
  // we retrieve all the inner values before allocation, so we don't need to register
  // roots
  auto node = mknod(btor,p0,p1,p2);
  value v_node = alloc_dependent_internal(p0_s.btor, node);
  return v_node;
}

#define REGISTER_API(APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__##APIF \
__attribute((used, section("caml_api_registry"))) = CamlApiRegistryEntry(#APIF,#WRAPPER,APIF);


#define API0(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_btor){\
    return boolector_api0(boolector_##APIF,v_btor);\
  }

#define API1(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0){\
    return boolector_api1_implied(boolector_##APIF,v_p0);\
  }

#define API2(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
  apireturn caml_boolector_##APIF (value v_p0, value v_p1){\
    return boolector_api2_implied(boolector_##APIF,v_p0, v_p1);\
  }

#define API3(APIF) \
  REGISTER_API(boolector_##APIF, caml_boolector_##APIF); \
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
  auto node = boolector_var(sort_s.btor.get(), sort_s.dep, symbol);
  return alloc_dependent_internal(sort_s.btor, node);
}

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
