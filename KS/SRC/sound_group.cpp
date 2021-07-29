#include "global.h"
#include "pstring.h"
#include "sound_group.h"
#include "hwaudio.h"

sound_group::sound_group()
{/*
  maxhistory = 0;
  clear_history();*/
}

// Picks the next random string and cycles the history.
sg_entry* sound_group::get_next()
{
/*
	if ( (unsigned)maxhistory > entries.size() )
		maxhistory = entries.size();
#if 0
	int i, histcount, r, idx;
	for ( i = 0; i < entries.size(); i++ )
		entries[i].flagged = false;

	histcount = 0;
	for ( i = 0; i < maxhistory; i++ )
		if ( history[i] != -1 )
		{
			entries[history[i]].flagged = true;
			histcount++;
		}

	r = random(entries.size() - histcount);
	for ( idx = 0, i = 0; i < r; i++, idx++ )
		while ( entries[idx].flagged )
			idx++;
#else
	// in theory, this could be an infinite loop :)
	// if anyone can see what's wrong with the above method let me know.
	// it should be a lot faster for larger sets with long histories.
	int i, idx;
	for (;;)
	{
		idx = random(entries.size());
		for ( i = 0; i < maxhistory; i++ )
			if ( history[i] == idx )
				break;
		if ( i == maxhistory )
			break;
	}
#endif
*/

/*
	for ( i = maxhistory - 1; i > 0; i-- )
		history[i] = history[i - 1];
	history[0] = idx;
*/
/*
  history[history_index++] = idx;
  if(history_index >= maxhistory)
    clear_history();
*/
/*
	return &entries[idx];


  if((pool.empty() && !entries.empty()) || (maxhistory > 0 && (int)(entries.size() - pool.size()) > maxhistory))
    clear_history();

  rational_t per_total = 0.0f;

  vector<sg_entry *>::iterator sgei = pool.begin();
  while( sgei != pool.end() )
  {
    per_total += (*sgei)->probability;
    ++sgei;
  }

  rational_t roll = per_total * random();

  per_total = 0.0f;
  sgei = pool.begin();
  while( sgei != pool.end() )
  {
    per_total += (*sgei)->probability;
    if( per_total >= roll )
    {
      sg_entry* entry = (*sgei);

      if(maxhistory != 0)
        pool.erase(sgei);

      return(entry);
    }
    else
      ++sgei;
  }
*/
  return(NULL);
}

void sound_group::copy(const sound_group &b)
{/*
  name = b.name;
  maxhistory = b.maxhistory;

  vector<sg_entry>::const_iterator sgei = b.entries.begin();
  while(sgei != b.entries.end())
  {
    entries.push_back(*sgei);
    ++sgei;
  }

  clear_history();*/
}

void sound_group::clear_history()
{/*
  pool.resize(0);
  pool.reserve(entries.size());

  vector<sg_entry>::iterator sgei = entries.begin();
  while(sgei != entries.end())
  {
    pool.push_back(&(*sgei));
    ++sgei;
  }
  
/*
	for ( unsigned i = 0; i < ARRAY_ELEMENTS( history ); i++ )
		history[i] = -1;

  history_index = 0;
*/
}

// Read a single group from a file, not including the soundgrp tag.
void serial_in( chunk_file& fs, sound_group* group )
{/*
	stringx id, val;
  bool group_is_voice = false;
	
  for( serial_in( fs, &id ); id!=chunkend_label; serial_in( fs, &id ) )
  {
    if(id == "name")
    {
      serial_in( fs, &val );
	    group->name = pstring( val );
    }
    else if(id == "history")
    {
	    serial_in( fs, &group->maxhistory );
//	    if ( group->maxhistory > 8 )
//		    error( "Sound group history max is %d.", ARRAY_ELEMENTS(group->history) );
//	    memset( group->history, 0, sizeof(group->history) );
    }
    else if(id == "voice")
    {
      group_is_voice = true;
    }
    else if(id == "sfx" || id == "non_voice")
    {
      group_is_voice = false;
    }
    else if(id == "sound:")
    {
		  group->entries.push_back();
		  sg_entry* entry = &group->entries.back();

      entry->is_voice = group_is_voice;

      for( serial_in( fs, &id ); id!=chunkend_label; serial_in( fs, &id ) )
      {
        if(id == "file")
        {
          serial_in( fs, &id );

		      filespec spec( id );
		      entry->name = pstring( spec.name );
		      sound_device::inst()->load_sound( id );
        }
        else if(id == "voice")
        {
  		    entry->is_voice = true;
        }
        else if(id == "sfx" || id == "non_voice" )
        {
  		    entry->is_voice = false;
        }
        else if(id == "freq")
        {
  		    serial_in( fs, &entry->probability );
        }
        else if(id == "volume")
        {
  		    serial_in( fs, &entry->volume );
        }
        else if(id == "volume_var")
        {
  		    serial_in( fs, &entry->volume_var );
        }
        else if(id == "pitch")
        {
  		    serial_in( fs, &entry->pitch );
        }
        else if(id == "pitch_var")
        {
  		    serial_in( fs, &entry->pitch_var );
        }
        else if(id == "min_dist")
        {
  		    serial_in( fs, &entry->min_dist );
        }
        else if(id == "max_dist")
        {
  		    serial_in( fs, &entry->max_dist );
        }
        else
          error("Bad keyword '%s' in 'sound:' chunk of file '%s'", id.c_str(), fs.get_filename().c_str());
      }
    }
    else
      error("Bad keyword '%s' in 'soundgrp:' chunk of file '%s'", id.c_str(), fs.get_filename().c_str());
	}

  group->clear_history();*/
}

// Read a whole list from a file.  Assumes that the sound_groups: chunk has already 
// been read, and scans for a chunkend or the end of the file.
void serial_in( chunk_file& fs, vector<sound_group>& groups )
{/*
	stringx id;
  for( serial_in( fs, &id ); id!=chunkend_label && id!=empty_string; serial_in( fs, &id ) )
  {
    if(id == "soundgrp:")
    {
		  groups.push_back();
		  serial_in( fs, &groups.back() );
    }
    else
      error("Bad keyword '%s' in 'sound_groups:' chunk of file '%s'", id.c_str(), fs.get_filename().c_str());
	}*/
}

