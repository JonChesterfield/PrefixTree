#include <cstddef>

extern "C" std::size_t life(const char* key)
{
  const std::size_t fail = 2;
  if (key[0] == 'd')
    {
      if (key[1] == 'e' && key[2] == 'a' && key[3] == 't' && key[4] == 'h')
        {
          return 0;
        }
      else
        {
          return fail;
        }
    }
  if (key[0] == 'l')
    {
      if (key[1] == 'i' && key[2] == 'f' && key[3] == 'e')
        {
          return 1;
        }
      else
        {
          return fail;
        }
    }

  return fail;
}
