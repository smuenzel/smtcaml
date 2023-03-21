#include <boolector/boolector.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/custom.h>

#include <memory>
#include <typeinfo>

void delete_btor(Btor*b){
  boolector_delete(b);
}

struct caml_boolector_btor {
  std::shared_ptr<Btor> btor;

  caml_boolector_btor(Btor* b) : btor(b,delete_btor){}
};

struct caml_boolector_node {
  std::shared_ptr<Btor> btor;
  BoolectorNode* node;

  caml_boolector_node(std::shared_ptr<Btor>&btor, BoolectorNode*node)
    : btor(btor), node(node) { }

  ~caml_boolector_node(){
    boolector_release(this->btor.get(), this->node);
  }
};

struct caml_boolector_sort {
  std::shared_ptr<Btor> btor;
  BoolectorSort sort;

  caml_boolector_sort(std::shared_ptr<Btor>&btor, BoolectorSort sort)
    : btor(btor), sort(sort) { }

  ~caml_boolector_sort(){
    boolector_release_sort(this->btor.get(), this->sort);
  }
};

template<typename T> static inline T& Custom_value(value v){
  return (*((T*)Data_custom_val(v)));
}


static inline Btor * Btor_value(value v){
  auto& s_btor = Custom_value<caml_boolector_btor>(v);
  return s_btor.btor.get();
}

static inline BoolectorNode *& Node_value(value v){
  return Custom_value<caml_boolector_node>(v).node;
}

static inline caml_boolector_node& Node_s_value(value v){
  return Custom_value<caml_boolector_node>(v);
}

static inline caml_boolector_sort& Sort_s_value(value v){
  return Custom_value<caml_boolector_sort>(v);
}

static inline BoolectorSort& Sort_value(value v){
  return Custom_value<caml_boolector_sort>(v).sort;
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
  value v_btor = caml_alloc_custom(&container_ops<caml_boolector_btor>::value,sizeof(caml_boolector_btor),1,10);
  new(&Custom_value<caml_boolector_btor>(v_btor)) caml_boolector_btor(boolector_new());
  return v_btor;
}

template<typename t_dep> struct t_dep_container { typedef void type; };
template<> struct t_dep_container<BoolectorNode*> { typedef caml_boolector_node type; };
template<> struct t_dep_container<BoolectorSort> { typedef caml_boolector_sort type; };

template<typename Value> 
static inline value alloc_dependent_internal(std::shared_ptr<Btor>& btor, Value dep){
  typedef typename t_dep_container<Value>::type Container;
  value v_container = caml_alloc_custom(&container_ops<Container>::value,sizeof(Container),1,50000);
  new(&Custom_value<Container>(v_container)) Container(btor, dep);
  return v_container;
}

template<typename Value>
static inline value alloc_dependent(value v_btor, Value dep){
  typedef typename t_dep_container<Value>::type Container;
  auto& s_btor = Custom_value<Container>(v_btor);
  return alloc_dependent_internal<Value>(s_btor.btor, dep);
}

template<typename F> inline value boolector_api0(F mkdep, value v_btor){
  auto btor = Btor_value(v_btor);
  auto dep = mkdep(btor);
  value v_dep = alloc_dependent<decltype(dep)>(v_btor, dep);
  return v_dep;
}

template<typename F> inline value
boolector_node_api2(F mknod, value v_btor, value v_p0, value v_p1){
  auto p0 = Node_value(v_p0);
  auto p1 = Node_value(v_p1);
  auto btor = Btor_value(v_btor);
  auto node = mknod(btor,p0,p1);
  // we retrieve all the inner values before allocation, so we don't need to register
  // roots
  value v_node = alloc_dependent(v_btor, node);
  return v_node;
}

template<typename F> inline value
boolector_node_api2_implied(F mknod, value v_p0, value v_p1){
  auto p0_s = Node_s_value(v_p0);
  auto p0 = p0_s.node;
  auto p1 = Node_value(v_p1);
  auto btor = p0_s.btor.get();
  // we retrieve all the inner values before allocation, so we don't need to register
  // roots
  auto node = mknod(btor,p0,p1);
  value v_node = alloc_dependent_internal(p0_s.btor, node);
  return v_node;
}

#define API0(APIF) \
  apireturn caml_##APIF (value v_btor){\
    return boolector_api0(APIF,v_btor);\
  }

#define API2(APIF) \
  apireturn caml_##APIF (value v_btor, value v_p0, value v_p1){\
    return boolector_node_api2(APIF,v_btor, v_p0, v_p1);\
  } \
  apireturn caml_##APIF##_implied(value v_p0, value v_p1){\
    return boolector_node_api2_implied(APIF,v_p0, v_p1);\
  } \

API0(boolector_false);
API0(boolector_true);
API2(boolector_implies);
API2(boolector_iff);
API2(boolector_eq);
API2(boolector_ne);

apireturn caml_boolector_var_implied(value v_sort, value v_symbol){
  auto sort_s = Sort_s_value(v_sort);
  auto symbol = String_val(v_symbol);
  auto node = boolector_var(sort_s.btor.get(), sort_s.sort, symbol);
  return alloc_dependent_internal(sort_s.btor, node);
}

API0(boolector_bool_sort);

