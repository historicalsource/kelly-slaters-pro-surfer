/*           Copyright (C) 2001 Sony Computer Entertainment America
                              All Rights Reserved
                               SCEA Confidential                                */

#ifndef vector_common_h
#define vector_common_h

/********************************************
 * This file contains the class 'vec_template' which provides all vector
 * methods that can be generalized, as well as the '*_base' classes that
 * are used to instanciate 'vec_template'.
 *
 * A few notes on implementation:
 *
 * - Everything here relies on Dylan's vu0 support patches to gcc, as well as
 *   Tyler's patches.  See the 'patches' project in cvs.
 *
 * - The classes are set up like this:
 *      *_base - provide the template and specific vector classes with info
 *               on what fields they contain for use by the template methods.
 *                              |
 *                      subclassed by
 *                              |
 *      vec_template - provides generalized methods for all vectors
 *                              |
 *              instances subclassed by
 *                              |
 *      vec_* - provide vector-specific methods and wrappers for template
 *              methods, when necessary.
 *
 * - Another way to implement this would be with traits and a template that
 *   is used directly.  This would make wrappers in the vec_* classes
 *   unnecessary, and make interaction with floats, ints, etc. easier.  I
 *   chose the structure described above for several reasons:  the extra
 *   level of indirection in the traits concept can be confusing, error
 *   messages from gcc involving two-parameter templates with default values
 *   are a mess, and forcing explicit definitions for each vector class, while
 *   tedious, makes it painfully obvious what is available.  But that's just
 *   my opinion...
 */

#include <stdio.h>
// #include <ps2s/debug.h>

/********************************************
 * Variables that represent VU0 registers.
 *
 * You can create a global variable that represents a VU0 special register by
 * listing the global as an input or output of any inline assembly that reads or
 * writes the register, respectively. By maintaining the correct order between
 * the inputs and outputs of the global, the compiler will order the writes
 * and reads of the VU0 register appropriately.
 *
 * This trick is mainly used here to allow use of the ACC by vector class methods.
 * Note that GCC doesn't know what's inside your inline asm statements, so you
 * don't actually have to write or read the global at all. Note also, you wouldn't
 * typically use this on a general VU0 register, since you wouldn't know which
 * assembly statements might use that register.  But if you keep some VU0 general
 * registers from the assembler (using compiler flag -mvu0-use-vf*-vf*) you could
 * use the same trick on your reserved registers.
 *
 * Binding the globals to GPR $0 seems to prevent any load and store side effects.
 */

register int vu0_ACC asm("$0");
// let's use vf0 for this one to avoid a compiler warning
register int vu0_Q asm("vf0") __attribute__ (( mode(TI) ));

/********************************************
 * constants
 */

namespace fields {
   static const unsigned int none = 0;
   static const unsigned int x = 1 << 0;
   static const unsigned int y = 1 << 1;
   static const unsigned int z = 1 << 2;
   static const unsigned int w = 1 << 3;
}

/********************************************
 * base classes
 * 
 * These store the attributes (traits) of the different
 * vector types and are used by vec_template (below).
 *
 * Note:  The vec128 member should really be declared in
 * the template since it is the same for all vectors, but
 * gcc will commit everything to memory if the template
 * inherits from an (eventually) empty class...
 */

typedef unsigned int    vec128_t __attribute__ (( mode(TI), aligned(16) ));

class x_base {
   public:
      static const unsigned int broadcast_field = fields::x;
      static const unsigned int valid_fields    = fields::x;
      vec128_t                                  vec128;
};

class y_base {
   public:
      static const unsigned int broadcast_field = fields::y;
      static const unsigned int valid_fields    = fields::y;
      vec128_t                                  vec128;
};

class z_base {
   public:
      static const unsigned int broadcast_field = fields::z;
      static const unsigned int valid_fields    = fields::z;
      vec128_t                                  vec128;
};

class w_base {
   public:
      static const unsigned int broadcast_field = fields::w;
      static const unsigned int valid_fields    = fields::w;
      vec128_t                                  vec128;
};

class xy_base {
   public:
      static const unsigned int broadcast_field = fields::none;
      static const unsigned int valid_fields    = fields::x | fields::y;
      vec128_t                                  vec128;
};

class xyz_base {
   public:
      static const unsigned int broadcast_field = fields::none;
      static const unsigned int valid_fields    = fields::x | fields::y | fields::z;
      vec128_t                                  vec128;
};

