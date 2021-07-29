#ifndef SINGLETON_H
#define SINGLETON_H
////////////////////////////////////////////////////////////////////////////////

// Evidently older VC++ had problems with a static variable being the instance,
// so we are using a pointer that's dynamically allocated instead.

template <class T>
class singleton_ptr
{
  T* p;
public:
  singleton_ptr(T* ap=NULL) { p=ap; }
  void operator=(T* ap) { p=ap; }
  operator T*() const { return p; }
#ifdef DEBUG
  ~singleton_ptr()
  {
    if (p)
    {
//!      debug_print("singleton wasn't cleaned up");
      delete p;
    }
  }
#endif // DEBUG
};

// a version of singleton that gives us full control over its
// creation and deletion.

// Note that instance will NOT be available until after the object
// has been fully constructed and operator NEW returns control to
// create_inst()!!  Similar restrictions exist for delete_inst()
class singleton
{
  protected:
    singleton() {}
    virtual ~singleton() {}
  private: // no copying allowed
    singleton(const singleton&);
    singleton& operator=(const singleton&);
};

#define DECLARE_SINGLETON(classname)                                \
  private:                                                          \
    friend class singleton_ptr<classname>;                          \
    static singleton_ptr<classname> instance;                       \
  public:                                                           \
    static inline classname* inst()                                 \
    {                                                               \
      assert(instance);                                             \
      return instance;                                              \
    }                                                               \
    static inline bool is_inst()                                    \
    {                                                               \
      return instance != NULL;                                      \
    }                                                               \
    static inline void create_inst()                                \
    {                                                               \
      assert (!instance);                                           \
      instance=NEW classname;                                       \
    }                                                               \
    static inline void delete_inst()                                \
    {                                                               \
      if (instance)                                                 \
      {                                                             \
        delete instance;                                            \
        instance=NULL;                                              \
      }                                                             \
      else                                                          \
      {                                                             \
/*!        debug_print("singleton already cleaned up");      !*/    \
      }                                                             \
    }

// defines the inst() function in a class for your singleton
#define DEFINE_SINGLETON(classname)                                 \
    singleton_ptr<classname> classname::instance = NULL;


#endif // SINGLETON_H
