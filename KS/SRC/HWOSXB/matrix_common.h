/*           Copyright (C) 2001 Sony Computer Entertainment America
                              All Rights Reserved
                               SCEA Confidential                                */

#ifndef matrix_common_h
#define matrix_common_h

#include "vector.h"

// mat_x3_template - template for 3 column matrices

template <class column_type>
class mat_x3_template
{
  public:  
    column_type col0, col1, col2;

    mat_x3_template() { }
    
    mat_x3_template( const mat_x3_template& mat )
    {
      col0 = mat.col0; col1 = mat.col1; col2 = mat.col2;
    }
    
    mat_x3_template( const column_type col_0, const column_type col_1, const column_type col_2 )
    {
      col0 = col_0; col1 = col_1; col2 = col_2;
    }
        
    void set_zero( ) {
      asm (
        " ### mat_x3::set_zero ### \n"
        "vsub    col0, col0, col0 \n"
        "vsub    col1, col1, col1 \n"
        "vsub    col2, col2, col2 \n"
        : "=j col0" (col0.vec128),
          "=j col1" (col1.vec128),
          "=j col2" (col2.vec128)
        );
    }

    void set_col0( column_type new_col ) { col0 = new_col; }  
    void set_col1( column_type new_col ) { col1 = new_col; }    
    void set_col2( column_type new_col ) { col2 = new_col; }
        
    void set_row0( vec_3 new_row ) {
      asm (
        " ### mat_x3::set_row0 ### \n"
        "vaddx.x       col0, vf00, new_row \n"
        "vaddy.x       col1, vf00, new_row \n"
        "vaddz.x       col2, vf00, new_row \n"
        : "+j col0" (col0.vec128),
          "+j col1" (col1.vec128),
          "+j col2" (col2.vec128)
        : "j new_row" (new_row.vec128)
        );
    }
    
    void set_row1( vec_3 new_row ) {
      asm (
        " ### mat_x3::set_row1 ### \n"
        "vaddx.y       col0, vf00, new_row \n"
        "vaddy.y       col1, vf00, new_row \n"
        "vaddz.y       col2, vf00, new_row \n"
        : "+j col0" (col0.vec128),
          "+j col1" (col1.vec128),
          "+j col2" (col2.vec128)
        : "j new_row" (new_row.vec128)
        );
    }
    
    void set_row2( vec_3 new_row ) {
      asm (
        " ### mat_x3::set_row2 ### \n"
        "vaddx.z        col0, vf00, new_row \n"
        "vaddy.z        col1, vf00, new_row \n"
        "vaddz.z        col2, vf00, new_row \n"
        : "+j col0" (col0.vec128),
          "+j col1" (col1.vec128),
          "+j col2" (col2.vec128)
        : "j new_row" (new_row.vec128)
        );
    }
    
    void set_row3( vec_3 new_row ) {
      asm (
        " ### mat_x3::set_row3 ### \n"
        "vmulx.w        col0, vf00, new_row \n"
        "vmuly.w        col1, vf00, new_row \n"
        "vmulz.w        col2, vf00, new_row \n"
        : "+j col0" (col0.vec128),
          "+j col1" (col1.vec128),
          "+j col2" (col2.vec128)
        : "j new_row" (new_row.vec128)
        );
    }

    column_type get_col0( ) const { return col0; }
    column_type get_col1( ) const { return col1; }
    column_type get_col2( ) const { return col2; }
    
    vec_3 get_row0( ) const {
      vec128_t row;
      asm (
        " ### mat_x3::get_row0 ### \n"
        "vaddx.x row_, vf00, col0 \n"
        "vaddx.y row_, vf00, col1 \n"
        "vaddx.z row_, vf00, col2 \n"
        : "=&j row_" (row)
        : "j col0" (col0.vec128),
          "j col1" (col1.vec128),
          "j col2" (col2.vec128)
        );
      return vec_3(row);    
    }    
    
