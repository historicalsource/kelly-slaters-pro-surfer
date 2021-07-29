// attribute_template.h: interface for the attribute template class.
//
// Used to create sub-classes of attribute 
//////////////////////////////////////////////////////////////////////

#ifndef ATTRIBUTE_TEMPLATE_CLASS_HEADER
#define ATTRIBUTE_TEMPLATE_CLASS_HEADER

#include "pstring.h"

/*** attribute template ***/
template <class type>
class basic_attribute 
{
  private:
    static pstring type_name;
    type value;

  public:
    /*** constructors/destructors ***/
    basic_attribute(const char *attribute_type_name, const type &new_value,
        const type &_min_value, const type &_max_value)
    {
      _construct(attribute_type_name, new_value, _min_value, _max_value);
    }

    basic_attribute(const char *attribute_type_name, const type &new_value)
    {
      // define some dummy variables to keep the constructor parallel with bounded_attribute
      // so that our macro can be cleaner
      type _min_value, _max_value;
      _construct(attribute_type_name, new_value, _min_value, _max_value);
    }

    void _construct(const char *attribute_type_name, const type &new_value,
        const type &_min_value, const type &_max_value)
    {
      if (type_name.is_set() == false) 
      {
        // Set the basic_attribute type's name
        type_name = attribute_type_name;
      }
#ifdef DEBUG
      else 
      {
        // catch typo's and inconsistencies
        assert(strcmp(attribute_type_name, type_name.c_str()) == 0);
      }
#endif
      value = new_value;
    }

    basic_attribute(const basic_attribute &other)
    {
      assert("The basic_attribute copy constructor should not be called, each basic_attribute must be unique.\n"==0);
    }

    virtual ~basic_attribute() 
    {
    }

    /*** get_class_name ***/
    virtual const pstring &get_class_name() const
    {
      return type_name;
    }

    /*** Type cast ***/
    const type &get () const
    {
      // Would report a get here, if we decide to do that.
      return value;
    }

    operator type () 
    {
      // Would report a get here, if we decide to do that.
      return value;
    }

    /*** Assignment operators ***/
    void set (const type &new_value)  
    {
      value = new_value;
    }

    basic_attribute& operator =(const type &new_value)  
    {
      value = new_value;

      return *this;
    }

    // Arithmetic operators
    basic_attribute& operator +=(const type &new_value) 
    {
      value += new_value;
      
      return *this;
    }

    basic_attribute& operator -=(const type &new_value) 
    {
      value -= new_value;
      
      return *this;
    }

    basic_attribute& operator *=(const type &new_value) 
    {
      value *= new_value;

      return *this;
    }

    basic_attribute& operator /=(const type &new_value) 
    {
      value /= new_value;
      
      return *this;
    }

    basic_attribute& operator %=(const type &new_value) 
    {
      value %= new_value;
      
      return *this;
    }

    // bit-wise assignments (& | ^ << >>)
    basic_attribute& operator &=(const type &new_value) 
    {
      value &= new_value;
      
      return *this;
    }

    basic_attribute& operator |=(const type &new_value) 
    {
      value |= new_value;
      
      return *this;
    }

    basic_attribute& operator ^=(const type &new_value) 
    {
      value ^= new_value;
      
      return *this;
    }

    basic_attribute& operator <<=(const type &new_value) 
    {
      value <<= new_value;
      
      return *this;
    }

    basic_attribute& operator >>=(const type &new_value) 
    {
      value >>= new_value;
      
      return *this;
    }
};

template <class type> pstring basic_attribute<type>::type_name = "";


/*** bounded_attribute template ***/
template <class type>
class bounded_attribute
{
  private:
    static pstring type_name;
    type max_value;
    type min_value;
    type value;

  public:
    /*** constructors/destructors ***/
    bounded_attribute(const char *attribute_type_name, const type &new_value,
        const type &_min_value, const type &_max_value)
    {
      if (type_name.is_set() == false) 
      {
        // Set the attribute type's name
        type_name = attribute_type_name;
      }
#ifdef DEBUG
      else 
      {
        // catch typo's and inconsistencies
        //assert(strcmp(attribute_type_name, type_name.c_str()) == 0);
      }
#endif

      max_value = _max_value;
      min_value = _min_value;
      value = new_value;

      assert(min_value <= max_value);
      assert(value >= min_value && value <= max_value);
    }

    bounded_attribute(const bounded_attribute &other)
    {
      assert("The bounded_attribute copy constructor should not be called, each bounded_attribute must be unique.\n"==0);
    }

    virtual ~bounded_attribute() 
    {
    }

    /*** get_class_name ***/
    virtual const pstring &get_class_name() const 
    {
      return type_name;
    }

    /*** get/set max and min ***/
    void set_max(const type &new_max)
    {
      max_value = new_max;

      if (value > max_value)
        value = max_value;
    }

    const type & get_max()
    {
      return max_value;
    }

    void set_min(const type &new_min)
    {
      min_value = new_min;
      if (value < min_value)
        value = min_value;
    }

    const type & get_min()
    {
      return min_value;
    }

    /*** Type cast ***/
    const type &get () const
    {
      // Would report a get here, if we decide to do that.
      return value;
    }

    operator type () 
    {
      // Would report a get here, if we decide to do that.
      return value;
    }

    /*** Assignment operators ***/
    void set(const type &new_value)  
    {
      if (new_value < min_value)
        value = min_value;
      else if (new_value > max_value)
        value = max_value;
      else
        value = new_value;
    }

    bounded_attribute& operator =(const type &new_value)  
    {
      if (new_value < min_value)
        value = min_value;
      else if (new_value > max_value)
        value = max_value;
      else
        value = new_value;

      return *this;
    }

    // Arithmetic operators
    bounded_attribute& operator +=(const type &new_value) 
    {
      value += new_value;

      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
      
      return *this;
    }

    bounded_attribute& operator -=(const type &new_value) 
    {
      value -= new_value;

      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
            
      return *this;
    }

    bounded_attribute& operator *=(const type &new_value) 
    {
      value *= new_value;

      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
      
      return *this;
    }

    bounded_attribute& operator /=(const type &new_value) 
    {
      value /= new_value;
      
      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
      
      return *this;
    }

    bounded_attribute& operator %=(const type &new_value) 
    {
      value %= new_value;
      
      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
      
      return *this;
    }

    // bit-wise assignments (& | ^ << >>)
    bounded_attribute& operator &=(const type &new_value) 
    {
      value &= new_value;
      
      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
      
      return *this;
    }

    bounded_attribute& operator |=(const type &new_value) 
    {
      value |= new_value;
      
      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
      
      return *this;
    }

    bounded_attribute& operator ^=(const type &new_value) 
    {
      value ^= new_value;
      
      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
      
      return *this;
    }

    bounded_attribute& operator <<=(const type &new_value) 
    {
      value <<= new_value;
      
      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
      
      return *this;
    }

    bounded_attribute& operator >>=(const type &new_value) 
    {
      value >>= new_value;
      
      if (value < min_value)
        value = min_value;
      else if (value > max_value)
        value = max_value;
      
      return *this;
    }
};

template <class type> pstring bounded_attribute<type>::type_name = "";

#endif

