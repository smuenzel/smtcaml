
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/threads.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include "caml/domain_state.h"

#include <boost/core/noncopyable.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <memory>
#include <typeinfo>
#include <functional>
#include <cassert>
#include <cstring>
#include <optional>

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
template<typename T> struct ApiTypename<const T&> : ApiTypename<T> {};

/* https://stackoverflow.com/a/75619411
 */ 
template<size_t ...Len>
constexpr auto cat(const std::array<char,Len>&...strings){
  constexpr size_t N = (... + Len) - sizeof...(Len);
  std::array<char, N + 1> result = {};
  result[N] = '\0';

  auto it = result.begin();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
  (void)((it = std::copy_n(strings.cbegin(), Len-1, it), 0), ...);
#pragma GCC diagnostic pop
  return result;
}

using std::to_array;

template<typename T, size_t L>
constexpr const std::array<T,L>& to_array(const std::array<T,L>& a) { return a; }

template<typename ...Ts>
constexpr auto cat(const Ts&...strings){
  return cat(to_array(strings)...);
}

template<typename T, size_t L>
constexpr bool array_contains(const std::array<T,L>&a, const T& v){
  auto begin = a.cbegin();
  auto end = a.cend();
  return (end != std::find(begin, end, v));
}

template<typename T> struct ApiTypename<std::vector<T> > {
  static constexpr const auto name_array = cat("(", ApiTypename<T>::name_array," array)");
  static constexpr auto name_len = name_array.size();
  static constexpr const char * name = &name_array[0];
};

template<typename T0, typename T1> struct ApiTypename<std::pair<T0,T1> > {
  static constexpr const auto name_array = cat("(", ApiTypename<T0>::name_array, "*", ApiTypename<T1>::name_array, ")");
  static constexpr auto name_len = name_array.size();
  static constexpr const char * name = &name_array[0];
};

template<typename T> struct ApiTypename<std::optional<T> > {
  static constexpr const auto name_array = cat("(", ApiTypename<T>::name_array," option)");
  static constexpr auto name_len = name_array.size();
  static constexpr const char * name = &name_array[0];
};

#define DECL_API_TYPE(c_type, caml_type) \
  template<> \
  struct CppCaml::ApiTypename<c_type>{ \
    static constexpr auto name_len = std::char_traits<char>::length(#caml_type);\
    static constexpr const std::array<char,name_len+1> name_array = std::to_array(#caml_type); \
    static constexpr const char * name = &name_array[0]; \
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

template<class C, typename R, typename... Ps>
constexpr auto resolveOverload(type_list<R,Ps...>, R (C::*fun)(Ps...)){
  return fun;
}

template<class C, typename R, typename... Ps>
constexpr auto resolveOverload(type_list<R,Ps...>, R (C::*fun)(Ps...) const){
  return fun;
}

struct ApiFunctionDescription {
  cstring return_type;
  const size_t parameter_count;
  const CamlLinkedList<cstring>* parameters;
  cstring class_name;

  template<typename R, typename... Ps>
    constexpr ApiFunctionDescription(type_list<R,Ps...>)
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps))
      , parameters(Params<Ps...>::p), class_name()
    {}

  template<typename R, typename... Ps>
    constexpr ApiFunctionDescription(R (*fun)(Ps...))
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps))
      , parameters(Params<Ps...>::p), class_name()
    {}

  template<typename R, typename P0, typename... Ps>
    constexpr ApiFunctionDescription(DropFirstArgument, R (*fun)(P0, Ps...))
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps))
      , parameters(Params<Ps...>::p), class_name()
    {}

  template<class C, typename R, typename... Ps>
    constexpr ApiFunctionDescription(cstring c, R (C::*fun)(Ps...) const)
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps) + 1)
      , parameters(Params<C*, Ps...>::p), class_name(c)
    {}

  template<class C, typename R, typename... Ps>
    constexpr ApiFunctionDescription(DropFirstArgument, cstring c, R (C::*fun)(Ps...) const)
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps) + 1)
      , parameters(Params<Ps...>::p), class_name(c)
    {}

  template<class C, typename R, typename... Ps>
    constexpr ApiFunctionDescription(cstring c, R (C::*fun)(Ps...))
    : return_type(ApiTypename<R>::name), parameter_count(sizeof...(Ps) + 1)
      , parameters(Params<C*, Ps...>::p), class_name(c)
    {}

  value to_value();
};

static constexpr const uint64_t marker = 0xe1176dafdeadbeefl;