class xyzw_base {
   public:
      static const unsigned int broadcast_field = fields::none;
      static const unsigned int valid_fields    = fields::x | fields::y | fields::z | fields::w;
      vec128_t                                  vec128;
};

class point_base {
   public:
      static const unsigned int broadcast_field = fields::none;
      static const unsigned int valid_fields    = fields::x | fields::y | fields::z;
      vec128_t                                  vec128;
};

class vector_base {
   public:
      static const unsigned int broadcast_field = fields::none;
      static const unsigned int valid_fields    = fields::x | fields::y | fields::z;
      vec128_t                                  vec128;
};

/********************************************
 * vec_template
 *
 * This template is used to provide methods that can be generalized
 * for all vector classes.  Class-specific methods are found in the
 * vec_* classes, which are derived from the template.
 */

class mat_44;

template <class base_vec>
class vec_template : public base_vec
{
      friend class mat_44;
   public:

      // constructors

      inline vec_template( ) { }
      inline vec_template( const vec128_t initVal ) { vec128 = initVal; }

      // accessors

      inline vec128_t get128( ) const { return vec128; }


      // mutators

      // this is called by all the operator = ()'s
      template <class rhs_type>
      inline void
      set( const rhs_type rhs ) {
         asm ( " ### set vec_l_fields = vec_r_fields ### \n"
               "vmove.mask	this, rhs \n"
               : "=j this" (*this)
               : "j rhs" (rhs),
               "0" (*this),
               "vu_mask l_fields" (this->valid_fields),
               "vu_mask r_fields" (rhs.valid_fields),
               "vu_mask mask" (this->valid_fields & rhs.valid_fields)
            );
      }
      // make sure that for vectors of the same type it's a simple assignment
      inline void
      set( const vec_template<base_vec> rhs ) {
         this->vec128 = rhs;
      }

      inline void
      set( float rhs ) {
         asm (  "### set all vec_fields fields to float ### \n"
                "pextlw rhs, rhs, rhs \n"
                "pextlw this, rhs, rhs \n"
                : "=r this" (*this)
                : "r rhs" (rhs)
            );
      }                                 

      inline void
      set_zero() {

         asm (  " ### vec_l_fields set_zero ### \n"
                "vsub.mask this, vf00, vf00 \n"
                : "=j this" (*this)
                : "vu_mask mask" (this->valid_fields),
                  "vu_mask l_fields" (this->valid_fields)
                );
      }

      static const unsigned int fields = (base_vec::broadcast_field << 4) | base_vec::valid_fields;

   private:
      // for readability -- this class is the left-hand operand (*this)
      // in the following methods
      typedef vec_template<base_vec> lhs_type;
   public:

      // math

      template <class rhs_type>
      inline lhs_type
      operator + ( rhs_type rhs ) const {
         const lhs_type& lhs = *this;
         vec128_t result;
         asm (
            " ### vec_l_fields + vec_r_fields ### \n"
            "vadd_bc._mask	result, lhs, rhs \n"
            : "=j result" (result)
            : "j lhs" (lhs), "j rhs" (rhs),
            "vu_binary _bc _mask" ((lhs.fields << 8) | rhs.fields),
            "vu_mask l_fields" (lhs.valid_fields),
            "vu_mask r_fields" (rhs.valid_fields)
            );
         return lhs_type(result);
      }
      inline lhs_type
      operator + ( float rhs ) const {
         vec128_t result;
         asm (
            " ### vec_l_fields + float ### \n"
            "ctc2	rhs, $vi21 \n"
            "vnop \n"
            "vaddi.mask	result, lhs, I \n"
            : "=j result" (result)
            : "r rhs" (rhs),
            "j lhs" (*this),
            "vu_mask mask" (this->valid_fields),
            "vu_mask l_fields" (this->valid_fields)
            );
         return lhs_type(result);
      }
      template <class rhs_type>
      inline void
      operator += ( rhs_type rhs ) {
         *this = *this + rhs;
      }

      template <class rhs_type>
      inline lhs_type
      operator - ( rhs_type rhs ) const {
         const lhs_type& lhs = *this;
         vec128_t result;
         asm (
            " ### vec_l_fields - vec_r_fields ### \n"
            "vsub_bc._mask %0, %1, %2 \n"
            : "=j" (result)
            : "j" (lhs), "j" (rhs),
            "vu_binary _bc _mask" ((lhs.fields << 8) | rhs.fields),
            "vu_mask l_fields" (lhs.valid_fields),
            "vu_mask r_fields" (rhs.valid_fields)
            );
         return lhs_type(result);
      }
      template <class rhs_type>
      inline void
      operator -= ( rhs_type rhs ) {
         *this = *this - rhs;
      }


