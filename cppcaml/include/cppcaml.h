
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/threads.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include "caml/domain_state.h"

#include <boost/core/noncopyable.hpp>

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

template<typename ...T> struct type_list {};

struct ApiFunctionDescription {
  cstring return_type;
  const size_t parameter_count;
  const CamlLinkedList<cstring>* parameters;

  template<typename R, typename... Ps>
    constexpr ApiFunctionDescription(type_list<R,Ps...>)
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps))
      , parameters(Params<Ps...>::p)
    {}

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

  template<typename R, typename... Ps>
  constexpr ApiRegistryEntry
   ( const char*name
   , const char*wrapper_name
   , type_list<R, Ps...> tl
   )
    : q1(marker), wrapper_name(wrapper_name), name(name), description(tl)
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

#define REGISTER_API_CUSTOM(APIF, WRAPPER,...) \
  static inline constexpr auto __caml_api_registry_var__##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(#APIF,#WRAPPER,CppCaml::type_list<__VA_ARGS__>());

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

enum class CamlRepresentationKind {
  Immediate
  , Value
  , ContainerSharedPointer
  , ContainerWithContext
};

template<typename T> struct CamlRepresentation{};

#define CAML_REPRESENTATION(cppvalue,repr) \
  template<> struct CppCaml::CamlRepresentation<cppvalue> { \
    static inline constexpr auto kind = CamlRepresentationKind::repr; \
  }

template<typename T> concept CamlRepresentable = requires {
  CamlRepresentation<T>::kind;
};

template<typename T, CamlRepresentationKind kind> concept represented_as =
  CamlRepresentable<T>
  && CamlRepresentation<T>::kind == kind;

template<typename T> concept represented_as_ContainerWithContext =
  represented_as<T,CamlRepresentationKind::ContainerWithContext>;

template<typename T> concept represented_as_ContainerSharedPointer =
  represented_as<T,CamlRepresentationKind::ContainerSharedPointer>;

template<typename T> concept represented_as_Immediate =
  represented_as<T,CamlRepresentationKind::Immediate>;

template<typename T> concept represented_as_Value =
  represented_as<T,CamlRepresentationKind::Value>;

template<typename T> struct ValueWithContextProperties {
  static_assert(CppCaml::always_false<T>::value , "You must specialize ValueWithContextProperties<> for your type");
};

template<typename T> struct T_value_wrapper{
  static_assert(CppCaml::always_false<T>::value , "You must specialize T_value_wrapper<> for your type");
};

template<typename T> T T_value(value v){
  return T_value_wrapper<T>::get(v);
}

template<typename T> struct SharedPointerProperties {
  static_assert(CppCaml::always_false<T>::value , "You must specialize SharedPointerProperties<> for your type");
};

template<typename T> concept SharedPointer = requires (T*t) {
  SharedPointerProperties<T>::delete_T(t);
};

template<typename T> struct ImmediateProperties {
  static_assert(CppCaml::always_false<T>::value , "You must specialize ImmediateProperties<> for your type");
};

template<> struct ImmediateProperties<bool> {
  static inline value to_value(bool b) { return Val_bool(b); }
  static inline bool of_value(value v) { return Bool_val(v); }
};

template<typename T> concept Immediate = requires (T t, value v) {
  ImmediateProperties<T>::to_value(t);
  ImmediateProperties<T>::of_value(v);
};

template<typename T> struct ValueProperties {
  static_assert(CppCaml::always_false<T>::value , "You must specialize ValueProperties<> for your type");
};

template<> struct ValueProperties<const char *> {
  static inline value to_value(const char * s) { return caml_copy_string(s?s:""); }
  static inline const char * of_value(value v) { return String_val(v); }
};

template<typename T> concept Value = requires (T t, value v) {
  ValueProperties<T>::to_value(t);
  ValueProperties<T>::of_value(v);
};

template<typename T>
  requires represented_as<T*,CamlRepresentationKind::ContainerSharedPointer>
struct ContainerSharedPointer : private boost::noncopyable {
  std::shared_ptr<T> pT;
  ContainerSharedPointer(T*p) : pT(p,SharedPointerProperties<T>::delete_T) { }
  ContainerSharedPointer(std::shared_ptr<T>&pT) : pT(pT) { }

  auto get() { return this->pT.get(); }

  static inline value allocate(T *p){
    typedef ContainerSharedPointer<T> This;
    value v_T =
      caml_alloc_custom(&ContainerOps<This>::value,sizeof(This),1,10);
    new(&Custom_value<This>(v_T)) This(p);
    return v_T;
  };
};

template<typename T> concept ValueWithContext = requires (value v) {
  typename ValueWithContextProperties<T>::Context;
  ValueWithContextProperties<T>::delete_T;
};

template<typename T>
  requires (ValueWithContext<T>
      && represented_as<T*,CamlRepresentationKind::ContainerWithContext>
      && not std::is_pointer<T>::value
      )
struct ContainerWithContext : private boost::noncopyable {
  typedef typename ValueWithContextProperties<T>::Context Context;

