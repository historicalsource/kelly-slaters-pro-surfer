/*           Copyright (C) 2001 Sony Computer Entertainment America
                              All Rights Reserved
                               SCEA Confidential                                */

#ifndef matrix_h
#define matrix_h

#include "matrix_common.h"

class zero_33;
class zero_44;
class zero_43;
class zero_34;
class mat_34;
class mat_43;
class transform_t;

/********************************************
 * mat_33 - a 3x3 matrix
 */

class mat_33 : public mat_x3_template<vec_3>
{
   public:
      mat_33( ) { }

      mat_33( const mat_x3_template<vec_3>& mat )
      {
         col0 = mat.col0; col1 = mat.col1; col2 = mat.col2;
      }

      mat_33( const vec_3 col_0, const vec_3 col_1, const vec_3 col_2) {
         col0 = col_0; col1 = col_1; col2 = col_2;
      }

      mat_33( const zero_33 );

      explicit inline mat_33(const mat_34& mat);
      explicit inline mat_33(const mat_43& mat);
      explicit inline mat_33(const mat_44& mat);

      vec_3 get_row3() const;     // undefined
      void  set_row3(vec_3);      // undefined

      void set_identity( ) {
         asm ( " ### mat_33::set_identity ### \n"
               "vsub      col0, col0, col0 \n"
               "vsub      col1, col1, col1 \n"
               "vmr32    col2, vf00       \n"
               "vaddw.x    col0, vf00, vf00 \n"
               "vaddw.y    col1, vf00, vf00 \n"
               : "=j col0" (col0),
               "=j col1" (col1),
               "=j col2" (col2)
            );
      }

      void
      set_scale( vec_3 scale ) {
         set_zero();
         col0 = (vec_x)scale;
         col1 = (vec_y)scale;
         col2 = (vec_z)scale;
      }

      void
      set(vec_4 quat) {
         vec128_t ones, q_yzx, q_zxy;

         asm ( "### set rotation from unit quaternion ### \n"

               "vmr32.xy q_yzx, quat \n"
               "vaddx.z q_yzx, vf00, quat \n"
               "vaddz.x q_zxy, vf00, quat \n"
               "vaddx.y q_zxy, vf00, quat \n"
               "vaddy.z q_zxy, vf00, quat \n"  
    
               "vmula ACC, quat, q_yzx \n"
               "vmsubw.z col0, q_zxy, quat \n"
               "vmsubw.x col1, q_zxy, quat \n"
               "vmsubw.y col2, q_zxy, quat \n"

               "vmula ACC, quat, q_zxy \n"
               "vmaddw.y col0, q_yzx, quat \n"
               "vmaddw.z col1, q_yzx, quat \n"
               "vmaddw.x col2, q_yzx, quat \n"
    
               "vmula ACC, q_zxy, q_zxy \n"
               "vmadd.x col0, q_yzx, q_yzx \n"
               "vmadd.y col1, q_yzx, q_yzx \n"
               "vmadd.z col2, q_yzx, q_yzx \n"

               "vmaxw ones, vf00, vf00 \n"
               "vadd col0, col0, col0 \n"
               "vadd col1, col1, col1 \n"
               "vadd col2, col2, col2 \n"

               "vsub.x col0, ones, col0 \n"
               "vsub.y col1, ones, col1 \n"
               "vsub.z col2, ones, col2 \n"

               : "=&j col0" (col0),
               "=&j col1" (col1),
               "=&j col2" (col2),
               "=&j q_yzx" (q_yzx),
               "=&j q_zxy" (q_zxy),
               "=&j ones" (ones), "=r" (vu0_ACC)
               : "j quat" (quat)
            );
      }

      mat_33( vec_4 quat ) {
         set(quat);
      }

      void
      set_rotate_x( float angle ) {
         float cs = cosf(angle);
         float sn = sinf(angle);
         col0.set( 1, 0, 0 );
         col1.set( 0, cs, sn );
         col2.set( 0, -sn, cs );
      }

      void
      set_rotate_y( float angle ) {
         float cs = cosf(angle);
         float sn = sinf(angle);
         col0.set( cs, 0, -sn );
         col1.set( 0, 1, 0 );
         col2.set( sn, 0, cs );
      }

      void
      set_rotate_z( float angle ) {
         float cs = cosf(angle);
         float sn = sinf(angle);
         col0.set( cs, sn, 0 );
         col1.set( -sn, cs, 0 );
         col2.set( 0, 0, 1 );
      }

      // negate

      mat_33 operator - () const;

      // transpose & inverse

      mat_33
      transpose() const;

      void
      transpose_in_place () {
         *this = transpose();
      }

      mat_33
      inverse() const;

      void
      inverse_in_place() {
         *this = inverse();
      }

      // matrix/scalar operations

      mat_33 operator * ( float scale ) const;
      mat_33 operator * ( vec_x scale ) const;
      mat_33 operator * ( vec_y scale ) const;
      mat_33 operator * ( vec_z scale ) const;
      mat_33 operator * ( vec_w scale ) const;

      void operator *= ( float scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_x scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_y scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_z scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_w scale ) {
         *this = *this * scale;
      }

      // matrix/vector operations

      vec_3
      operator * ( vec_3 vec ) const {
         return mat_x3_template<vec_3>::operator * (vec);
      }

      vec_3
      trans_mult( vec_3 vec ) const
      {
         vec128_t result, temp0, temp1, temp2, ones;

         asm ( "### mat_33 trans_mult vec_3 ### \n"
               "vmul temp0, col0, vec \n"
               "vmaxw ones, vf00, vf00 \n"
               "vmul temp1, col1, vec \n"
               "vmul temp2, col2, vec \n"
               "vadday.x ACC, temp0, temp0 \n"
               "vmaddz.x result, ones, temp0 \n"
               "vaddax.y ACC, temp1, temp1 \n"
               "vmaddz.y result, ones, temp1 \n"
               "vaddax.z ACC, temp2, temp2 \n"
               "vmaddy.z result, ones, temp2 \n"
               : "=&j result" (result),
               "=&j temp0" (temp0),
               "=&j temp1" (temp1),
               "=&j temp2" (temp2),
               "=&j ones" (ones), "=r" (vu0_ACC)
               : "j col0" (col0),
               "j col1" (col1),
               "j col2" (col2),
               "j vec" (vec)
            );

         return vec_3(result);
      }

      mat_33 mult_tilde ( vec_3 vec ) const;

      // matrix/matrix operations

      mat_33 operator + ( const mat_33& mat ) const;
      mat_33 operator - ( const mat_33& mat ) const;

      void   operator += ( const mat_33& mat ) {
         *this = *this + mat;
      }
      void   operator -= ( const mat_33& mat ) {
         *this = *this - mat;
      }

      mat_33 operator * ( const mat_33& mat ) const;
      mat_34 operator * ( const mat_34& mat ) const;

      mat_33 trans_mult( const mat_33 &mat ) const;
      mat_34 trans_mult( const mat_34 &mat ) const;

      mat_33 mult_trans( const mat_33 &mat ) const;
      mat_34 mult_trans( const mat_43 &mat ) const;

      mat_33 &
      operator = ( const mat_33& mat ) {
         col0 = mat.col0;
         col1 = mat.col1;
         col2 = mat.col2;
         return *this;
      }

      // matrix/zero_matrix operations

      mat_33 operator + ( const zero_33 zero ) const;
      mat_33 operator - ( const zero_33 zero ) const;

      void operator += ( const zero_33 zero );
      void operator -= ( const zero_33 zero );

      zero_33 operator * ( const zero_33 zero ) const;
      zero_34 operator * ( const zero_34 zero ) const;

      zero_33 trans_mult( const zero_33 zero ) const;
      zero_34 trans_mult( const zero_34 zero ) const;

      zero_33 mult_trans( const zero_33 zero ) const;
      zero_34 mult_trans( const zero_43 zero ) const;

      mat_33& operator = ( const zero_33 zero );

      void print( ) const {
         get_row0().print();
         get_row1().print();
         get_row2().print();
      }

      void print(const char *mat_name) const {
         printf("%s:\n", mat_name);
         print();
      }
};

/********************************************
 * mat_43 - a 4x3 matrix
 */

class mat_43 : public mat_x3_template<vec_4>
{
   public:
      mat_43( ) { }

      mat_43( const mat_x3_template<vec_4>& mat )
      {
         col0 = mat.col0; col1 = mat.col1; col2 = mat.col2;
      }

      mat_43( const zero_43 );

      mat_43( const vec_4 col_0, const vec_4 col_1, const vec_4 col_2 ) {
         col0 = col_0; col1 = col_1; col2 = col_2;
      }

      explicit inline mat_43(const mat_44& mat);

      // negate

      mat_43 operator - () const;

      // transpose

      mat_34 transpose() const;

      // matrix/scalar operations

      mat_43 operator * ( float scale ) const;
      mat_43 operator * ( vec_x vec ) const;
      mat_43 operator * ( vec_y vec ) const;
      mat_43 operator * ( vec_z vec ) const;
      mat_43 operator * ( vec_w vec ) const;

