// GAS sound list file parser (used also by the sound interleaver tool)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <kernel.h>
#include <sys/file.h>
#include "GasIopSystem.h"

enum TokenType {
  NO_TOKEN = 0,
  TOKEN_SIZE, 
  TOKEN_FREQ, 
  TOKEN_VOLL, 
  TOKEN_VOLR, 
  TOKEN_RVOLL, 
  TOKEN_RVOLR, 
  TOKEN_STEREO, 
  TOKEN_MONO, 
  TOKEN_LOOP, 
  TOKEN_EE, 
  TOKEN_IOP, 
  TOKEN_SPU, 
  TOKEN_CD, 
  TOKEN_NUMBER, 
  TOKEN_FILENAME
};

/*** internal functions ***/
int process_line(char *line, GasSource *entry);
int get_token(char **token, int *token_type, int *num_value);
void unget_token();
void new_line(char *line);

#define STRING_LENGTH 256

/*** _gas_iop_load_list_file ***/ 
int _gas_load_list_file(char *filename, int file_size, GasSource **file_list, short *file_list_count)
{
  char line[STRING_LENGTH];
  int list;
  char *list_buf;
  int list_size;
  int i, j, k;
  short line_count;

  // read in the list file
  list = open (filename, O_RDONLY);
  if (list == 0)
  {
    gas_printf("Error opening file %s\n", filename);
    return 0;
  }

  list_size = file_size;
  list_buf = (char *) AllocSysMemory (0, list_size, NULL);
  if (list_buf == NULL)
  {
    gas_printf("Error allocating enough memory for the list buffer, what's up with that?!?\n");
    return 0;
  }
  i = read (list, (unsigned char *) list_buf, list_size);

  if (i != list_size)
    list_size = i;
  close(list);

  // now process all of the files in the list file, writing out a new list file
  line_count = 0;
  for (i=0; i<list_size; ++i)
  {
    if (list_buf[i] == '\n' || list_buf[i] == 0x0A)
    {
      ++line_count;
    }
  }

  *file_list = (GasSource *)AllocSysMemory(0, (sizeof(GasSource) * line_count), NULL);
  *file_list_count = line_count;

  k=0;
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

    // now use the line
    process_line(line, &(*file_list)[i]);
  }

  // free the list buffer, we don't need it anymore
  if (list_buf != NULL)
    FreeSysMemory(list_buf);

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

/*** percentile_to_volume ***/
// in sony-land 100% volume is 0x3fff, in the list file, I store volume as a 0 to 100
// value, so do the conversion here.
unsigned short percentile_to_volume(int percentile)
{
  percentile *= SCE_VOLUME_MAX;
  percentile /= 100;
  return (unsigned short)percentile;
}


// cd_markup 0 or 1 is only ok values
void fixup_filenames(char *full_filename, char *short_filename, int cd_markup)
{
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

  gas_printf("Full %s, short %s\n", full_filename, short_filename);
}


/*** process_line ***/
int process_line(char *line, GasSource *entry)
{
  char *token = NULL;
  int token_type;
  int token_num;

  // open up the input file(s)
  new_line(line);
  if (get_token(&token, &token_type, &token_num) != TOKEN_FILENAME)
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
  strncpy(entry->full_filename, token, FILENAME_LENGTH);
  entry->full_filename[FILENAME_LENGTH-1] = '\0';

  entry->flag.loop = 0;
  entry->flag.stereo = 0;
  entry->flag.src_type = 0;

  // process the header info, and update it as req'd
  while (get_token(&token, &token_type, &token_num))
  {
    switch (token_type)
    {
      // numeric values
      case TOKEN_SIZE:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for size\n");
        }
        else
          entry->size = token_num;
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

      case TOKEN_RVOLL:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for rvoll\n");
        }
        else
          entry->rvoll = percentile_to_volume(token_num);
        break;

      case TOKEN_RVOLR:
        if (get_token(&token, &token_type, &token_num) != TOKEN_NUMBER)
        {
          unget_token();
          gas_printf("Error processing parameters, expected a number value for rvolr\n");
        }
        else
          entry->rvolr = percentile_to_volume(token_num);
        break;

      // flags
      case TOKEN_LOOP:  entry->flag.loop = 1;   break;
      case TOKEN_STEREO:entry->flag.stereo = 1; break;
      case TOKEN_MONO:  entry->flag.stereo = 0; break;
      case TOKEN_EE:    entry->flag.src_type = SRC_TYPE_EE;  break;
      case TOKEN_IOP:   entry->flag.src_type = SRC_TYPE_IOP; break;
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
  if (entry->flag.src_type == SRC_TYPE_CD)
    fixup_filenames(entry->full_filename, entry->short_filename, 1);
  else
    fixup_filenames(entry->full_filename, entry->short_filename, 0);
  
  entry->flag.loaded = 0;

  // init any src_type specific stuff
  switch (entry->flag.src_type)
  {
    case SRC_TYPE_SPU:
      entry->src.spu.addr = 0x5000; // default 'bad' value (until it gets overridden with a good one)
      break;
    case SRC_TYPE_EE:  break;
    case SRC_TYPE_IOP: 
      entry->src.iop.iop_buf_addr = NULL;
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
void strupr(char *lift_me_up);

/*** get_token ***/
// token is a pointer which we change to point to the token to use
int get_token(char **token, int *token_type, int *num_value)
{
  int ret;
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

  strupr(gToken);
  *token = gToken;

  if (isdigit(gToken[0]))
  {
    *token_type = TOKEN_NUMBER;
    *num_value = atoi(gToken);
    *token = NULL;
  }
  else if (strcmp(gToken, "SIZE") == 0)
    *token_type = TOKEN_SIZE;
  else if (strcmp(gToken, "FREQ") == 0)
    *token_type = TOKEN_FREQ;
  else if (strcmp(gToken, "LOOP") == 0)
    *token_type = TOKEN_LOOP;
  else if (strcmp(gToken, "VOLL") == 0)
    *token_type = TOKEN_VOLL;
  else if (strcmp(gToken, "VOLR") == 0)
    *token_type = TOKEN_VOLR;
  else if (strcmp(gToken, "RVOLL") == 0)
    *token_type = TOKEN_RVOLL;
  else if (strcmp(gToken, "RVOLR") == 0)
    *token_type = TOKEN_RVOLR;
  else if (strcmp(gToken, "STEREO") == 0)
    *token_type = TOKEN_STEREO;
  else if (strcmp(gToken, "MONO") == 0)
    *token_type = TOKEN_MONO;
  else if (strcmp(gToken, "EE") == 0)
    *token_type = TOKEN_EE;
  else if (strcmp(gToken, "IOP") == 0)
    *token_type = TOKEN_IOP;
  else if (strcmp(gToken, "SPU") == 0)
    *token_type = TOKEN_SPU;
  else if (strcmp(gToken, "CD") == 0)
    *token_type = TOKEN_CD;
  else 
  {
    // a filename or something
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

void strupr(char *lift_me_up)
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