    vec_3 get_row1( ) const {
      vec128_t row;
      asm (
        " ### mat_x3::get_row1 ### \n"
        "vaddy.x row_, vf00, col0 \n"
        "vaddy.y row_, vf00, col1 \n"
        "vaddy.z row_, vf00, col2 \n"
        : "=&j row_" (row)
        : "j col0" (col0.vec128),
          "j col1" (col1.vec128),
          "j col2" (col2.vec128)
        );
      return vec_3(row);    
    }    
    
    vec_3 get_row2( ) const {
      vec128_t row;
      asm (
        " ### mat_x3::get_row2 ### \n"
        "vaddz.x row_, vf00, col0 \n"
        "vaddz.y row_, vf00, col1 \n"
        "vaddz.z row_, vf00, col2 \n"
        : "=&j row_" (row)
        : "j col0" (col0.vec128),
          "j col1" (col1.vec128),
          "j col2" (col2.vec128)
        );
      return vec_3(row);    
    }    
    
    vec_3 get_row3( ) const {
      vec128_t row;
      asm (
        " ### mat_x3::get_row3 ### \n"
        "vaddw.x row_, vf00, col0 \n"
        "vaddw.y row_, vf00, col1 \n"
        "vaddw.z row_, vf00, col2 \n"
        : "=&j row_" (row)
        : "j col0" (col0.vec128),
          "j col1" (col1.vec128),
          "j col2" (col2.vec128)
        );
      return vec_3(row);    
    }

    column_type
    operator * ( vec_3 vec ) const {
      vec128_t result;
      asm (
        " ### mat_x3 * vec_3 ### \n"
        "vmulax    ACC, col0, vec      \n"
        "vmadday    ACC, col1, vec      \n"
        "vmaddz    result, col2, vec      \n"
        : "=&j result" (result), "=r" (vu0_ACC)
        : "j vec" (vec.vec128),
          "j col0" (col0.vec128), "j col1" (col1.vec128), "j col2" (col2.vec128)
        );
      return column_type(result);
    }

    template<class mat_column_type>
    column_type
    mult_trans_col0( const mat_x3_template<mat_column_type>& mat ) const {
      vec128_t result;
      asm ( "### mat_x3 mult_trans mat_x3, column 0 of result ### \n"
            "vmulax ACC, col0, mat0 \n"
            "vmaddax ACC, col1, mat1 \n"
            "vmaddx res0, col2, mat2 \n"
          : "=&j res0" (result), "=r" (vu0_ACC)
          : "j col0" (col0.vec128),
            "j col1" (col1.vec128),
            "j col2" (col2.vec128),
            "j mat0" (mat.col0.vec128),
            "j mat1" (mat.col1.vec128),
            "j mat2" (mat.col2.vec128)
          );
      return column_type(result);
    }

    template<class mat_column_type>
    column_type
    mult_trans_col1( const mat_x3_template<mat_column_type>& mat ) const {
      vec128_t result;
      asm ( "### mat_x3 mult_trans mat_x3, column 1 of result ### \n"
            "vmulay ACC, col0, mat0 \n"
            "vmadday ACC, col1, mat1 \n"
            "vmaddy res1, col2, mat2 \n"
          : "=&j res1" (result), "=r" (vu0_ACC)
          : "j col0" (col0.vec128),
            "j col1" (col1.vec128),
            "j col2" (col2.vec128),
            "j mat0" (mat.col0.vec128),
            "j mat1" (mat.col1.vec128),
            "j mat2" (mat.col2.vec128)
          );
      return column_type(result);
    }

    template<class mat_column_type>
    column_type
    mult_trans_col2( const mat_x3_template<mat_column_type>& mat ) const {
      vec128_t result;
      asm ( "### mat_x3 mult_trans mat_x3, column 2 of result ### \n"
            "vmulaz ACC, col0, mat0 \n"
            "vmaddaz ACC, col1, mat1 \n"
            "vmaddz res2, col2, mat2 \n"
          : "=&j res2" (result), "=r" (vu0_ACC)
          : "j col0" (col0.vec128),
            "j col1" (col1.vec128),
            "j col2" (col2.vec128),
            "j mat0" (mat.col0.vec128),
            "j mat1" (mat.col1.vec128),
            "j mat2" (mat.col2.vec128)
          );
      return column_type(result);
    }

