// GAS sound list file parser (used also by the sound interleaver tool)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef ADPCM_TOOL
#include <kernel.h>
#include <sys/file.h>
#endif
#include "GasSystem.h"
#include "../gas.h"

enum TokenType {
  NO_TOKEN = 0,
  TOKEN_SIZE,
  TOKEN_PADSIZE,
  TOKEN_OFFSET,
  TOKEN_FREQ, 
  TOKEN_VOLL, 
  TOKEN_VOLR, 
  TOKEN_VOLUME, 
  TOKEN_STEREO, 
  TOKEN_MONO, 
  TOKEN_LOOP, 
  TOKEN_REVERB, 
  TOKEN_SPU, 
  TOKEN_CD,
  TOKEN_COLLECTION,
  TOKEN_NUMBER, 
  TOKEN_FILENAME,
  /*TOKEN_SFX,
  TOKEN_MUSIC,
  TOKEN_AMBIENT,
  TOKEN_VOICE*/
  TOKEN_SOUNDTYPE
};

extern const char *nslSourceTypesStr[];

/*** internal functions ***/
int process_line(char *line, GasSource *entry);
int get_token(char **token, int *token_type, int *num_value);
void unget_token();
void new_line(char *line);

#define STRING_LENGTH 256


/*** _gas_rpc_load_list_file ***/ 
int _gas_rpc_load_list_file(char *filename, int file_size, GasSource **file_list, short *file_list_count)
{
  // Assumed semaphore locks: system_state_sema

  char line[STRING_LENGTH];
#ifndef ADPCM_TOOL
  int list;
#else
  FILE *list;
#endif
  GasSource *temp_file_list;
  char *list_buf;
  char *line_ptr;
  int list_size;
  int i, j, k;
  short entry;
  short line_count;
  gas_printf("Loading soundlist file\n");
  // read in the list file
#ifndef ADPCM_TOOL
  if ((strncmp(filename, gas.host_prefix, strlen(gas.host_prefix))==0) ||
      (strncmp(filename, "cdrom0:", 7)==0))
    list = open (filename, O_RDONLY);
  else 
    list=0;
  if (list<=0)
  {
    list = _gas_rpc_get_fd(filename);
    if (list <= 0)
    {
      gas_printf("Error opening file (even tried _gas_rpc_get_fd) %s\n", filename);
      return 0;
    }

  }

#else
  list = fopen (filename, "rb");
  if (list == 0)
  {
    gas_printf("Error opening file %s\n", filename);
    return 0;
  }

#endif

#ifndef ADPCM_TOOL
  list_size = lseek(list, 0, SEEK_END);
#else
  list_size = fseek(list, 0, SEEK_END);
#endif
  if (list_size <= 0) return 0;
#ifndef ADPCM_TOOL
  lseek(list, 0, SEEK_SET);
#else
  fseek(list, 0, SEEK_SET);
#endif

#ifndef ADPCM_TOOL
  list_buf = (char *) AllocSysMemory (0, list_size, NULL);
#else
  list_buf = (char *) malloc(list_size);
#endif
  if (list_buf == NULL)
  {
    gas_printf("Error allocating enough memory for the list buffer, what's up with that?!?\n");
    return 0;
  }
#ifndef ADPCM_TOOL
  i = read (list, (unsigned char *) list_buf, list_size);
#else
  i = fread (list_buf, list_size, 1, list);
#endif

  if (i != list_size)
  {
    gas_printf("Warning, attempted to read %d bytes but only got %d\n", list_size, i);
    list_size = i;
  }
#ifndef ADPCM_TOOL
  close(list);
#else
  fclose(list);
#endif

  // now process all of the files in the list file, writing out a new list file
  line_count = 0;
  for (i=0; i<list_size; ++i)
  {
    if (list_buf[i] == '\n' || list_buf[i] == 0x0A)
    {
      ++line_count;
    }
  }

#ifndef ADPCM_TOOL
  temp_file_list = (GasSource *)AllocSysMemory(0, (sizeof(GasSource) * line_count), NULL);
  gas_printf( "Allocated at 0x%X for file_list\n", (unsigned int)temp_file_list );
#else
  temp_file_list = (GasSource *)malloc(sizeof(GasSource) * line_count);
#endif

  if (temp_file_list == NULL)
  {
    gas_printf("Error allocating memory while loading snd file");
#ifndef ADPCM_TOOL
    FreeSysMemory(list_buf);
#else
    free(list_buf);
#endif
  }
  k=0;
  entry = 0;
  for (i=0; i<line_count; ++i)
  {
    line[0] = '\0';

    // grab the line of text from the list buffer
    for (j=0; k<list_size; k++, j++)
    {
      if (list_buf[k] == '\n')
      {
        line[j] = '\0';
        ++k;
        break;
      }
      line[j] = list_buf[k];
    }
    // just in case (cap off strings so they don't run past the end of the world)
    line[STRING_LENGTH-1] = '\0';
  
    // strip comments
    line_ptr = line;
    while( *line_ptr != '\0' && *line_ptr !=';' ) line_ptr++;
    *line_ptr = '\0';
    // convert /'s
    line_ptr = line;
    while( *line_ptr != '\0' )
    {
      if( *line_ptr == '/' ) *line_ptr = '\\';
      line_ptr++;
    }
    
    // strip whitespace
    line_ptr = line;
    while( *line_ptr == ' ' || *line_ptr == '\t' || *line_ptr == '\\' ) line_ptr++;

    // now use the line
    if( line_ptr[0] != '\0' )
    {
      if( process_line(line_ptr, &(temp_file_list)[entry]) == 1 )
        entry++;
    }
  }
  *file_list_count = entry;
#ifndef ADPCM_TOOL
    FreeSysMemory(list_buf);
#else
    free(list_buf);
#endif
 
  #ifndef ADPCM_TOOL
    *file_list = (GasSource *)AllocSysMemory(0, (sizeof(GasSource) * entry), NULL);
    gas_printf( "Allocated at 0x%X for file_list\n", (unsigned int)*file_list );
  #else
    *file_list = (GasSource *)malloc(sizeof(GasSource) * entry);
  #endif

  if (file_list == NULL)
  {
    gas_printf("Error allocating memory while loading snd file");
#ifndef ADPCM_TOOL
    FreeSysMemory(temp_file_list);
#else
    free(temp_file_list);
#endif
    return 0;
  }

  memcpy(*file_list, temp_file_list, sizeof(GasSource)*entry);
  


  // free the list buffer, we don't need it anymore
  if (list_buf != NULL)
#ifndef ADPCM_TOOL
    FreeSysMemory(temp_file_list);
#else
    free(temp_file_list);
#endif
  return 1;
}

