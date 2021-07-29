#ifndef ENTITY_INTERFACE_CLASS_HEADER
#define ENTITY_INTERFACE_CLASS_HEADER


/*** ENTITY_INTERFACE macro ***/
#define ENTITY_INTERFACE(itype)                                           \
  protected:                                                              \
    itype##_interface * my_##itype##_interface;                           \
  public:                                                                 \
    bool has_##itype##_ifc() const { return (my_##itype##_interface != NULL); }     \
    itype##_interface * itype##_ifc() const                               \
    {                                                                     \
      assert(my_##itype##_interface);                                     \
      return my_##itype##_interface;                                      \
    }                                                                     \
    itype##_interface * create_##itype##_ifc();                           \
    void destroy_##itype##_ifc();                                         \


#define ENTITY_INTERFACE_CPP(cls, itype)                                  \
    itype##_interface * cls##::create_##itype##_ifc()                     \
    {                                                                     \
      assert(!my_##itype##_interface);                                    \
      my_##itype##_interface = NEW itype##_interface(this);               \
      return my_##itype##_interface;                                      \
    }                                                                     \
    void cls##::destroy_##itype##_ifc()                                   \
    {                                                                     \
      assert(my_##itype##_interface);                                     \
      delete my_##itype##_interface;                                      \
      my_##itype##_interface = NULL;                                      \
    }                                                                     \



class pstring;
class vector3d;
/*** class entity_interface ***/
class generic_interface
{
  public:
    virtual ~generic_interface() {}

    virtual bool get_ifc_num(const pstring &att, rational_t &val)     { return(false); }
    virtual bool set_ifc_num(const pstring &att, rational_t val)      { return(false); }
    virtual bool get_ifc_vec(const pstring &att, vector3d &val)       { return(false); }
    virtual bool set_ifc_vec(const pstring &att, const vector3d &val) { return(false); }
    virtual bool get_ifc_str(const pstring &att, stringx &val)        { return(false); }
    virtual bool set_ifc_str(const pstring &att, const stringx &val)  { return(false); }
};

#define IFC_DATA_GET_NUM_MACRO(a)  if(has_##a##_ifc() && ##a##_ifc()->get_ifc_num(att, val)) return(true);
#define IFC_DATA_SET_NUM_MACRO(a)  if(has_##a##_ifc() && ##a##_ifc()->set_ifc_num(att, val)) return(true);
#define IFC_DATA_GET_VEC_MACRO(a)  if(has_##a##_ifc() && ##a##_ifc()->get_ifc_vec(att, val)) return(true);
#define IFC_DATA_SET_VEC_MACRO(a)  if(has_##a##_ifc() && ##a##_ifc()->set_ifc_vec(att, val)) return(true);
#define IFC_DATA_GET_STR_MACRO(a)  if(has_##a##_ifc() && ##a##_ifc()->get_ifc_str(att, val)) return(true);
#define IFC_DATA_SET_STR_MACRO(a)  if(has_##a##_ifc() && ##a##_ifc()->set_ifc_str(att, val)) return(true);

#define IFC_INTERNAL_GET_MACRO(a,b)       { static pstring pstring_get(##a##);  if(att == pstring_get)  { val = ##b##; return(true); } }
#define IFC_INTERNAL_SET_MACRO(a,b)       { static pstring pstring_set(##a##);  if(att == pstring_set)  { ##b## = val; return(true); } }
#define IFC_INTERNAL_SET_F2I_MACRO(a,b)   { static pstring pstring_set(##a##);  if(att == pstring_set)  { ##b## = (int) val; return(true); } }
#define IFC_INTERNAL_FUNC_MACRO(a,b)      { static pstring pstring_func(##a##); if(att == pstring_func) { ##b##; return(true); } }

#define GENERIC_INTERFACE_DEC(itype)                                      \
class itype;                                                              \
class itype##_interface : public generic_interface                        \
{                                                                         \
protected:                                                                \
  itype *my_##itype;                                                      \
                                                                          \
public:                                                                   \
  itype##_interface(itype *x)   { my_##itype = x; assert(my_##itype); }   \
  virtual ~itype##_interface()  { my_##itype = NULL; }                    \
                                                                          \
  itype *get_my_##itype() const { return(my_##itype); }                   \
};                                                                        \

GENERIC_INTERFACE_DEC(bone)
GENERIC_INTERFACE_DEC(entity)

#endif//ENTITY_INTERFACE_CLASS_HEADER

