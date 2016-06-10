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
#include "string.hpp"
#include "catch.hpp"

template <char C>
char get()
{
  return C;
}

TEST_CASE("str_const is usable at compile time")
{
  constexpr str_const s = "foobar";
  static_assert(6 == s.size(), "");
  static_assert(s[0] == 'f', "");
  static_assert(s[1] == 'o', "");
  static_assert(s[2] == 'o', "");
  static_assert(s[3] == 'b', "");
  static_assert(s[4] == 'a', "");
  static_assert(s[5] == 'r', "");
}

TEST_CASE("equality")
{
  constexpr str_const f = "foo";
  constexpr str_const b = "bar";

  static_assert(f == f, "");
  static_assert(b == b, "");
  static_assert(f != b, "");

  static_assert(f == "foo", "");
  static_assert(f != "foo\0", "");
  static_assert(f != "fo\0", "");
}

TEST_CASE("total order on single character")
{
  constexpr str_const z = "\x00";
  constexpr str_const o = "\x01";
  static_assert(1 == z.size(), "");
  static_assert(1 == o.size(), "");

  static_assert(z != o, "");

  static_assert(!(z < z), "");
  static_assert(!(o < o), "");

  static_assert(z <= z, "");
  static_assert(o <= o, "");

  static_assert(z < o, "");
  static_assert(z <= o, "");
  static_assert(o > z, "");
  static_assert(o >= z, "");
}

TEST_CASE("end of string is smaller than any character")
{
  constexpr str_const foo = "foo";
  constexpr str_const foobar = "foo\x00";

  static_assert(foo < foobar, "");
  static_assert(foobar < "foobar2", "");
  static_assert("fo" < foobar, "");

  static_assert(foo <= foo, "");
  static_assert(!(foo < foo), "");
}

TEST_CASE("strings of same length are sorted numerically, characterwise")
{
  constexpr str_const t = "\x05\x06\x07";
  static_assert(t < "\x06\x06\x07", "");
  static_assert(t < "\x05\x07\x07", "");
  static_assert(t < "\x05\x06\x08", "");
  static_assert("\x04\x06\x07" < t, "");
  static_assert("\x05\x05\x07" < t, "");
  static_assert("\x05\x06\x06" < t, "");
}

TEST_CASE("shorter strings with higher prefix are greater")
{
  constexpr str_const t = "\x05\x06\x07";
  static_assert(t < "\x06","");
  static_assert(t < "\x08","");
  static_assert(t < "\x05\x07","");
  static_assert(t < "\x05\x06\x08","");
  static_assert("\x04" < t,"");
  static_assert("\x05\x05" < t,"");
  static_assert("\x05\x06\0x05" < t,"");
}

TEST_CASE("regression cases")
{
  static_assert(str_const("\x30\x80\x30\x80\x30\x10") <
		str_const("\x80\x40\x10\x10\x10\x70\x10"),"");
}