    template<class mat_column_type>
    column_type
    mult_trans_col3( const mat_x3_template<mat_column_type>& mat ) const {
      vec128_t result;
      asm ( "### mat_x3 mult_trans mat_x3, column 3 of result ### \n"
            "vmulaw ACC, col0, mat0 \n"
            "vmaddaw ACC, col1, mat1 \n"
            "vmaddw res3, col2, mat2 \n"
          : "=&j res3" (result), "=r" (vu0_ACC)
          : "j col0" (col0.vec128),
            "j col1" (col1.vec128),
            "j col2" (col2.vec128),
            "j mat0" (mat.col0.vec128),
            "j mat1" (mat.col1.vec128),
            "j mat2" (mat.col2.vec128)
          );
      return column_type(result);
    }
};

// mat_x4_template - template for 4 column matrices

template <class column_type>
class mat_x4_template
{
  public:
    column_type col0, col1, col2, col3;

    mat_x4_template( ) { }

    mat_x4_template( const mat_x4_template& mat )
    {
      col0 = mat.col0; col1 = mat.col1; col2 = mat.col2; col3 = mat.col3;
    }

    mat_x4_template( const column_type col_0, const column_type col_1,
                     const column_type col_2, const column_type col_3 )
    {
      col0 = col_0; col1 = col_1; col2 = col_2; col3 = col_3;
    }

    void set_zero( ) {
      asm (
        " ### mat_x4::set_zero ### \n"
        "vsub    col0, col0, col0 \n"
        "vsub    col1, col1, col1 \n"
        "vsub    col2, col2, col2 \n"
        "vsub    col3, col3, col3 \n"
        : "=j col0" (col0.vec128),
          "=j col1" (col1.vec128),
          "=j col2" (col2.vec128),
          "=j col3" (col3.vec128)
        );
    }

    void set_col0( column_type new_col ) { col0 = new_col; }
    void set_col1( column_type new_col ) { col1 = new_col; }
    void set_col2( column_type new_col ) { col2 = new_col; }
    void set_col3( column_type new_col ) { col3 = new_col; }

    void set_row0( vec_4 new_row ) {
      asm (
        " ### mat_x4::set_row0 ### \n"
        "vaddx.x       col0, vf00, new_row \n"
        "vaddy.x       col1, vf00, new_row \n"
        "vaddz.x       col2, vf00, new_row \n"
        "vaddw.x       col3, vf00, new_row \n"
        : "+j col0" (col0.vec128),
          "+j col1" (col1.vec128),
          "+j col2" (col2.vec128),
          "+j col3" (col3.vec128)
        : "j new_row" (new_row.vec128)
        );
    }

    void set_row1( vec_4 new_row ) {
      asm (
        " ### mat_x4::set_row1 ### \n"
        "vaddx.y       col0, vf00, new_row \n"
        "vaddy.y       col1, vf00, new_row \n"
        "vaddz.y       col2, vf00, new_row \n"
        "vaddw.y       col3, vf00, new_row \n"
        : "+j col0" (col0.vec128),
          "+j col1" (col1.vec128),
          "+j col2" (col2.vec128),
          "+j col3" (col3.vec128)
        : "j new_row" (new_row.vec128)
        );
    }

    void set_row2( vec_4 new_row ) {
      asm (
        " ### mat_x4::set_row2 ### \n"
        "vaddx.z        col0, vf00, new_row \n"
        "vaddy.z        col1, vf00, new_row \n"
        "vaddz.z        col2, vf00, new_row \n"
        "vaddw.z        col3, vf00, new_row \n"
        : "+j col0" (col0.vec128),
          "+j col1" (col1.vec128),
          "+j col2" (col2.vec128),
          "+j col3" (col3.vec128)
        : "j new_row" (new_row.vec128)
        );
    }

    void set_row3( vec_4 new_row ) {
      asm (
        " ### mat_x4::set_row3 ### \n"
        "vmulx.w        col0, vf00, new_row \n"
        "vmuly.w        col1, vf00, new_row \n"
        "vmulz.w        col2, vf00, new_row \n"
        "vmulw.w        col3, vf00, new_row \n"
        : "+j col0" (col0.vec128),
          "+j col1" (col1.vec128),
          "+j col2" (col2.vec128),
          "+j col3" (col3.vec128)
        : "j new_row" (new_row.vec128)
        );
    }

