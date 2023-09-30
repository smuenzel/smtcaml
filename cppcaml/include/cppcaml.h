
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

#define REGISTER_API(APIPREFIX,APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__##APIPREFIX ## _ ##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(#APIF,#WRAPPER,APIF);

#define REGISTER_API_MEMBER(APIPREFIX,CLASS, APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__ ##APIPREFIX ## _ ##CLASS ## _##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(#CLASS "__" #APIF,#WRAPPER,#CLASS, &CLASS :: APIF);

#define REGISTER_API_MEMBER_IMPLIED(APIPREFIX,CLASS, APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__ ##APIPREFIX ## _ ##CLASS ## _##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(CppCaml::DropFirstArgument(), #CLASS "__" #APIF,#WRAPPER,#CLASS, &CLASS :: APIF);

#define REGISTER_API_MEMBER_OVERLOAD(APIPREFIX,CLASS, APIF, SUFFIX, WRAPPER, ...) \
  static inline constexpr auto __caml_api_registry_var__##APIPREFIX ## _ ##CLASS ## _##APIF ## _ ## SUFFIX \
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

#define REGISTER_API_IMPLIED_FIRST(APIPREFIX, APIF, WRAPPER) \
  static inline constexpr auto __caml_api_registry_var__##APIPREFIX ## __ ##APIF \
__attribute((used, section("caml_api_registry"))) = \
    CppCaml::ApiRegistryEntry(CppCaml::DropFirstArgument(),#APIF,#WRAPPER,APIF);

/////////////////////////////////////
// Function Autogen
/////////////////////////////////////
///
#define API0__(APIPREFIX, APIF) \
  REGISTER_API(APIPREFIX,APIF, caml_ ##APIPREFIX ##__##APIF); \
  apireturn caml_ ##APIPREFIX ## __##APIF(value v_unit){ \
    return CppCaml::call_api(APIF); \
  }

#define API1__(APIPREFIX, APIF) \
  REGISTER_API(APIPREFIX,APIF, caml_ ##APIPREFIX ##__##APIF); \
  apireturn caml_ ##APIPREFIX ## __##APIF(value v0){ \
    return CppCaml::call_api(APIF, v0); \
  }

#define API2__(APIPREFIX, APIF) \
  REGISTER_API(APIPREFIX,APIF, caml_ ##APIPREFIX ##__##APIF); \
  apireturn caml_ ##APIPREFIX ## __##APIF(value v0, value v1){ \
    return CppCaml::call_api(APIF, v0, v1); \
  }

#define API1_IMPLIED__(APIPREFIX, APIF) \
  REGISTER_API_IMPLIED_FIRST(APIPREFIX,APIF, caml_ ##APIPREFIX ##__##APIF); \
  apireturn caml_ ##APIPREFIX ## __##APIF(value v0){ \
    return CppCaml::call_api_implied_first(APIF, v0); \
  }

#define API2_IMPLIED__(APIPREFIX, APIF) \
  REGISTER_API_IMPLIED_FIRST(APIPREFIX,APIF, caml_ ##APIPREFIX ##__##APIF); \
  apireturn caml_ ##APIPREFIX ## __##APIF(value v0, value v1){ \
    return CppCaml::call_api_implied_first(APIF, v0, v1); \
  }

#define API3_IMPLIED__(APIPREFIX, APIF) \
  REGISTER_API_IMPLIED_FIRST(APIPREFIX,APIF, caml_ ##APIPREFIX ##__##APIF); \
  apireturn caml_ ##APIPREFIX ## __##APIF(value v0, value v1, value v2){ \
    return CppCaml::call_api_implied_first(APIF, v0, v1, v2); \
  }

#define API3__(APIPREFIX, APIF) \
  REGISTER_API(APIPREFIX,APIF, caml_ ##APIPREFIX ##__##APIF); \
  apireturn caml_ ##APIPREFIX ## __##APIF(value v0, value v1, value v2){ \
    return CppCaml::call_api(APIF, v0, v1, v2); \
  }

#define APIM0__(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER(APIPREFIX,CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_c){ \
    return CppCaml::call_api_class(&CLASS :: APIF, v_c); \
  }

#define APIM1__(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER(APIPREFIX,CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_c, value v_p0){ \
    return CppCaml::call_api_class(&CLASS :: APIF, v_c, v_p0); \
  }

