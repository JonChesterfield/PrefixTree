#include <cstdint>

void gen_sanity();

#ifdef PRE
void gen_lookup_every_successful_prefix();
void gen_lookup_every_failing_prefix();
#endif
#ifdef STL
void gen_lookup_every_successful_stl();
void gen_lookup_every_failing_stl();
#endif

#if defined (PRE) && defined (STL)
void gen_sanity();
#endif

void value_sink(uint64_t) {}

int main()
{
#if defined (PRE) && defined (STL)
  gen_sanity();
#endif
  
  for (volatile int i = 0; i < 1000000; i++)
    {
#ifdef PRE
#ifdef FAILING
      gen_lookup_every_failing_prefix();
#else
      gen_lookup_every_successful_prefix();
#endif
#endif
#ifdef STL
#ifdef FAILING
      gen_lookup_every_failing_stl();
#else
      gen_lookup_every_successful_stl();
#endif
#endif
    }
}