      void operator *= ( float scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_x scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_y scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_z scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_w scale ) {
         *this = *this * scale;
      }

      // matrix/vector operations

      vec_4
      operator * ( vec_3 vec ) const {
         return mat_x3_template<vec_4>::operator * (vec);
      }

      vec_3
      trans_mult( vec_4 vec ) const {
         vec128_t result, temp0, temp1, temp2, ones;

         asm ( "### mat_43 trans_mult vec_4 ### \n"
               "vmul temp0, col0, vec \n"
               "vmaxw ones, vf00, vf00 \n"
               "vmul temp1, col1, vec \n"
               "vmul temp2, col2, vec \n"
               "vadday.x ACC, temp0, temp0 \n"
               "vmaddaz.x ACC, ones, temp0 \n"
               "vmaddw.x result, ones, temp0 \n"
               "vaddax.y ACC, temp1, temp1 \n"
               "vmaddaz.y ACC, ones, temp1 \n"
               "vmaddw.y result, ones, temp1 \n"
               "vaddax.z ACC, temp2, temp2 \n"
               "vmadday.z ACC, ones, temp2 \n"
               "vmaddw.z result, ones, temp2 \n"
               : "=&j result" (result),
               "=&j temp0" (temp0), "=&j temp1" (temp1), "=&j temp2" (temp2), "=&j ones" (ones), "=r" (vu0_ACC)
               : "j col0" (col0),
               "j col1" (col1),
               "j col2" (col2),
               "j vec" (vec)
            );

         return vec_3(result);
      }

      vec_3
      trans_mult( vector_t vec ) const {
         vec128_t result, temp0, temp1, temp2, ones;

         asm ( "### mat_43 trans_mult vector_t ### \n"
               "vmul temp0, col0, vec \n"
               "vmaxw ones, vf00, vf00 \n"
               "vmul temp1, col1, vec \n"
               "vmul temp2, col2, vec \n"
               "vadday.x ACC, temp0, temp0 \n"
               "vmaddz.x result, ones, temp0 \n"
               "vaddax.y ACC, temp1, temp1 \n"
               "vmaddz.y result, ones, temp1 \n"
               "vaddax.z ACC, temp2, temp2 \n"
               "vmaddy.z result, ones, temp2 \n"
               : "=&j result" (result),
               "=&j temp0" (temp0), "=&j temp1" (temp1), "=&j temp2" (temp2), "=&j ones" (ones), "=r" (vu0_ACC)
               : "j col0" (col0),
               "j col1" (col1),
               "j col2" (col2),
               "j vec" (vec)
            );

         return vec_3(result);
      }

      vec_3
      trans_mult( point_t vec ) const {
         vec128_t result, temp0, temp1, temp2, ones;

         asm ( "### mat_43 trans_mult point_t ### \n"
               "vmul temp0, col0, vec \n"
               "vmaxw ones, vf00, vf00 \n"
               "vmul temp1, col1, vec \n"
               "vmul temp2, col2, vec \n"
               "vadday.x ACC, temp0, temp0 \n"
               "vmaddaz.x ACC, ones, temp0 \n"
               "vmaddw.x result, ones, col0 \n"
               "vaddax.y ACC, temp1, temp1 \n"
               "vmaddaz.y ACC, ones, temp1 \n"
               "vmaddw.y result, ones, col1 \n"
               "vaddax.z ACC, temp2, temp2 \n"
               "vmadday.z ACC, ones, temp2 \n"
               "vmaddw.z result, ones, col2 \n"
               : "=&j result" (result),
               "=&j temp0" (temp0), "=&j temp1" (temp1), "=&j temp2" (temp2), "=&j ones" (ones), "=r" (vu0_ACC)
               : "j col0" (col0), "j col1" (col1), "j col2" (col2), "j vec" (vec)
            );

         return vec_3(result);
      }

      // matrix/matrix operations

      mat_43 mult_tilde ( vec_3 vec ) const;

      mat_43 operator + ( const mat_43& mat ) const;
      mat_43 operator - ( const mat_43& mat ) const;

      void   operator += ( const mat_43& mat ) {
         *this = *this + mat;
      }
      void   operator -= ( const mat_43& mat ) {
         *this = *this - mat;
      }

      mat_43 operator * ( const mat_33& mat ) const;
      mat_44 operator * ( const mat_34& mat ) const;

      mat_33 trans_mult ( const mat_43& mat ) const;
      mat_34 trans_mult ( const mat_44& mat ) const;
      mat_34 trans_mult ( const transform_t& mat ) const;

      mat_44 mult_trans ( const mat_43& mat ) const;
      mat_43 mult_trans ( const mat_33& mat ) const;

      mat_43 &
      operator = ( const mat_33& mat ) {
         col0 = mat.col0;
         col1 = mat.col1;
         col2 = mat.col2;
         return *this;
      }

      // matrix/zero_matrix operations

      mat_43 operator + ( const zero_43 zero ) const;
      mat_43 operator - ( const zero_43 zero ) const;

      void operator += ( const zero_43 zero );
      void operator -= ( const zero_43 zero );

      zero_43 operator * ( const zero_33 zero ) const;
      zero_44 operator * ( const zero_34 zero ) const;

      zero_33 trans_mult ( const zero_43 zero ) const;
      zero_34 trans_mult ( const zero_44 zero ) const;

      zero_44 mult_trans ( const zero_43 zero ) const;
      zero_43 mult_trans ( const zero_33 zero ) const;

      mat_43& operator = ( const zero_43 zero );

      void print( ) const {
         get_row0().print();
         get_row1().print();
         get_row2().print();
         get_row3().print();
      }

      void print(const char *mat_name) const {
         printf("%s:\n", mat_name);
         print();
      }
};

/********************************************
 * mat_34 - a 3x4 matrix
 */

class mat_34 : public mat_x4_template<vec_3>
{
   public:
      mat_34( ) { }

      mat_34( const mat_x4_template<vec_3>& mat )
      {
         col0 = mat.col0; col1 = mat.col1; col2 = mat.col2; col3 = mat.col3;
      }

      mat_34( const vec_3 col_0, const vec_3 col_1,
              const vec_3 col_2, const vec_3 col_3 ) {
         col0 = col_0; col1 = col_1; col2 = col_2; col3 = col_3;
      }

      mat_34( const zero_34 );

      explicit inline mat_34(const mat_44& mat);

      vec_4 get_row3() const;     // undefined
      void set_row3(vec_4);       // undefined

      // negate

      mat_34 operator - () const;

      // transpose

      mat_43 transpose() const;

      // matrix/scalar operations

      mat_34 operator * ( float scale ) const;
      mat_34 operator * ( vec_x vec ) const;
      mat_34 operator * ( vec_y vec ) const;
      mat_34 operator * ( vec_z vec ) const;
      mat_34 operator * ( vec_w vec ) const;

      void operator *= ( float scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_x scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_y scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_z scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_w scale ) {
         *this = *this * scale;
      }

      // matrix/vector operations

      vec_3
      operator * ( vec_4 vec ) const {
         return mat_x4_template<vec_3>::operator * (vec);
      }

      vec_3
      operator * ( vector_t vec ) const {
         return mat_x4_template<vec_3>::operator * (vec);
      }

      vec_3
      operator * ( point_t vec ) const {
         return mat_x4_template<vec_3>::operator * (vec);
      }

      vec_4
      trans_mult( vec_3 vec ) const {
         vec128_t result, temp0, temp1, temp2, temp3;

         asm ( "### mat_34 trans_mult vec_3 ### \n"
               "vmul temp3, col3, vec \n"
               "vmul temp0, col0, vec \n"
               "vmul temp1, col1, vec \n"
               "vmul temp2, col2, vec \n"
               "vmulx.w temp3, vf00, temp3 \n"
               "vaddy.x temp0, temp0, temp0 \n"
               "vaddx.y temp1, temp1, temp1 \n"
               "vaddx.z temp2, temp2, temp2 \n"
               "vaddy.w temp3, temp3, temp3 \n"
               "vaddz.x result, temp0, temp0 \n"
               "vaddz.y result, temp1, temp1 \n"
               "vaddy.z result, temp2, temp2 \n"
               "vaddz.w result, temp3, temp3 \n"
               : "=&j result" (result),
               "=&j temp0" (temp0), "=&j temp1" (temp1), "=&j temp2" (temp2), "=&j temp3" (temp3)
               : "j col0" (col0), "j col1" (col1), "j col2" (col2), "j col3" (col3), "j vec" (vec)
            );

         return vec_4(result);
      }

      // matrix/matrix operations

      mat_34 operator + ( const mat_34& mat ) const;
      mat_34 operator - ( const mat_34& mat ) const;

      void   operator += ( const mat_34& mat ) {
         *this = *this + mat;
      }
      void   operator -= ( const mat_34& mat ) {
         *this = *this - mat;
      }

      mat_33 operator * ( const mat_43& mat ) const;
      mat_34 operator * ( const mat_44& mat ) const;
      mat_34 operator * ( const transform_t& mat ) const;