#define APIM2__(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER(APIPREFIX,CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_c, value v_p0,value v_p1){ \
    return CppCaml::call_api_class(&CLASS :: APIF, v_c, v_p0, v_p1); \
  }

#define APIM1_IMPLIED__(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER_IMPLIED(APIPREFIX,CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_p0){ \
    return CppCaml::call_api_class_implied(&CLASS :: APIF, v_p0); \
  }

#define APIM2_IMPLIED__(APIPREFIX, CLASS,APIF) \
  REGISTER_API_MEMBER_IMPLIED(APIPREFIX,CLASS,APIF, caml_ ##APIPREFIX ##__##CLASS ## __##APIF); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF(value v_p0, value v_p1){ \
    return CppCaml::call_api_class_implied(&CLASS :: APIF, v_p0, v_p1); \
  }

#define APIM2_OVERLOAD_IMPLIED_(APIPREFIX, CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(APIPREFIX,CLASS,APIF,SUFFIX, caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_p0, value v_p1){ \
    return CppCaml::call_api_class_implied(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_p0, v_p1); \
  }

#define APIM1_OVERLOAD_(APIPREFIX, CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(APIPREFIX,CLASS,APIF,SUFFIX, caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_c, value v_p0){ \
    return CppCaml::call_api_class(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_c, v_p0); \
  }

#define APIM2_OVERLOAD_(APIPREFIX, CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(APIPREFIX,CLASS,APIF,SUFFIX, caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_c, value v_p0, value v_p1){ \
    return CppCaml::call_api_class(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_c, v_p0, v_p1); \
  }

#define APIM3_OVERLOAD_(APIPREFIX, CLASS,APIF,SUFFIX,...) \
  REGISTER_API_MEMBER_OVERLOAD(APIPREFIX,CLASS,APIF,SUFFIX, caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX, __VA_ARGS__); \
  apireturn caml_ ##APIPREFIX ##__##CLASS ## __##APIF ## __overload__ ##SUFFIX (value v_c, value v_p0, value v_p1, value v_p2){ \
    return CppCaml::call_api_class(CppCaml::resolveOverload<CLASS>(CppCaml::type_list<__VA_ARGS__>(),&CLASS :: APIF), v_c, v_p0, v_p1, v_p2); \
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
concept PropertyNoContext = requires {
  requires(!PropertyHasContext<T>);
};