  std::shared_ptr<Context> pContext;
  T* t;

  ContainerWithContext(std::shared_ptr<Context>& pContext, T* t)
    : pContext(pContext), t(t) { }

  ~ContainerWithContext(){
    ValueWithContextProperties<T>::delete_T(this->pContext.get(), this->t);
  }

  static inline value allocate(std::shared_ptr<Context>& context, T* t){
    typedef ContainerWithContext<T> This;
    value v_container =
      caml_alloc_custom(&ContainerOps<This>::value,sizeof(This),1,500000);
    new(&Custom_value<This>(v_container)) This(context, t);
    return v_container;
  };
};

template<typename T>
requires represented_as<T,CamlRepresentationKind::ContainerWithContext>
struct T_value_wrapper<T> {
  static inline T get(value v) {
    typedef typename std::remove_pointer<T>::type Traw;
    return Custom_value<ContainerWithContext<Traw>>(v).t;
  }
};

template<typename T>
requires represented_as_Immediate<T>
struct T_value_wrapper<T>{
  static inline T get(value v) { return ImmediateProperties<T>::of_value(v); }
};


template<ValueWithContext T>
auto Context_value(value v) {
  auto container = Custom_value<ContainerWithContext<T>>(v);
  return container.pContext.get();
};
 
/////////////////////////////////////////////////////////////////////////////////////////
/// calling of api functions
///

template<typename T> struct normalize_pointer_argument {
  typedef typename std::remove_const<typename std::remove_pointer<T>::type>::type type;
};

// Needed so that we can expand the parameter pack. There must be a better way.....
template<typename T_first, typename T_second> struct first_type { typedef T_first type; };

template<typename R, typename A0, typename... As>
requires 
( represented_as_ContainerWithContext<R *>
  && represented_as_ContainerSharedPointer<typename normalize_pointer_argument<A0>::type *>
  )
inline value
apiN(R* (*mknod)(A0, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<A0raw>>(v_p0);
  auto context = context_s.get();
  // we retrieve all the inner values before allocation,
  // so we don't need to register roots
  auto dep = mknod(context,T_value<As>(v_ps)...);
  typedef CppCaml::ContainerWithContext<R> Container;
  value v_dep = Container::allocate(context_s.pT, dep);
  return v_dep;
}

template<typename R, typename A0, typename... As>
requires 
( represented_as_Immediate<R>
&&  represented_as_ContainerSharedPointer<typename normalize_pointer_argument<A0>::type *>
)
inline value
apiN(R (*mknod)(A0, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<A0raw>>(v_p0);
  auto context = context_s.get();
  auto ret = mknod(context,T_value<As>(v_ps)...);
  return ImmediateProperties<R>::to_value(ret);
}

//TODO: Combine
template<typename R, typename A0, typename... As>
requires 
( represented_as_Value<R>
&&  represented_as_ContainerSharedPointer<typename normalize_pointer_argument<A0>::type *>
)
inline value
apiN(R (*mknod)(A0, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<A0raw>>(v_p0);
  auto context = context_s.get();
  auto ret = mknod(context,T_value<As>(v_ps)...);
  return ValueProperties<R>::to_value(ret);
}

template<typename A0, typename... As>
requires 
  represented_as_ContainerSharedPointer<typename normalize_pointer_argument<A0>::type *>
inline value
apiN(void (*mknod)(A0, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<A0raw>>(v_p0);
  auto context = context_s.get();
  mknod(context,T_value<As>(v_ps)...);
  return Val_unit;
}


template<typename R, typename A0, typename A1, typename... As>
requires 
( represented_as_ContainerWithContext<R *>
  && represented_as_ContainerWithContext<typename normalize_pointer_argument<A1>::type *>
  )
inline value
apiN_implied_context(R* (*mknod)(A0, A1, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A1>::type A1raw;
  auto&p0_s = Custom_value<CppCaml::ContainerWithContext<A1raw>>(v_p0);
  auto p0 = p0_s.t;
  auto context = p0_s.pContext.get();
  // we retrieve all the inner values before allocation,
  // so we don't need to register roots
  auto dep = mknod(context,p0,T_value<As>(v_ps)...);
  typedef CppCaml::ContainerWithContext<R> Container;
  value v_dep = Container::allocate(p0_s.pContext, dep);
  return v_dep;
}

template<typename A0, typename A1, typename... As>
requires 
  CppCaml::represented_as_ContainerWithContext<typename normalize_pointer_argument<A1>::type *>
inline value
apiN_implied_context(void (*mknod)(A0, A1, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A1>::type A1raw;
  auto&p0_s = Custom_value<CppCaml::ContainerWithContext<A1raw>>(v_p0);
  auto p0 = p0_s.t;
  auto context = p0_s.pContext.get();
  mknod(context,p0,T_value<As>(v_ps)...);
  return Val_unit;
}

};

/////////////////////////////////////////////////////////////////////////////////////////
///Default Representation

CAML_REPRESENTATION(bool,Immediate);
CAML_REPRESENTATION(const char*, Value);