      mat_43 trans_mult ( const mat_33& mat ) const;
      mat_44 trans_mult ( const mat_34& mat ) const;

      mat_33 mult_trans ( const mat_34& mat ) const;
      mat_34 mult_trans ( const mat_44& mat ) const;

      mat_34 &
      operator = ( const mat_33& mat ) {
         col0 = mat.col0;
         col1 = mat.col1;
         col2 = mat.col2;
         return *this;
      }

      // matrix/zero_matrix operations

      mat_34 operator + ( const zero_34 zero ) const;
      mat_34 operator - ( const zero_34 zero ) const;

      void operator += ( const zero_34 zero );
      void operator -= ( const zero_34 zero );

      zero_33 operator * ( const zero_43 zero ) const;
      zero_34 operator * ( const zero_44 zero ) const;

      zero_43 trans_mult ( const zero_33 zero ) const;
      zero_44 trans_mult ( const zero_34 zero ) const;

      zero_33 mult_trans ( const zero_34 zero ) const;
      zero_34 mult_trans ( const zero_44 zero ) const;

      mat_34& operator = ( const zero_34 zero );

      void print( ) const {
         get_row0().print();
         get_row1().print();
         get_row2().print();
      }

      void print(const char *mat_name) const {
         printf("%s:\n", mat_name);
         print();
      }
};

/********************************************
 * mat_44 - a 4x4 matrix
 */

class mat_44 : public mat_x4_template<vec_4>
{
   public:

      mat_44( ) { }

      mat_44( const mat_x4_template<vec_4>& mat )
      {
         col0 = mat.col0; col1 = mat.col1; col2 = mat.col2; col3 = mat.col3;
      }

      mat_44( const zero_44 );

      mat_44( const vec_4 col_0, const vec_4 col_1,
              const vec_4 col_2, const vec_4 col_3)
      {
         col0 = col_0; col1 = col_1; col2 = col_2; col3 = col_3;
      }

      explicit inline mat_44(const transform_t& mat);

      void set_identity( ) {
         asm ( " ### mat_44::set_identity ### \n"
               "vsub      col0, col0, col0 \n"
               "vsub      col1, col1, col1 \n"
               "vmr32    col2, vf00 \n"
               "vmove    col3, vf00 \n"
               "vaddw.x    col0, vf00, vf00 \n"
               "vaddw.y    col1, vf00, vf00 \n"
               : "=j col0" (col0),
               "=j col1" (col1),
               "=j col2" (col2),
               "=j col3" (col3)
            );
      }

      void
      set_scale( vec_3 scale ) {
         set_identity();
         col0 = (vec_x)scale;
         col1 = (vec_y)scale;
         col2 = (vec_z)scale;
      }

      void
      set_scale( vec_4 scale ) {
         set_identity();
         col0 = (vec_x)scale;
         col1 = (vec_y)scale;
         col2 = (vec_z)scale;
         col3 = (vec_w)scale;
      }

      void
      set_translate( vec_3 xlate_amount ) {
         set_identity();
         col3 = xlate_amount;
      }

      void
      set_rotate_x( float angle ) {
         float cs = cosf(angle);
         float sn = sinf(angle);
         col0.set( 1, 0, 0, 0 );
         col1.set( 0, cs, sn, 0 );
         col2.set( 0, -sn, cs, 0 );
         col3.set( 0, 0, 0, 1 );
      }

      void
      set_rotate_y( float angle ) {
         float cs = cosf(angle);
         float sn = sinf(angle);
         col0.set( cs, 0, -sn, 0 );
         col1.set( 0, 1, 0, 0 );
         col2.set( sn, 0, cs, 0 );
         col3.set( 0, 0, 0, 1 );
      }

      void
      set_rotate_z( float angle ) {
         float cs = cosf(angle);
         float sn = sinf(angle);
         col0.set( cs, sn, 0, 0 );
         col1.set( -sn, cs, 0, 0 );
         col2.set( 0, 0, 1, 0 );
         col3.set( 0, 0, 0, 1 );
      }

      void set_rotate( float angle, vec_xyz axis );

      // negate

      mat_44 operator - () const;

      // transpose

      mat_44 transpose() const;

      void
      transpose_in_place()
      {
         *this = transpose();
      }

      // matrix/scalar operations

      mat_44 operator * ( float scale ) const;
      mat_44 operator * ( vec_x vec ) const;
      mat_44 operator * ( vec_y vec ) const;
      mat_44 operator * ( vec_z vec ) const;
      mat_44 operator * ( vec_w vec ) const;

      void operator *= ( float scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_x scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_y scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_z scale ) {
         *this = *this * scale;
      }
      void operator *= ( vec_w scale ) {
         *this = *this * scale;
      }

      // matrix/vector operations

      vec_4
      operator * ( vec_4 vec ) const {
         return mat_x4_template<vec_4>::operator * (vec);
      }

      vec_4
      operator * ( vector_t vec ) const {
         return mat_x4_template<vec_4>::operator * (vec);
      }

      vec_4
      operator * ( point_t vec ) const {
         return mat_x4_template<vec_4>::operator * (vec);
      }

      vec_4
      trans_mult( vec_4 vec ) const {
         vec128_t result, temp0, temp1, temp2, temp3;

         asm ( "### mat_44 trans_mult vec_4 ### \n"
               "vmul temp0, col0, vec \n"
               "vmul temp1, col1, vec \n"
               "vmul temp2, col2, vec \n"
               "vmul temp3, col3, vec \n"
               "vaddy.x temp0, temp0, temp0 \n"
               "vaddx.y temp1, temp1, temp1 \n"
               "vaddx.z temp2, temp2, temp2 \n"
               "vaddx.w temp3, temp3, temp3 \n"
               "vaddz.x temp0, temp0, temp0 \n"
               "vaddz.y temp1, temp1, temp1 \n"
               "vaddy.z temp2, temp2, temp2 \n"
               "vaddy.w temp3, temp3, temp3 \n"
               "vaddw.x result, temp0, temp0 \n"
               "vaddw.y result, temp1, temp1 \n"
               "vaddw.z result, temp2, temp2 \n"
               "vaddz.w result, temp3, temp3 \n"
               : "=&j result" (result),
               "=&j temp0" (temp0), "=&j temp1" (temp1), "=&j temp2" (temp2), "=&j temp3" (temp3)
               : "j col0" (col0),
               "j col1" (col1),
               "j col2" (col2),
               "j col3" (col3),
               "j vec" (vec)
            );

         return vec_4(result);
      }

      vec_4
      trans_mult( vector_t vec ) const {
         vec128_t result, temp0, temp1, temp2, temp3;

         asm ( "### mat_44 trans_mult vector_t ### \n"
               "vmul temp3, col3, vec \n"
               "vmul temp0, col0, vec \n"
               "vmul temp1, col1, vec \n"
               "vmul temp2, col2, vec \n"
               "vmulx.w temp3, vf00, temp3 \n"
               "vaddy.x temp0, temp0, temp0 \n"
               "vaddx.y temp1, temp1, temp1 \n"
               "vaddx.z temp2, temp2, temp2 \n"
               "vaddy.w temp3, temp3, temp3 \n"
               "vaddz.x result, temp0, temp0 \n"
               "vaddz.y result, temp1, temp1 \n"
               "vaddy.z result, temp2, temp2 \n"
               "vaddz.w result, temp3, temp3 \n"
               : "=&j result" (result),
               "=&j temp0" (temp0), "=&j temp1" (temp1), "=&j temp2" (temp2), "=&j temp3" (temp3)
               : "j col0" (col0),
               "j col1" (col1),
               "j col2" (col2),
               "j col3" (col3),
               "j vec" (vec)
            );

         return vec_4(result);
      }

      vec_4
      trans_mult( point_t vec ) const {
         vec128_t result, temp0, temp1, temp2, temp3;

         asm ( "### mat_44 trans_mult point_t ### \n"
               "vmul temp0, col0, vec \n"
               "vmul temp1, col1, vec \n"
               "vmul temp2, col2, vec \n"
               "vmul temp3, col3, vec \n"
               "vaddy.x temp0, temp0, temp0 \n"
               "vaddx.y temp1, temp1, temp1 \n"
               "vaddx.z temp2, temp2, temp2 \n"
               "vaddx.w temp3, col3, temp3 \n"
               "vaddz.x temp0, temp0, temp0 \n"
               "vaddz.y temp1, temp1, temp1 \n"
               "vaddy.z temp2, temp2, temp2 \n"
               "vaddy.w temp3, temp3, temp3 \n"
               "vaddw.x result, temp0, col0 \n"
               "vaddw.y result, temp1, col1 \n"
               "vaddw.z result, temp2, col2 \n"
               "vaddz.w result, temp3, temp3 \n"
               : "=&j result" (result),
               "=&j temp0" (temp0), "=&j temp1" (temp1), "=&j temp2" (temp2), "=&j temp3" (temp3)
               : "j col0" (col0),
               "j col1" (col1),
               "j col2" (col2),
               "j col3" (col3),
               "j vec" (vec)
            );
         return vec_4(result);
      }

