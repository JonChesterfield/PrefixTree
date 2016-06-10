# PrefixTree
A compile time (C++11) associative array for prefix lookups.

This is an implementation of an associative array for prefix lookups.

Given a compile time table of prefixes and a runtime string, this datastructure will return an iterator (or index) to the matching prefix.

Open problems:

  Signed chars are a pain re: sorting

  Template depth - either the table size is limited to 256, which is far too small, or we can't recurse through it linearly. 

  Compile time