      template <class rhs_type>
      inline lhs_type
      operator * ( rhs_type rhs ) const {
         const lhs_type& lhs = *this;
         vec128_t result;
         asm (
            " ### vec_l_fields * vec_r_fields ### \n"
            "vmul_bc._mask  %0, %1, %2 \n"
            : "=j" (result)
            : "j" (lhs), "j" (rhs),
            "vu_binary _bc _mask" ((lhs.fields << 8) | rhs.fields),
            "vu_mask l_fields" (lhs.valid_fields),
            "vu_mask r_fields" (rhs.valid_fields)
            );

         return lhs_type(result);
      }
      inline lhs_type
      operator * ( float rhs ) const {
         vec128_t result;
         asm (
            " ### vec_l_fields * float ### \n"
            "ctc2	rhs, $vi21 \n"
            "vnop \n"
            "vmuli.mask	result, this, I \n"
            : "=j result" (result)
            : "r rhs" (rhs),
            "j this" (*this),
            "vu_mask mask" (this->valid_fields),
            "vu_mask l_fields" (this->valid_fields)
            : "vf31"
            );
         return lhs_type(result);
      }
      template <class rhs_type>
      inline void
      operator *= ( rhs_type rhs ) {
         *this = *this * rhs;
      }

      template <class rhs_type>
      inline lhs_type
      operator / ( rhs_type rhs ) const {
         const lhs_type& lhs = *this;
         vec128_t result;
         asm (
            " ### vec_l_fields / vec__r_bc ### \n"
            "vdiv		Q, vf00w, rhs_r_bc \n"
            "vwaitq \n"
            "vmulq.l_fields	result, lhs, Q \n"
            : "=j result" (result)
            : "j lhs" (lhs), "j rhs" (rhs),
            "vu_mask l_fields" (lhs.valid_fields),
            "vu_bc _r_bc" (rhs.broadcast_field)
            );
         return lhs_type(result);
      }
      inline lhs_type
      operator / ( float rhs ) const {
         return *this * (1.0f/rhs);
      }
      template <class rhs_type>
      inline void
      operator /= ( rhs_type rhs ) {
         *this = *this / rhs;
      }
      inline void
      operator /= ( float rhs ) {
         *this = *this / rhs;
      }

      inline lhs_type
      operator - () const {
         vec128_t result;
         asm (
            " ### negate vec_l_fields ### \n"
            "vsuba.mask ACC, vf00, vf00 \n"
            "vmsubw.mask result, this, vf00w \n"
                
            : "=j result" (result), "=r" (vu0_ACC)
            : "j this" (*this),
            "vu_mask mask" (this->valid_fields),
            "vu_mask l_fields" (this->valid_fields)
            );
         return lhs_type(result);
      }

      // negate for vectors with no w field.

      inline lhs_type
      no_w_negate () const {
         vec128_t result;
         asm (
            " ### negate vec_l_fields (no w) ### \n"
            "vsub.mask result, vf00, this \n"
                
            : "=j result" (result)
            : "j this" (*this),
            "vu_mask mask" (this->valid_fields),
            "vu_mask l_fields" (this->valid_fields)
            );
         return lhs_type(result);
      }

      // add and multiply-add functions.
      // You can write the accumulator in one method and add or
      // madd to it in another - data dependency is respected
      // due to the vu0_ACC global (see declaration above)·
      // However, you are responsible for matching the type
      // written to and read from the accumulator - use with
      // care.

      // to_a (to accumulator): ACC = this

      inline void
      to_a () const {
         asm (
                " ### ACC = vec_fields ### \n"
                "vmulaw.mask ACC, this, vf00 \n"
                : "=r" (vu0_ACC)
                : "j this" (*this),
                "vu_mask mask" (this->valid_fields),
                "vu_mask fields" (this->valid_fields)
            );
      }

      // from_a (from accumulator): this = ACC

      inline void
      from_a () {
         asm (
                " ### vec_fields = ACC ### \n"
                "vmaddx.mask this, vf00, vf00 \n"
                : "=j this" (*this)
                : "r" (vu0_ACC),
                "vu_mask mask" (this->valid_fields),
                "vu_mask fields" (this->valid_fields)
            );
      }

      // mula (multiply, to accumulator): ACC = this * rhs