      // matrix/matrix operations

      mat_44 operator + ( const mat_44& mat ) const;
      mat_44 operator - ( const mat_44& mat ) const;

      void   operator += ( const mat_44& mat ) {
         *this = *this + mat;
      }
      void   operator -= ( const mat_44& mat ) {
         *this = *this - mat;
      }

      mat_44 operator * ( const mat_44& mat ) const;
      mat_43 operator * ( const mat_43& mat ) const;
      mat_44 operator * ( const transform_t& mat ) const;

      mat_44 trans_mult ( const mat_44& mat ) const;
      mat_43 trans_mult ( const mat_43& mat ) const;
      mat_44 trans_mult ( const transform_t& mat ) const;

      mat_44 mult_trans( const mat_44& mat ) const;
      mat_43 mult_trans( const mat_34& mat ) const;

      mat_44 &
      operator = ( const mat_33& mat ) {
         col0 = mat.col0;
         col1 = mat.col1;
         col2 = mat.col2;
         return *this;
      }

      mat_44 &
      operator = ( const mat_34& mat ) {
         col0 = mat.col0;
         col1 = mat.col1;
         col2 = mat.col2;
         col3 = mat.col3;
         return *this;
      }

      mat_44 &
      operator = ( const mat_43& mat ) {
         col0 = mat.col0;
         col1 = mat.col1;
         col2 = mat.col2;
         return *this;
      }

      mat_44 &
      operator = ( const mat_44& mat ) {
         col0 = mat.col0;
         col1 = mat.col1;
         col2 = mat.col2;
         col3 = mat.col3;
         return *this;
      }

      // matrix/zero_matrix operations

      mat_44 operator + ( const zero_44 zero ) const;
      mat_44 operator - ( const zero_44 zero ) const;

      void operator += ( const zero_44 zero );
      void operator -= ( const zero_44 zero );

      zero_44 operator * ( const zero_44 zero ) const;
      zero_43 operator * ( const zero_43 zero ) const;

      zero_44 trans_mult ( const zero_44 zero ) const;
      zero_43 trans_mult ( const zero_43 zero ) const;

      zero_44 mult_trans( const zero_44 zero ) const;
      zero_43 mult_trans( const zero_34 zero ) const;

      mat_44& operator = ( const zero_44 zero );

      void print( ) const {
         get_row0().print();
         get_row1().print();
         get_row2().print();
         get_row3().print();
      }

      void print(const char *mat_name) const {
         printf("%s:\n", mat_name);
         print();
      }
};

/********************************************
 * transform_t - a transform type consisting of
 * 3 vectors and a point
 */

class transform_t
{
   public:
      vector_t col0;
      vector_t col1;
      vector_t col2;
      point_t col3;

      transform_t( ) { }

      transform_t( const transform_t& mat )
      {
         col0 = mat.col0;
         col1 = mat.col1;
         col2 = mat.col2;
         col3 = mat.col3;
      }

      transform_t( const mat_33& rotation, vec_3 translation ) {
         col0 = vector_t(rotation.col0);
         col1 = vector_t(rotation.col1);
         col2 = vector_t(rotation.col2);
         col3 = point_t(translation);
      }

      transform_t( const vector_t col_0, const vector_t col_1,
                   const vector_t col_2, const point_t col_3 ) {
         col0 = col_0; col1 = col_1; col2 = col_2; col3 = col_3;
      }

      void set_identity( ) {
         asm ( " ### transform_t::set_identity ### \n"
               "vsub      col0, col0, col0 \n"
               "vsub      col1, col1, col1 \n"
               "vmr32    col2, vf00 \n"
               "vmove    col3, vf00 \n"
               "vaddw.x    col0, vf00, vf00 \n"
               "vaddw.y    col1, vf00, vf00 \n"
               : "=j col0" (col0),
               "=j col1" (col1),
               "=j col2" (col2),
               "=j col3" (col3)
            );
      }

      void set_zero( ) {
         asm ( " ### transform_t::set_zero ### \n"
               "vsub    col0, col0, col0 \n"
               "vsub    col1, col1, col1 \n"
               "vsub    col2, col2, col2 \n"
               "vsub    col3, col3, col3 \n"
               : "=j col0" (col0),
               "=j col1" (col1),
               "=j col2" (col2),
               "=j col3" (col3)
            );
      }

      void
      set_translation( vec_3 translation ) {
         col3 = point_t(translation);
      }

      void
      set_rotation( mat_33 rotation ) {
         col0 = vector_t(rotation.col0);
         col1 = vector_t(rotation.col1);
         col2 = vector_t(rotation.col2);
      }

      void
      set( mat_33 rotation, vec_3 translation ) {
         set_rotation(rotation);
         set_translation(translation);
      }

      void
      set_rotate_x( float angle ) {
         float cs = cosf(angle);
         float sn = sinf(angle);
         col0.set( 1, 0, 0 );
         col1.set( 0, cs, sn );
         col2.set( 0, -sn, cs );
         col3.set_zero();
      }

      void
      set_rotate_y( float angle ) {
         float cs = cosf(angle);
         float sn = sinf(angle);
         col0.set( cs, 0, -sn );
         col1.set( 0, 1, 0 );
         col2.set( sn, 0, cs );
         col3.set_zero();
      }

      void
      set_rotate_z( float angle ) {
         float cs = cosf(angle);
         float sn = sinf(angle);
         col0.set( cs, sn, 0 );
         col1.set( -sn, cs, 0 );
         col2.set( 0, 0, 1 );
         col3.set_zero();
      }

      void set_col0( vector_t new_col ) { col0 = new_col; }
      void set_col1( vector_t new_col ) { col1 = new_col; }
      void set_col2( vector_t new_col ) { col2 = new_col; }
      void set_col3( point_t new_col ) { col3 = new_col; }

      void set_row0( vec_4 new_row ) {
         asm (
            " ### transform_t::set_row0 ### \n"
            "vaddx.x       col0, vf00, new_row \n"
            "vaddy.x       col1, vf00, new_row \n"
            "vaddz.x       col2, vf00, new_row \n"
            "vaddw.x       col3, vf00, new_row \n"
            : "+j col0" (col0),
            "+j col1" (col1),
            "+j col2" (col2),
            "+j col3" (col3)
            : "j new_row" (new_row)
            );
      }

      void set_row1( vec_4 new_row ) {
         asm (
            " ### transform_t::set_row1 ### \n"
            "vaddx.y       col0, vf00, new_row \n"
            "vaddy.y       col1, vf00, new_row \n"
            "vaddz.y       col2, vf00, new_row \n"
            "vaddw.y       col3, vf00, new_row \n"
            : "+j col0" (col0),
            "+j col1" (col1),
            "+j col2" (col2),
            "+j col3" (col3)
            : "j new_row" (new_row)
            );
      }

      void set_row2( vec_4 new_row ) {
         asm (
            " ### transform_t::set_row2 ### \n"
            "vaddx.z        col0, vf00, new_row \n"
            "vaddy.z        col1, vf00, new_row \n"
            "vaddz.z        col2, vf00, new_row \n"
            "vaddw.z        col3, vf00, new_row \n"
            : "+j col0" (col0),
            "+j col1" (col1),
            "+j col2" (col2),
            "+j col3" (col3)
            : "j new_row" (new_row)
            );
      }

      vector_t get_col0( ) const { return col0; }
      vector_t get_col1( ) const { return col1; }
      vector_t get_col2( ) const { return col2; }
      point_t get_col3( ) const { return col3; }

      vec_4 get_row0( ) const {
         vec128_t row;
         asm ( " ### transform_t::get_row0 ### \n"
               "vaddx.x row, vf00, col0 \n"
               "vaddx.y row, vf00, col1 \n"
               "vaddx.z row, vf00, col2 \n"
               "vmulx.w row, vf00, col3 \n"
               : "=&j row" (row)
               : "j col0" (col0),
               "j col1" (col1),
               "j col2" (col2),
               "j col3" (col3)
            );
         return vec_4(row);
      }

      vec_4 get_row1( ) const {
         vec128_t row;
         asm ( " ### transform_t::get_row1 ### \n"
               "vaddy.x row, vf00, col0 \n"
               "vaddy.y row, vf00, col1 \n"
               "vaddy.z row, vf00, col2 \n"
               "vmuly.w row, vf00, col3 \n"
               : "=&j row" (row)
               : "j col0" (col0),
               "j col1" (col1),
               "j col2" (col2),
               "j col3" (col3)
            );
         return vec_4(row);
      }

      vec_4 get_row2( ) const {
         vec128_t row;
         asm ( " ### transform_t::get_row2 ### \n"
               "vaddz.x row, vf00, col0 \n"
               "vaddz.y row, vf00, col1 \n"
               "vaddz.z row, vf00, col2 \n"
               "vmulz.w row, vf00, col3 \n"
               : "=&j row" (row)
               : "j col0" (col0),
               "j col1" (col1),
               "j col2" (col2),
               "j col3" (col3)
            );
         return vec_4(row);
      }

