
// Mike:  sorry about moving global.h outside of ARCH_ENGINE, but it made
// precompiled headers easier.  Can you just add an empty global.h into
// the exporter projects?

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#ifdef ARCH_ENGINE
	#include "oserrmsg.h"
#else
	#ifndef TARGET_GC
		#define nglPrintf printf
	#endif
#endif

#include <ctype.h>
#include "pstring.h"

#ifdef PROJECT_KELLYSLATER
#include "stringx.h"
#endif

char pstring::output_cache[PSTRING_CACHE_SIZE][PSTRING_MAX_LENGTH_PLUS_ONE];
unsigned pstring::output_index = 0;


/* The 'alphabet' of a pstring */
const char pstring::pc_to_ascii[64] =
{ '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', ' ', '_', ':', '"', '\'',
   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',  'P',
   'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '(', ')', '[', ']', '<',  '>',
   '=', '+', '-', '*', '/', '%', '&', '|', '!', '#', '$', '?', '.', ',', ';', '\\'
};


/* The table is used to translate between 7-bit ascii characters to 6 bit characters
 * all illegal characters are set to 0xff (-1).
 */
const char pstring::ascii_to_pc[128] =
{
    0,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
   11,  56,  14,  57,  58,  53,  54,  15,  42,  43,  51,  49,  61,  50,  60,  52,
   10,   1,   2,   3,   4,   5,   6,   7,   8,   9,  13,  62,  46,  48,  47,  59,
   -1,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,
   31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  44,  63,  45,  -1,  12,
   -1,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,
   31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  -1,  55,  -1,  -1,  -1
};



//#ifdef ARCH_ENGINE
pstring::pstring(const stringx &target_string)
{
	pack_string(target_string.c_str());
}

pstring& pstring::operator = (const stringx &str)
{
  pack_string(str.c_str());
  return *this;
}

bool pstring::operator == (const stringx &check_me) const
{
  return(*this == pstring(check_me));
}
//#endif



/*** pack_string ***/
void pstring::pack_string(const char *the_string)
{
  int shift;
  int i=0, k=0;
  int end_found=0;

  // Convert over the characters

  for (unsigned j=0; j<PSTRING_PCHUNK_NUMBER; ++j)
  {
    shift = PSTRING_SHIFT_START;
    pchunk[j] = 0;
    if (end_found)
      continue;

    while (shift >= PSTRING_PCHUNK_LENGTH_BITS)
    {
      if (the_string[i] != '\0')
      {
        int temp = ascii_to_pc[the_string[i]];

        // Check for illegal characters
			  if ( temp == -1 )
          error( "Character cannot be packed: %c (%s)", the_string[i], the_string );

        // check the table, make sure it's right
        assert (toupper(the_string[i]) == pc_to_ascii[temp]);

        pchunk[j] |= uint64(temp) << shift;
      }
      else
      {
        end_found = 1;
        break;
      }

	    shift -= PSTRING_SHIFT_BY;
	    ++i;
    }

    // store the length of the pchunk
    k = i - (j * PSTRING_PCHUNK_LENGTH);

    assert(k <= PSTRING_PCHUNK_LENGTH);
    pchunk[j] |= k;
  }

  if (end_found == 0)
  {
    // Catch strings that are too long, might want to be more forgiving and make it just
    // a warning and not an assert
    unsigned len = strlen(the_string);
//    if (len > PSTRING_MAX_LENGTH)
    if (the_string[i] != '\0')
      debug_print("String is too long (%d) '%s'\n", len, the_string);
  }
}


/*** unpack_string ***/
const char * pstring::unpack_string() const
{
	int shift;
	unsigned i=0;

  // Convert over the characters
  unsigned temp_chr;

  for (unsigned j=0; j<PSTRING_PCHUNK_NUMBER; ++j)
  {
    shift = PSTRING_SHIFT_START;

	  while (shift >= PSTRING_PCHUNK_LENGTH_BITS)
    {
      temp_chr = char(pchunk[j] >> shift) & (PSTRING_PCHAR_MASK);

      assert(temp_chr < 64);  // 64 characters in the pstring alphabet
      output_cache[output_index][i] = pc_to_ascii[temp_chr];

      if (output_cache[output_index][i] == '\0')
        break;
	    shift -= PSTRING_SHIFT_BY;
      ++i;
    }
  }

  output_cache[output_index][i] = '\0';
  char * ret_val = output_cache[output_index];

  output_index++;
  if (output_index >= PSTRING_CACHE_SIZE)
    output_index = 0;

  return (const char *)ret_val;
}

