
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
template<typename T> struct ApiTypename<T&> : ApiTypename<T> {};

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

  template<class C, typename R, typename... Ps>
    constexpr ApiFunctionDescription(R (C::*fun)(Ps...) const)
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps) + 1)
      , parameters(Params<C*, Ps...>::p)
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

  template<class C, typename R, typename... Ps>
  constexpr ApiRegistryEntry
   ( const char*name
   , const char*wrapper_name
   , R (C::*fun)(Ps...) const
   )
    : q1(marker), wrapper_name(wrapper_name), name(name), description(fun)
  { }


  value to_value();
};

template<typename T>
value list_to_caml(value (*convert)(T), const CamlLinkedList<cstring>* l){
  if(l == nullptr){
    return Val_long(0);
  } else {
    CAMLparam0();
    CAMLlocal3(v_l,v_ret,v_tmp);
    v_l = caml_alloc(2,0);
    v_ret = v_l;
    Store_field(v_l, 0, convert(l->data));
    Field(v_l,1) = Val_unit;
    while(l->next){
      l = l->next;
      v_tmp = caml_alloc(2,0);
      Store_field(v_l, 1, v_tmp);
      v_l = v_tmp;
      Store_field(v_l,0, convert(l->data));
      Field(v_l,1) = Val_unit;
    };
    CAMLreturn(v_ret);
  }
}

#define REGISTER_API(APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(#APIF,#WRAPPER,APIF);

#define REGISTER_API_MEMBER(CLASS, APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__##CLASS ## _##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(#APIF,#WRAPPER,&CLASS :: APIF);

#define REGISTER_API_CUSTOM(APIF, WRAPPER,...) \
  static inline constexpr auto __caml_api_registry_var__##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(#APIF,#WRAPPER,CppCaml::type_list<__VA_ARGS__>());

#define REGISTER_API_CONSTRUCTOR(CLASS,WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__make_##CLASS \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry("make_" #CLASS,#WRAPPER,CppCaml::type_list<CLASS*>());

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
  , CustomWithContext
  , ContainerSharedPointer
  , ContainerWithContext
  , InlinedWithContext
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

template<typename T> concept represented_as_InlinedWithContext =
  represented_as<T,CamlRepresentationKind::InlinedWithContext>;

template<typename T> concept represented_as_ContainerSharedPointer =
  represented_as<T,CamlRepresentationKind::ContainerSharedPointer>;

template<typename T> concept represented_as_Immediate =
  represented_as<T,CamlRepresentationKind::Immediate>;

template<typename T> concept represented_as_Value =
  represented_as<T,CamlRepresentationKind::Value>;

template<typename T> concept represented_as_CustomWithContext =
  represented_as<T,CamlRepresentationKind::CustomWithContext>;

template<typename T> struct CustomWithContextProperties {
  static_assert(CppCaml::always_false<T>::value ,
      "You must specialize CustomWithContextProperties<> for your type");
};

template<typename T> concept CustomWithContext =
requires (T t, value v, std::shared_ptr<typename CustomWithContextProperties<T>::Context>& ctx) {
  typename CustomWithContextProperties<T>::Context;
  CustomWithContextProperties<T>::to_value(ctx, t);
  CustomWithContextProperties<T>::of_value(v);
};

template<typename T> struct ValueWithContextProperties {
  static_assert(CppCaml::always_false<T>::value ,
      "You must specialize ValueWithContextProperties<> for your type");
};

template<typename T> struct T_value_wrapper{
  static_assert(CppCaml::always_false<T>::value,
      "You must specialize T_value_wrapper<> for your type");
};

template<typename T> std::remove_reference<T>::type T_value(value v){
  return T_value_wrapper<T>::get(v);
}

template<typename T> struct SharedPointerProperties {
  static_assert(CppCaml::always_false<T>::value,
      "You must specialize SharedPointerProperties<> for your type");
};

template<typename T> concept SharedPointer = requires (T*t) {
  SharedPointerProperties<T>::delete_T(t);
};