      vec_4 get_row3( ) const {
         vec128_t row;
         asm ( " ### transform_t::get_row3 ### \n"
               "vmove %0, vf00 \n"
               : "=j" (row)
            );
         return vec_4(row);
      }

      // inverses

      transform_t
      inverse() const;

      void
      inverse_in_place() {
         *this = inverse();
      }

      transform_t
      orthonormal_inverse() const;

      void
      orthonormal_inverse_in_place()
      {
         vec128_t temp;
         asm (
            "### transform_t::orthonormal_inverse_in_place ### \n"
            "vadd.xz temp, vf00, col1 \n"
            "vaddx.y temp, vf00, col2 \n"
            "vaddy.x col1, vf00, col0 \n"
            "vaddy.z col1, vf00, col2 \n"
            "vaddz.x col2, vf00, col0 \n"
            "vaddy.z col0, vf00, temp \n"
            "vaddx.y col0, vf00, temp \n"
            "vaddz.y col2, vf00, temp \n"
            "vadda ACC, vf00, vf00 \n"
            "vmsubay ACC, col1, col3 \n"
            "vmsubax ACC, col0, col3 \n"
            "vmsubz col3, col2, col3 \n"
            : "+j col0" (col0),
            "+j col1" (col1),
            "+j col2" (col2),
            "+j col3" (col3),
            "=j temp" (temp), "=r" (vu0_ACC)
            );
      }

      // matrix/vector operations

      vec_4
      operator * ( vec_4 vec ) const {
         vec128_t result;
         asm (
            " ### transform_t * vec_4 ### \n"
            "vmulax    ACC, col0, vec      \n"
            "vmadday    ACC, col1, vec      \n"
            "vmaddaz    ACC, col2, vec      \n"
            "vmaddw.xyz   result, col3, vec      \n"
            "vmove.w   result, vec     \n"
            : "=&j result" (result), "=r" (vu0_ACC)
            : "j vec" (vec),
            "j col0" (col0), "j col1" (col1),
            "j col2" (col2), "j col3" (col3)
            );
         return vec_4(result);
      }

      vector_t
      operator * ( vector_t vec ) const {
         vec128_t result;
         asm (
            " ### transform_t * vector_t ### \n"
            "vmulax    ACC, col0, vec      \n"
            "vmadday    ACC, col1, vec    \n"
            "vmaddz    result, col2, vec  \n"
            : "=&j result" (result), "=r" (vu0_ACC)
            : "j vec" (vec),
            "j col0" (col0), "j col1" (col1), "j col2" (col2)
            );
         return vector_t(result);
      }

      point_t
      operator * ( point_t pt ) const {
         vec128_t result;
         asm (
            " ### transform_t * point_t ### \n"
            "vmulax    ACC, col0, pt      \n"
            "vmadday    ACC, col1, pt      \n"
            "vmaddaz    ACC, col2, pt      \n"
            "vmaddw    result, col3, vf00    \n"
            : "=&j result" (result), "=r" (vu0_ACC)
            : "j pt" (pt),
            "j col0" (col0), "j col1" (col1),
            "j col2" (col2), "j col3" (col3)
            );
         return point_t(result);
      }

      // matrix/matrix operations

      transform_t operator * ( const transform_t& mat ) const;
      mat_44 operator * ( const mat_44& mat ) const;
      mat_43 operator * ( const mat_43& mat ) const;

      void operator = ( const transform_t& xform ) {
         col0 = xform.col0;
         col1 = xform.col1;
         col2 = xform.col2;
         col3 = xform.col3;
      }

      void print( ) const {
         get_row0().print();
         get_row1().print();
         get_row2().print();
         get_row3().print();
      }

      void print(const char *mat_name) const {
         printf("%s:\n", mat_name);
         print();
      }
};

/********************************************
 * matrix operations
 */

// vec_3

inline
mat_33
vec_xyz::operator ~() const return result;
{
   asm ( "### make tilde matrix from vec_xyz ### \n"
         "vsub res0, res0, res0 \n"
         "vsub res1, res1, res1 \n"
         "vmr32 res2, vf00 \n"
         "vaddw.x res0, vf00, vf00 \n"
         "vaddw.y res1, vf00, vf00 \n"
         "vopmula ACC, vec, res0 \n"
         "vopmsub res0, res0, vec \n"
         "vopmula ACC, vec, res1 \n"
         "vopmsub res1, res1, vec \n"
         "vopmula ACC, vec, res2 \n"
         "vopmsub res2, res2, vec \n"
         : "=&j res0" (result.col0),
         "=&j res1" (result.col1),
         "=&j res2" (result.col2), "=r" (vu0_ACC)
         : "j vec" (vec128)
      );
}

inline
mat_33
vec_xyz::tilde_mult( const mat_33& mat ) const return result;
{
   result.col0 = this->cross(mat.col0);
   result.col1 = this->cross(mat.col1);
   result.col2 = this->cross(mat.col2);
}

inline
mat_34
vec_xyz::tilde_mult( const mat_34& mat ) const return result;
{
   result.col0 = this->cross(mat.col0);
   result.col1 = this->cross(mat.col1);
   result.col2 = this->cross(mat.col2);
   result.col3 = this->cross(mat.col3);
}

inline
vec_xyz
vec_xyz::operator * ( const mat_33& mat ) const
{
   vec128_t ones, temp0, temp1, temp2, result;

   asm ( "### vec_xyz (row) * mat_33 ## \n"
         "vmul temp0, vec, col0 \n"
         "vmaxw ones, vf00, vf00 \n"
         "vmul temp1, vec, col1 \n"
         "vmul temp2, vec, col2 \n"
         "vadday.x ACC, temp0, temp0 \n"
         "vmaddz.x res, ones, temp0 \n"
         "vaddax.y ACC, temp1, temp1 \n"
         "vmaddz.y res, ones, temp1 \n"
         "vaddax.z ACC, temp2, temp2 \n"
         "vmaddy.z res, ones, temp2 \n"
         : "=&j res" (result), "=&j ones" (ones),
         "=&j temp0" (temp0),
         "=&j temp1" (temp1),
         "=&j temp2" (temp2), "=r" (vu0_ACC)
         : "j col0" (mat.col0),
         "j col1" (mat.col1),
         "j col2" (mat.col2),
         "j vec" (vec128)
      );
   return vec_xyz(result);
}

inline
vec_xyzw
vec_xyz::operator * ( const mat_34& mat ) const
{
   vec128_t ones, temp0, temp1, temp2, temp3, result;

   asm ( "### vec_xyz (row) * mat_34 ## \n"
         "vmul temp0, vec, col0 \n"
         "vmaxw ones, vf00, vf00 \n"
         "vmul temp1, vec, col1 \n"
         "vmul temp2, vec, col2 \n"
         "vmul temp3, vec, col3 \n"
         : "=&j ones" (ones),
         "=&j temp0" (temp0),
         "=&j temp1" (temp1),
         "=&j temp2" (temp2),
         "=&j temp3" (temp3)
         : "j col0" (mat.col0),
         "j col1" (mat.col1),
         "j col2" (mat.col2),
         "j col3" (mat.col3),
         "j vec" (vec128)
      );

   asm ( "vmulx.w temp3, vf00, temp3 \n"
         "vadday.x ACC, temp0, temp0 \n"
         "vmaddz.x res, ones, temp0 \n"
         "vaddax.y ACC, temp1, temp1 \n"
         "vmaddz.y res, ones, temp1 \n"
         "vaddax.z ACC, temp2, temp2 \n"
         "vmaddy.z res, ones, temp2 \n"
         "vadday.w ACC, temp3, temp3 \n"
         "vmaddz.w res, ones, temp3 \n"
         : "=&j res" (result), "=r" (vu0_ACC)
         : "j ones" (ones),
         "j temp0" (temp0),
         "j temp1" (temp1),
         "j temp2" (temp2),
         "j temp3" (temp3)
      );

   return vec_xyzw(result);
}

inline
mat_33
vec_xyz::tensor_mult( const vec_3 vec ) const return result
{
   result.col0 = vec * vec_x(*this);
   result.col1 = vec * vec_y(*this);
   result.col2 = vec * vec_z(*this);
}

inline
mat_43
vec_xyz::tensor_mult( const vec_4 vec ) const return result
{
   result.col0 = vec * vec_x(*this);
   result.col1 = vec * vec_y(*this);
   result.col2 = vec * vec_z(*this);
}

// vec_4

