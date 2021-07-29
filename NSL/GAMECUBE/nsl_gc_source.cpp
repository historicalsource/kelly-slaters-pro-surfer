#include "gamecube/nsl_gc.h"
#include "gamecube/nsl_gc_stream.h"

static nlInt32 _nslFindUnusedSource( void );

static void _nslReleaseSource( nlUint32 slot );

static nslSourceId _nslLoadSource( nslMoniker moniker )
{
	NSL_ENSURE_ON_VAL( NSL_INVALID_ID );
	NSL_CHECK_MONIKER( moniker );

	if( nsl.sourceCount >= NSL_NUM_SOURCES )
		return NSL_INVALID_ID;

	nslSourceId found = nslGetSource( moniker, false );
	if( found != NSL_INVALID_ID )
		return found;

	nslSourceInfo* info = _nslGetSourceInfo( moniker );
	if( !info )
		return NSL_INVALID_ID;

	if( (int) info->offset == -1 )
		return NSL_INVALID_ID;

	int slot = _nslFindUnusedSource( );
	
	if( slot < 0 )
		return NSL_INVALID_ID;

	nslSource* source = &nsl.sources[slot];

	source->info = info;
	source->volume = info->volume;
	source->pitch = 1.0f;
	source->minDist = NSL_DEFAULT_MIN_SOUND_DIST;
	source->maxDist = NSL_DEFAULT_MAX_SOUND_DIST;

	// FIXME: at some point, this may be false
	source->loaded = true;
	source->id += NSL_ID_INCREMENT;

	nsl.sourceCount++;
	source->used = true;

	nslVerbosePrintf( "nslLoadSource: loading source '%s' (%d).\n", source->info->name, source->id );
	
	source->streamId = -1; // no stream support!
	return source->id;
}

static nslSourceId _nslStreamLoadSource( nslMoniker moniker, const char *streamFilename, const nslStreamInfo *streamInfo )
{
	nslSourceId srcId;
	
	srcId = _nslLoadSource( moniker );
	if( srcId != NSL_INVALID_ID )
	{
		nslSource* source;
		
		source = &nsl.sources[ NSL_GET_SLOT_FROM_ID(srcId) ];
		source->streamId = nslStreamOpen(	streamFilename,
											streamInfo,
											source->info->flags & NSL_SOURCE_INFO_REVERB );
		
		NSL_ASSERT(source->streamId != -1);
		if( source->streamId != -1 )
			return srcId;
			
		_nslReleaseSource( NSL_GET_SLOT_FROM_ID(srcId) );
		return NSL_INVALID_ID;
	}
	return srcId;
}

static nslSourceId _nslGetSource( nslMoniker moniker, bool fatal )
{
	NSL_ENSURE_ON_VAL( NSL_INVALID_ID );
	NSL_CHECK_MONIKER( moniker );

	int seen = 0;

	for( int i = 0; i < NSL_NUM_SOURCES && seen < nsl.sourceCount; i++ ) {

		if( nsl.sources[i].used ) {

#ifdef NSL_LOAD_SOURCE_BY_NAME
			if( _nslStrCaseCmp( moniker, nsl.sources[i].info->name ) == 0 ) {
				return nsl.sources[i].id;
			}
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
			if( moniker == nsl.sources[i].info->alias ) {
				return nsl.sources[i].id;
			}
#endif

			++seen;
		}

	}
  
	if( fatal ) {
#ifdef NSL_LOAD_SOURCE_BY_NAME
    nslFatal( "nslGetSource: unable to find source '%s'.\n", moniker );
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
    nslFatal( "nslGetSource: unable to find source [%d].\n", moniker );
#endif
	}

	return NSL_INVALID_ID;
}

#ifdef NSL_LOAD_SOURCE_BY_NAME
nslSourceId nslLoadSource( const char * name )
{
	return _nslLoadSource( name );
}
nslSourceId nslStreamLoadSource( const char *name, const char *streamFilename, const nslStreamInfo *streamInfo )
{
	return _nslStreamLoadSource( name, streamFilename, streamInfo );
}
nslSourceId nslGetSource( const char * name, bool fatal )
{
	return _nslGetSource(name,fatal);
}
const char* nslGetSourceName( nslSourceId id )
{
	NSL_ENSURE_ON_VAL( NULL );
	NSL_ENSURE_RID_VAL( id, "" );
	nslSource* source = &nsl.sources[ NSL_GET_SLOT_FROM_ID( id ) ];
	return source->info->name;
}
#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS
nslSourceId nslLoadSource( nlUint16 alias )
{
	return _nslLoadSource( alias );
}
nslSourceId nslStreamLoadSource( nlUint16 alias, const char *streamFilename, const nslStreamInfo *streamInfo )
{
	return _nslStreamLoadSource( alias, streamFilename, streamInfo );
}
nslSourceId nslGetSource( nlUint16 alias, bool fatal )
{
	return _nslGetSource(alias,fatal);
}
nlUint16 nslGetSourceName( nslSourceId id )
{
	NSL_ENSURE_ON_VAL( NULL );
	NSL_ENSURE_RID_VAL( id, NULL );
	nslSource* source = &nsl.sources[ NSL_GET_SLOT_FROM_ID( id ) ];
	return source->info->alias;
}
#endif


