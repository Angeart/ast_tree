//  This file is a "Hello, world!" in C++ language for wandbox-vscode.
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <memory>
#include <vector>
#include <array>
#include <boost/core/demangle.hpp>
#include <boost/variant.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/find.hpp>
#include <boost/serialization/strong_typedef.hpp>
// #include <boost/preprocessor/list/for_each.hpp>
// #include <boost/preprocessor/variadic/to_list.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/for_each.hpp>

namespace util {
    template<class... T>
    struct inherit{
        template<class...Args>
        inherit(Args&&...){};
    };

    template<class T>
    struct inherit<T> : T{
        template<class...Args>
        inherit(Args&&...args) : T(std::forward<Args>(args)...){};
    };

    template<class H, class... T>
    struct inherit<H,T...> : H, inherit<T...> {
        template<class...Args>
        inherit(Args&&... args) : H(args...), inherit<T...>(std::forward<Args>(args)...){};
    };
}

namespace util {
    namespace class_identity_detail {
        using class_identifier_t = uint32_t;
        struct identity_factory {
            static class_identifier_t get_id() {
                static class_identifier_t value = 0;
                return value++;
            }
        };

        template<class T>
        struct identity_holder {
            using value_type = class_identifier_t;
            using class_type = T;
            identity_holder() : value(identity_factory::get_id()) {};
            static class_identifier_t get_identity() {
                static const identity_holder temp;
                return temp.get_internal_identity();
            }
        private:
            class_identifier_t get_internal_identity() const {
                return this->value;
            }
            const class_identifier_t value;
        };
        template<class T, class...Bases>
        struct class_identity : public inherit<Bases...>{
            //using inherit<Bases...>::inherit;
            template<class...Args>
            class_identity(Args&&... args) : inherit<Bases...>(std::forward<Args>(args)...) {};
            using base_type = class_identity;
            using identity_type = identity_holder<T>;
            virtual typename identity_type::value_type get_identity() const {
                return identity_type::get_identity();
            }
            static typename identity_type::value_type get_identity_static() {
                return identity_type::get_identity();
            }
        };
    }
    namespace types {
        using class_identifier_t = class_identity_detail::class_identifier_t;
    }
    template<class T, class... Bases>
    using class_identity = class_identity_detail::class_identity<T, Bases...>;
}

namespace util {
    template <typename T>
    struct skip {
        T& t;
        std::size_t n;
        skip(T& v, std::size_t s) : t(v), n(s) {}
        auto begin() -> decltype(std::begin(t)) {
            return std::next(std::begin(t), n);
        }
        auto end() -> decltype(std::end(t)) {
            return std::end(t);
        }
    };
}

template<class target_t>
struct is_dumpable_t {
    template<class U> static auto check_internal(std::nullptr_t n) -> decltype(std::declval<U>().dump(), std::true_type{});
    template<class U> static auto check_internal(...) -> std::false_type;
    static constexpr auto check() -> bool { return decltype(check_internal<target_t>(nullptr))::value; }
};

template<class target_t>
constexpr auto is_dumpable() -> bool { return is_dumpable_t<target_t>::check(); }

template<class target_t>
struct is_streamable_t {
    template<class U> static auto check_internal(std::nullptr_t n) -> decltype(std::cout << std::declval<U>(), std::true_type{});
    template<class U> static auto check_internal(...) -> std::false_type;
    static constexpr auto check() -> bool { return decltype(check_internal<target_t>(nullptr))::value; }
};

template<class target_t>
constexpr auto is_streamable() -> bool { return is_streamable_t<target_t>::check(); }

struct unused_t {};

struct indent_t {
    static constexpr uint32_t width = 4u;
};

template<class target_t>
std::string type2str() {
    std::stringstream ss;
    ss << "[" << boost::core::demangle(typeid(target_t).name()) << "]";
    return ss.str();
}