      template <class rhs_type>
      inline void
      mula ( rhs_type rhs ) const {
         const lhs_type& lhs = *this;
         asm (
                " ### ACC = vec_l_fields * vec_r_fields ### \n"
                "vmula_bc._mask ACC, lhs, rhs \n"
                : "=r" (vu0_ACC)
                : "j lhs" (lhs), "j rhs" (rhs),
                "vu_binary _bc _mask" ((lhs.fields << 8) | rhs.fields),
                "vu_mask l_fields" (lhs.valid_fields),
                "vu_mask r_fields" (rhs.valid_fields)
            );
      }

      inline void
      mula ( float rhs ) const {
         asm (
                " ### ACC = vec_l_fields * float ### \n"
                "ctc2	rhs, $vi21 \n"
                "vnop \n"
                "vmulai.mask ACC, lhs, I \n"
                : "=r" (vu0_ACC)
                : "r rhs" (rhs),
                "j lhs" (*this),
                "vu_mask mask" (this->valid_fields),
                "vu_mask l_fields" (this->valid_fields)
            );
      }

      // aadd (accumulator add): result = ACC + this
      // asub (accumulator subtract): result = ACC - this

      inline lhs_type
      aadd( ) const {
         vec128_t result;
         asm (
                " ### ACC + vec_l_fields ### \n"
                "vmaddw.mask result, this, vf00 \n"
                
                : "=j result" (result)
                : "j this" (*this),
                  "vu_mask mask" (this->valid_fields),
                  "vu_mask l_fields" (this->valid_fields),
                  "r" (vu0_ACC)
            );
         return lhs_type(result);
      }

      inline lhs_type
      asub( ) const {
         vec128_t result;
         asm (
                " ### ACC - vec_l_fields ### \n"
                "vmsubw.mask result, this, vf00 \n"
                
                : "=j result" (result)
                : "j this" (*this),
                  "vu_mask mask" (this->valid_fields),
                  "vu_mask l_fields" (this->valid_fields),
                  "r" (vu0_ACC)
            );
         return lhs_type(result);
      }

      // adda (add, to accumulator): ACC = this + rhs
      // suba (subtract, to accumulator): ACC = this - rhs

      template <class rhs_type>
      inline void
      adda ( rhs_type rhs ) const {
         const lhs_type& lhs = *this;
         asm (
                " ### ACC = vec_l_fields + vec_r_fields ### \n"
                "vadda_bc._mask ACC, lhs, rhs \n"
                : "=r" (vu0_ACC)
                : "j lhs" (lhs), "j rhs" (rhs),
                "vu_binary _bc _mask" ((lhs.fields << 8) | rhs.fields),
                "vu_mask l_fields" (lhs.valid_fields),
                "vu_mask r_fields" (rhs.valid_fields)
            );
      }

      inline void
      adda ( float rhs ) const {
         asm (
                " ### ACC = vec_l_fields + float ### \n"
                "ctc2	rhs, $vi21 \n"
                "vnop \n"
                "vaddai.mask ACC, lhs, I \n"
                : "=r" (vu0_ACC)
                : "r rhs" (rhs),
                "j lhs" (*this),
                "vu_mask mask" (this->valid_fields),
                "vu_mask l_fields" (this->valid_fields)
            );
      }

      template <class rhs_type>
      inline void
      suba ( rhs_type rhs ) const {
         const lhs_type& lhs = *this;
         asm (
                " ### ACC = vec_l_fields - vec_r_fields ### \n"
                "vsuba_bc._mask ACC, lhs, rhs \n"
                : "=r" (vu0_ACC)
                : "j lhs" (lhs), "j rhs" (rhs),
                "vu_binary _bc _mask" ((lhs.fields << 8) | rhs.fields),
                "vu_mask l_fields" (lhs.valid_fields),
                "vu_mask r_fields" (rhs.valid_fields)
            );
      }

      inline void
      suba ( float rhs ) const {
         asm (
                " ### ACC = vec_l_fields - float ### \n"
                "ctc2	rhs, $vi21 \n"
                "vnop \n"
                "vsubai.mask ACC, lhs, I \n"
                : "=r" (vu0_ACC)
                : "r rhs" (rhs),
                "j lhs" (*this),
                "vu_mask mask" (this->valid_fields),
                "vu_mask l_fields" (this->valid_fields)
            );
      }

      // aadda (accumulator add, to accumulator): ACC = ACC + this
      // asuba (accumulator subtract, to accumulator): ACC = ACC - this

