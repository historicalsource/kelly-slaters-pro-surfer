#ifndef VECTORX_H
#define VECTORX_H
#if 0
////////////////////////////////////////////////////////////////////////////////
/*
  VECTORX

  we've got our own STRINGX...it's time for VECTORX!
*/
////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <vector>

extern vector<void*> global_vectorx_list;

template<class _Ty,class _A = allocator<_Ty> >
  class vectorx : public vector<_Ty,_A>
  {
  public:
    explicit vectorx(const char* _file,int _line,const _A& _Al = _A())
		  : vector<_Ty,_A>(_Al) 
        {
#ifndef NDEBUG
        line = _line;
        strcpy( file, _file );
        global_vectorx_list.push_back( this );
#endif        
        }
/* gnu doesn't like this

    typedef vector<_Ty,_A>::size_type vector_size_t;

    explicit vectorx(const char* _file,int _line,vector_size_t _N, const _Ty& _V = _Ty(), const _A& _Al = _A())
		  : vector<_Ty,_A>( _N, _V, _Al )
        {
#ifndef NDEBUG
        line = _line;
        strcpy( file, _file );
        global_vectorx_list.push_back( this );
#endif
        }

    vectorx(const char* _file,int _line,const vector<_Ty,_A>::_Myt& _X)
		  : vector<_Ty,_A>( _X )
		    {
#ifndef NDEBUG
        line = _line;
        strcpy( file, _file );
        global_vectorx_list.push_back( this );
#endif
        }

      vectorx(const char* _file,int _line,const vector<_Ty,_A>::_It _F, 
      vector<_Ty,_A>::_It _L, const _A& _Al = _A())
		  : vector<_Ty,_A>( _F, _L, _Al )
		    {
#ifndef NDEBUG
        line = _line;
        strcpy( file, _file );
        global_vectorx_list.push_back( this );
#endif
        }
*/
	  ~vectorx()
		    {
#ifndef NDEBUG
        vector<void*>::iterator it;
        for (it = global_vectorx_list.begin(); it != global_vectorx_list.end(); it++)
          {
          if ((*it)==this)
            {
            global_vectorx_list.erase(it);
            break;
            }
          }
#endif
        }
    static void dump_to_log(void);

  private:
#ifndef NDEBUG
#if defined(VERSION_R10) || defined(VERSION_R10_HYBRID)
	#define _MAX_PATH 512
#endif
    char file[_MAX_PATH];
    int  line;
#endif
  };

#endif
#endif