struct dumper_t : boost::static_visitor<void> {
    dumper_t() : indent(0u) {};
    dumper_t(uint32_t indent_) : indent(indent_) {};
    void set_indent(uint32_t indent_) { indent = indent_; }
    template<class target_t>
    auto operator()(const target_t& d) const -> std::enable_if_t<is_dumpable<target_t>(),void> {
        print_name<target_t>() << std::endl;
        d.dump(this->indent + indent_t::width);
    };
    template<class target_t>
    auto operator()(const target_t& d) const -> std::enable_if_t<!is_dumpable<target_t>() && !is_streamable<target_t>(), void> {
        print_name<target_t>() << " ";
        std::cout << "--" << std::endl;
    };
    template<class target_t>
    auto operator()(const target_t& d) const -> std::enable_if_t<is_streamable<target_t>(),void> {
        print_name<target_t>() << " ";
        std::cout << d << std::endl;
    }
private:
    template<class target_t>
    std::ostream& print_name() const {
        return std::cout << put_indent() << "[" << boost::core::demangle(typeid(target_t).name()) << "]";
    }
private:
    const std::string put_indent() const {
        std::string str = "";
        for([[maybe_unused]]uint32_t i = 0u; i < this->indent; ++i)str += " ";
        return str;
    }
    uint32_t indent;
};
// template<>
// auto dumper_t::operator()<unused_t>(const unused_t& d) const -> void {
// };

template<class ... types_t>
struct sequence_holder_t {
    using container_t = boost::mpl::vector<types_t...>;
    template<size_t index_v>
    using at_t = typename boost::mpl::at<container_t,boost::mpl::int_<index_v>>::type;
    template<class selected_t>
    static constexpr size_t at_v = boost::mpl::find<container_t,selected_t>::type::pos::value;
};

template<class ... elems_t>
struct bundle_t {
    using fusion_t = boost::fusion::vector<elems_t...>;
    using elem_size_t = std::integral_constant<size_t, sizeof...(elems_t)>;
    using resolver_t = sequence_holder_t<elems_t...>;
    fusion_t v;
    template<class input_t>
    bundle_t(const input_t& in) {
        boost::fusion::at_c<resolver_t::template at_v<input_t>>(this->v) = in;
    }
    bundle_t() {};
    virtual void dump(uint32_t indent = 0) const {
        dumper_t dumper{indent};
        boost::fusion::for_each(this->v,dumper);
    }
    template<class input_t, size_t index_v = resolver_t::template at_v<input_t>>
    void add(const input_t& in) {
        boost::fusion::at_c<resolver_t::template at_v<input_t>>(this->v) = in;
    }
    template<class input_t>
    auto get() -> input_t {
        return boost::fusion::at_c<resolver_t::template at_v<input_t>>(this->v);
    }
};

// struct int32_value_t : util::class_identity<int32_value_t, bundle_t<int>> {
//     using base_type::base_type;
// };

// struct float_value_t : util::class_identity<float_value_t, bundle_t<float>> {
//     using base_type::base_type;
// };

// struct test_t : util::class_identity<test_t, bundle_t<int32_value_t, float_value_t>> {
//     using base_type::base_type;
// };

// struct string_value_t : util::class_identity<string_value_t, bundle_t<std::string>> {
//     using base_type::base_type;
// };

// struct func_t : util::class_identity<func_t, bundle_t<string_value_t,test_t>> {
//     using base_type::base_type;
// };

#define SAPPHIRE_AST_DEF_ASSIGN_OP(name, ...) \
    template<class input_t> \
    name& operator=(const input_t& in) { \
        std::cout << "---------------------------" << std::endl; \
        std::cout << "assign : " << type2str<input_t>() << " -> " << type2str<decltype(*this)>() << std::endl; \
        this->add(internal_converter::cast(in)); \
        return *this; \
    }

#define SAPPHIRE_ASR_DEF_CONSTRUCTOR_INTERNAL_OP() ||

#define SAPPHIRE_AST_DEF_CONSTRUCTOR_INTERNAL(z, n, seq) \
    BOOST_PP_IF(n, SAPPHIRE_ASR_DEF_CONSTRUCTOR_INTERNAL_OP, BOOST_PP_EMPTY)() std::is_convertible<input_t,BOOST_PP_SEQ_ELEM(n,seq)>::value