bool pstring::concatinate(const char *the_string)
{
  unsigned len;
  unsigned i, j;
  int shift;
  unsigned end_found = 0;
  assert(the_string != NULL);

  // catch strs that are too long
  if (strlen(the_string) + length() > PSTRING_MAX_LENGTH)
    return false;

  for (j=0; j<PSTRING_PCHUNK_NUMBER; ++j)
  {
    len = unsigned (pchunk[j] & PSTRING_PCHUNK_LENGTH_MASK);
    if (len != PSTRING_PCHUNK_LENGTH)
      break;
  }

  i = 0;
  pchunk[j] &= ~PSTRING_PCHUNK_LENGTH_MASK;
  shift = PSTRING_SHIFT_START - (len * PSTRING_SHIFT_BY);

  for (; j<PSTRING_PCHUNK_NUMBER; ++j, shift = PSTRING_SHIFT_START, len=0)
  {
    while (shift >= PSTRING_PCHUNK_LENGTH_BITS)
    {
      if (the_string[i] != '\0')
      {
        int temp = ascii_to_pc[the_string[i]];

        // Check for illegal characters
			  if ( temp == -1 )
          error( "Character cannot be packed: %c (%s)", the_string[i], the_string );

        // check the table, make sure it's right
        assert (toupper(the_string[i]) == pc_to_ascii[temp]);

        pchunk[j] |= uint64(temp) << shift;
      }
      else
      {
        // signal a break out value
        end_found = 1;
        break;
      }

	    shift -= PSTRING_SHIFT_BY;
	    ++i;
      ++len;
    }

    // store the length of the pchunk
    assert(len <= PSTRING_PCHUNK_LENGTH);
    pchunk[j] |= len;
    if (end_found == 1)
      break;
  }

  if (end_found == 0)
  {
    // Catch strings that are too long, might want to be more forgiving and make it just
    // a warning and not an assert
//    if (len > PSTRING_MAX_LENGTH)
    if (the_string[i] != '\0')
    {
      debug_print("String '%s' is too long to concatonate onto '%s'\n", the_string, this->c_str());
      return false;
    }
  }

  return true;
}


#if 0 //#ifdef PSTRING_UNIT_TESTER
// main() for unit-testing pstring

int main()
{
  pstring joe("joe");
  printf("Joe is named(%d) %s\n", joe.length(), joe.c_str());

  pstring max1("1234567890abcdefghijklmnopqrstuvwxyz12345");
  printf("Max length + 1 test(%d) %s\n", max1.length(), max1.c_str());

  pstring max2("1234567890abcdefghijklmnopqrstuvwxyz1234");
  printf("Max length test(%d) %s\n", max2.length(), max2.c_str());

  if (max1 == max2)
    printf("This should happen (max1 == max2)\n");
  else
    printf("This shouldn't happen (max1 == max2)!!\n");

  if (max1 != joe)
    printf("This should happen (max1 != joe)\n");
  else
    printf("This shouldn't happen (max1 != joe)!!\n");

  if (max1 > joe)
    printf("This shouldn't happen (max1 > joe)!!\n");
  else
    printf("This should happen (max1 > joe)\n");

  if (max1 < joe)
    printf("This should happen (max1 < joe)\n");
  else
    printf("This shouldn't happen (max1 < joe)!!\n");

  if (max1 >= max2)
    printf("This should happen (max1 >= max2)\n");
  else
    printf("This shouldn't happen (max1 >= max2)!!\n");

  if (max1 > max2)
    printf("This shouldn't happen (max1 > max2)!!\n");
  else
    printf("This should happen (max1 > max2)\n");

  pstring cat;
  cat = "I";
  cat.concatinate(" am");
  cat.concatinate(" a");
  cat.concatinate(" concatinated");
  cat.concatinate(" string and I am neat");
  printf("%s (%d)\n", cat.c_str(), cat.length());

  cat.crop('i');

  printf("%s (%d)\n", cat.c_str(), cat.length());

  return 0;
}

//#endif

#endif
