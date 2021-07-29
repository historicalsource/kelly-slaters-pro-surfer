#include "global.h"

#include "convex_box.h"

convex_box::convex_box( const convex_box& _box )
{

  for( int i = 0; i < 6; i++ ) {
    planes[i] = _box.planes[i];
  }

  bbox = _box.bbox;
}

void serial_in( chunk_file& fs, convex_box* box )
{
	chunk_flavor cf;

	for( serial_in( fs, &cf ); cf != CHUNK_END; serial_in( fs, &cf ) ) {

    if( chunk_flavor( "planes" ) == cf ) {

      for( int i = 0; i < 6; i++ ) {
        vector4d plane;

        serial_in( fs, &plane );
        box->planes[i] = plane;
      }

    } else if( chunk_flavor( "bbox" ) == cf ) {
      bounding_box bbox;

      serial_in( fs, &bbox.vmin );
      serial_in( fs, &bbox.vmax );
      box->bbox = bbox;
    } else {
      stringx msg = stringx( "unknown chunk type '" ) + cf.to_stringx() + "' in '" + fs.get_name( );
      error( msg.c_str( ) );
    }

	}

}