/*** freq_to_pitch_one ***/
// converts the sample frequency into the SCE-defined magic values to obtain a pitch of 
// 1.0 for any given bitrate.
unsigned short freq_to_pitch_one(int freq)
{
  int ret;
  ret = (freq*4096)/48000;
  return (unsigned short)ret;
}

#ifndef ADPCM_TOOL
/*** percentile_to_volume ***/
// in sony-land 100% volume is 0x3fff, in the list file, I store volume as a 0 to 100
// value, so do the conversion here.
unsigned short percentile_to_volume(int percentile)
{
  percentile *= SCE_VOLUME_MAX;
  percentile /= 100;
  return (unsigned short)percentile;
}

int volume_to_percentile(unsigned short volume)
{
  int vol = volume;
  vol *= 100;
  vol /= SCE_VOLUME_MAX;
  return (int)vol;
}

#else
unsigned short percentile_to_volume(int percentile)
{
  return (unsigned short)percentile;
}
int volume_to_percentile(unsigned short volume)
{
  return (int)vol;
}
#endif


// cd_markup 0 or 1 is only ok values
void fixup_filenames(char *full_filename, char *short_filename, int cd_markup)
{
#ifndef ADPCM_TOOL
  int decimal, last_slash;
  int amt;
  int pos = strlen(full_filename) - 1;
  while (full_filename[pos] != '.' && pos != -1)
  {
    full_filename[pos] = toupper(full_filename[pos]);
    --pos;
  }
  // catch the cases where we didn't find a period or we might blow our array bounds
  if (pos <= 0 || pos >= (43 - (cd_markup * 2)))
    return;
  decimal = pos;
  full_filename[pos+1] = 'T';
  full_filename[pos+2] = 'V';
  full_filename[pos+3] = 'B';
  if (cd_markup)
  {
    full_filename[pos+4] = ';';
    full_filename[pos+5] = '1';
    full_filename[pos+6] = '\0';
  }
  else
  {
    full_filename[pos+4] = '\0';
  }
  while (full_filename[pos] != '\\' && pos != -1)
  {
    full_filename[pos] = toupper(full_filename[pos]);
    --pos;
  }
  last_slash = pos;
  while (pos > -1)
  {
    full_filename[pos] = toupper(full_filename[pos]);
    --pos;
  }
  if (last_slash < decimal)
  {
    amt = (decimal - last_slash-1);
    strncpy(short_filename, (full_filename + last_slash+1), amt);
    short_filename[amt] = '\0';
  }
  else
    strcpy(short_filename, full_filename);

#ifdef DEBUG_OUTPUT
  gas_printf("Full %s, short %s\n", full_filename, short_filename);
#endif
#endif //ADPCM_TOOL
}


