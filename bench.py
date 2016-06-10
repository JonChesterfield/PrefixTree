import random

def common_prefix(x,y):
    if (len(y) < len(x)):
        return False
    for i in range(0,len(x)):
        if x[i] != y[i]:
            return False
    return True
    
def would_match_on_prefix(keys,x):
    assert(common_prefix((3,),(3,4,)))
    assert(common_prefix((3,),(3,)))
    assert(common_prefix((1,),(1,1,)))
    assert(common_prefix((1,2,),(1,2,1,)))
    assert(common_prefix((2,4),(2,4,5,)))
    assert(common_prefix((1,2,),(1,2,1,)))
    assert(common_prefix((1,),(1,3,)))

    for i in keys:
        if common_prefix(i,x):
            return True
    return False

def hexlist(size):
    # Should go from 0, but I'm fairly sure my prefix tree mishandles '\0' at present
    # In particular, embedded '\0' in the const char * string are considered
    # end of string.
    return tuple([random.randint(1, 255) for x in range(size)])

def arr_hexlist(minsize, maxsize, length):
    return [hexlist(random.randint(minsize,maxsize+1)) for x in range(length+1)]

def arr_values(length):
    # Hold back 2^64-1 as a sentinel
    return [random.randint(0,pow(2,64)-2) for x in range(length+1)]

def print_hexlist_as_cstr(h):
    def tohex(val, nbits = 8):
        return hex((val + (1 << nbits)) % (1 << nbits))

    def hexasc(s):
        h = tohex(s)
        return '\\x' + h[2:].zfill(2)
    
    print('"',end='')
    for i in h:
        print("{0}".format(hexasc(i)),end='')
    print('"',end='')

def write_table(name,length,minsize,maxsize):
    potkeyset = arr_hexlist(minsize,maxsize,length)
    potkeyset.sort()
    keyset = []
    for i,v in enumerate(potkeyset[:-1]):
        if not common_prefix(potkeyset[i],potkeyset[i+1]):
            keyset.append(v)
    keyset = set(keyset)

    badkeyset = set(arr_hexlist(minsize,maxsize,length))
    badkeys = list(badkeyset - keyset)
    keys = list(keyset)
    keys.sort()

    prekeys = []
    misskeys = []

    for v in badkeys:
        if (would_match_on_prefix(keys,v)):
            prekeys.append(v)
        else:
            misskeys.append(v)

    misskeys.sort()
    prekeys.sort()
    
    # Will want prefix matching keys later
    badkeys = misskeys
    badkeys.sort()
    
    length = len(keys)
    vals = arr_values(length)

    print("""#ifdef PRE
struct {0} : prefix::crtp<{0},uint64_t>
{{
  static constexpr element table[] = {{""".format(name))

    for i in range(length):
        print("    {",end='')
        print_hexlist_as_cstr(keys[i])
        print(",{0}u}},".format(vals[i]))
    print("""  }};
}};
constexpr decltype({0}::table) {0}::table;
static_assert({0}::ordered<0, {0}::size()>(), "Ordered");
#endif

#ifdef STL

struct map_equal
{{
  bool operator()(const char * x, const char * y) const
  {{
      return strcmp(x,y) == 0;
  }}
}};

struct map_hash // djb2
{{
  std::size_t operator()(const char * str) const
  {{
   std::size_t hash = 5381;
    char c;   
    while ((c = *str++))
    {{
    hash = hash * 33 + c;
    }}
    return hash;
  }}
}};

typedef std::unordered_map<const char *,uint64_t,map_hash,map_equal> map_type;

static map_type get_stl_unordered_map_{0}()
{{
  map_type map;
""".format(name))
    for i in range(length):
        print("    map[",end="")
        print_hexlist_as_cstr(keys[i])
        print("] = {0}u;".format(vals[i]))
    print("""
  return map;
}}

static map_type stl_map = get_stl_unordered_map_{0}();
#endif
""".format(name))

    print("""
#ifdef PRE
uint64_t {0}_lookup_prefix(const char * key)
{{
  auto search = {0}::lookup(key);
  if (search == {0}::end())
  {{
    return std::numeric_limits<uint64_t>::max();
  }}
  else
  {{
    return *search;
  }}
}}
#endif
#ifdef STL
uint64_t {0}_lookup_stl(const char * str)
{{
  auto search = stl_map.find(str);
  if (search == stl_map.end())
  {{
    return std::numeric_limits<uint64_t>::max();
  }}
  else
  {{
    return search->second;
  }}
}}
#endif
""".format(name))

    if True:
        print("""// Correctness sanity check
    void {0}_sanity()
{{""".format(name))
        for i in range(length):
            print('''  {
    const char * key = ''',end='')
            print_hexlist_as_cstr(keys[i])
            print('''; // Present
#ifdef PRE
    assert({1}u == {0}_lookup_prefix(key));
#endif
#ifdef STL
    assert({1}u == {0}_lookup_stl(key));
#endif
  }}'''.format(name,vals[i]))

        for i in range(len(badkeys)):
            print('''  {
    const char * key = ''',end='')
            print_hexlist_as_cstr(badkeys[i])
            print('''; // Absent
#ifdef PRE
    assert({1}u == {0}_lookup_prefix(key));
#endif
#ifdef STL
    assert({1}u == {0}_lookup_stl(key));
#endif
  }}'''.format(name,pow(2,64)-1))
        
        print('}\n')

    
    def lookup_every_string(tag,func,keylist):
        print("""void {0}_lookup_every_{1}_{2}()
{{""".format(name,tag,func))
        for k in keylist:
            print('''  value_sink({0}_lookup_{2}('''.format(name,tag, func),end='')
            print_hexlist_as_cstr(k)
            print('''));''')
        print('}\n')
    print('#ifdef PRE')
    lookup_every_string("successful","prefix",keys)
    lookup_every_string("failing","prefix",badkeys)
    print('#endif //PRE')
    print('#ifdef STL')
    lookup_every_string("successful","stl",keys)
    lookup_every_string("failing","stl",badkeys)
    print('#endif //STL')


minsize = 1
maxsize = 32
length = 50

print('''#include "prefix.hpp"
#include <cstring>
#include <cassert>
#include <unordered_map>

#ifndef PRE
#ifndef STL
#error "require at least one of PRE and STL to be defined"
#endif
#endif

void value_sink(uint64_t);

''')
write_table("gen",length,minsize,maxsize)