struct ApiEnumEntry {
  const size_t q1;
  const char * enumName;
  const char * memberName;
  const value memberValue;

  constexpr ApiEnumEntry()
    : q1(0), enumName(""), memberName(""), memberValue(0)
  { }

  constexpr ApiEnumEntry
    ( const char*enumName
    , const char*memberName
    , const value memberValue
    )
    : q1(marker), enumName(enumName), memberName(memberName), memberValue(memberValue)
    { }

  value to_value();
};

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
   , const char*class_name
   , R (C::*fun)(Ps...) const
   )
    : q1(marker), wrapper_name(wrapper_name), name(name), description(class_name, fun)
  { }

  template<class C, typename R, typename... Ps>
  constexpr ApiRegistryEntry
   ( const char*name
   , const char*wrapper_name
   , const char*class_name
   , R (C::*fun)(Ps...)
   )
    : q1(marker), wrapper_name(wrapper_name), name(name), description(class_name, fun)
  { }

  template<class C, typename R, typename... Ps>
  constexpr ApiRegistryEntry
   ( DropFirstArgument d
   , const char*name
   , const char*wrapper_name
   , const char*class_name
   , R (C::*fun)(Ps...) const
   )
    : q1(marker), wrapper_name(wrapper_name), name(name), description(d, class_name, fun)
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
    CppCaml::ApiRegistryEntry(#CLASS "__" #APIF,#WRAPPER,#CLASS, &CLASS :: APIF);

#define REGISTER_API_MEMBER_IMPLIED(CLASS, APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__##CLASS ## _##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(CppCaml::DropFirstArgument(), #CLASS "__" #APIF,#WRAPPER,#CLASS, &CLASS :: APIF);

#define REGISTER_API_MEMBER_OVERLOAD(CLASS, APIF, SUFFIX, WRAPPER, ...) \
  static inline constexpr auto __caml_api_registry_var__##CLASS ## _##APIF ## _ ## SUFFIX \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(#CLASS "__" #APIF "__" #SUFFIX,#WRAPPER,#CLASS, CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(), &CLASS :: APIF));

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

/////////////////////////////////////
// Function Autogen
/////////////////////////////////////

#define API_GET_CONTEXT(APIPREFIX, TYPE, CTYPE) \
  REGISTER_API_CUSTOM(TYPE ##__get_context, caml_ ##APIPREFIX ##__##TYPE ## __get_context, typename CppCaml::InlinedWithContext<CTYPE>::Context*,CTYPE); \
  apireturn caml_ ##APIPREFIX ##__##TYPE ## __get_context(value v){ \
    return CppCaml::get_context_caml<CTYPE>(v); \
  }

#define APIM0_(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER(CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_c){ \
    return CppCaml::apiN_class(&CLASS :: APIF, v_c); \
  }

#define APIM1_(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER(CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_c, value v_p0){ \
    return CppCaml::apiN_class(&CLASS :: APIF, v_c, v_p0); \
  }

#define APIM2_(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER(CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_c, value v_p0, value v_p1){ \
    return CppCaml::apiN_class(&CLASS :: APIF, v_c, v_p0, v_p1); \
  }

#define APIM1_IMPLIED_(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER_IMPLIED(CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_p0){ \
    return CppCaml::apiN_class_implied(&CLASS :: APIF, v_p0); \
  }

#define APIM2_IMPLIED_(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER_IMPLIED(CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_p0, value v_p1){ \
    return CppCaml::apiN_class_implied(&CLASS :: APIF, v_p0, v_p1); \
  }

#define APIM1_OVERLOAD_(APIPREFIX, CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(CLASS,APIF,SUFFIX, caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_c, value v_p0){ \
    return CppCaml::apiN_class(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_c, v_p0); \
  }

#define APIM2_OVERLOAD_(APIPREFIX, CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(CLASS,APIF,SUFFIX, caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_c, value v_p0, value v_p1){ \
    return CppCaml::apiN_class(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_c, v_p0, v_p1); \
  }

#define APIM2_OVERLOAD_IMPLIED_(APIPREFIX, CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(CLASS,APIF,SUFFIX, caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_p0, value v_p1){ \
    return CppCaml::apiN_class_implied(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_p0, v_p1); \
  }

#define APIM3_OVERLOAD_(APIPREFIX, CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(CLASS,APIF,SUFFIX, caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_c, value v_p0, value v_p1, value v_p2){ \
    return CppCaml::apiN_class(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_c, v_p0, v_p1, v_p2); \
  }