#define SAPPHIRE_AST_DEF_CONSTRUCTOR(name, ...) \
    template<class input_t, typename = std::enable_if_t<BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)), SAPPHIRE_AST_DEF_CONSTRUCTOR_INTERNAL, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))>> \
    explicit name(const input_t& in) { \
        std::cout << std::endl << "===========================" << std::endl; \
        std::cout << "constructor " << type2str<decltype(*this)>() << " : "; \
        dumper_t()(in); \
        this->add(internal_converter::cast(in)); \
    } \
    template<class input_t, typename = std::enable_if_t<BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)), SAPPHIRE_AST_DEF_CONSTRUCTOR_INTERNAL, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))>> \
    explicit name(input_t&& in) { \
        std::cout << std::endl << "===========================" << std::endl; \
        std::cout << "move constructor " << type2str<decltype(*this)>() << " : "; \
        dumper_t()(in); \
        this->add(internal_converter::cast(in)); \
    }

#define SAPPHIRE_AST_DEF_CONVERTER_INTERNAL(r, _, type) \
    template<class from_t,class to_t = std::enable_if_t<std::is_convertible<from_t,type>{},type>> \
    static type cast(const from_t& in) { \
        std::cout << "cast to " << BOOST_PP_STRINGIZE(type) << std::endl; \
        return to_t(in); \
    } \
    template<class from_t,class to_t = std::enable_if_t<std::is_convertible<from_t,type>{},type>> \
    static type cast(from_t&& in) { \
        std::cout << "move cast to " << BOOST_PP_STRINGIZE(type) << std::endl; \
        return std::move(to_t(in)); \
    } \
    static type cast(const type& in) { \
        std::cout << "same type cast... skipped " << BOOST_PP_STRINGIZE(type) << std::endl; \
        return in; \
    } \
    static type cast(type&& in) { \
        std::cout << "same type cast... skipped " << BOOST_PP_STRINGIZE(type) << std::endl; \
        return in; \
    }

#define SAPPHIRE_AST_DEF_CONVERTER(...) \
    struct internal_converter { \
        BOOST_PP_SEQ_FOR_EACH(SAPPHIRE_AST_DEF_CONVERTER_INTERNAL, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
    }; \

#define SAPPHIRE_AST_DEF_DEFAULT_CONSTRUCTOR(name) \
    name() {}

#define SAPPHIRE_AST_DEF(name, ...) \
struct name : util::class_identity<name,bundle_t<__VA_ARGS__>> { \
    SAPPHIRE_AST_DEF_DEFAULT_CONSTRUCTOR(name) \
    SAPPHIRE_AST_DEF_CONSTRUCTOR(name, __VA_ARGS__) \
    SAPPHIRE_AST_DEF_ASSIGN_OP(name, __VA_ARGS__) \
    SAPPHIRE_AST_DEF_CONVERTER(__VA_ARGS__) \
}


SAPPHIRE_AST_DEF(int32_value_t, int);
SAPPHIRE_AST_DEF(float_value_t, float);
SAPPHIRE_AST_DEF(test_t, int32_value_t, float_value_t);
SAPPHIRE_AST_DEF(string_value_t, std::string);
SAPPHIRE_AST_DEF(func_t, string_value_t, test_t);


int main()
{
    try {
        test_t t;
        t = float_value_t(10.0f);
        t = int32_value_t(99);
        func_t f;
        f = string_value_t("hoge");
        f = t;
        f.dump();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << func_t::resolver_t::template at_v<test_t> << std::endl;
    std::cout <<
        boost::core::demangle(typeid(
            func_t::resolver_t::at_t<1>::resolver_t::template at_t<1>
        ).name())
    << std::endl;
    return EXIT_SUCCESS;
}

//  C++ language references:
//  https://msdn.microsoft.com/library/3bstk3k5.aspx
//  http://www.cplusplus.com/
//  https://isocpp.org/
//  http://www.open-std.org/jtc1/sc22/wg21/

//  Boost libraries references:
//  http://www.boost.org/doc/