inline
vec_xyz
vec_xyzw::operator * ( const mat_43& mat ) const
{
   vec128_t ones, temp0, temp1, temp2, result;

   asm ( "### vec_xyzw (row) * mat_43 ## \n"
         "vmul temp0, vec, col0 \n"
         "vmaxw ones, vf00, vf00 \n"
         "vmul temp1, vec, col1 \n"
         "vmul temp2, vec, col2 \n"
         "vadday.x ACC, temp0, temp0 \n"
         "vmaddaz.x ACC, ones, temp0 \n"
         "vmaddw.x res, ones, temp0 \n"
         "vaddax.y ACC, temp1, temp1 \n"
         "vmaddaz.y ACC, ones, temp1 \n"
         "vmaddw.y res, ones, temp1 \n"
         "vaddax.z ACC, temp2, temp2 \n"
         "vmadday.z ACC, ones, temp2 \n"
         "vmaddw.z res, ones, temp2 \n"
         : "=&j res" (result), "=&j ones" (ones),
         "=&j temp0" (temp0),
         "=&j temp1" (temp1),
         "=&j temp2" (temp2), "=r" (vu0_ACC)
         : "j col0" (mat.col0),
         "j col1" (mat.col1),
         "j col2" (mat.col2),
         "j vec" (vec128)
      );

   return vec_xyz(result);
}

inline
vec_xyzw
vec_xyzw::operator * ( const mat_44& mat ) const
{
   vec128_t ones, temp0, temp1, temp2, temp3, result;

   asm ( "### vec_xyzw (row) * mat_44 ## \n"
         "vmaxw ones, vf00, vf00 \n"
         "vmul temp0, vec, col0 \n"
         "vmul temp1, vec, col1 \n"
         "vmul temp2, vec, col2 \n"
         "vmul temp3, vec, col3 \n"
         : "=&j ones" (ones),
         "=&j temp0" (temp0),
         "=&j temp1" (temp1),
         "=&j temp2" (temp2),
         "=&j temp3" (temp3)
         : "j col0" (mat.col0),
         "j col1" (mat.col1),
         "j col2" (mat.col2),
         "j col3" (mat.col3),
         "j vec" (vec128)
      );

   asm ( "vadday.x ACC, temp0, temp0 \n"
         "vmaddaz.x ACC, ones, temp0 \n"
         "vmaddw.x res, ones, temp0 \n"
         "vaddax.y ACC, temp1, temp1 \n"
         "vmaddaz.y ACC, ones, temp1 \n"
         "vmaddw.y res, ones, temp1 \n"
         "vaddax.z ACC, temp2, temp2 \n"
         "vmadday.z ACC, ones, temp2 \n"
         "vmaddw.z res, ones, temp2 \n"
         "vaddax.w ACC, temp3, temp3 \n"
         "vmadday.w ACC, ones, temp3 \n"
         "vmaddz.w res, ones, temp3 \n"
         : "=&j res" (result), "=r" (vu0_ACC)
         : "j ones" (ones),
         "j temp0" (temp0),
         "j temp1" (temp1),
         "j temp2" (temp2),
         "j temp3" (temp3)
      );

   return vec_xyzw(result);
}

inline
mat_34
vec_xyzw::tensor_mult( const vec_3 vec ) const return result
{
   result.col0 = vec * vec_x(*this);
   result.col1 = vec * vec_y(*this);
   result.col2 = vec * vec_z(*this);
   result.col3 = vec * vec_w(*this);
}

inline
mat_44
vec_xyzw::tensor_mult( const vec_4 vec ) const return result
{
   result.col0 = vec * vec_x(*this);
   result.col1 = vec * vec_y(*this);
   result.col2 = vec * vec_z(*this);
   result.col3 = vec * vec_w(*this);
}

// mat_33

// explicit constructors

inline
mat_33::mat_33(const mat_43& mat)
{
   col0 = vec_3(mat.col0);
   col1 = vec_3(mat.col1);
   col2 = vec_3(mat.col2);
}

inline
mat_33::mat_33(const mat_34& mat)
{
   col0 = mat.col0;
   col1 = mat.col1;
   col2 = mat.col2;
}

inline
mat_33::mat_33(const mat_44& mat)
{
   col0 = vec_3(mat.col0);
   col1 = vec_3(mat.col1);
   col2 = vec_3(mat.col2);
}

// negate

inline
mat_33
mat_33::operator - () const return result;
{
   result.col0 = -col0;
   result.col1 = -col1;
   result.col2 = -col2;
}

// transpose & inverse

inline
mat_33
mat_33::transpose() const return result;
{
   vec128_t temp0, temp1, temp2, temp3;

   asm ( "### mat_33::transpose ### \n"
         "pextlw temp0, col1, col0 \n"
         "pextuw temp1, col1, col0 \n"
         "pextlw temp2, $0, col2 \n"
         "pextuw temp3, $0, col2 \n"
         : "=&r temp0" (temp0),
         "=&r temp1" (temp1),
         "=&r temp2" (temp2),
         "=&r temp3" (temp3)
         : "r col0" (col0),
         "r col1" (col1),
         "r col2" (col2)
      );

   asm ( "pcpyld res0, temp2, temp0 \n"
         "pcpyud res1, temp0, temp2 \n"
         "pcpyld res2, temp3, temp1 \n"
         : "=&r res0" (result.col0),
         "=&r res1" (result.col1),
         "=&r res2" (result.col2)
         : "r temp0" (temp0),
         "r temp1" (temp1),
         "r temp2" (temp2),
         "r temp3" (temp3)
      );
}

inline
mat_33
mat_33::inverse() const return result;
{
   vec128_t temp, determinant;

   asm ( "### mat_33::inverse ### \n"

         "vopmula.xyz ACC, col0, col1                # inv2 = crossproduct(col0, col1) \n"
         "vopmsub.xyz inv2, col1, col0 \n"
         "vopmula.xyz ACC, col1, col2                # inv0 = crossproduct(col1, col2) \n"
         "vopmsub.xyz inv0, col2, col1 \n"
         "                                           # stall \n"
         "vmul determinant, col2, inv2               # determinant(R) = dotproduct(col2, inv2) \n"
         "vaddw.x temp, vf00, vf00  \n"
         "vopmula.xyz ACC, col2, col0                # inv1 = crossproduct(col2, col0) \n"
         "vopmsub.xyz inv1, col0, col2  \n"
         "vadday.x ACCx, determinant, determinant \n"
         "vmaddz.x determinant, temp, determinant \n"

         "vaddx.y temp, vf00, inv2                   # Do an in-place transpose, produces determinant(R)*Rinv \n"
         "vadd.xz temp, vf00, inv1 \n"
         "vaddy.x inv1, vf00, inv0 \n"
         "vdiv Q, vf00w, determinantx                # Q = 1/determinant(R) \n"
         "vaddy.z inv1, vf00, inv2 \n"
         "vaddz.x inv2, vf00, inv0 \n"
         "vaddy.z inv0, vf00, temp \n"
         "vaddx.y inv0, vf00, temp \n"
         "vaddz.y inv2, vf00, temp \n"
         "vwaitq \n"
         "vmulq.xyz inv1, inv1, Q \n"
         "vmulq.xyz inv0, inv0, Q                    # Multiply by 1/determinant(R) \n"
         "vmulq.xyz inv2, inv2, Q \n"

         : "=&j inv0" (result.col0),
         "=&j inv1" (result.col1),
         "=&j inv2" (result.col2),
         "=&j temp" (temp), "=&j determinant" (determinant), "=r" (vu0_ACC)
         : "j col0" (col0),
         "j col1" (col1),
         "j col2" (col2)
      );
}

// matrix/scalar operations

inline
mat_33
mat_33::operator * ( float scale ) const return result;
{
   vec_x vec(scale);

   result.col0 = col0 * vec;
   result.col1 = col1 * vec;
   result.col2 = col2 * vec;
}

inline
mat_33
mat_33::operator * ( const vec_x scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
}

inline
mat_33
mat_33::operator * ( const vec_y scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
}

inline
mat_33
mat_33::operator * ( const vec_z scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
}

inline
mat_33
mat_33::operator * ( const vec_w scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
}

// matrix/vector operations

inline
mat_33
mat_33::mult_tilde ( vec_3 vec ) const return result;
{
   asm ( "### mat_33 mult_tilde vec_3 ### \n"
         "vmulaz ACC, col1, vec \n"
         "vmsuby res0, col2, vec \n"
         "vmulax ACC, col2, vec \n"
         "vmsubz res1, col0, vec \n"
         "vmulay ACC, col0, vec \n"
         "vmsubx res2, col1, vec \n"
         : "=&j res0" (result.col0), "=&j res1" (result.col1), "=&j res2" (result.col2), "=r" (vu0_ACC)
         : "j col0" (col0), "j col1" (col1), "j col2" (col2), "j vec" (vec)
      );
}

// matrix/matrix operations

inline
mat_33
mat_33::operator + ( const mat_33& mat ) const return result;
{
   result.col0 = col0 + mat.get_col0();
   result.col1 = col1 + mat.get_col1();
   result.col2 = col2 + mat.get_col2();
}

inline
mat_33
mat_33::operator - ( const mat_33& mat ) const return result;
{
   result.col0 = col0 - mat.get_col0();
   result.col1 = col1 - mat.get_col1();
   result.col2 = col2 - mat.get_col2();
}

inline
mat_33
mat_33::operator * ( const mat_33& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
}

inline
mat_34
mat_33::operator * ( const mat_34& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
   result.col3 = *this * mat.get_col3();
}

inline
mat_33
mat_33::trans_mult( const mat_33& mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
}