      inline lhs_type
      aadda( ) const {
         vec128_t result;
         asm (
                " ### ACC = ACC + vec_l_fields ### \n"
                "vmaddaw.mask ACC, this, vf00 \n"
                
                : "+r" (vu0_ACC)
                : "j this" (*this),
                  "vu_mask mask" (this->valid_fields),
                  "vu_mask l_fields" (this->valid_fields)
            );
         return lhs_type(result);
      }

      inline lhs_type
      asuba( ) const {
         vec128_t result;
         asm (
                " ### ACC = ACC - vec_l_fields ### \n"
                "vmsubaw.mask ACC, this, vf00 \n"
                
                : "+r" (vu0_ACC)
                : "j this" (*this),
                  "vu_mask mask" (this->valid_fields),
                  "vu_mask l_fields" (this->valid_fields)
            );
         return lhs_type(result);
      }

      // madd (multiply, add with accumulator): result = ACC + this * rhs
      // msub (multiply, subtract from accumulator): result = ACC - this * rhs

      template <class rhs_type>
      inline lhs_type
      madd( rhs_type rhs ) const {
         vec128_t result;
         asm (
                " ### ACC + vec_l_fields * vec_r_fields ### \n"
                "vmadd_bc._mask result, this, rhs \n"
                
                : "=j result" (result)
                : "j this" (*this), "j rhs" (rhs),
                  "vu_binary _bc _mask" (this->fields << 8 | rhs.fields),
                  "vu_mask l_fields" (this->valid_fields),
                  "vu_mask r_fields" (rhs.valid_fields),
                  "r" (vu0_ACC)
            );
         return lhs_type(result);
      }

      inline lhs_type
      madd( float rhs ) const {
         vec128_t result;
         asm (
                " ### ACC + vec_l_fields * float ### \n"
                "ctc2	rhs, $vi21 \n"
                "vnop \n"
                "vmaddi._mask result, this, I \n"               
                : "=j result" (result)
                : "j this" (*this), "r rhs" (rhs),
                  "vu_mask _mask" (this->valid_fields),
                  "vu_mask l_fields" (this->valid_fields),
                  "r" (vu0_ACC)
            );
         return lhs_type(result);
      }

      template <class rhs_type>
      inline lhs_type
      msub( rhs_type rhs ) const {
         vec128_t result;
         asm (
                " ### ACC - vec_l_fields * vec_r_fields ### \n"
                "vmsub_bc._mask result, this, rhs \n"
                
                : "=j result" (result)
                : "j this" (*this), "j rhs" (rhs),
                  "vu_binary _bc _mask" (this->fields << 8 | rhs.fields),
                  "vu_mask l_fields" (this->valid_fields),
                  "vu_mask r_fields" (rhs.valid_fields),
                  "r" (vu0_ACC)
            );
         return lhs_type(result);
      }

      inline lhs_type
      msub( float rhs ) const {
         vec128_t result;
         asm (
                " ### ACC - vec_l_fields * float ### \n"
                "ctc2	rhs, $vi21 \n"
                "vnop \n"
                "vmsubi._mask result, this, I \n"               
                : "=j result" (result)
                : "j this" (*this), "r rhs" (rhs),
                  "vu_mask _mask" (this->valid_fields),
                  "vu_mask l_fields" (this->valid_fields),
                  "r" (vu0_ACC)
            );
         return lhs_type(result);
      }

      // madda (multiply, add with accumulator, to accumulator): ACC = ACC + this * rhs
      // msuba (multiply, subtract from accumulator, to accumulator): ACC = ACC - this * rhs

      template <class rhs_type>
      inline void
      madda ( rhs_type rhs ) const {
         const lhs_type& lhs = *this;
         asm (
                " ### ACC = ACC + vec_l_fields * vec_r_fields ### \n"
                "vmadda_bc._mask ACC, lhs, rhs \n"
                : "+r" (vu0_ACC)
                : "j lhs" (lhs), "j rhs" (rhs),
                "vu_binary _bc _mask" ((lhs.fields << 8) | rhs.fields),
                "vu_mask l_fields" (lhs.valid_fields),
                "vu_mask r_fields" (rhs.valid_fields)
            );
      }

      inline void
      madda ( float rhs ) const {
         asm (
                " ### ACC = ACC + vec_l_fields * float ### \n"
                "ctc2	rhs, $vi21 \n"
                "vnop \n"
                "vmaddai.mask ACC, lhs, I \n"
                : "+r" (vu0_ACC)
                : "r rhs" (rhs),
                "j lhs" (*this),
                "vu_mask mask" (this->valid_fields),
                "vu_mask l_fields" (this->valid_fields)
            );
      }