nslSourceId nslGetSourceByIndex(int index)
{
	if (index < NSL_NUM_SOURCES)
	{
		if (nsl.sources[index].used)
		{
			return nsl.sources[index].id;
		}
	}
	return NSL_INVALID_ID;
}


void nslSetSourceRange( nslSourceId id, float minDist, float maxDist )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_RID( id );
	nslSource* source = &nsl.sources[ NSL_GET_SLOT_FROM_ID( id ) ];
	source->minDist = minDist;
	source->maxDist = maxDist;
}

void nslSetSourceParam( nslSourceId id,	nslSoundParamEnum param, float value )
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_RID( id );

	nslSource* source = &nsl.sources[ NSL_GET_SLOT_FROM_ID( id ) ];
	switch( param ) 
	{
		case NSL_SOUNDPARAM_VOLUME: 
			source->volume = value;
			break;
		case NSL_SOUNDPARAM_PITCH:
			source->pitch = value;
			break;
		case NSL_SOUNDPARAM_MINDIST:
			source->minDist = value;
			break;
		case NSL_SOUNDPARAM_MAXDIST:
			source->maxDist = value;
			break;
		default:
			nslFatal( "Unhandled parameter in nslSetSourceParam.\n" );
			break;
	}
}

float nslGetSourceParam( nslSourceId id, nslSoundParamEnum param )
{
	NSL_ENSURE_ON_VAL( 0.0f );
	NSL_ENSURE_RID_VAL( id, 0.0f );
	
	float value = 0.0f;
	
	nslSource* source = &nsl.sources[ NSL_GET_SLOT_FROM_ID( id ) ];
	switch( param ) 
	{
		case NSL_SOUNDPARAM_VOLUME: 
		  	value = source->volume;
		  	break;
		case NSL_SOUNDPARAM_PITCH:
			value = source->pitch;
			break;
		case NSL_SOUNDPARAM_MINDIST:
			value = source->minDist;
			break;
		case NSL_SOUNDPARAM_MAXDIST:
			value = source->maxDist;
			break;
		default:
			nslFatal( "Unhandled parameter in nslGetSourceParam.\n" );
			break;
	}

	return value;
}


void nslSetReverb(nslSourceId whichSource, bool new_setting)
{
	NSL_ENSURE_ON( );
	NSL_ENSURE_RID( whichSource );

	nslSource* source = &nsl.sources[NSL_GET_SLOT_FROM_ID(whichSource)];

	if (new_setting)
	{
		source->info->flags |= NSL_SOURCE_INFO_REVERB;
	}
	else
	{
		source->info->flags &= ~NSL_SOURCE_INFO_REVERB;
	}
}


bool nslGetReverb(nslSourceId whichSource)
{
	NSL_ENSURE_ON_VAL( false );
	NSL_ENSURE_RID_VAL( whichSource, false );

	nslSource* source = &nsl.sources[NSL_GET_SLOT_FROM_ID(whichSource)];

	return (source->info->flags & NSL_SOURCE_INFO_REVERB) > 0;
}


static nlInt32 _nslFindUnusedSource( void )
{
	int i;
	for( i=0; i < NSL_NUM_SOURCES; i++ )
	{
		nslSource* source = &nsl.sources[i];
		if( !source->used )
			return i;
	}
	return -1;
}

static void _nslReleaseSource( nlUint32 slot )
{
	NSL_ASSERT( slot < NSL_NUM_SOURCES );
	
	nslSource* source = &nsl.sources[slot];
	if( source->used )
		nslVerbosePrintf( "_nslReleaseSource: releasing source '%s' (%d).\n", source->info->name, source->id );

	// leave id untouched in order to produce unique ids
	source->info = NULL;
	source->volume = 1.0f;
	source->pitch = 1.0f;
	source->minDist = -1.0f;
	source->maxDist = -1.0f;
	source->loaded = true;
	if( source->used )
		nsl.sourceCount--;
	source->used = false;
}

void _nslReleaseAllSources( void )
{
	int i;
	for( i = 0; i < NSL_NUM_SOURCES; i++ )
		_nslReleaseSource( (nlUint32) i );
	nsl.sourceCount = 0;
}

float nslGetSourceLength( nslSourceId id )
{
	NSL_ENSURE_ON_VAL( 0.0f );
	NSL_ENSURE_RID_VAL( id, 0.0f );
	nslSource* source = &nsl.sources[ NSL_GET_SLOT_FROM_ID( id ) ];
	return source->info ? (float)source->info->samples / (float)source->info->frequency : 0.0f;
}