inline
mat_34
mat_33::trans_mult( const mat_34& mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
   result.col3 = this->trans_mult(mat.get_col3());
}

inline
mat_33
mat_33::mult_trans( const mat_33 &mat ) const return result;
{
   result.col0 = this->mult_trans_col0(mat);
   result.col1 = this->mult_trans_col1(mat);
   result.col2 = this->mult_trans_col2(mat);
}

inline
mat_34
mat_33::mult_trans( const mat_43 &mat ) const return result;
{
   result.col0 = this->mult_trans_col0(mat);
   result.col1 = this->mult_trans_col1(mat);
   result.col2 = this->mult_trans_col2(mat);
   result.col3 = this->mult_trans_col3(mat);
}
// mat_44

// explicit constructors

inline
mat_44::mat_44(const transform_t& mat)
{
   col0 = vec_4(mat.col0);
   col1 = vec_4(mat.col1);
   col2 = vec_4(mat.col2);
   col3 = vec_4(mat.col3);
}

// negate

inline
mat_44
mat_44::operator - () const return result;
{
   result.col0 = -col0;
   result.col1 = -col1;
   result.col2 = -col2;
   result.col3 = -col3;
}

// transpose

inline
mat_44
mat_44::transpose() const return result;
{
   vec128_t temp0, temp1, temp2, temp3;

   asm ( "### mat_44::transpose ### \n"
         "pextlw temp0, col1, col0 \n"
         "pextuw temp1, col1, col0 \n"
         "pextlw temp2, col3, col2 \n"
         "pextuw temp3, col3, col2 \n"
         : "=&r temp0" (temp0),
         "=&r temp1" (temp1),
         "=&r temp2" (temp2),
         "=&r temp3" (temp3)
         : "r col0" (col0),
         "r col1" (col1),
         "r col2" (col2),
         "r col3" (col3)
      );

   asm ( "pcpyld res0, temp2, temp0 \n"
         "pcpyud res1, temp0, temp2 \n"
         "pcpyld res2, temp3, temp1 \n"
         "pcpyud res3, temp1, temp3 \n"
         : "=&r res0" (result.col0),
         "=&r res1" (result.col1),
         "=&r res2" (result.col2),
         "=&r res3" (result.col3)
         : "r temp0" (temp0),
         "r temp1" (temp1),
         "r temp2" (temp2),
         "r temp3" (temp3)
      );
}

// matrix/scalar operations

inline
mat_44
mat_44::operator * ( float scale ) const return result;
{
   vec_x vec(scale);

   result.col0 = col0 * vec;
   result.col1 = col1 * vec;
   result.col2 = col2 * vec;
   result.col3 = col3 * vec;
}

inline
mat_44
mat_44::operator * ( const vec_x scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
   result.col3 = col3 * scale;
}

inline
mat_44
mat_44::operator * ( const vec_y scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
   result.col3 = col3 * scale;
}

inline
mat_44
mat_44::operator * ( const vec_z scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
   result.col3 = col3 * scale;
}

inline
mat_44
mat_44::operator * ( const vec_w scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
   result.col3 = col3 * scale;
}

// matrix/matrix operations

inline
mat_44
mat_44::operator + ( const mat_44& mat ) const return result;
{
   result.col0 = col0 + mat.get_col0();
   result.col1 = col1 + mat.get_col1();
   result.col2 = col2 + mat.get_col2();
   result.col3 = col3 + mat.get_col3();
}

inline
mat_44
mat_44::operator - ( const mat_44& mat ) const return result;
{
   result.col0 = col0 - mat.get_col0();
   result.col1 = col1 - mat.get_col1();
   result.col2 = col2 - mat.get_col2();
   result.col3 = col3 - mat.get_col3();
}

inline
mat_44
mat_44::operator * ( const mat_44& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
   result.col3 = *this * mat.get_col3();
}

inline
mat_43
mat_44::operator * ( const mat_43& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
}

inline
mat_44
mat_44::operator * ( const transform_t& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
   result.col3 = *this * mat.get_col3();
}

inline
mat_44
mat_44::trans_mult( const mat_44 &mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
   result.col3 = this->trans_mult(mat.get_col3());
}

inline
mat_43
mat_44::trans_mult( const mat_43& mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
}

inline
mat_44
mat_44::trans_mult( const transform_t& mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
   result.col3 = this->trans_mult(mat.get_col3());
}

inline
mat_44
mat_44::mult_trans( const mat_44& mat ) const return result;
{
   result.col0 = this->mult_trans_col0(mat);
   result.col1 = this->mult_trans_col1(mat);
   result.col2 = this->mult_trans_col2(mat);
   result.col3 = this->mult_trans_col3(mat);
}

inline
mat_43
mat_44::mult_trans( const mat_34& mat ) const return result;
{
   result.col0 = this->mult_trans_col0(mat);
   result.col1 = this->mult_trans_col1(mat);
   result.col2 = this->mult_trans_col2(mat);
}

// mat_43

// explicit constructors

inline
mat_43::mat_43(const mat_44& mat)
{
   col0 = mat.col0;
   col1 = mat.col1;
   col2 = mat.col2;
}

// negate

inline
mat_43
mat_43::operator - () const return result;
{
   result.col0 = -col0;
   result.col1 = -col1;
   result.col2 = -col2;
}

// transpose

inline
mat_34
mat_43::transpose() const return result;
{
   vec128_t temp0, temp1, temp2, temp3;

   asm ( "### mat_43::transpose ### \n"
         "pextlw temp0, col1, col0 \n"
         "pextuw temp1, col1, col0 \n"
         "pextlw temp2, $0, col2 \n"
         "pextuw temp3, $0, col2 \n"
         : "=&r temp0" (temp0),
         "=&r temp1" (temp1),
         "=&r temp2" (temp2),
         "=&r temp3" (temp3)
         : "r col0" (col0),
         "r col1" (col1),
         "r col2" (col2)
      );

   asm ( "pcpyld res0, temp2, temp0 \n"
         "pcpyud res1, temp0, temp2 \n"
         "pcpyld res2, temp3, temp1 \n"
         "pcpyud res3, temp1, temp3 \n"
         : "=&r res0" (result.col0),
         "=&r res1" (result.col1),
         "=&r res2" (result.col2),
         "=&r res3" (result.col3)
         : "r temp0" (temp0),
         "r temp1" (temp1),
         "r temp2" (temp2),
         "r temp3" (temp3)
      );
}

// matrix/scalar operations

inline
mat_43
mat_43::operator * ( float scale ) const return result;
{
   vec_x vec(scale);

   result.col0 = col0 * vec;
   result.col1 = col1 * vec;
   result.col2 = col2 * vec;
}

inline
mat_43
mat_43::operator * ( const vec_x scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
}

inline
mat_43
mat_43::operator * ( const vec_y scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
}

inline
mat_43
mat_43::operator * ( const vec_z scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
}

inline
mat_43
mat_43::operator * ( const vec_w scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
}

// matrix/vector operations

inline
mat_43
mat_43::mult_tilde ( vec_3 vec ) const return result;
{
   asm ( "### mat_43 mult_tilde vec_3 ### \n"
         "vmulaz ACC, col1, vec \n"
         "vmsuby res0, col2, vec \n"
         "vmulax ACC, col2, vec \n"
         "vmsubz res1, col0, vec \n"
         "vmulay ACC, col0, vec \n"
         "vmsubx res2, col1, vec \n"
         : "=&j res0" (result.col0), "=&j res1" (result.col1), "=&j res2" (result.col2), "=r" (vu0_ACC)
         : "j col0" (col0), "j col1" (col1), "j col2" (col2), "j vec" (vec)
      );
}

// matrix/matrix operations

inline
mat_43
mat_43::operator + ( const mat_43& mat ) const return result;
{
   result.col0 = col0 + mat.get_col0();
   result.col1 = col1 + mat.get_col1();
   result.col2 = col2 + mat.get_col2();
}    
    
inline
mat_43
mat_43::operator - ( const mat_43& mat ) const return result;
{
   result.col0 = col0 - mat.get_col0();
   result.col1 = col1 - mat.get_col1();
   result.col2 = col2 - mat.get_col2();
}    
    
inline
mat_43
mat_43::operator * ( const mat_33& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
}

inline
mat_44
mat_43::operator * ( const mat_34& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
   result.col3 = *this * mat.get_col3();
}

inline
mat_33
mat_43::trans_mult( const mat_43& mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
}

inline
mat_34
mat_43::trans_mult( const mat_44& mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
   result.col3 = this->trans_mult(mat.get_col3());
}

inline
mat_34
mat_43::trans_mult( const transform_t& mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
   result.col3 = this->trans_mult(mat.get_col3());
}

inline
mat_44
mat_43::mult_trans ( const mat_43& mat ) const return result;
{
   result.col0 = this->mult_trans_col0(mat);
   result.col1 = this->mult_trans_col1(mat);
   result.col2 = this->mult_trans_col2(mat);
   result.col3 = this->mult_trans_col3(mat);
}

