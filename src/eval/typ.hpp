#pragma once

#include "grammar/parsing.hpp"
#include "style/segment.hpp"

#include <variant>      // variant_alternative_t, variant_size_v,
                        // holds_alternative
#include <algorithm>    // min
#include <optional>     // optional

using evaluated = std::variant<fail,
                               bool,
                               unsigned,
                               std::string,
                               color,
                               sep,
                               segment,
                               theme,
                               std::optional<fail>,
                               std::optional<bool>,
                               std::optional<unsigned>,
                               std::optional<std::string>,
                               std::optional<color>,
                               std::optional<sep>,
                               std::optional<segment>,
                               std::optional<theme>>;

inline bool is_segment(const evaluated& elem)
{
    return std::holds_alternative<segment>(elem);
}

inline bool is_fail(const evaluated& elem)
{
    return std::holds_alternative<fail>(elem);
}

// TODO: Refactor this.
enum class typ : unsigned
{
    fail     = 0,
    boolean  = 1,
    integer  = 2,
    string   = 3,
    color    = 4,
    sep      = 5,
    segment  = 6,
    theme    = 7,
    any      = 64,

    optional_fail     = 8 + 0,
    optional_boolean  = 8 + 1,
    optional_integer  = 8 + 2,
    optional_string   = 8 + 3,
    optional_color    = 8 + 4,
    optional_sep      = 8 + 5,
    optional_segment  = 8 + 6,
    optional_theme    = 8 + 7,
    optional_any      = 8 + 64,
};

constexpr typ optional(typ t)
{
    switch (t)
    {
        case typ::fail:    return typ::optional_fail;
        case typ::boolean: return typ::optional_boolean;
        case typ::integer: return typ::optional_integer;
        case typ::string:  return typ::optional_string;
        case typ::color:   return typ::optional_color;
        case typ::sep:     return typ::optional_sep;
        case typ::segment: return typ::optional_segment;
        case typ::theme:   return typ::optional_theme;
        case typ::any:     return typ::optional_any;
        default: return t;
    }
}

constexpr bool is_optional(typ t)
{
    switch (t)
    {
        case typ::optional_fail:
        case typ::optional_boolean:
        case typ::optional_integer:
        case typ::optional_string:
        case typ::optional_color:
        case typ::optional_sep:
        case typ::optional_segment:
        case typ::optional_theme:
        case typ::optional_any:     return true;
        default: return false;
    }
}

constexpr typ from_optional(typ t)
{
    switch (t)
    {
        case typ::optional_fail:    return typ::fail;
        case typ::optional_boolean: return typ::boolean;
        case typ::optional_integer: return typ::integer;
        case typ::optional_string:  return typ::string;
        case typ::optional_color:   return typ::color;
        case typ::optional_sep:     return typ::sep;
        case typ::optional_segment: return typ::segment;
        case typ::optional_theme:   return typ::theme;
        case typ::optional_any:     return typ::any;
        default: return t;
    }
}

inline auto typ_to_str(typ t) -> const char*
{
    switch (t)
    {
        case typ::fail:             return "fail";
        case typ::boolean:          return "bool";
        case typ::integer:          return "int";
        case typ::string:           return "str";
        case typ::color:            return "color";
        case typ::sep:              return "sep";
        case typ::segment:          return "segment";
        case typ::theme:            return "theme";
        case typ::any:              return "any";
        case typ::optional_fail:    return "[fail]";
        case typ::optional_boolean: return "[bool]";
        case typ::optional_integer: return "[int]";
        case typ::optional_string:  return "[str]";
        case typ::optional_color:   return "[color]";
        case typ::optional_sep:     return "[sep]";
        case typ::optional_segment: return "[segment]";
        case typ::optional_theme:   return "[theme]";
        case typ::optional_any:     return "[any]";
        default: return "unknown";
    }
}

inline typ typ_of(const evaluated& val)
{
    return static_cast<typ>(val.index());
}

template<typ Type>
struct get_type
{
    using type = std::conditional_t<
        Type == typ::any,
        evaluated,
        std::conditional_t< Type == typ::optional_any,
            std::optional<evaluated>,
            std::variant_alternative_t<
                std::min<unsigned>(
                    unsigned(Type),
                    std::variant_size_v<evaluated> - 1),
                evaluated>>>;
};

template<typ Type>
using get_type_t = typename get_type<Type>::type;

using eval_vec = std::vector<evaluated>;

