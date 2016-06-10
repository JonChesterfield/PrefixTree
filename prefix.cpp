/*
 * This file is part of PrefixTree
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Jon Chesterfield
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#define PREFIX_TESTING_ACCESS
#include "prefix.hpp"
#include "string.hpp"
#include "catch.hpp"

TEST_CASE("none") { CHECK(true); }

struct simple : prefix::crtp<simple>
{
  static constexpr element table[] = {
      "barz", "foo", "wombat",
  };
};
constexpr decltype(simple::table) simple::table;
static_assert(simple::size() == 3, "");
static_assert(simple::ordered<0, simple::size()>(), "Ordered");

TEST_CASE("simple table")
{
  static_assert(3 == simple::size(), "");
  static_assert(str_const("barz") == simple::get<0>(), "");
  static_assert(str_const("foo") == simple::get<1>(), "");
  static_assert(str_const("wombat") == simple::get<2>(), "");

  CHECK(0 == simple::lookup("barz"));
  CHECK(1 == simple::lookup("foo"));
  CHECK(2 == simple::lookup("wombat"));
  CHECK(3 == simple::lookup("badger"));
}

TEST_CASE("max size")
{
  static_assert(4 == simple::max_key_size<0, 1>(), "");
  static_assert(4 == simple::max_key_size<0, 2>(), "");
  static_assert(6 == simple::max_key_size<0, 3>(), "");

  static_assert(3 == simple::max_key_size<1, 2>(), "");
  static_assert(6 == simple::max_key_size<1, 3>(), "");

  static_assert(6 == simple::max_key_size<2, 3>(), "");
}

TEST_CASE("find_upper_bound")
{
  static_assert(3 == simple::find_upper_bound<0, 3, 0, 'z'>(), "");
  static_assert(3 == simple::find_upper_bound<0, 3, 0, '\0'>(), "");

  static_assert(1 == simple::find_upper_bound<0, 3, 0, 'b'>(), "");
  static_assert(2 == simple::find_upper_bound<0, 3, 0, 'f'>(), "");
  static_assert(3 == simple::find_upper_bound<0, 3, 0, 'w'>(), "");

  static_assert(1 == simple::find_upper_bound<0, 3, 1, 'a'>(), "");
  static_assert(3 == simple::find_upper_bound<0, 3, 1, 'o'>(), "");

  static_assert(1 == simple::find_upper_bound<0, 3, 2, 'r'>(), "");
  static_assert(2 == simple::find_upper_bound<0, 3, 2, 'o'>(), "");
  static_assert(3 == simple::find_upper_bound<0, 3, 2, 'm'>(), "");

  static_assert(1 == simple::find_upper_bound<0, 3, 3, 'z'>(), "");
  static_assert(3 == simple::find_upper_bound<0, 3, 3, 'b'>(), "");
  static_assert(3 == simple::find_upper_bound<0, 3, 4, 'a'>(), "");
  static_assert(3 == simple::find_upper_bound<0, 3, 5, 't'>(), "");
}

TEST_CASE("find_lower_bound")
{
  static_assert(3 == simple::find_lower_bound<0, 3, 0, 'z'>(), "");
  static_assert(3 == simple::find_lower_bound<0, 3, 0, '\0'>(), "");

  static_assert(0 == simple::find_lower_bound<0, 3, 0, 'b'>(), "");
  static_assert(1 == simple::find_lower_bound<0, 3, 0, 'f'>(), "");
  static_assert(2 == simple::find_lower_bound<0, 3, 0, 'w'>(), "");

  static_assert(0 == simple::find_lower_bound<0, 3, 1, 'a'>(), "");
  static_assert(1 == simple::find_lower_bound<0, 3, 1, 'o'>(), "");

  static_assert(0 == simple::find_lower_bound<0, 3, 2, 'r'>(), "");
  static_assert(1 == simple::find_lower_bound<0, 3, 2, 'o'>(), "");
  static_assert(2 == simple::find_lower_bound<0, 3, 2, 'm'>(), "");

  static_assert(0 == simple::find_lower_bound<0, 3, 3, 'z'>(), "");
  static_assert(2 == simple::find_lower_bound<0, 3, 3, 'b'>(), "");
  static_assert(2 == simple::find_lower_bound<0, 3, 4, 'a'>(), "");
  static_assert(2 == simple::find_lower_bound<0, 3, 5, 't'>(), "");
}

struct single_zero : prefix::crtp<single_zero, double>
{
  static constexpr element table[] = {{"", 0}};
};
constexpr decltype(single_zero::table) single_zero::table;
static_assert(single_zero::ordered<0, single_zero::size()>(), "Ordered");

TEST_CASE("the empty string is a valid prefix for any string")
{
  constexpr std::size_t imax =
      single_zero::max_key_size<0, single_zero::size()>();
  static_assert(imax == 0, "");
  CHECK(0 == (single_zero::scan<0, single_zero::size(), 0, imax>("")));
  CHECK(0 == (single_zero::scan<0, single_zero::size(), 0, imax>("a")));
  CHECK(0 == (single_zero::scan<0, single_zero::size(), 0, imax>("\0")));
  CHECK(0 == (single_zero::scan<0, single_zero::size(), 0, imax>("ab")));
}

struct single_single : prefix::crtp<single_single, str_const>
{
  static constexpr element table[] = {{"p", "foo"}};
};
constexpr decltype(single_single::table) single_single::table;
static_assert(single_single::ordered<0, single_single::size()>(), "Ordered");

TEST_CASE("scan single row single character")
{
  constexpr std::size_t imax =
      single_single::max_key_size<0, single_single::size()>();
  static_assert(imax == 1, "");
  CHECK(0 == (single_single::scan<0, single_single::size(), 0, imax>("prep")));
  CHECK(1 == (single_single::scan<0, single_single::size(), 0, imax>("")));
  CHECK(1 == (single_single::scan<0, single_single::size(), 0, imax>("fred")));
  CHECK("foo" == *single_single::lookup("p"));
  CHECK(single_single::end() == single_single::lookup("X"));
}

struct single_double : prefix::crtp<single_double, int>
{
  static constexpr element table[] = {{"ok", 0}};
};
constexpr decltype(single_double::table) single_double::table;
static_assert(single_double::ordered<0, single_double::size()>(), "Ordered");

TEST_CASE("scan single row double character")
{
  constexpr std::size_t imax =
      single_double::max_key_size<0, single_double::size()>();

  static_assert(imax == 2, "");
  CHECK(0 == (single_double::scan<0, single_double::size(), 0, imax>("ok")));
  CHECK(0 == (single_double::scan<0, single_double::size(), 0, imax>("oke")));

  CHECK(0 == (single_double::scan<0, single_double::size(), 1, imax>("xk")));
  CHECK(0 == (single_double::scan<0, single_double::size(), 1, imax>("xke")));

  CHECK(1 == (single_double::scan<0, single_double::size(), 0, imax>("")));
  CHECK(1 == (single_double::scan<0, single_double::size(), 0, imax>("o")));
  CHECK(1 == (single_double::scan<0, single_double::size(), 0, imax>("ko")));
}

struct double_quad : prefix::crtp<double_quad, int>
{
  static constexpr element table[] = {{"fail", 0}, {"ok", 0}};
};
constexpr decltype(double_quad::table) double_quad::table;
static_assert(double_quad::ordered<0, double_quad::size()>(), "Ordered");

TEST_CASE("scan two rows, four characters")
{
  constexpr std::size_t sz = double_quad::size();
  static_assert(sz == 2, "");

  CHECK(4 == (double_quad::max_key_size<0, 1>()));
  CHECK(2 == (double_quad::max_key_size<1, 2>()));
  CHECK(4 == (double_quad::max_key_size<0, 2>()));

  constexpr std::size_t imax =
      double_quad::max_key_size<0, double_quad::size()>();
  static_assert(imax == 4, "");

  CHECK(imax == 4);
  CHECK(2 == (double_quad::scan<0, 1, 0, 4>("fai")));
  CHECK(0 == (double_quad::scan<0, 1, 0, 4>("fail")));
  CHECK(0 == (double_quad::scan<0, 1, 0, 4>("failX")));
  CHECK(0 == (double_quad::scan<0, 1, 0, 4>("fail\0")));

  CHECK(2 == (double_quad::scan<1, 2, 0, 2>("o")));
  CHECK(1 == (double_quad::scan<1, 2, 0, 2>("ok")));
  CHECK(1 == (double_quad::scan<1, 2, 0, 2>("okX")));

  CHECK(0 == (double_quad::scan<0, 1, 0, 4>("fail")));
  CHECK(1 == (double_quad::scan<0, sz, 0, 4>("ok")));
}

TEST_CASE("lookup")
{
  CHECK(0 == double_quad::lookup_index("fail"));
  CHECK(1 == double_quad::lookup_index("ok"));
  CHECK(2 == double_quad::lookup_index(""));
  CHECK(2 == double_quad::lookup_index("foo"));
}

struct simple_prefix : prefix::crtp<simple_prefix, int>
{
  static constexpr element table[] = {{"bar", 0}, {"foo", 0}};
};
constexpr decltype(simple_prefix::table) simple_prefix::table;
static_assert(simple_prefix::size() == 2, "");
static_assert(simple_prefix::ordered<0, simple_prefix::size()>(), "Ordered");

TEST_CASE("prefix")
{
  SECTION("Strings match if equal to or longer than those in the table")
  {
    CHECK(1 == simple_prefix::lookup_index("foo"));
    CHECK(1 == simple_prefix::lookup_index("foos"));
    CHECK(0 == simple_prefix::lookup_index("bars"));
    CHECK(1 == simple_prefix::lookup_index("foobar"));
    CHECK(0 == simple_prefix::lookup_index("barfoo"));
  }

  SECTION("But don't match if they're shorter")
  {
    CHECK(2 == simple_prefix::lookup_index("f"));
    CHECK(2 == simple_prefix::lookup_index("fo"));
  }
}

struct unordered : prefix::crtp<unordered, int>
{
  static constexpr element table[] = {
      {"foo", 0}, {"bar", 1}, {"abc", 2},
  };
};
constexpr decltype(unordered::table) unordered::table;
static_assert(3 == unordered::size(), "");

TEST_CASE("unordered")
{
  static_assert(unordered::element("foo", 0) == unordered::table[0], "");
  static_assert(unordered::element("bar", 1) == unordered::table[1], "");
  static_assert(unordered::element("abc", 2) == unordered::table[2], "");
  static_assert(unordered::size() == 3, "");
  static_assert(str_const("foo") == unordered::get<0>(), "");
  static_assert(str_const("bar") == unordered::get<1>(), "");
  static_assert(str_const("abc") == unordered::get<2>(), "");

  // Fields compare as expected
  static_assert(unordered::get<0>() > unordered::get<1>(), "");
  static_assert(unordered::get<1>() > unordered::get<2>(), "");
  static_assert(unordered::get<0>() > unordered::get<2>(), "");

  // Table as a whole claims to be unordered
  static_assert(!(unordered::ordered<0, unordered::size()>()),
                "Unexpectedly ordered");

  // Table in pairs also claims to be unordered
  static_assert(!(unordered::ordered<0, 2>()), "Unexpectedly ordered");
  static_assert(!(unordered::ordered<1, 3>()), "Unexpectedly ordered");
}

struct cinfo : prefix::crtp<cinfo, int>
{
  static constexpr element table[] = {
      {"abc", 0}, {"def", 0}, {"k", 0}, {"ok", 0},
  };
};
constexpr decltype(cinfo::table) cinfo::table;
static_assert(cinfo::ordered<0, cinfo::size()>(), "Ordered");

static_assert(cinfo::contains_char<0, cinfo::size(), 0, 'a'>() == true, "");
static_assert(cinfo::contains_char<0, cinfo::size(), 0, 'd'>() == true, "");
static_assert(cinfo::contains_char<0, cinfo::size(), 0, 'k'>() == true, "");
static_assert(cinfo::contains_char<0, cinfo::size(), 0, 'o'>() == true, "");
static_assert(cinfo::contains_char<0, cinfo::size(), 1, 'b'>() == true, "");
static_assert(cinfo::contains_char<0, cinfo::size(), 1, 'e'>() == true, "");
static_assert(cinfo::contains_char<0, cinfo::size(), 1, 'k'>() == true, "");
static_assert(cinfo::contains_char<0, cinfo::size(), 2, 'c'>() == true, "");
static_assert(cinfo::contains_char<0, cinfo::size(), 2, 'f'>() == true, "");

static_assert(cinfo::contains_char<0, cinfo::size() - 1, 0, 'o'>() == false,
              "");
static_assert(cinfo::contains_char<0, cinfo::size(), 1, 'o'>() == false, "");
static_assert(cinfo::contains_char<0, cinfo::size(), 2, '\0'>() == false, "");

TEST_CASE("smallest at I")
{
  static_assert(cinfo::smallest_char<0, cinfo::size(), 0>() == 'a', "");
  static_assert(cinfo::smallest_char<0, cinfo::size(), 1>() == 'b', "");
  static_assert(cinfo::smallest_char<0, cinfo::size(), 2>() == 'c', "");

  static_assert(cinfo::smallest_char<1, cinfo::size(), 0>() == 'd', "");
  static_assert(cinfo::smallest_char<2, cinfo::size(), 0>() == 'k', "");
  static_assert(cinfo::smallest_char<3, cinfo::size(), 0>() == 'o', "");

  static_assert(cinfo::smallest_char<1, cinfo::size(), 1>() == 'e', "");
  static_assert(cinfo::smallest_char<2, cinfo::size(), 1>() == 'k', "");
  static_assert(cinfo::smallest_char<3, cinfo::size(), 1>() == 'k', "");

  static_assert(cinfo::smallest_char<1, cinfo::size() - 1, 0>() == 'd', "");
  static_assert(cinfo::smallest_char<1, cinfo::size() - 1, 1>() == 'e', "");
}

TEST_CASE("largest at I")
{
  static_assert(cinfo::largest_char<0, cinfo::size(), 0>() == 'o', "");
  static_assert(cinfo::largest_char<0, cinfo::size(), 1>() == 'k', "");
  static_assert(cinfo::largest_char<0, cinfo::size(), 2>() == 'f', "");

  static_assert(cinfo::largest_char<0, cinfo::size() - 1, 0>() == 'k', "");
  static_assert(cinfo::largest_char<0, cinfo::size() - 2, 0>() == 'd', "");
  static_assert(cinfo::largest_char<0, cinfo::size() - 3, 0>() == 'a', "");

  static_assert(cinfo::largest_char<1, cinfo::size() - 1, 1>() == 'e', "");
  static_assert(cinfo::largest_char<1, cinfo::size() - 2, 1>() == 'e', "");

  static_assert(cinfo::largest_char<1, cinfo::size() - 1, 0>() == 'k', "");
  static_assert(cinfo::largest_char<1, cinfo::size() - 1, 1>() == 'e', "");
}

#if 0
struct big_table : prefix::crtp<big_table>
{
  static constexpr element table[] = {
      "alignas (since C++11)",   "alignof (since C++11)",
      "and",                     "and_eq",
      "asm",                     "auto(1)",
      "bitand",                  "bitor",
      "bool",                    "break",
      "case",                    "catch",
      "char",                    "char16_t (since C++11)",
      "char32_t (since C++11)",  "class",
      "compl",                   "concept (concepts TS)",
      "const",                   "const_cast",
      "constexpr (since C++11)", "continue",
      "decltype (since C++11)",  "default(1)",
      "delete(1)",               "do",
      "double",                  "dynamic_cast",
      "else",                    "enum",
      "explicit",                "export(1)",
      "extern",                  "false",
      "float",                   "for",
      "friend",                  "goto",
      "if",                      "inline",
      "int",                     "long",
      "mutable",                 "namespace",
      "new",                     "noexcept (since C++11)",
      "not",                     "not_eq",
      "nullptr (since C++11)",   "operator",
      "or",                      "or_eq",
      "private",                 "protected",
      "public",                  "register",
      "reinterpret_cast",        "requires (concepts TS)",
      "return",                  "short",
      "signed",                  "sizeof",
      "static",                  "static_assert (since C++11)",
      "static_cast",             "struct",
      "switch",                  "template",
      "this",                    "thread_local (since C++11)",
      "throw",                   "true",
      "try",                     "typedef",
      "typeid",                  "typename",
      "union",                   "unsigned",
      "using(1)",                "virtual",
      "void",                    "volatile",
      "wchar_t",                 "while",
      "xor",                     "xor_eq",
  };
};
constexpr decltype(big_table::table) big_table::table;

TEST_CASE("minimal")
{
  CHECK(big_table::lookup_index("register") != big_table::size());
}
#endif

struct signed_regression : prefix::crtp<signed_regression, uint64_t>
{
  // A failure case found by fuzz testing (signed char comparison)
  static constexpr element table[] = {
      {"\x10\x40\x80\x70\x40\x30\x60", 8409224550904534268u},
      {"\x30\x70\x10\x80\x50\x10", 1019428663781356102u},
      {"\x40\x20\x50\x40\x30\x10\x10\x50", 425806957580378887u},
      {"\x80\x10\x10\x40", 14276665825079423988u},
      {"\x80\x50\x30\x50", 11020186555195962799u},
  };
};
constexpr decltype(signed_regression::table) signed_regression::table;
static_assert(signed_regression::ordered<0, signed_regression::size()>(),
              "Ordered");

TEST_CASE("signed_regression")
{
  CHECK(0 == signed_regression::lookup_index("\x10\x40\x80\x70\x40\x30\x60"));
  CHECK(1 == signed_regression::lookup_index("\x30\x70\x10\x80\x50\x10"));
  CHECK(2 ==
        signed_regression::lookup_index("\x40\x20\x50\x40\x30\x10\x10\x50"));
  CHECK(3 == signed_regression::lookup_index("\x80\x10\x10\x40"));
  CHECK(4 == signed_regression::lookup_index("\x80\x50\x30\x50"));

  CHECK(8409224550904534268u ==
        *signed_regression::lookup("\x10\x40\x80\x70\x40\x30\x60"));
  CHECK(1019428663781356102u ==
        *signed_regression::lookup("\x30\x70\x10\x80\x50\x10"));
  CHECK(425806957580378887u ==
        *signed_regression::lookup("\x40\x20\x50\x40\x30\x10\x10\x50"));
  CHECK(14276665825079423988u ==
        *signed_regression::lookup("\x80\x10\x10\x40"));
  CHECK(11020186555195962799u ==
        *signed_regression::lookup("\x80\x50\x30\x50"));
}

struct common_prefix_regression
    : prefix::crtp<common_prefix_regression, uint64_t>
{
  // A failure case found by fuzz testing (common prefix in table)
  static constexpr element table[] = {
      {"\x9a\x50\x85\xa4", 2430525381136175272u},
      {"\xb9", 15886070940351924572u},
      {"\xb9\x85", 16229721192883529338u},
  };
};
constexpr decltype(
    common_prefix_regression::table) common_prefix_regression::table;
static_assert(
    common_prefix_regression::ordered<0, common_prefix_regression::size()>(),
    "Ordered");

TEST_CASE("common_prefix_regression")
{
  CHECK(0 == common_prefix_regression::lookup_index("\x9a\x50\x85\xa4"));
  // CHECK(1 == common_prefix_regression::lookup_index("\xb9"));
  CHECK(2 == common_prefix_regression::lookup_index("\xb9\x85"));

  CHECK(2430525381136175272u ==
        *common_prefix_regression::lookup("\x9a\x50\x85\xa4"));
  // CHECK(15886070940351924572u == *common_prefix_regression::lookup("\xb9"));
  CHECK(16229721192883529338u == *common_prefix_regression::lookup("\xb9\x85"));
}