/* returns 0 on error, 1 on successful entry and 2 on collection line */
/*** process_line ***/
int process_line(char *line, GasSource *entry)
{
  char *token = NULL;
  int token_type;
  int token_num;

  entry->rawStreamHostFd = -1; // IMPORTANT!

  // open up the input file(s)
  new_line(line);
#ifdef DEBUG_OUTPUT
  gas_printf( "%s\n", line );
#endif
  get_token(&token, &token_type, &token_num);

#ifndef ADPCM_TOOL
    if( token_type == TOKEN_COLLECTION
      && get_token(&token, &token_type, &token_num) == TOKEN_FILENAME)
    {
      strncpy( gas.spu_heaps[gas.num_heaps -1].spu_collection, token, FILENAME_LENGTH );
      gas.spu_heaps[gas.num_heaps -1].spu_collection[FILENAME_LENGTH-1] = '\0';
      return 2;
    }
#endif


#ifdef NSL_LOAD_SOURCE_BY_NAME

  if (token_type != TOKEN_FILENAME && token_type != TOKEN_NUMBER )
  {
    gas_printf("Error processing line %s", line);
    if (token != NULL)
    {
      gas_printf(" expected a filename, found %s.\n", token);
    }
    else
    {
      gas_printf("\n");
    }
    return 0;
  }

  strncpy(entry->filename, token, FILENAME_LENGTH);
  entry->filename[FILENAME_LENGTH-1] = '\0';

#endif

#ifdef NSL_LOAD_SOURCE_BY_ALIAS

	// What is the alias for this sound source?

	if (token_type != TOKEN_NUMBER)
	{
	    gas_printf("Error processing line %s", line);
		if (token != NULL)
		{
			gas_printf(" expected an id, found %s.\n", token);
		}
		else
		{
			gas_printf("\n");
		}
		return 0;
	}

	entry->aliasID = token_num;

#endif

	entry->flag.reverb = 0;
  entry->flag.loop = 0;
  entry->flag.stereo = 0;
#ifndef ADPCM_TOOL
  entry->flag.src_type = SRC_TYPE_CD;
#else
  entry->flag.src_type = SRC_TYPE_NONE;
  entry->vag_buffer = NULL;
#endif
  entry->voll = entry->volr = percentile_to_volume( 100 );
  entry->size = 0;

  // process the header info, and update it as req'd
  while (get_token(&token, &token_type, &token_num))
  {
    switch (token_type)
    {
      // numeric values
      case TOKEN_OFFSET:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for offset\n");
        }
        else
          entry->offset = token_num;
        break;

      case TOKEN_SIZE:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for size\n");
        }
        else
          entry->size = token_num;
        break;
      case TOKEN_PADSIZE:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for size\n");
        }
        else
          entry->padsize = token_num;
        break;

      case TOKEN_FREQ:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for size\n");
        }
        else
          entry->pitch_one = freq_to_pitch_one(token_num);
        break;

      case TOKEN_VOLL:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for voll\n");
        }
        else
          entry->voll = percentile_to_volume(token_num);
        break;

      case TOKEN_VOLR:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for volr\n");
        }
        else
          entry->volr = percentile_to_volume(token_num);
        break;

      case TOKEN_VOLUME:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for volume\n");
        }
        else
          entry->volr = entry->voll = percentile_to_volume(token_num);
        break;

      // flags
      case TOKEN_LOOP:  entry->flag.loop = 1; break;

		case TOKEN_REVERB: 
		{
			entry->flag.reverb = 1;
			break;
		}
      
     /* case TOKEN_SFX:     entry->sound_type = SOUND_TYPE_SFX;     break;
      case TOKEN_MUSIC:   entry->sound_type = SOUND_TYPE_MUSIC;   break;
      case TOKEN_AMBIENT: entry->sound_type = SOUND_TYPE_AMBIENT; break;
      case TOKEN_VOICE:   entry->sound_type = SOUND_TYPE_VOICE;   break;
*/
      case TOKEN_SOUNDTYPE:
        entry->sound_type = token_num;
        break;
      case TOKEN_STEREO:entry->flag.stereo = 1; break;
      case TOKEN_MONO:  entry->flag.stereo = 0; break;
      case TOKEN_SPU:   entry->flag.src_type = SRC_TYPE_SPU; break;
      case TOKEN_CD:    entry->flag.src_type = SRC_TYPE_CD;  break;

      // bad token
      case TOKEN_NUMBER:
        gas_printf("Error, what's this number (%d) doing here?\n", token_num);
        break;

      case TOKEN_FILENAME:
        gas_printf("Error, what's this '%s' doing here?\n", token);
        break;

      case NO_TOKEN:
        break;
    }
  }
  // do filename fixup
