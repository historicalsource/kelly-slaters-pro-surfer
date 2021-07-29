// Localization support.
#ifndef LOCALIZE_H
#define LOCALIZE_H


void load_locales();
stringx localize_text( stringx src );
stringx localize_text_safe( stringx src );

// localize a VO stream filename
stringx localize_VO_stream( const stringx& filename );


#endif