/////////////////////////////////////

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

template<typename T> struct CamlRepresentation<std::vector<T>> {
  static inline constexpr auto kind = CamlRepresentationKind::Value;
};

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

template<> struct ValueProperties<std::string> {
  static inline value to_value(std::string s) { return caml_copy_string(s.c_str()); }
  static inline std::string of_value(value v) { return std::string(String_val(v)); }
};

template<typename T> struct ValueProperties<std::vector<T>> {

  static inline value to_value(const std::vector<T>&vec) {
    return Val_none;
    /*
    CAMLparam0();
    CAMLlocal1(v_ret);
    v_ret = caml_alloc(vec.size(), 0);
    for(size_t i = 0; i < vec.size(); i++){
      Store_field(v_ret,i,ValueProperties<T>::to_value(vec[i]));
    };
    CAMLreturn(v_ret);
    */
  }

  static inline std::vector<T> of_value(value v) { 
    auto len = Wosize_val(v);
    auto f = [](value v){
      return T_value_wrapper<T>::get(v);
    };
    auto begin = boost::make_transform_iterator(&Field(v, 0), f);
    auto end = boost::make_transform_iterator(&Field(v, len), f);

    return std::vector<T>(begin, end);
  }
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
/*
  requires (ValueWithContext<T>
      && represented_as<T*,CamlRepresentationKind::ContainerWithContext>
      && not std::is_pointer<T>::value
      )
      */
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
struct T_value_wrapper<const std::vector<T>&> {
  static inline std::vector<T> get(value v) {
    auto len = Wosize_val(v);
    auto f = [](value v){
      return T_value_wrapper<T>::get(v);
    };
    auto begin = boost::make_transform_iterator(&Field(v, 0), f);
    auto end = boost::make_transform_iterator(&Field(v, len), f);

    return std::vector<T>(begin, end);
  }
};

template<typename T0, typename T1>
struct T_value_wrapper<std::pair<T0,T1>> {
  static inline std::pair<T0,T1> get(value v) {
    return std::make_pair(T_value_wrapper<T0>::get(Field(v,0)), T_value_wrapper<T1>::get(Field(v,1)));
  }
};

template<typename T>
struct T_value_wrapper<const std::optional<T>&> {
  static inline std::optional<T> get(value v) {
    if(Is_none(v)) return std :: nullopt;
    else return T_value_wrapper<T>::get(Some_val(v));
  }
};

template<>
struct T_value_wrapper<std::string> {
  static inline std::string get(value v) {
    return std::string(String_val(v));
  }
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
requires represented_as_InlinedWithContext<T>
struct T_value_wrapper<T> {
  static inline T get(value v) {
    return Custom_value<InlinedWithContext<T>>(v).t;
  }
};

template<typename T>
requires represented_as_InlinedWithContext<T>
struct T_value_wrapper<const T&> {
  static inline const T&get(value v) {
    return Custom_value<InlinedWithContext<T>>(v).t;
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

template<typename T>
requires represented_as_InlinedWithContext<T>
value get_context_caml(value v){
  typedef CppCaml::InlinedWithContext<T> Inlined;
  auto&context_s = Custom_value<CppCaml::InlinedWithContext<T>>(v);
  return ContainerSharedPointer<typename Inlined::Context>::allocate(context_s.pContext);
}

/////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
concept Returnable = requires {
  [](T b){return b;};
};

// TODO
template<typename T> class CamlConversion {
  static_assert(CppCaml::always_false<T>::value,
      "You must specialize CamlConversion<> for your type");
};

template<typename T> class CamlConversionProperties {
  /*
  static_assert(CppCaml::always_false<T>::value,
      "You must specialize CamlConversionProperties<> for your type");
      */
};

enum class CamlAllocates {
  No_allocation, Allocation
};

template<typename T, CamlRepresentationKind kind>
concept PropertyRepresented = requires {
  requires(CamlConversionProperties<T>::representation_kind == kind);
};

template<typename T, typename C>
concept PropertyContext =
  std::same_as<typename CamlConversionProperties<T>::Context, C>;

template<typename T>
concept PropertyHasContext = requires {
  typename CamlConversionProperties<T>::Context;
};

template<typename T>
concept PropertyHasDeleterWithContext = requires {
  requires PropertyHasContext<T>;
  requires requires(CamlConversionProperties<T>::Context*c, T&t) {
    CamlConversionProperties<T>::delete_T(c,t);
  };
};

template<typename T>
concept PropertyHasDeleter = requires {
  requires requires(T&t) {
    CamlConversionProperties<T>::delete_T(t);
  };
};

template<typename T>
requires PropertyHasDeleter<T>
struct SimpleDeleter{
  void operator()(T t){
    CamlConversionProperties<T>::delete_T(t);
  };
};

template<typename T, typename C>
struct ContextContainer : private boost::noncopyable {
  typedef C Context;

  std::shared_ptr<Context> pContext;
  T t;

  ContextContainer(std::shared_ptr<Context>& pContext, T&&t)
    : pContext(pContext), t(t) { }

  ~ContextContainer(){
    if constexpr(PropertyHasDeleterWithContext<T>) {
      CamlConversionProperties<T>::delete_T(pContext.get(),t);
    } else if constexpr(PropertyHasDeleter<T>) {
      CamlConversionProperties<T>::delete_T(t);
    }
  }

  static inline value allocate(std::shared_ptr<Context>& context, T& t){
    typedef ContextContainer<T,C> This;
    value v_container =
      caml_alloc_custom(&ContainerOps<This>::value,sizeof(This),1,500000);
    new(&Custom_value<This>(v_container)) This(context, std::move(t));
    return v_container;
  };
};

template<typename T>
struct SharedPointerContainer : private boost::noncopyable {
  std::shared_ptr<T> t;

  SharedPointerContainer(std::shared_ptr<T>&& t) : t(std::move(t)) {}

  static inline value allocate(std::shared_ptr<T>&& t){
    typedef SharedPointerContainer<T> This;
    value v_container =
      caml_alloc_custom(&ContainerOps<This>::value,sizeof(This),1,500000);
    new(&Custom_value<This>(v_container)) This(std::move(t));
    return v_container;
  };
};


template<typename A,typename B> concept same_as_or_reference = requires {
  requires std::same_as<A,B> || std::same_as<A,B&>;
};

// TODO
template<typename T>
concept CamlConvertible = requires {
  typename CamlConversion<T>::RepresentationType;
  requires std::same_as<decltype(CamlConversion<T>::allocates),const CamlAllocates>;
};

// TODO
template<typename T>
concept CamlOfValue = requires {
  requires CamlConvertible<T>;
  requires requires (CamlConversion<T>::RepresentationType& r){
    { CamlConversion<T>::get_underlying(r) } -> same_as_or_reference<T>;
  };
  requires requires(value v) {
    { CamlConversion<T>::of_value(v) } -> same_as_or_reference<typename CamlConversion<T>::RepresentationType>;
  };
};

// TODO
template<typename T>
concept CamlToValue = requires {
  requires CamlConvertible<T>;
  CamlConversion<T>::to_value;
};

template<typename T>
concept CamlToValueNoContext = requires {
  requires CamlToValue<T>;
  requires requires(T t){CamlConversion<T>::to_value(t);};
};

template<typename T>
concept CamlBidirectional = requires {
  requires CamlOfValue<T>;
  requires CamlToValue<T>;
};

// TODO
template<typename T>
concept CamlHasContext = requires (T a, CamlConversion<T>::RepresentationType& ra) {
  requires CamlConvertible<T>;
  typename CamlConversion<T>::Context;
  //requires std::same_as<typename CamlConversion<T>::Context,C>;
  { CamlConversion<T>::get_context(ra) } -> std::same_as<std::shared_ptr<typename CamlConversion<T>::Context>&>;
};

template<typename T>
requires 
(PropertyRepresented<T,CamlRepresentationKind::ContainerWithContext>
&& PropertyHasContext<T>)
struct CamlConversion<T> {
  typedef typename CamlConversionProperties<T>::Context Context;
  typedef ContextContainer<T, Context> RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  static inline std::shared_ptr<Context>& get_context(RepresentationType& r){
    return r.pContext;
  }

  static inline T& get_underlying(RepresentationType& r){
    return r.t;
  }

  static inline value to_value(std::shared_ptr<Context>&ctx, T&t){
    return RepresentationType::allocate(ctx, t);
  }

  static inline RepresentationType& of_value(value v){
    return Custom_value<RepresentationType>(v);
  }
};

template<typename T> concept pointer = std::is_pointer_v<T>;

template<typename T>
requires
(PropertyRepresented<T,CamlRepresentationKind::ContainerSharedPointer>
 && std::is_pointer_v<T>)
struct CamlConversion<T> {
  typedef typename std::remove_pointer<T>::type Tnopointer;
  typedef std::shared_ptr<Tnopointer> RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  static inline T get_underlying(RepresentationType&r) { return r.get(); }
  static inline RepresentationType& of_value(value v) {
    return Custom_value<SharedPointerContainer<Tnopointer>>(v).t;
  }
  /* No [to_value] function, since the shared_ptr construction should be
   * done manually to make sure it's right
   */
};

struct Void {};

template<> struct CamlConversion<Void> {
  typedef struct Void RepresentationType;
  static const auto allocates = CamlAllocates::No_allocation;
  static inline value to_value(Void) { return Val_unit; }
};

template<> struct CamlConversion<int> {
  typedef int RepresentationType;
  static const auto allocates = CamlAllocates::No_allocation;

  static inline value to_value(int x) { return Val_long(x); }
  static inline RepresentationType of_value(value v) { return Long_val(v); }
  static inline int&get_underlying(RepresentationType&r) { return r; }
};
static_assert(CamlBidirectional<int>);

template<> struct CamlConversion<const char *> {
  typedef const char * RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  static inline value to_value(const char *x) { return caml_copy_string(x); }
  static inline RepresentationType of_value(value v) { return String_val(v); }
  static inline auto&get_underlying(RepresentationType&r) { return r; }
};
static_assert(CamlBidirectional<const char *>);

template<> struct CamlConversion<std::string> {
  typedef std::string RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  static inline value to_value(const std::string&x) { return caml_copy_string(x.c_str()); }
  static inline RepresentationType of_value(value v) { return String_val(v); }
  static inline auto&get_underlying(RepresentationType&r) { return r; }
};
static_assert(CamlBidirectional<std::string>);

template<> struct CamlConversionProperties<std::string>{
  static constexpr bool allow_const = true;
};

template<typename T>
concept PropertyAllowConst = requires {
  requires(CamlConversionProperties<T>::allow_const == true);
};

template<typename T>
concept PropertyAllowConstRef = requires {
  requires(CamlConversionProperties<T>::allow_const_ref == true);
};

template<typename T> 
requires PropertyAllowConst<T> && CamlBidirectional<T>
struct CamlConversion<const T> {
  typedef CamlConversion<T> N;
  typedef const N::RepresentationType RepresentationType;
  static const auto allocates = N::allocates;

  static inline value to_value(const T&x) { return N::to_value(x); }
  static inline RepresentationType of_value(value v) { return N::of_value(v); }
  static inline auto&get_underlying(RepresentationType&r) { return r; }
};

/*
template<typename T> 
requires PropertyAllowConstRef<T> && CamlToValue<T> && CamlHasContext<T>
struct CamlConversion<const T&> {
  typedef typename CamlConversionProperties<T>::Context Context;
  typedef CamlConversion<T> N;
  typedef const N::RepresentationType& RepresentationType;
  static const auto allocates = N::allocates;

  static inline RepresentationType of_value(value v) { return N::of_value(v); }
  static inline const T& get_underlying(RepresentationType&r) {
    return CamlConversion<T>::get_underlying(r);
  }

  static inline std::shared_ptr<Context>& get_context(RepresentationType& r){
    return CamlConversion<T>::get_context(r);
  }
};
*/

static_assert(CamlBidirectional<const std::string>);

// Needed so that we can expand the parameter pack. There must be a better way.....
template<typename T_first, typename T_second> struct first_type { typedef T_first type; };

template<typename C, typename T>
requires CamlHasContext<T>
std::shared_ptr<C>& extract_context(typename CamlConversion<T>::RepresentationType&t){
  return CamlConversion<T>::get_context(t);
}

template<typename C, typename T>
requires std::same_as<std::shared_ptr<C>,T>
std::shared_ptr<C>& extract_context(typename CamlConversion<T>::RepresentationType&t){
  return CamlConversion<T>::get_underlying(t);
}

template<typename C, typename T>
requires std::same_as<std::shared_ptr<C>,typename CamlConversion<T>::RepresentationType>
std::shared_ptr<C>& extract_context(typename CamlConversion<T>::RepresentationType&t){
  return t;
}

template<typename Construct, typename FromThis>
concept CamlConstructible = requires {
  requires requires
    (CamlConversion<FromThis>::RepresentationType from){
      extract_context<Construct,FromThis>(from);
    };
};

template<typename Construct, typename FromThis>
concept CamlContextConstructible = requires {
  requires CamlToValue<Construct>;
  requires
    ( CamlHasContext<Construct> 
      && CamlConstructible<typename CamlConversion<Construct>::Context,FromThis>
    ) 
    || CamlToValueNoContext<Construct>;
};

// https://stackoverflow.com/a/51454205
template<typename F, typename... Ps, typename Result = std::invoke_result_t<F,Ps...>>
requires std::same_as<Result,void>
inline Void invoke_void(F&& f, Ps&&... ps){
  std::invoke(std::forward<F>(f), std::forward<Ps>(ps)...);
  return {};
};

template<typename F, typename... Ps, typename Result = std::invoke_result_t<F,Ps...>>
requires (!std::same_as<Result,void>)
inline Result invoke_void(F&& f, Ps&&... ps){
  return std::invoke(std::forward<F>(f), std::forward<Ps>(ps)...);
};

template<typename V>
struct ReplaceVoid{
  typedef V type;
};

template<>
struct ReplaceVoid<void>{
  typedef Void type;
};


template<typename T> using NormalizeArgument = std::remove_const_t<std::remove_reference_t<T>>;

template<typename T> using ConversionNormalized = CamlConversion<NormalizeArgument<T>>;

template<typename R, typename Rv = ReplaceVoid<R>::type, typename A0c, typename... Asc, typename A0 = NormalizeArgument<A0c>>
requires
( CamlToValue<Rv> && CamlOfValue<A0> && (CamlOfValue<NormalizeArgument<Asc>> && ...)
  && CamlContextConstructible<Rv,A0>
)
inline value
call_api(R (*fun)(A0c, Asc...), value v0, typename first_type<value,Asc>::type... v_ps){
  if constexpr(CamlHasContext<R>) {
    typedef typename CamlConversion<Rv>::Context Context;
    auto r0 = CamlConversion<A0>::of_value(v0);
    auto context = extract_context<Context,A0>(r0);
    auto ret =
      (*fun)
        ( CamlConversion<A0>::get_underlying(r0)
        , ConversionNormalized<Asc>::get_underlying(ConversionNormalized<Asc>::of_value(v_ps))...
        );
    auto v_ret = CamlConversion<Rv>::to_value(context, ret);
    return v_ret;
  } else {
    auto ret =
      invoke_void
        ( fun
        , CamlConversion<A0>::get_underlying(CamlConversion<A0>::of_value(v0))
        , ConversionNormalized<Asc>::get_underlying(ConversionNormalized<Asc>::of_value(v_ps))...
        );
    auto v_ret = CamlConversion<Rv>::to_value(ret);
    return v_ret;
  }
}

template<typename R, typename Rv = ReplaceVoid<R>::type, typename A0c, typename A1c, typename... As, typename A0 = NormalizeArgument<A0c>, typename A1 = NormalizeArgument<A1c>>
requires
( CamlToValue<Rv> && CamlOfValue<A1> && (CamlOfValue<NormalizeArgument<As>> && ...)
  && CamlContextConstructible<Rv,A1>
  && CamlConstructible<A0,A1>
)
inline value
call_api_implied_first(R (*fun)(A0c, A1c, As...), value v1, typename first_type<value,As>::type... v_ps){
  auto& r1 = CamlConversion<A1>::of_value(v1);
  auto context = extract_context<typename std::remove_pointer<A0>::type,A1>(r1);
  auto ret =
    invoke_void
      ( fun
      , context.get()
      , CamlConversion<A1>::get_underlying(r1)
      , ConversionNormalized<As>::get_underlying(ConversionNormalized<As>::of_value(v_ps))...
      );
  if constexpr(CamlHasContext<Rv>) {
    auto v_ret = CamlConversion<Rv>::to_value(context, ret);
    return v_ret;
  } else {
    auto v_ret = CamlConversion<Rv>::to_value(ret);
    return v_ret;
  }
}

template<typename R, typename Rv = ReplaceVoid<R>::type, typename C, typename... As>
requires
( CamlToValue<Rv> && CamlOfValue<C*> && (CamlOfValue<NormalizeArgument<As>> && ...)
  && CamlContextConstructible<Rv,C*>
)
inline value
call_api_class(R (C::*fun)(As...), value c, typename first_type<value,As>::type... v_ps){
  if constexpr(CamlHasContext<R>) {
    typedef typename CamlConversion<Rv>::Context Context;
    auto r0 = CamlConversion<C*>::of_value(c);
    auto context = extract_context<Context,C*>(r0);
    auto ret =
      invoke_void
        ( fun
        , CamlConversion<C*>::get_underlying(r0)
        , ConversionNormalized<As>::get_underlying(ConversionNormalized<As>::of_value(v_ps))...
        );
    auto v_ret = CamlConversion<Rv>::to_value(context, ret);
    return v_ret;
  } else {
    auto ret =
      invoke_void
        ( fun
        , CamlConversion<C*>::get_underlying(CamlConversion<C>::of_value(c))
        , ConversionNormalized<As>::get_underlying(ConversionNormalized<As>::of_value(v_ps))...
        );
    auto v_ret = CamlConversion<Rv>::to_value(ret);
    return v_ret;
  }
}

template<typename R, typename Rv = ReplaceVoid<R>::type, typename C, typename A0c, typename... As, typename A0 = NormalizeArgument<A0c>>
requires
( CamlToValue<Rv> && CamlOfValue<C*> && CamlOfValue<A0> && (CamlOfValue<NormalizeArgument<As>> && ...)
  && CamlContextConstructible<Rv,A0>
  && CamlConstructible<C*,A0>
)
inline value
call_api_class_implied(R (C::*fun)(A0c, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  if constexpr(CamlHasContext<R>) {
    typedef typename CamlConversion<Rv>::Context Context;
    auto& r0 = CamlConversion<A0>::of_value(v_p0);
    auto c0 = extract_context<C,A0>(r0);
    auto context = extract_context<Context,A0>(r0);
    auto ret =
      invoke_void
        ( fun
        , c0.get()
        , CamlConversion<A0>::get_underlying(r0)
        , ConversionNormalized<As>::get_underlying(ConversionNormalized<As>::of_value(v_ps))...
        );
    auto v_ret = CamlConversion<Rv>::to_value(context, ret);
    return v_ret;
  } else {
    auto& r0 = CamlConversion<A0>::of_value(v_p0);
    auto c0 = extract_context<C,A0>(r0);
    auto ret =
      invoke_void
        ( fun
        , c0.get()
        , CamlConversion<A0>::get_underlying(r0)
        , ConversionNormalized<As>::get_underlying(ConversionNormalized<As>::of_value(v_ps))...
        );
    auto v_ret = CamlConversion<Rv>::to_value(ret);
    return v_ret;
  }
}

template<typename C, typename R, typename... Args>
auto inline class_function_no_const(R (C::*f)(Args...) const){
  return (R (C::*)(Args...))(f);
};

template<typename R, typename Rv = ReplaceVoid<R>::type, typename C, typename... As>
inline value
call_api_class(R (C::*fun)(As...) const, value c, typename first_type<value,As>::type... v_ps){
  return call_api_class(class_function_no_const(fun), c, v_ps...);
};

template<typename R, typename Rv = ReplaceVoid<R>::type, typename C, typename A0, typename... As>
inline value
call_api_class_implied(R (C::*fun)(A0, As...) const, value v_p0, typename first_type<value,As>::type... v_ps){
  return call_api_class_implied(class_function_no_const(fun), v_p0, v_ps...);
};

/////////////////////////////////////////////////////////////////////////////////////////
/// calling of api functions
///

template<typename X, template<typename> typename T, template<typename> typename... Ts> struct chain_remove{
  typedef typename T<typename chain_remove<X, Ts...>::type>::type type;
};

template<typename X, template<typename> typename T> struct chain_remove<X,T>{
  typedef typename T<X>::type type;
};

template<typename T> struct normalize_pointer_argument {
  typedef chain_remove<T,std::remove_const,std::remove_reference,std::remove_const,std::remove_pointer>::type type;
};

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
( (represented_as_Immediate<R> || represented_as_Value<R> || std::same_as<void,R>)
&&  represented_as_ContainerSharedPointer<typename normalize_pointer_argument<A0>::type *>
)
inline value
apiN(R (*fun)(A0, As...), value v_p0, typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<A0raw>>(v_p0);
  auto context = context_s.get();
  if constexpr (std::same_as<void,R>) {
    fun(context,T_value<As>(v_ps)...);
    return Val_unit;
  } else {
    auto ret = fun(context,T_value<As>(v_ps)...);
    if constexpr (represented_as_Immediate<R>)
      return ImmediateProperties<R>::to_value(ret);
    else if constexpr (represented_as_Value<R>)
      return ValueProperties<R>::to_value(ret);
  }
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

template<typename C, typename R, typename... Ps, typename... Psp>
requires
(represented_as_Immediate<R> || represented_as_Value<R> || std::same_as<void, R>)
inline value call_wrap_class(C* c, R (C::*fun)(Ps...) const, Psp... params){
  if constexpr (std::same_as<void,R>){
    (c->*fun)(params...);
    return Val_unit;
  } else {
    auto ret = (c->*fun)(params...);
    if constexpr (represented_as_Immediate<R>)
      return ImmediateProperties<R>::to_value(ret);
    else if constexpr (represented_as_Value<R>)
      return ValueProperties<R>::to_value(ret);
  }
}

template<typename C, typename R, typename... As>
requires
(
 (represented_as_ContainerSharedPointer<C*> || represented_as_InlinedWithContext<C>)
 && (represented_as_Immediate<R> || represented_as_Value<R> || std::same_as<void, R>)
)
inline value
apiN_class(R (C::*fun)(As...) const, value v_c, typename first_type<value,As>::type... v_ps){
  C*context;
  if constexpr (represented_as_ContainerSharedPointer<C*>){
    auto&context_s = Custom_value<CppCaml::ContainerSharedPointer<C>>(v_c);
    context = context_s.get();
  } else if (represented_as_InlinedWithContext<C>){
    auto&context_s = Custom_value<CppCaml::InlinedWithContext<C>>(v_c);
    context = &context_s.t;
  }

  return call_wrap_class(context,fun,T_value<As>(v_ps)...);
}

template<typename C, typename R, typename A0, typename... As>
requires
(
 represented_as_InlinedWithContext<A0>
 && (represented_as_Immediate<R> || represented_as_Value<R> || std::same_as<void, R>)
)
inline value
apiN_class_implied(R (C::*fun)(A0, As...) const, value v_p0  ,typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&p0_s = Custom_value<CppCaml::InlinedWithContext<A0raw>>(v_p0);
  auto context = p0_s.pContext.get();
  
  return call_wrap_class(context,fun,T_value<A0>(v_p0),T_value<As>(v_ps)...);
}

template<typename C, typename R, typename A0, typename... As>
requires
(
 represented_as_InlinedWithContext<A0>
 && represented_as_InlinedWithContext<R>
)
inline value
// CR smuenzel: need to figure out const reference stuff....
apiN_class_implied(R (C::*fun)(const A0&, As...) const, value v_p0  ,typename first_type<value,As>::type... v_ps){
  typedef typename normalize_pointer_argument<A0>::type A0raw;
  auto&p0_s = Custom_value<CppCaml::InlinedWithContext<A0raw>>(v_p0);
  auto context_s = p0_s.pContext;
  auto context = context_s.get();
  
  auto dep = (context->*fun)(T_value<A0>(v_p0), T_value<As>(v_ps)...);

  typedef CppCaml::InlinedWithContext<R> Container;
  value v_dep = Container::allocate(context_s, std::move(dep));
  return v_dep;
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

// CR smuenzel: only differs in const-ness....
template<typename C, typename R, typename... As>
requires
(
 represented_as_ContainerSharedPointer<C *>
 && represented_as_InlinedWithContext<R>
)
inline value
apiN_class(R (C::*fun)(As...), value v_c, typename first_type<value,As>::type... v_ps){
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
CAML_REPRESENTATION(std::string, Value);


#define CPPCAML_ENUM_LOCAL(E,X) X,
#define CPPCAML_ENUM_GLOBAL_V(E,X) \
  static inline constexpr auto __caml_api_enum_var__## E ##_##X \
  __attribute((used, section("caml_api_enum"))) = \
  CppCaml::ApiEnumEntry(#E,#X,Val_long(Local::X));

#define CPPCAML_ENUM_CASE(E,X) \
  case E::X: return Val_long(Local::X);

#define CPPCAML_ENUM_REVERSE_CASE(E,X) \
  case Local::X : return E::X;

#define MAKE_ENUM_IMMEDIATE_PROPERTIES(ENUM_NAME, ENUM, DEFAULT) \
template<> struct CppCaml::ImmediateProperties<ENUM_NAME> {\
  enum class Local { \
    ENUM(CPPCAML_ENUM_LOCAL) \
  }; \
\
  ENUM(CPPCAML_ENUM_GLOBAL_V) \
\
  static inline value to_value(ENUM_NAME b) {\
    switch(b){\
      ENUM(CPPCAML_ENUM_CASE)\
      default: return Val_long(Local::DEFAULT);\
    } \
  } \
  \
  static inline ENUM_NAME of_value(value v) { \
    switch((Local)Long_val(v)) { \
      ENUM(CPPCAML_ENUM_REVERSE_CASE) \
      default: assert(false); \
    } \
 \
  } \
};