    column_type get_col0( ) const { return col0; }
    column_type get_col1( ) const { return col1; }
    column_type get_col2( ) const { return col2; }
    column_type get_col3( ) const { return col3; }

    vec_4 get_row0( ) const {
      vec128_t row;
      asm (
        " ### mat_x4::get_row0 ### \n"
        "vaddx.x row_, vf00, col0 \n"
        "vaddx.y row_, vf00, col1 \n"
        "vaddx.z row_, vf00, col2 \n"
        "vmulx.w row_, vf00, col3 \n"
        : "=&j row_" (row)
        : "j col0" (col0.vec128),
          "j col1" (col1.vec128),
          "j col2" (col2.vec128),
          "j col3" (col3.vec128)
        );
      return vec_4(row);
    }

    vec_4 get_row1( ) const {
      vec128_t row;
      asm (
        " ### mat_x4::get_row1 ### \n"
        "vaddy.x row_, vf00, col0 \n"
        "vaddy.y row_, vf00, col1 \n"
        "vaddy.z row_, vf00, col2 \n"
        "vmuly.w row_, vf00, col3 \n"
        : "=&j row_" (row)
        : "j col0" (col0.vec128),
          "j col1" (col1.vec128),
          "j col2" (col2.vec128),
          "j col3" (col3.vec128)
        );
      return vec_4(row);
    }

    vec_4 get_row2( ) const {
      vec128_t row;
      asm (
        " ### mat_x4::get_row2 ### \n"
        "vaddz.x row_, vf00, col0 \n"
        "vaddz.y row_, vf00, col1 \n"
        "vaddz.z row_, vf00, col2 \n"
        "vmulz.w row_, vf00, col3 \n"
        : "=&j row_" (row)
        : "j col0" (col0.vec128),
          "j col1" (col1.vec128),
          "j col2" (col2.vec128),
          "j col3" (col3.vec128)
        );
      return vec_4(row);
    }

    vec_4 get_row3( ) const {
      vec128_t row;
      asm (
        " ### mat_x4::get_row3 ### \n"
        "vaddw.x row_, vf00, col0 \n"
        "vaddw.y row_, vf00, col1 \n"
        "vaddw.z row_, vf00, col2 \n"
        "vmulw.w row_, vf00, col3 \n"
        : "=&j row_" (row)
        : "j col0" (col0.vec128),
          "j col1" (col1.vec128),
          "j col2" (col2.vec128),
          "j col3" (col3.vec128)
        );
      return vec_4(row);
    }

    column_type
    operator * ( vec_4 vec ) const {
      vec128_t result;
      asm (
        " ### mat_x4 * vec_4 ### \n"
        "vmulax    ACC, col0, vec      \n"
        "vmadday    ACC, col1, vec      \n"
        "vmaddaz    ACC, col2, vec      \n"
        "vmaddw    result, col3, vec      \n"
        : "=&j result" (result), "=r" (vu0_ACC)
        : "j vec" (vec.vec128),
          "j col0" (col0.vec128), "j col1" (col1.vec128),
          "j col2" (col2.vec128), "j col3" (col3.vec128)
        );
      return column_type(result);
    }

    column_type
    operator * ( vector_t vec ) const {
      vec128_t result;
      asm (
        " ### mat_x4 * vector_t ### \n"
        "vmulax    ACC, col0, vec      \n"
        "vmadday    ACC, col1, vec      \n"
        "vmaddz    result, col2, vec      \n"
        : "=&j result" (result), "=r" (vu0_ACC)
        : "j vec" (vec.vec128),
          "j col0" (col0.vec128), "j col1" (col1.vec128), "j col2" (col2.vec128)
        );
      return column_type(result);
    }