inline
mat_43
mat_43::mult_trans ( const mat_33& mat ) const return result;
{
   result.col0 = this->mult_trans_col0(mat);
   result.col1 = this->mult_trans_col1(mat);
   result.col2 = this->mult_trans_col2(mat);
}

// mat_34

// explicit constructors

inline
mat_34::mat_34(const mat_44& mat)
{
   col0 = vec_3(mat.col0);
   col1 = vec_3(mat.col1);
   col2 = vec_3(mat.col2);
   col3 = vec_3(mat.col3);
}

// negate

inline
mat_34
mat_34::operator - () const return result;
{
   result.col0 = -col0;
   result.col1 = -col1;
   result.col2 = -col2;
   result.col3 = -col3;
}

// transpose

inline
mat_43
mat_34::transpose() const return result;
{
   vec128_t temp0, temp1, temp2, temp3;

   asm ( "### mat_34::transpose ### \n"
         "pextlw temp0, col1, col0 \n"
         "pextuw temp1, col1, col0 \n"
         "pextlw temp2, col3, col2 \n"
         "pextuw temp3, col3, col2 \n"
         : "=&r temp0" (temp0),
         "=&r temp1" (temp1),
         "=&r temp2" (temp2),
         "=&r temp3" (temp3)
         : "r col0" (col0),
         "r col1" (col1),
         "r col2" (col2),
         "r col3" (col3)
      );

   asm ( "pcpyld res0, temp2, temp0 \n"
         "pcpyud res1, temp0, temp2 \n"
         "pcpyld res2, temp3, temp1 \n"
         : "=&r res0" (result.col0),
         "=&r res1" (result.col1),
         "=&r res2" (result.col2)
         : "r temp0" (temp0),
         "r temp1" (temp1),
         "r temp2" (temp2),
         "r temp3" (temp3)
      );
}

// matrix/scalar operations

inline
mat_34
mat_34::operator * ( float scale ) const return result;
{
   vec_x vec(scale);

   result.col0 = col0 * vec;
   result.col1 = col1 * vec;
   result.col2 = col2 * vec;
   result.col3 = col3 * vec;
}

inline
mat_34
mat_34::operator * ( const vec_x scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
   result.col3 = col3 * scale;
}

inline
mat_34
mat_34::operator * ( const vec_y scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
   result.col3 = col3 * scale;
}

inline
mat_34
mat_34::operator * ( const vec_z scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
   result.col3 = col3 * scale;
}

inline
mat_34
mat_34::operator * ( const vec_w scale ) const return result;
{
   result.col0 = col0 * scale;
   result.col1 = col1 * scale;
   result.col2 = col2 * scale;
   result.col3 = col3 * scale;
}

// matrix/matrix operations

inline
mat_34
mat_34::operator + ( const mat_34& mat ) const return result;
{
   result.col0 = col0 + mat.get_col0();
   result.col1 = col1 + mat.get_col1();
   result.col2 = col2 + mat.get_col2();
   result.col3 = col3 + mat.get_col3();
}    

inline
mat_34
mat_34::operator - ( const mat_34& mat ) const return result;
{
   result.col0 = col0 - mat.get_col0();
   result.col1 = col1 - mat.get_col1();
   result.col2 = col2 - mat.get_col2();
   result.col3 = col3 - mat.get_col3();
}    

inline
mat_33
mat_34::operator * ( const mat_43& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
}

inline
mat_34
mat_34::operator * ( const mat_44& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
   result.col3 = *this * mat.get_col3();
}

inline
mat_34
mat_34::operator * ( const transform_t& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
   result.col3 = *this * mat.get_col3();
}    

inline
mat_43
mat_34::trans_mult( const mat_33& mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
}

inline
mat_44
mat_34::trans_mult( const mat_34& mat ) const return result;
{
   result.col0 = this->trans_mult(mat.get_col0());
   result.col1 = this->trans_mult(mat.get_col1());
   result.col2 = this->trans_mult(mat.get_col2());
   result.col3 = this->trans_mult(mat.get_col3());
}

inline
mat_33
mat_34::mult_trans ( const mat_34& mat ) const return result;
{
   result.col0 = this->mult_trans_col0(mat);
   result.col1 = this->mult_trans_col1(mat);
   result.col2 = this->mult_trans_col2(mat);
}

inline    
mat_34
mat_34::mult_trans ( const mat_44& mat ) const return result;
{
   result.col0 = this->mult_trans_col0(mat);
   result.col1 = this->mult_trans_col1(mat);
   result.col2 = this->mult_trans_col2(mat);
   result.col3 = this->mult_trans_col3(mat);
}

// transform_t

// inverses

inline
transform_t
transform_t::inverse() const return result;
{
   vec128_t temp, determinant;

   asm ( "### transform_t::inverse, part I ### \n"
         "vopmula.xyz ACC, col0, col1                  # inv2 = crossproduct(col0, col1) \n"
         "vopmsub.xyz inv2, col1, col0 \n"
         "vopmula.xyz ACC, col1, col2                  # inv0 = crossproduct(col1, col2) \n"
         "vopmsub.xyz inv0, col2, col1 \n"
         "                                             # stall \n"
         "vmul determinant, col2, inv2                 # determinant(R) = dotproduct(col2, inv2) \n"
         "vaddw.x temp, vf00, vf00  \n"
         "vopmula.xyz ACC, col2, col0                  # inv1 = crossproduct(col2, col0) \n"
         "vopmsub.xyz inv1, col0, col2  \n"
         "vadday.x ACC, determinant, determinant \n"
         "vmaddz.x determinant, temp, determinant \n"

         "vaddx.y temp, vf00, inv2                     # Do an in-place transpose, produces determinant(R)*Rinv \n"
         "vadd.xz temp, vf00, inv1 \n"
         "vaddy.x inv1, vf00, inv0 \n"
         "vdiv Q, vf00w, determinantx                  # Q = 1/determinant(R) \n"
         "vaddy.z inv1, vf00, inv2 \n"
         "vaddz.x inv2, vf00, inv0 \n"
         "vaddy.z inv0, vf00, temp \n"
         "vaddx.y inv0, vf00, temp \n"
         "vaddz.y inv2, vf00, temp \n"
         : "=&j inv0" (result.col0),
         "=&j inv1" (result.col1),
         "=&j inv2" (result.col2),
         "=&j temp" (temp),
         "=&j determinant" (determinant), "=r" (vu0_ACC), "=j" (vu0_Q)
         : "j col0" (col0),
         "j col1" (col1),
         "j col2" (col2)
      );

   asm ( "### transform_t::inverse, part II ### \n"

         "vadda ACC, vf00, vf00                        # Compute determinant(R)*Rinv -T \n"
         "vmsubay ACC, inv1, col3 \n"
         "vmsubax ACC, inv0, col3 \n"
         "vmsubz inv3, inv2, col3 \n"

         "vmulq.xyz inv0, inv0, Q                      # Multiply by 1/determinant(R) \n"
         "vmulq.xyz inv1, inv1, Q \n"
         "vmulq.xyz inv2, inv2, Q \n"
         "vmulq.xyz inv3, inv3, Q \n"
         : "+j inv0" (result.col0),
         "+j inv1" (result.col1),
         "+j inv2" (result.col2),
         "=&j inv3" (result.col3),
         "=r" (vu0_ACC)
         : "j col3" (col3), "j" (vu0_Q)
      );
}

inline
transform_t
transform_t::orthonormal_inverse() const return result;
{
   asm ( "### transform_t::orthonormal_inverse ### \n"
         "vadd.x inv0, vf00, col0 \n"
         "vadd.y inv1, vf00, col1 \n"
         "vadd.z inv2, vf00, col2 \n"
         "vaddx.y inv0, vf00, col1 \n"
         "vaddy.x inv1, vf00, col0 \n"
         "vaddz.x inv2, vf00, col0 \n"
         "vaddx.z inv0, vf00, col2 \n"
         "vaddy.z inv1, vf00, col2 \n"
         "vaddz.y inv2, vf00, col1 \n"
         "vadda ACC, vf00, vf00 \n"
         "vmsubax ACC, inv0, col3 \n"
         "vmsubay ACC, inv1, col3 \n"
         "vmsubz inv3, inv2, col3 \n"
         : "=&j inv0" (result.col0),
         "=&j inv1" (result.col1),
         "=&j inv2" (result.col2),
         "=&j inv3" (result.col3), "=r" (vu0_ACC)
         : "j col0" (col0),
         "j col1" (col1),
         "j col2" (col2),
         "j col3" (col3)
      );
}
    
// matrix/matrix operations

inline
transform_t
transform_t::operator * ( const transform_t& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
   result.col3 = *this * mat.get_col3();
}

inline
mat_44
transform_t::operator * ( const mat_44& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
   result.col3 = *this * mat.get_col3();
}

inline
mat_43
transform_t::operator * ( const mat_43& mat ) const return result;
{
   result.col0 = *this * mat.get_col0();
   result.col1 = *this * mat.get_col1();
   result.col2 = *this * mat.get_col2();
}

#endif // matrix_h