template<typename T>
concept PropertyHasDeleterWithContext = requires {
  requires PropertyHasContext<T>;
  requires requires(CamlConversionProperties<T>::Context*c, T*t) {
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

  static inline value allocate(std::shared_ptr<T> t){
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

template<typename T>
concept CamlConvertible = requires {
  typename CamlConversion<T>::RepresentationType;
};

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

template<typename T>
concept CamlHasContext = requires (T a, CamlConversion<T>::RepresentationType& ra) {
  requires CamlConvertible<T>;
  typename CamlConversion<T>::Context;
  //requires std::same_as<typename CamlConversion<T>::Context,C>;
  //{ CamlConversion<T>::get_context(ra) } -> std::same_as<std::shared_ptr<typename CamlConversion<T>::Context>&>;
};

template<typename T>
concept CamlNoContext = requires {
  requires(!CamlHasContext<T>);
};

template<typename T>
concept CamlToValueBase = requires {
  requires CamlConvertible<T>;
  requires std::same_as<decltype(CamlConversion<T>::allocates),const CamlAllocates>;
};

template<typename T>
concept CamlToValueNoContext = requires {
  requires CamlToValueBase<T>;
  requires requires(T t){CamlConversion<T>::to_value(t);};
};

template<typename T>
concept CamlToValueContext = requires {
  requires CamlToValueBase<T>;
  requires CamlHasContext<T>;
  requires requires(std::shared_ptr<typename CamlConversion<T>::Context> c, T t){CamlConversion<T>::to_value(c, t);};
};

template<typename T>
concept CamlToValue = requires {
  requires CamlConvertible<T>;
  requires std::same_as<decltype(CamlConversion<T>::allocates),const CamlAllocates>;
  requires (CamlToValueNoContext<T> || CamlToValueContext<T>);
};


template<typename T>
concept CamlBidirectional = requires {
  requires CamlOfValue<T>;
  requires CamlToValue<T>;
};

template<typename T>
requires 
(PropertyRepresented<T,CamlRepresentationKind::ContainerWithContext>
&& PropertyHasContext<T>)
struct CamlConversion<T> {
  typedef typename CamlConversionProperties<T>::Context Context;
  typedef ContextContainer<T, Context> RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  static inline T& get_underlying(RepresentationType& r){
    return r.t;
  }

  static inline std::shared_ptr<Context>& get_context(RepresentationType& r){
    return r.pContext;
  }

  static inline value to_value(std::shared_ptr<Context>&ctx, T&t){
    return RepresentationType::allocate(ctx, t);
  }

  static inline RepresentationType&of_value(value v){
    return Custom_value<RepresentationType>(v);
  }
};

template<typename T>
requires 
(PropertyRepresented<T,CamlRepresentationKind::ContainerWithContext>
&& PropertyHasContext<T>)
struct CamlConversion<T*> {
  typedef T* RepresentationType;

  static inline T* get_underlying(RepresentationType r){
    return r;
  }

  static inline RepresentationType of_value(value v){
    return &CamlConversion<T>::get_underlying(CamlConversion<T>::of_value(v));
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

  static inline value to_value(T t){
    typedef SharedPointerContainer<Tnopointer> This;
    if constexpr(PropertyHasDeleter<T>){
      RepresentationType r(t,SimpleDeleter<T>());
      return This::allocate(r);
    } else {
      RepresentationType r(t);
      return This::allocate(r);
    }
  }
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
  static inline int get_underlying(RepresentationType r) { return r; }
};
static_assert(CamlBidirectional<int>);

template<> struct CamlConversion<unsigned int> {
  typedef unsigned int RepresentationType;
  static const auto allocates = CamlAllocates::No_allocation;

  static inline value to_value(unsigned int x) { return Val_long(x); }
  static inline RepresentationType of_value(value v) { return Long_val(v); }
  static inline unsigned int get_underlying(RepresentationType r) { return r; }
};
static_assert(CamlBidirectional<unsigned int>);

template<> struct CamlConversion<long unsigned int> {
  typedef long unsigned int RepresentationType;
  static const auto allocates = CamlAllocates::No_allocation;

  static inline value to_value(long unsigned int x) { return Val_long(x); }
  static inline RepresentationType of_value(value v) { return Long_val(v); }
  static inline long unsigned int get_underlying(RepresentationType r) { return r; }
};
static_assert(CamlBidirectional<long unsigned int>);

template<> struct CamlConversion<bool> {
  typedef bool RepresentationType;
  static const auto allocates = CamlAllocates::No_allocation;

  static inline value to_value(bool x) { return Val_bool(x); }
  static inline RepresentationType of_value(value v) { return Bool_val(v); }
  static inline bool get_underlying(RepresentationType r) { return r; }
};
static_assert(CamlBidirectional<int>);

template<> struct CamlConversion<const char *> {
  typedef const char * RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  static inline value to_value(const char *x) { return caml_copy_string(x?x:""); }
  static inline RepresentationType of_value(value v) { return String_val(v); }
  static inline auto get_underlying(RepresentationType r) { return r; }
};
static_assert(CamlBidirectional<const char *>);

template<> struct CamlConversion<std::string> {
  typedef std::string RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  static inline value to_value(const std::string&x) { return caml_copy_string(x.c_str()); }
  static inline RepresentationType of_value(value v) { return String_val(v); }
  static inline auto get_underlying(RepresentationType r) { return r; }
};
static_assert(CamlBidirectional<std::string>);

template<> struct CamlConversionProperties<std::string>{
  static constexpr bool allow_const = true;
};

template<typename T>
struct CamlConversion<std::optional<T>>{
  typedef CamlConversion<T> N;
  typedef std::optional<typename N::RepresentationType> RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  static inline RepresentationType of_value(value v) {
    if(Is_none(v)) return std :: nullopt;
    else return N::of_value(Some_val(v));
  }

  static inline std::optional<T> get_underlying(RepresentationType r){
    if(r){
      auto v = r.value();
      return std::optional(N::get_underlying(v));
    }else{
      return std::nullopt;
    }
  }

  static inline value to_value(std::optional<T> t){
    if(t) {
      CAMLparam0();
      CAMLlocal2(inner, outer);
      inner = N::to_value(t.value());
      outer = caml_alloc_small(1,0);
      Field(outer,0) = inner;
      CAMLreturn(outer);
    } else {
      return Val_none;
    }
  }
};
static_assert(CamlBidirectional<std::optional<std::string>>);

template<typename T>
requires CamlNoContext<T>
struct CamlConversion<std::vector<T>>{
  typedef CamlConversion<T> N;
  typedef std::vector<T> RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  template <typename Q = T>
  requires CamlToValue<Q>
  static inline value to_value(std::vector<T>&vec) {
    CAMLparam0();
    CAMLlocal1(v_ret);
    v_ret = caml_alloc(vec.size(), 0);
    for(size_t i = 0; i < vec.size(); i++){
      Store_field(v_ret,i,N::to_value(vec[i]));
    };
    CAMLreturn(v_ret);
  }

  static inline RepresentationType of_value(value v) { 
    auto len = Wosize_val(v);
    auto f = [](value v){
      return N::get_underlying(N::of_value(v));
    };
    auto begin = boost::make_transform_iterator(&Field(v, 0), f);
    auto end = boost::make_transform_iterator(&Field(v, len), f);

    return std::vector<T>(begin, end);
  }

  static inline std::vector<T> get_underlying(RepresentationType r) { return r; }

};
static_assert(CamlBidirectional<std::vector<std::string>>);

template<typename T>
requires CamlHasContext<T>
struct CamlConversion<std::vector<T>>{
  typedef CamlConversion<T> N;
  typedef CamlConversion<T>::Context Context;

  typedef std::pair<std::optional<std::shared_ptr<Context>>, std::vector<T>> RepresentationType;
  static const auto allocates = CamlAllocates::Allocation;

  static inline std::shared_ptr<Context>& get_context(RepresentationType& r){
    return r.first;
  }

  template <typename Q = T>
  requires CamlToValue<Q>
  static inline value to_value(std::shared_ptr<Context>&ctx, std::vector<T>&vec) {
    CAMLparam0();
    CAMLlocal1(v_ret);
    v_ret = caml_alloc(vec.size(), 0);
    for(size_t i = 0; i < vec.size(); i++){
      value v = N::to_value(ctx,vec[i]);
      Store_field(v_ret,i,v);
    };
    CAMLreturn(v_ret);
  }

  static inline RepresentationType of_value(value v) { 
    auto len = Wosize_val(v);
    std::optional<std::shared_ptr<Context>> context = std::nullopt;
    auto f = [&context](value v){
      auto& of_value = N::of_value(v);
      context = N::get_context(of_value);
      return N::get_underlying(of_value);
    };
    auto begin = boost::make_transform_iterator(&Field(v, 0), f);
    auto end = boost::make_transform_iterator(&Field(v, len), f);

    return std::make_pair(context,std::vector<T>(begin, end));
  }

  static inline std::vector<T> get_underlying(RepresentationType r) { return r.second; }

};

template<typename T0, typename T1>
struct CamlConversion<std::pair<T0,T1>>{
  typedef CamlConversion<T0> N0;
  typedef CamlConversion<T1> N1;
  static const auto allocates = CamlAllocates::Allocation;

  static inline value to_value(std::pair<N0,N1>&p){
    CAMLparam0();
    CAMLlocal3(v_pair,v_first,v_second);
    v_first = N0::to_value(p.first);
    v_second = N1::to_value(p.second);
    v_pair = caml_alloc_small(2,0);
    Field(v_pair,0) = v_first;
    Field(v_pair,1) = v_second;
    CAMLreturn(v_pair);
  }

  typedef std::pair<T0,T1> RepresentationType;

  static inline RepresentationType of_value(value v) {
    return
      std::make_pair
      ( N0::get_underlying(N0::of_value(Field(v,0)))
      , N1::get_underlying(N1::of_value(Field(v,1)))
      );
  }

  static inline RepresentationType get_underlying(RepresentationType r) { return r; }
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

template<typename T> struct remove_pconst { typedef T type; };
template<typename T> struct remove_pconst<const T*> { typedef T* type; };
// const char* is special
template<> struct remove_pconst<const char*> { typedef const char* type; };

template<typename T> using remove_pconst_t = typename remove_pconst<T>::type;

template<typename T> using NormalizeArgument =
  remove_pconst_t<std::remove_const_t<std::remove_reference_t<T>>>;

template<typename T> using ConversionNormalized = CamlConversion<NormalizeArgument<T>>;

template<typename R, typename Rv = ReplaceVoid<R>::type, typename... Asc>
requires
( CamlToValue<Rv> && (CamlOfValue<NormalizeArgument<Asc>> && ...)
  && CamlNoContext<Rv>
)
inline value
call_api(R (*fun)(Asc...), typename first_type<value,Asc>::type... v_ps){
    auto ret =
      invoke_void
        ( fun
        , ConversionNormalized<Asc>::get_underlying(ConversionNormalized<Asc>::of_value(v_ps))...
        );
    auto v_ret = CamlConversion<Rv>::to_value(ret);
    return v_ret;
}

template<typename R, typename Rv = ReplaceVoid<R>::type, typename A0c, typename... Asc, typename A0 = NormalizeArgument<A0c>>
requires
( CamlToValue<Rv> && CamlOfValue<A0> && (CamlOfValue<NormalizeArgument<Asc>> && ...)
  && CamlHasContext<Rv>
  && CamlContextConstructible<Rv,A0>
)
inline value
call_api(R (*fun)(A0c, Asc...), value v0, typename first_type<value,Asc>::type... v_ps){
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
}

template<typename C, typename T> void return_destructor(C&, T&) {  };

template<typename R, typename Rv = ReplaceVoid<R>::type, typename A0c, typename A1c, typename... As, typename A0 = NormalizeArgument<A0c>, typename A1 = NormalizeArgument<A1c>>
requires
( CamlToValue<Rv> && CamlOfValue<A1> && (CamlOfValue<NormalizeArgument<As>> && ...)
  && CamlContextConstructible<Rv,A1>
  && CamlConstructible<A0,A1>
)
inline value
call_api_implied_first(void (*return_destructor)(A0c&, Rv&), R (*fun)(A0c, A1c, As...), value v1, typename first_type<value,As>::type... v_ps){
  auto& r1 = CamlConversion<A1>::of_value(v1);
  auto context = extract_context<typename std::remove_pointer<A0>::type,A1>(r1);
  auto context_p = context.get();
  auto ret =
    invoke_void
      ( fun
      , context_p
      , CamlConversion<A1>::get_underlying(r1)
      , ConversionNormalized<As>::get_underlying(ConversionNormalized<As>::of_value(v_ps))...
      );
  if constexpr(CamlHasContext<Rv>) {
    auto v_ret = CamlConversion<Rv>::to_value(context, ret);
    return_destructor(context_p, ret);
    return v_ret;
  } else {
    auto v_ret = CamlConversion<Rv>::to_value(ret);
    return_destructor(context_p, ret);
    return v_ret;
  }
}

template<typename R, typename Rv = ReplaceVoid<R>::type,  typename A0c, typename A1c, typename... As>
inline value
call_api_implied_first(R (*fun)(A0c, A1c, As...), value v1, typename first_type<value,As>::type... v_ps){
  return call_api_implied_first(return_destructor<A0c,Rv>,fun,v1,v_ps...);
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
        , CamlConversion<C*>::get_underlying(CamlConversion<C*>::of_value(c))
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

template<typename R, typename C, typename... As>
inline value
call_api_class(R (C::*fun)(As...) const, value c, typename first_type<value,As>::type... v_ps){
  return call_api_class(class_function_no_const(fun), c, v_ps...);
};

template<typename R, typename Rv = ReplaceVoid<R>::type, typename C, typename A0, typename... As>
inline value
call_api_class_implied(R (C::*fun)(A0, As...) const, value v_p0, typename first_type<value,As>::type... v_ps){
  return call_api_class_implied(class_function_no_const(fun), v_p0, v_ps...);
};

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
template<> struct CppCaml::CamlConversion<ENUM_NAME> {\
  enum class Local { \
    ENUM(CPPCAML_ENUM_LOCAL) \
  }; \
  \
  ENUM(CPPCAML_ENUM_GLOBAL_V)\
  \
  typedef ENUM_NAME RepresentationType;\
  static const auto allocates = CamlAllocates::No_allocation;\
  \
  static inline value to_value(ENUM_NAME b) {\
    switch(b){\
      ENUM(CPPCAML_ENUM_CASE)\
      default: return Val_long(Local::DEFAULT);\
    } \
  } \
  static inline ENUM_NAME of_value(value v) { \
    switch((Local)Long_val(v)) { \
      ENUM(CPPCAML_ENUM_REVERSE_CASE) \
      default: assert(false); \
    } \
  } \
  static inline ENUM_NAME get_underlying(RepresentationType r) { return r; }\
};\
static_assert(CppCaml::CamlBidirectional<ENUM_NAME>);