    column_type
    operator * ( point_t pt ) const {
      vec128_t result;
      asm (
        " ### mat_x4 * point_t ### \n"
        "vmulax    ACC, col0, pt      \n"
        "vmadday    ACC, col1, pt      \n"
        "vmaddaz    ACC, col2, pt      \n"
        "vmaddw    result, col3, vf00    \n"
        : "=&j result" (result), "=r" (vu0_ACC)
        : "j pt" (pt.vec128),
          "j col0" (col0.vec128), "j col1" (col1.vec128),
          "j col2" (col2.vec128), "j col3" (col3.vec128)
        );
      return column_type(result);
    }

    template<class mat_column_type>
    column_type
    mult_trans_col0( const mat_x4_template<mat_column_type>& mat ) const {
      vec128_t result;
      asm ( "### mat_x4 mult_trans mat_x4, column 0 of result ### \n"
            "vmulax ACC, col0, mat0 \n"
            "vmaddax ACC, col1, mat1 \n"
            "vmaddax ACC, col2, mat2 \n"
            "vmaddx res0, col3, mat3 \n"
          : "=&j res0" (result), "=r" (vu0_ACC)
          : "j col0" (col0.vec128),
            "j col1" (col1.vec128),
            "j col2" (col2.vec128),
            "j col3" (col3.vec128),
            "j mat0" (mat.col0.vec128),
            "j mat1" (mat.col1.vec128),
            "j mat2" (mat.col2.vec128),
            "j mat3" (mat.col3.vec128)
          );
      return column_type(result);
    }

    template<class mat_column_type>
    column_type
    mult_trans_col1( const mat_x4_template<mat_column_type>& mat ) const {
      vec128_t result;
      asm ( "### mat_x4 mult_trans mat_x4, column 1 of result ### \n"
            "vmulay ACC, col0, mat0 \n"
            "vmadday ACC, col1, mat1 \n"
            "vmadday ACC, col2, mat2 \n"
            "vmaddy res1, col3, mat3 \n"
          : "=&j res1" (result), "=r" (vu0_ACC)
          : "j col0" (col0.vec128),
            "j col1" (col1.vec128),
            "j col2" (col2.vec128),
            "j col3" (col3.vec128),
            "j mat0" (mat.col0.vec128),
            "j mat1" (mat.col1.vec128),
            "j mat2" (mat.col2.vec128),
            "j mat3" (mat.col3.vec128)
          );
      return column_type(result);
    }

    template<class mat_column_type>
    column_type
    mult_trans_col2( const mat_x4_template<mat_column_type>& mat ) const {
      vec128_t result;
      asm ( "### mat_x4 mult_trans mat_x4, column 2 of result ### \n"
            "vmulaz ACC, col0, mat0 \n"
            "vmaddaz ACC, col1, mat1 \n"
            "vmaddaz ACC, col2, mat2 \n"
            "vmaddz res2, col3, mat3 \n"
          : "=&j res2" (result), "=r" (vu0_ACC)
          : "j col0" (col0.vec128),
            "j col1" (col1.vec128),
            "j col2" (col2.vec128),
            "j col3" (col3.vec128),
            "j mat0" (mat.col0.vec128),
            "j mat1" (mat.col1.vec128),
            "j mat2" (mat.col2.vec128),
            "j mat3" (mat.col3.vec128)
          );
      return column_type(result);
    }

    template<class mat_column_type>
    column_type
    mult_trans_col3( const mat_x4_template<mat_column_type>& mat ) const {
      vec128_t result;
      asm ( "### mat_x4 mult_trans mat_x4, column 3 of result ### \n"
            "vmulaw ACC, col0, mat0 \n"
            "vmaddaw ACC, col1, mat1 \n"
            "vmaddaw ACC, col2, mat2 \n"
            "vmaddw res3, col3, mat3 \n"
          : "=&j res3" (result), "=r" (vu0_ACC)
          : "j col0" (col0.vec128),
            "j col1" (col1.vec128),
            "j col2" (col2.vec128),
            "j col3" (col3.vec128),
            "j mat0" (mat.col0.vec128),
            "j mat1" (mat.col1.vec128),
            "j mat2" (mat.col2.vec128),
            "j mat3" (mat.col3.vec128)
          );        
      return column_type(result);
    }
};

#endif // matrix_common_h
