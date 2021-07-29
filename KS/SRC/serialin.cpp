////////////////////////////////////////////////////////////////////////////////
/*
	serialin.cpp
 
	various data structure loading functions
*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "character.h"
#include "face.h"
#include "material.h"
#include "oserrmsg.h"
#include "txtcoord.h"
#include "vsplit.h"
#include "wedge.h"
#include "textfile.h"
#include "chunkfile.h"


////////////////////////////////////////////////////////////////////////////////
// face
////////////////////////////////////////////////////////////////////////////////
void serial_in( chunk_file& io, face* f, const chunk_flavor& face_flavor )
  {
  // vanilla face
  serial_in( io, &f->wedge_refs[0] );
  serial_in( io, &f->wedge_refs[1] );
  serial_in( io, &f->wedge_refs[2] );
  int temp_material;
  serial_in( io, &temp_material );
  f->my_material = temp_material;
  f->level_of_detail = 0;
  f->flags = 0;  // default
  // extensions
  if ( face_flavor == CHUNK_FACE_WITH_FLAGS ) 
    serial_in( io, &f->flags );
  else if ( face_flavor == CHUNK_PROGRESSIVE_FACE )
    {
    for(;;)
      {
      chunk_flavor cf;
      serial_in( io, &cf );
      if( cf == chunk_flavor("flags") )
        {
        serial_in( io, &f->flags );
        }
      else if( cf == chunk_flavor("detail") )
        {
        serial_in( io, &f->level_of_detail );
        }
      else if( cf == CHUNK_END )
        {
        break;
        }
      else
        {
        stringx comp = io.get_name() + ": unknown chunk '" + cf.to_stringx() + "'.";
        error( comp.c_str() );
        }
      }
    }
  }

////////////////////////////////////////////////////////////////////////////////
// vsplit
////////////////////////////////////////////////////////////////////////////////
void serial_in( chunk_file& io, vsplit* v )
  {
/*
  io >> v->old_vert;
  io >> v->new_vert_ids[0];
  io >> v->new_vert_ids[1];
  io >> v->new_face_ids[0];
  io >> v->new_face_ids[1];
  int nwsplits;
  io >> nwsplits;
  v->wedge_splits = vector<wedge_split>(nwsplits);
  for(int j=0;j<v->wedge_splits.size();j++)
    {
    io >> v->wedge_splits[j].old_wedge_id;
    io >> v->wedge_splits[j].new_wedge_id;
    }
*/
  serial_in(io,&v->old_vert);
  serial_in(io,&v->new_vert_ids[0]);
  serial_in(io,&v->new_vert_ids[1]);
  serial_in(io,&v->new_face_ids[0]);
  serial_in(io,&v->new_face_ids[1]);
  int nwsplits;
  serial_in(io,&nwsplits);
  v->wedge_splits=vector<wedge_split>(nwsplits);
  for(unsigned j=0;j<v->wedge_splits.size();j++)
    {
    serial_in(io,&v->wedge_splits[j].old_wedge_id);
    serial_in(io,&v->wedge_splits[j].new_wedge_id);
    }
  }

////////////////////////////////////////////////////////////////////////////////
// wedge
////////////////////////////////////////////////////////////////////////////////
void serial_in( chunk_file& io, wedge* w, vert_ref* vert_ref_for_wedge )
  {
  // this is only used for collision geometry now
  // set defaults
  w->lower_detail = UNINITIALIZED_WEDGE_REF;
  w->level_of_detail = 0;

  // wedges always begin with vert ref
  serial_in( io, vert_ref_for_wedge );

  // but then they're flexible
  for(;;)
    {
    chunk_flavor cf;
    serial_in( io, &cf );
    if( cf==CHUNK_TEXTURE_COORD)
      {
      // ignored
      texture_coord tc;
      serial_in( io, &tc );
      tc.x = tc.x;
      tc.y = tc.y;
      }
    else if(cf==CHUNK_NORMAL)
      {
      vector3d garbage;
      serial_in( io, &garbage);
      }
    else if(cf==CHUNK_COLOR)
      {
      // ignored
      color temp_color;
      serial_in( io, &temp_color );
      }
    else if(cf==chunk_flavor("detail"))
      {
      int level_of_detail;
      serial_in( io, &level_of_detail );
      w->level_of_detail = level_of_detail;
      serial_in( io, &w->lower_detail );
      }
    else if(cf==chunk_flavor("lodstart"))
      {
      int lod_start;
      serial_in( io, &lod_start );
      assert( lod_start==INT_MAX );
      }
    else if(cf==CHUNK_END)
      break;

    else
      {
      stringx composite = stringx("Unknown sub-chunk ")+cf.to_stringx()+stringx(" in wedge in file ")+
                         io.get_name();
      error(composite.c_str());
      }
    }

  }

////////////////////////////////////////////////////////////////////////////////
//  texture_coord
////////////////////////////////////////////////////////////////////////////////
void serial_in( chunk_file& io, texture_coord* tc )
  {
/*
  io >> tc->x >> tc->y;
*/
  serial_in(io,&tc->x);
  serial_in(io,&tc->y);
  }