template<typename T> struct ImmediateProperties {
  static_assert(CppCaml::always_false<T>::value,
      "You must specialize ImmediateProperties<> for your type");
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
  static_assert(CppCaml::always_false<T>::value ,
      "You must specialize ValueProperties<> for your type");
};

template<> struct ValueProperties<const char *> {
  static inline value to_value(const char * s) { return caml_copy_string(s?s:""); }
  static inline const char * of_value(value v) { return String_val(v); }
};

template<> struct ValueProperties<const std::string> {
  static inline value to_value(const std::string s) { return caml_copy_string(s.c_str()); }
  static inline const std::string of_value(value v) { return std::string(String_val(v)); }
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

  static inline value allocate(std::shared_ptr<T>&pT){
    typedef ContainerSharedPointer<T> This;
    value v_T =
      caml_alloc_custom(&ContainerOps<This>::value,sizeof(This),1,10);
    new(&Custom_value<This>(v_T)) This(pT);
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
  requires (ValueWithContext<T>
      && represented_as<T,CamlRepresentationKind::InlinedWithContext>
      )
struct InlinedWithContext : private boost::noncopyable {
  typedef typename ValueWithContextProperties<T>::Context Context;

  std::shared_ptr<Context> pContext;
  T t;

  InlinedWithContext(std::shared_ptr<Context>& pContext, T&&t)
    : pContext(pContext), t(t) { }

  static inline value allocate(std::shared_ptr<Context>& context, T&& t){
    typedef InlinedWithContext<T> This;
    value v_container =
      caml_alloc_custom(&ContainerOps<This>::value,sizeof(This),1,500000);
    new(&Custom_value<This>(v_container)) This(context, std::move(t));
    return v_container;
  };
};

template<typename T>
requires represented_as_ContainerWithContext<T>
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

template<typename T>
requires represented_as_Value<T>
struct T_value_wrapper<T&>{
  static inline T get(value v) { return ValueProperties<T>::of_value(v); }
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
apiN(R* (*fun)(A0, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<A0raw>>(v_p0);
  auto context = context_s.get();
  // we retrieve all the inner values before allocation,
  // so we don't need to register roots
  auto dep = fun(context,T_value<As>(v_ps)...);
  typedef CppCaml::ContainerWithContext<R> Container;
  value v_dep = Container::allocate(context_s.pT, dep);
  return v_dep;
}

template<typename R, typename A0, typename... As>
requires
( (represented_as_Immediate<R> || represented_as_Value<R>)
&&  represented_as_ContainerSharedPointer<typename normalize_pointer_argument<A0>::type *>
)
inline value
apiN(R (*fun)(A0, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<A0raw>>(v_p0);
  auto context = context_s.get();
  auto ret = fun(context,T_value<As>(v_ps)...);
  if constexpr (represented_as_Immediate<R>)
    return ImmediateProperties<R>::to_value(ret);
  else if constexpr (represented_as_Value<R>)
    return ValueProperties<R>::to_value(ret);
}

template<typename R, typename A0, typename... As>
requires
( represented_as_CustomWithContext<R>
&& CustomWithContext<R>
&&  represented_as_ContainerSharedPointer<typename normalize_pointer_argument<A0>::type *>
)
inline value
apiN(R (*fun)(A0, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<A0raw>>(v_p0);
  auto context = context_s.get();
  auto ret = fun(context,T_value<As>(v_ps)...);
  return CustomWithContextProperties<R>::to_value(context_s.pT, ret);
}

template<typename A0, typename... As>
requires
  represented_as_ContainerSharedPointer<typename normalize_pointer_argument<A0>::type *>
inline value
apiN(void (*fun)(A0, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<A0raw>>(v_p0);
  auto context = context_s.get();
  fun(context,T_value<As>(v_ps)...);
  return Val_unit;
}


template<typename R, typename A0, typename A1, typename... As>
requires
( represented_as_ContainerWithContext<R *>
  && represented_as_ContainerWithContext<typename normalize_pointer_argument<A1>::type *>
  )
inline value
apiN_implied_context(R* (*fun)(A0, A1, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A1>::type A1raw;
  auto&p0_s = Custom_value<CppCaml::ContainerWithContext<A1raw>>(v_p0);
  auto p0 = p0_s.t;
  auto context = p0_s.pContext.get();
  // we retrieve all the inner values before allocation,
  // so we don't need to register roots
  auto dep = fun(context,p0,T_value<As>(v_ps)...);
  typedef CppCaml::ContainerWithContext<R> Container;
  value v_dep = Container::allocate(p0_s.pContext, dep);
  return v_dep;
}

template<typename R, typename A0, typename A1, typename... As>
requires
( (represented_as_Immediate<R> || represented_as_Value<R>)
  && represented_as_ContainerWithContext<typename normalize_pointer_argument<A1>::type *>
  )
inline value
apiN_implied_context_free(R (*fun)(A0, A1, As...), void (*free_fun)(A0,R), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A1>::type A1raw;
  auto&p0_s = Custom_value<CppCaml::ContainerWithContext<A1raw>>(v_p0);
  auto p0 = p0_s.t;
  auto context = p0_s.pContext.get();
  auto ret = fun(context,p0,T_value<As>(v_ps)...);
  value v_ret;
  if constexpr (represented_as_Immediate<R>)
    v_ret = ImmediateProperties<R>::to_value(ret);
  else if constexpr (represented_as_Value<R>)
    v_ret = ValueProperties<R>::to_value(ret);
  free_fun(context, ret);
  return v_ret;
}

template<typename A0, typename A1>
static void dummy_free_fun(A0, A1){}

template<typename R, typename A0, typename A1, typename... As>
inline value
apiN_implied_context(R (*fun)(A0, A1, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  return apiN_implied_context_free(fun, dummy_free_fun, v_p0, v_ps...);
}

template<typename A0, typename A1, typename... As>
requires
  CppCaml::represented_as_ContainerWithContext<typename normalize_pointer_argument<A1>::type *>
inline value
apiN_implied_context(void (*fun)(A0, A1, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A1>::type A1raw;
  auto&p0_s = Custom_value<CppCaml::ContainerWithContext<A1raw>>(v_p0);
  auto p0 = p0_s.t;
  auto context = p0_s.pContext.get();
  fun(context,p0,T_value<As>(v_ps)...);
  return Val_unit;
}

template<typename C, typename... As>
requires
(
 represented_as_ContainerSharedPointer<C *>
)
inline value
apiN_class(void (C::*fun)(As...) const, value v_c, typename first_type<value,As>::type... v_ps){
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<C>>(v_c);
  auto context = context_s.get();

  (context->*fun)(T_value<As>(v_ps)...);
  return Val_unit;
}

template<typename C, typename R, typename... As>
requires
(
 represented_as_ContainerSharedPointer<C *>
 && (represented_as_Immediate<R> || represented_as_Value<R>)
)
inline value
apiN_class(R (C::*fun)(As...) const, value v_c, typename first_type<value,As>::type... v_ps){
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<C>>(v_c);
  auto context = context_s.get();

  auto ret = (context->*fun)(T_value<As>(v_ps)...);
  if constexpr (represented_as_Immediate<R>)
    return ImmediateProperties<R>::to_value(ret);
  else if constexpr (represented_as_Value<R>)
    return ValueProperties<R>::to_value(ret);
}

template<typename C, typename R, typename... As>
requires
(
 represented_as_ContainerSharedPointer<C *>
 && represented_as_InlinedWithContext<R>
)
inline value
apiN_class(R (C::*fun)(As...) const, value v_c, typename first_type<value,As>::type... v_ps){
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<C>>(v_c);
  auto context = context_s.get();

  auto dep = (context->*fun)(T_value<As>(v_ps)...);

  typedef CppCaml::InlinedWithContext<R> Container;
  value v_dep = Container::allocate(context_s.pT, std::move(dep));
  return v_dep;
}

};

/////////////////////////////////////////////////////////////////////////////////////////
///Default Representation

CAML_REPRESENTATION(bool,Immediate);
CAML_REPRESENTATION(const char*, Value);
CAML_REPRESENTATION(const std::string, Value);