/*
  if (entry->flag.src_type == SRC_TYPE_CD)
    fixup_filenames(entry->full_filename, entry->short_filename, 1);
  else
    fixup_filenames(entry->full_filename, entry->short_filename, 0);
*/  
  entry->flag.loaded = 0;

  // init any src_type specific stuff
  switch (entry->flag.src_type)
  {
    case SRC_TYPE_SPU:
      entry->src.spu.addr = 0x5000; // default 'bad' value (until it gets overridden with a good one)
      break;
    case SRC_TYPE_CD:  break;
    default:
      break;
  }

  return 1;
}

int gScanPos;
char *gLine = NULL;
char gToken[FILENAME_LENGTH];
int gStoredToken = 0;
int gStoredType = 0;
int gStoredNum = 0;

int _build_token(char *gLine, char *gToken);
void my_strupr(char *lift_me_up);


/*** get_token ***/
// token is a pointer which we change to point to the token to use
int get_token(char **token, int *token_type, int *num_value)
{
  int ret, i;
  if (gStoredToken)
  {
    *token = gToken;
    *token_type = gStoredType;
    *num_value = gStoredNum;

    gStoredToken = 0;
    return *token_type;
  }

  gToken[0] = '\0';
  while(isspace(gLine[gScanPos]) && (gLine[gScanPos] != '\0'))
    ++gScanPos;

  ret = _build_token(&gLine[gScanPos], gToken);
  gScanPos += ret;

  if (ret == 0)
  {
    *token = NULL;
    return NO_TOKEN;
  }

  my_strupr(&gToken[0]);
  *token = gToken;

  if (isdigit(gToken[0]))
  {
    *token_type = TOKEN_NUMBER;
    *num_value = atoi(gToken);
  }
  else if (strcmp(gToken, "SIZE") == 0)
    *token_type = TOKEN_PADSIZE;
  else if (strcmp(gToken, "REALSIZE") == 0)
    *token_type = TOKEN_SIZE;
  else if (strcmp(gToken, "OFFSET") == 0)
    *token_type = TOKEN_OFFSET;
  else if (strcmp(gToken, "FREQ") == 0)
    *token_type = TOKEN_FREQ;
  else if (strcmp(gToken, "LOOP") == 0)
    *token_type = TOKEN_LOOP;
  else if (strcmp(gToken, "REVERB") == 0)
    *token_type = TOKEN_REVERB;
  else if (strcmp(gToken, "VOLL") == 0)
    *token_type = TOKEN_VOLL;
  else if (strcmp(gToken, "VOLR") == 0)
    *token_type = TOKEN_VOLR;
  else if (strcmp(gToken, "VOLUME") == 0)
    *token_type = TOKEN_VOLUME;
  else if (strcmp(gToken, "STEREO") == 0)
    *token_type = TOKEN_STEREO;
  else if (strcmp(gToken, "MONO") == 0)
    *token_type = TOKEN_MONO;
  else if (strcmp(gToken, "SPU") == 0)
    *token_type = TOKEN_SPU;
  else if (strcmp(gToken, "COLLECTION") == 0)
    *token_type = TOKEN_COLLECTION;
  else if (strcmp(gToken, "CD") == 0)
    *token_type = TOKEN_CD;
/*  else if (strcmp(gToken, "VOICE") == 0)
    *token_type = TOKEN_VOICE;
  else if (strcmp(gToken, "SFX") == 0)
    *token_type = TOKEN_SFX;
  else if (strcmp(gToken, "MUSIC") == 0)
    *token_type = TOKEN_MUSIC;
  else if (strcmp(gToken, "AMBIENT") == 0)
    *token_type = TOKEN_AMBIENT;
*/  
  else 
  {
    int found = 0;
    i = 0;
    while (strcmp(nslSourceTypesStr[i], "N/A") != 0)
    { 
      if (strcmp(gToken, nslSourceTypesStr[i]) == 0)
      {
        *token_type = TOKEN_SOUNDTYPE;
        *num_value  = i;
        found = 1;
        break;
      }
      i++;
    }
    // a filename or something
    if (!found)
      *token_type = TOKEN_FILENAME;
  }
  gStoredNum = *num_value;
  gStoredType = *token_type;

  return *token_type;
}

// because we don't have sscanf on the iop...sigh
int _build_token(char *line, char *token)
{
  int i=0;
  while ( (!isspace(line[i])) && (line[i] != '\0') )
  {
    token[i] = line[i];
    ++i;
  }
  token[i] = '\0';
  return i;
}

void my_strupr(char *lift_me_up)
{
  int i=0;
  while ( lift_me_up[i] != '\0' )
  {
    lift_me_up[i] = toupper(lift_me_up[i]);
    ++i;
  }
}


/*** unget_token ***/
// pushes back the last token, only works for one-token's worth
void unget_token()
{
  gStoredToken = 1;
}


/*** new_line ***/
// passes in a new line to get tokens from
void new_line(char *line)
{
  gLine = line;
  gScanPos = 0;
}
