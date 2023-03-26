
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

namespace CppCaml {

template<typename T>
struct always_false : std::false_type {};

template<typename T>
struct ApiTypename{
  static_assert(always_false<T>::value , "You must specialize ApiTypename<> for your type");
};

template<typename T> struct ApiTypename<const T> : ApiTypename<T> {};
template<typename T> struct ApiTypename<const T*> : ApiTypename<T*> {};

#define DECL_API_TYPE(c_type, caml_type) \
  template<> \
  struct CppCaml::ApiTypename<c_type>{ \
    static constexpr const char * name = #caml_type; \
  } 

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
    CamlLinkedList(ApiTypename<P>::name,Params<Ps...>::p);
  static inline constexpr const CamlLinkedList<cstring>* p = &pp;
};

struct DropFirstArgument{};

struct ApiFunctionDescription {
  cstring return_type;
  const size_t parameter_count;
  const CamlLinkedList<cstring>* parameters;

  template<typename R, typename... Ps>
    constexpr ApiFunctionDescription(R (*fun)(Ps...))
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps))
      , parameters(Params<Ps...>::p)
    {}

  template<typename R, typename P0, typename... Ps>
    constexpr ApiFunctionDescription(DropFirstArgument, R (*fun)(P0, Ps...))
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps))
      , parameters(Params<Ps...>::p)
    {}

  value to_value();
};

static constexpr const uint64_t marker = 0xe1176dafdeadbeefl;

struct ApiRegistryEntry {
  const size_t q1;
  const char * wrapper_name;
  const char * name;
  ApiFunctionDescription description;

  template<typename R, typename... Ps>
  constexpr ApiRegistryEntry
   ( const char*name
   , const char*wrapper_name
   , R (*fun)(Ps...)
   )
    : q1(marker), wrapper_name(wrapper_name), name(name), description(fun)
  { }

  template<typename R, typename P0, typename... Ps>
  constexpr ApiRegistryEntry
   ( DropFirstArgument d
   , const char*name
   , const char*wrapper_name
   , R (*fun)(P0, Ps...)
   )
    : q1(marker), wrapper_name(wrapper_name), name(name), description(d,fun)
  { }

  value to_value();
};

template<typename T>
value list_to_caml(value (*convert)(T), const CamlLinkedList<cstring>* l){
  if(l == nullptr){
    return Val_long(0);
  } else {
    CAMLparam0();
    CAMLlocal1(v_l);
    v_l = caml_alloc(2,0);
    Store_field(v_l, 0, convert(l->data));
    Store_field(v_l, 1, list_to_caml(convert,l->next));
    CAMLreturn(v_l);
  }
}

#define REGISTER_API(APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(#APIF,#WRAPPER,APIF);

#define REGISTER_API_IMPLIED_FIRST(APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(CppCaml::DropFirstArgument(),#APIF,#WRAPPER,APIF);

template<typename T> static inline T& Custom_value(value v){
  return (*((T*)Data_custom_val(v)));
}


template<typename T> void finalize_custom(value v_custom){
  Custom_value<T>(v_custom).~T();
}

template<typename Container> struct ContainerOps {
  static inline struct custom_operations value =
    { typeid(Container).name()
    , &finalize_custom<Container>
    , custom_compare_default
    , custom_hash_default
    , custom_serialize_default
    , custom_deserialize_default
    , custom_compare_ext_default
    , custom_fixed_length_default
    };
};

template<typename T, void (*delete_T)(T*)>
struct ContainerSharedPointer{
  std::shared_ptr<T> pT;
  ContainerSharedPointer(T*p) : pT(p,delete_T) { }
  ContainerSharedPointer(std::shared_ptr<T>&pT) : pT(pT) { }

  auto get() { return this->pT.get(); }
};

template<typename T, typename Context, void (*delete_T)(Context*, T*)>
struct ContainerWithContext{
  std::shared_ptr<Context> pContext;
  T* t;

  ContainerWithContext(std::shared_ptr<Context>& pContext, T* t)
    : pContext(pContext), t(t) { }

  ~ContainerWithContext(){ delete_T(this->pContext.get(), this->t);}

  static inline value allocate(std::shared_ptr<Context>& context, T* t){
    typedef ContainerWithContext<T,Context,delete_T> This;
    value v_container =
      caml_alloc_custom(&ContainerOps<This>::value,sizeof(This),1,500000);
    new(&Custom_value<This>(v_container)) This(context, t);
    return v_container;
  };
};

template<typename T> T T_value(value v){
  static_assert(CppCaml::always_false<T>::value , "You must specialize T_value<> for your type");
}

};
