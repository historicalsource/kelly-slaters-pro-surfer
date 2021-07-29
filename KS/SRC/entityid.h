#ifndef ENTITYID_H
#define ENTITYID_H
// entity id support

class charstarwrap
  {
  public:
    char* str;
    charstarwrap(char* s) {str=s;}
    bool operator<(const charstarwrap& csw) const
      {
      return (strcmp(str, csw.str)<0);
      }
    bool operator==(const charstarwrap& csw) const
      {
      return (strcmp(str, csw.str)==0);
      }
  };


char* strdupcpp(const char* str);

typedef map<charstarwrap,unsigned int
  , less<charstarwrap>
  #ifdef TARGET_PS2
	, malloc_alloc
	#endif
  > name_to_number_map;

#endif