      template <class rhs_type>
      inline void
      msuba ( rhs_type rhs ) const {
         const lhs_type& lhs = *this;
         asm (
                " ### ACC = ACC - vec_l_fields * vec_r_fields ### \n"
                "vmsuba_bc._mask ACC, lhs, rhs \n"
                : "+r" (vu0_ACC)
                : "j lhs" (lhs), "j rhs" (rhs),
                "vu_binary _bc _mask" ((lhs.fields << 8) | rhs.fields),
                "vu_mask l_fields" (lhs.valid_fields),
                "vu_mask r_fields" (rhs.valid_fields)
            );
      }

      inline void
      msuba ( float rhs ) const {
         asm (
                " ### ACC = ACC - vec_l_fields * float ### \n"
                "ctc2	rhs, $vi21 \n"
                "vnop \n"
                "vmsubai.mask ACC, lhs, I \n"
                : "+r" (vu0_ACC)
                : "r rhs" (rhs),
                "j lhs" (*this),
                "vu_mask mask" (this->valid_fields),
                "vu_mask l_fields" (this->valid_fields)
            );
      }

      inline lhs_type
      abs() const {
         vec128_t result;
         asm (
                " ### absolute value of vec_fields ### \n"
                "vabs.mask result, this \n"
                : "=j result" (result)
                : "j this" (*this),
                  "vu_mask mask" (this->valid_fields),
                  "vu_mask fields" (this->valid_fields)
                );
                  return lhs_type(result);
      }

      inline lhs_type
      sign() const {
         vec128_t result;
         int temp;
         int ones = 0x3f800000;
         asm (
                " ### signs of vec_fields ### \n"
                "vmulx.mask result, this, vf00 \n"
                "pextlw ones, ones, ones \n"
                "pextlw ones, ones, ones \n"
                "qmfc2 temp, result \n"
                "por temp, temp, ones \n"
                "qmtc2 temp, result \n"
                : "=j result" (result), "=r temp" (temp), "+r ones" (ones)
                : "j this" (*this),
                  "vu_mask mask" (this->valid_fields),
                  "vu_mask fields" (this->valid_fields)
         );
         return lhs_type(result);
      }

      template <class rhs_type>
      inline lhs_type
      max(rhs_type rhs) const {
         vec128_t result;
         asm (  " ### maximum of vec_l_fields and vec_r_fields ### \n"
                "vmax_bc._mask result, this, rhs \n"
                : "=j result" (result)
                : "j this" (*this), "j rhs" (rhs),
                  "vu_binary _bc _mask" (this->fields << 8 | rhs.fields),
                  "vu_mask l_fields" (this->valid_fields),
                  "vu_mask r_fields" (rhs.valid_fields)         
                );
         return lhs_type(result);
      }
                
      template <class rhs_type>
      inline lhs_type
      min(rhs_type rhs) const {
         vec128_t result;
         asm (  " ### minimum of vec_l_fields and vec_r_fields ### \n"
                "vmini_bc._mask result, this, rhs \n"
                : "=j result" (result)
                : "j this" (*this), "j rhs" (rhs),
                  "vu_binary _bc _mask" (this->fields << 8 | rhs.fields),
                  "vu_mask l_fields" (this->valid_fields),
                  "vu_mask r_fields" (rhs.valid_fields)         
                );
         return lhs_type(result);
      }

      inline void print( void ) const {
         float x, y, z, w;
         vec128_t temp0;
         asm volatile (
            "mtsab	$0, 4		# get ready to shift right 4 bytes	\n"
            "mtc1	%5, %0		# x = value.x	\n"
            "qfsrv	%4, $0, %5 	# temp0 = value >> 8	\n"
            "mtc1	%4, %1		# y = value.y \n"
            "qfsrv	%4, $0, %4 	# temp0 >>= 8 \n"
            "mtc1	%4, %2		# z = value.z \n"
            "qfsrv	%4, $0, %4 	# temp0 >>= 8 \n"
            "mtc1	%4, %3		# w = value.w \n"
            : "=f" (x), "=f" (y), "=f" (z), "=f" (w), "=r" (temp0)
            : "r" (vec128)
            );

         printf("(%f %f %f %f)\n", x, y, z, w);
      }

      inline void print( const char *vec_name ) const {
         printf("%s: \n", vec_name);
         print();
      }
};

#endif // vector_common_h
