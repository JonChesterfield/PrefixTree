// A prefix trie. Use example:

#include "prefix.hpp"
#include "catch.hpp"
#include <string>

struct example : prefix::crtp<example, std::size_t>
{
  static constexpr element table[] = {
      {"barz", 4}, {"foo", 7}, {"wombat", 12},
  };
};
constexpr decltype(example::table) example::table;

TEST_CASE("inline example")
{
  static_assert(example::size() == 3, "");
  static_assert(example::ordered<0, 3>(), "");

  // Lookup returns an iterator
  CHECK(example::begin() == example::lookup("barz"));
  CHECK(example::end() == example::lookup("badger"));

  // Which dereferences to the value
  CHECK(4 == *example::lookup("barz"));
  CHECK(7 == *example::lookup("foo"));
  CHECK(12 == *example::lookup("wombat"));

  // Match is by prefix in the table
  CHECK(example::end() == example::lookup("fo"));
  CHECK(7 == *example::lookup("foo"));
  CHECK(7 == *example::lookup("foob"));
  CHECK(7 == *example::lookup("foobar"));
  CHECK(example::end() == example::lookup("wom"));

  // The alternative is lookup_index, which doesn't return an iterator
  CHECK(1 == example::lookup_index("foobar"));
  // Though the index can still be used to lookup in the table
  CHECK(7 == example::table[example::lookup_index("foobar")].value);
}

// For types that can't be put in a constexpr table
// Or for indexing external tables of data

static const std::string dyn_str[] = {
    "some bars", "and some foo", "and a wombat",
};

struct dynamic : prefix::crtp<dynamic>  // No second type parameter
{
  static constexpr element table[] = {
      "barz", "foo", "wombat",
  };
};
constexpr decltype(dynamic::table) dynamic::table;

TEST_CASE("dynamic")
{
  static_assert(dynamic::size() == 3, "");

  // Lookup returns index instead of an iterator
  CHECK(0 == dynamic::lookup("barz"));
  CHECK(1 == dynamic::lookup("foo"));
  CHECK(2 == dynamic::lookup("wombat"));

  // Which can index into the external table
  CHECK("and some foo" == dyn_str[dynamic::lookup("foo")]);

  // In this case, lookup and lookup_index are equivalent
  CHECK(dynamic::lookup_index("foobar") == dynamic::lookup("foobar"));
}
