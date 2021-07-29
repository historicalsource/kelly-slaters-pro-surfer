// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "GameSaver.h"

#if defined(TARGET_GC)
#include "osfile.h"
#elif defined(TARGET_XBOX)
#include "osfile.h"
// please don't include these directly #include "hwosxb/xb_file.h"
#else
#include "libmc.h"
#include "osfile.h"
// please don't include these directly HWOSPS2\\ps2_file.h"
#include <libcdvd.h>
#endif /* TARGET_XBOX JIV DEBUG */
#ifndef TARGET_GC
#include <sys\types.h>
#endif


unsigned short ascii_special[33][2] = {
	{0x8140, 32},		/*   */
	{0x8149, 33},		/* ! */
	{0x8168, 34},		/* " */
	{0x8194, 35},		/* # */
	{0x8190, 36},		/* $ */
	{0x8193, 37},		/* % */
	{0x8195, 38},		/* & */
	{0x8166, 39},		/* ' */
	{0x8169, 40},		/* ( */
	{0x816a, 41},		/* ) */
	{0x8196, 42},		/* * */
	{0x817b, 43},		/* + */
	{0x8143, 44},		/* , */
	{0x817c, 45},		/* - */
	{0x8144, 46},		/* . */
	{0x815e, 47},		/* / */
	{0x8146, 58},		/* : */
	{0x8147, 59},		/* ; */
	{0x8171, 60},		/* < */
	{0x8181, 61},		/* = */
	{0x8172, 62},		/* > */
	{0x8148, 63},		/* ? */
	{0x8197, 64},		/* @ */
	{0x816d, 91},		/* [ */
	{0x818f, 92},		/* \ */
	{0x816e, 93},		/* ] */
	{0x814f, 94},		/* ^ */
	{0x8151, 95},		/* _ */
	{0x8165, 96},		/* ` */
	{0x816f, 123},		/* { */
	{0x8162, 124},		/* | */
	{0x8170, 125},		/* } */
	{0x8150, 126},		/* ~ */
};

int MemoryCardSema = -1;
static unsigned short ascii_table[3][2] = {
	{0x824f, 0x30},	/* 0-9  */
	{0x8260, 0x41},	/* A-Z  */
	{0x8281, 0x61},	/* a-z  */
};

char Sjis2Ascii(unsigned char *);
void Sjis2AsciiString(unsigned char *, char *);
int IsSjis(unsigned char *);
long IsAscii(char *);
u_short Ascii2Sjis(unsigned char);
void AsciiString2Sjis(u_char *, u_short *);
short SwapShort(u_short);

/////////////////////////////////////////////////
char Sjis2Ascii(unsigned char *character)
{
	unsigned char byte1 = (unsigned char)*character;
	unsigned char byte2 = (unsigned char)*(character + 1);
    char output = 0;	// avoid "uninitialized" warning (dc 01/29/02)
	int i;


	if(byte1 == 0x81 || byte1 == 0x82)
	{
		if(byte1 == 0x82)
		{
			if(((byte2 >= 0x4f) && (byte2 <= 0x59)) || ((byte2 >= 0x60) && (byte2 <= 0x7a)))
				output = byte2 - 0x1f;

			else if((byte2 >= 0x81) && (byte2 <= 0x9b))
				output = byte2 - 0x20;

			else
			{
    			return 0;
			}
    }

		else
		{
			for(i = 0; i < 33; i++)
			{
				if(byte2 == (ascii_special[i][0] & 0x00ff))
				{
					output = ascii_special[i][1];
					break;
				}
			}

			if(i == 33)
			{
				output = 0;
				return 0;
			}
		}
  	}

	else
	{
    	return 0;
	}

    return output;
}

/////////////////////////////////////////////////
void Sjis2AsciiString(unsigned char *title, char *string)
{
	int i = 0;
	int length;
	char temp;

	length = strlen((char *)title) / 2;

	for(i = 0; i < length; i++)
	{
		if((temp = Sjis2Ascii(title)) == 0)
		{
			strcpy(string, ".Kanji.");
			i = 7;
			break;
		}

		string[i] = temp;
		title += 2;
  	}
	string[i] = 0x00;
}

/////////////////////////////////////////////////
int IsSjis(unsigned char *title)
{
    if(((*title >= 129) && (*title <= 159)) || ((*title >= 224) && (*title <= 239)))
    	return 1;

    else if(*title < 129)
    	return 0;

    else
    	return -1;
}

//////////////////////////////////////////////////
long IsAscii(char *c)
{
    if(!(*c >> 7))
    	return 1;
    else
    	return 0;
}

//////////////////////////////////////////////////
u_short Ascii2Sjis(unsigned char ascii_code)
{
	u_short sjis_code = 0;
	unsigned char stmp = 0;	// avoid "uninitialized" warning (dc 01/29/02)
	unsigned char stmp2 = 0;

	if((ascii_code >= 0x20) && (ascii_code <= 0x2f))
		stmp2 = 1;

	else if((ascii_code >= 0x30) && (ascii_code <= 0x39))
		stmp = 0;

	else if((ascii_code >= 0x3a) && (ascii_code <= 0x40))
		stmp2 = 11;

	else if((ascii_code >= 0x41) && (ascii_code <= 0x5a))
		stmp = 1;

	else if((ascii_code >= 0x5b) && (ascii_code <= 0x60))
		stmp2 = 37;

	else if((ascii_code >= 0x61) && (ascii_code <= 0x7a))
		stmp = 2;

	else if((ascii_code >= 0x7b) && (ascii_code <= 0x7e))
		stmp2 = 63;

	else
	{
		printf("bad ASCII code 0x%x\n", ascii_code);
		return 0;
	}

	if (stmp2)
	   	sjis_code = ascii_special[ascii_code - 0x20 - (stmp2 - 1)][0];
	else
		sjis_code = ascii_table[stmp][0] + ascii_code - ascii_table[stmp][1];

	return sjis_code;
}

//////////////////////////////////////////////////
void AsciiString2Sjis(u_char *input, u_short *output)
{
	int i=0;
	int len;
	u_short sjis;
	u_char temp1, temp2;

	len = strlen((char *)input);

	for(i = 0; i < len; i++)
	{
		sjis = Ascii2Sjis(input[i]);
		temp1 = sjis;
		temp2 = sjis >> 8;
		output[i] = temp2 | (temp1 << 8);
  }

	output[i] = 0x0000;
}

//////////////////////////////////////////////////
short SwapShort(u_short input)
{
//	u_char temp1, temp2;
	return (input >> 8) | (input << 8);
}




DEFINE_SINGLETON(GenericGameSaver)
int GenericGameSaver::deleteFile(int port, int slot, saveInfo s)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::deleteFile");
  return GSErrorOther;
#else
  char mainName[100];
  char dirList[100];
  int cmd, result;
  bool exists, worked;
  sceMcTblGetDir listing[10] __attribute__((aligned (64)));
  int numEntries, type, free, formatted, retVal;


  retVal = getInfo(port, slot, &type, &free, &formatted);
  if (retVal != GSOk)
    return retVal;


  strcpy(mainName, "/");
  strcat(mainName, PRODUCT_CODE);
  strcat(mainName, s.shortname);

  strcpy(dirList, mainName);
  strcat(dirList, "/*");

  if (sceMcGetDir(port, slot, dirList, 0, 10, listing) != 0)
    if (sceMcGetDir(port, slot, dirList, 0, 10, listing) != 0)
      return GSErrorOther;


  if (sceMcSync(0, &cmd, &result) != 1)
  {
    if (sceMcGetDir(port, slot, dirList, 0, 10, listing) != 0)
    {
      if (sceMcGetDir(port, slot, dirList, 0, 10, listing) != 0)
      {
        return GSErrorOther;
      }
      else
      {
        if (sceMcSync(0, &cmd, &result) != 1)
          return GSErrorOther;
      }
    }
    else
    {
      if (sceMcSync(0, &cmd, &result) != 1)
        return GSErrorOther;
    }
  }
  numEntries = result;
  if (result >= 0)
  {
    exists = true;
  }
  else if (result == -4)
  {
    exists = false;
  }
  else
  {
    return GSErrorOther;
  }



  // Lets delete it!
  if (exists)
  {
    for (int i=0; i < numEntries; i++)
    {
      // We're in the dir
      // Delete the files
      if ((strcmp(".", (char *)listing[i].EntryName) != 0) &&
        (strcmp("..", (char *)listing[i].EntryName) != 0))
      {

        char daName[100];
        strcpy(daName, mainName);
        strcat(daName, "/");
        strcat(daName, (char *)listing[i].EntryName);
        //**************************************************
        // First the icon.sys
        worked = (sceMcDelete(port, slot, daName ) == 0);
        worked &= (sceMcSync(0, &cmd, &result) == 1);
        if (!worked)
        {
          worked = (sceMcDelete(port, slot, daName) == 0);
          worked &= (sceMcSync(0, &cmd, &result) == 1);
          if (!worked)
            return GSErrorOther;
        }
        if ((result!=0) && (result != -4)) return GSErrorOther;
      }
    }

    //**************************************************
    // Delete the dir
    worked = (sceMcDelete(port, slot, mainName) == 0);
    worked &= (sceMcSync(0, &cmd, &result) == 1);
    if (!worked)
    {
      worked = (sceMcDelete(port, slot, mainName) == 0);
      worked &= (sceMcSync(0, &cmd, &result) == 1);
      if (!worked)
        return GSErrorOther;
    }
    if ((result!=0) && (result != -4)) return GSErrorOther;
  }
  else return GSErrorDoesNotExist;
  return GSOk;

#endif /* TARGET_XBOX JIV DEBUG */
}

int GenericGameSaver::getInfo(int port, int slot, int *type, int *free, int *formatted)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::getInfo");
  return GSOk;
#else
  int cmd, result;//, re1t_val = 1 ;
  int retry = 2;
//  int retVal;

  while (retry)
  {
    if (sceMcGetInfo(port, slot, type, free, formatted) != 0)
    {
      if (sceMcGetInfo(port, slot, type, free, formatted) != 0)
        return GSErrorOther;
    }
    else if (sceMcSync(0, &cmd, &result) != 1)
    {
      return GSErrorOther;
    }
    else
    {
      switch (result)
      {
        case -1: break;
        case -2: retry--;
        case 0:
          if (!*formatted)
          {
            if (retry == 2)
              retry--;
            else
              return GSErrorUnformatted;
            break;
          }
          if (!type) return GSErrorNoMedia;
          if (*type != 2) return GSErrorUnknownMedia;
          else return GSOk;

        break;
        default:
          if (retry == 2) retry--;
          else return GSErrorOther;
      }
    }
		for (int stall=0; stall<100000; stall++) continue; 
  }

  return GSOk;

#endif /* TARGET_XBOX JIV DEBUG */
}

int GenericGameSaver::init()
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::init");

  icon.Buf = NULL;

  MemoryCardSema = 0;

  return GSOk;
#else
  if (load_iop_module("mcman.irx", "host0:\\ks\\data") == false)
    if (load_iop_module("mcman.irx", "cdrom0:") == false)
      return GSErrorCountNotLoadModules;

  if (load_iop_module("mcserv.irx", "host0:\\ks\\data") == false)
    if (load_iop_module("mcserv.irx", "cdrom0:") == false)
      return GSErrorCountNotLoadModules;

  // Do we have a card?
  // Currently we just check to make sure there is a
  // card in the first slot.
  // This will be more tolerant later
  arg = new GameSaverThreadArgs;
  icon.Buf = NULL;
  sceMcInit();
  SemaParam p;
  p.initCount = 1;
  p.maxCount = 1;

  MemoryCardSema = CreateSema(&p);

  return GSOk;
#endif /* TARGET_XBOX JIV DEBUG */
}




int GenericGameSaver::format(int port, int slot)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::format");
  return GSErrorOther;
#else
  int cmd, result, type, free, formatted;//, retry = 2;
  int ret = getInfo(port, slot, &type, &free, &formatted);
  result = -1;
  switch (ret)
  {
    case GSOk: return GSOk;
    case GSErrorUnformatted: break;
    default: return GSErrorOther;
  }

  /*while (retry)
  {
    if (sceMcFormat(port, slot) != 0)
      retry--;
    else if (sceMcSync(0, &cmd, &result) != 1)
      retry--;
    else if (result == 0)
      return GSOk;
    else
    {
      retry--;
    }
  }*/

  ret = sceMcFormat(port, slot);
  if (ret != 0)
  {
	ret = sceMcFormat(port, slot);
	if (ret != 0)
		return GSErrorOther;
  }
  ret = sceMcSync(0, &cmd, &result);
  if (ret != 1)
  {
	ret = sceMcSync(0, &cmd, &result);
	if (ret != 1)
		return GSErrorOther;
  }
  if (result != 0)
	  return GSErrorOther;
  else
	  return GSOk;

#endif /* TARGET_XBOX JIV DEBUG */
}
int GenericGameSaver::getFileListing(int port, int slot, saveInfo *info, void(*progressCallback)(void *, float), void *userData)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::getFileListing");
  return 0;
#else
  char name[33];
  int cmd, result;
  int entries = -1;
  int counter = 0;
  int formatted, type, free;
  int infoRes = getInfo(port, slot, &type, &free, &formatted);

  if (infoRes != GSOk)
    return infoRes;


  strcpy(name, "/");
  strcat(name, PRODUCT_CODE);
  strcat(name, "*");

  if (sceMcGetDir(port, slot, name , 0, MAXITEMS, tble) != 0)
    if (sceMcGetDir(port, slot, name , 0, MAXITEMS, tble) != 0)
      return GSErrorOther;

  if (sceMcSync(0, &cmd, &result) != 1)
    return GSErrorOther;

  if (progressCallback)
    progressCallback(userData, .05);

  entries = result;

  if (entries < 0)
    return GSErrorOther;


  for (int i=0; i < entries; i++)
  {
    char totalfile[100];
//    char buffer[200];
//    char mode[32];
//    int pos;

    // First we grab the shortname, everything after PRODUCT_CODE

    // Next read the index.nfo file
    strcpy(totalfile, "/");
    strcat(totalfile, (char *)tble[i].EntryName);
    strcat(totalfile, "/");
    strcat(totalfile, "index.nfo");

    // If we have a valid file..
    if (readData(port, slot, totalfile, &(info[counter]), sizeof(saveInfo)) >= 0)
    {
      info[counter].shortname[7] = '\0';
      info[counter].desc[67] = '\0';
      counter++;
    }
  }
  for (int i=0; i < counter; i++)
  {
    info[i].valid = isFileValid(port, slot, info[i].shortname);
    if (info[i].valid < 0)
    {
      info[i].valid = 0;
      return GSErrorOther;
    }
  }


  return counter;
#endif /* TARGET_XBOX JIV DEBUG */
}

void GenericGameSaver::readFileHelper(void *data)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::readFileHelper" );
#else
  char fname[100];

  GameSaverThreadArgs *args = (GameSaverThreadArgs *)data;
  WaitSema(MemoryCardSema);

  if (strlen(args->info->shortname) == 0)
  {
    args->progressCallback(args->userCallbackData, GSErrorOther);
    SignalSema(MemoryCardSema);
    ExitDeleteThread();
    return;
  }
  strcpy(fname, "/");
  strcat(fname, PRODUCT_CODE);
  strcat(fname, args->info->shortname);
  strcat(fname, "/");
  strcat(fname, PRODUCT_CODE);
  strcat(fname, args->info->shortname);

  int error = readData(args->port, args->slot, fname, args->buffer, args->size);
  if (error != GSOk)
  {
    if (args->progressCallback)
      args->progressCallback(args->userCallbackData, error);
    SignalSema(MemoryCardSema);
    ExitDeleteThread();
    return;
  }


  if (args->progressCallback)
    args->progressCallback(args->userCallbackData, 100);
  SignalSema(MemoryCardSema);
  ExitDeleteThread();
#endif /* TARGET_XBOX JIV DEBUG */
  return;
}

void GenericGameSaver::readFile(int port, int slot,
                               void *buffer, int maxSize,
                               void (*progressCallback)(void *, int),
                               void *userCallbackData)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::readFile" );
#else
  ThreadParam th_param;
  int dathread;
  th_param.entry = GenericGameSaver::readFileHelper;
  th_param.stack = this->dastack;
  th_param.stackSize = MC_STACK_SIZE;
  th_param.initPriority = 1;
  th_param.gpReg = &_gp;
  th_param.option = 0;
  arg->userCallbackData = userCallbackData;
  arg->progressCallback = progressCallback;
  arg->buffer = buffer;
  arg->size = maxSize;
  arg->info = &fInfo;
  arg->port = port;
  arg->slot = slot;

  // Validate the file
  arg->info->valid = isFileValid(arg->port, arg->slot, arg->info->shortname);

  if (arg->info->valid < 0)
  {
    progressCallback(userCallbackData, arg->info->valid);
    return;
  }

  dathread = CreateThread(&th_param);
  if (dathread < 0)
  {
	progressCallback(userCallbackData, -1);
    return;
  }
  if (StartThread(dathread, arg) < 0)
  {
	progressCallback(userCallbackData, -1);
    return;
  }

#endif /* TARGET_XBOX JIV DEBUG */
}




int GenericGameSaver::readData(int port, int slot,
                               char *name,
                               void *data, int maxSize)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::readData");
  return 0;
#else
  int fp, type, free, formatted, cmd, result;
  int infoRes = getInfo(port, slot, &type, &free, &formatted);
  int bytes =0;
  if (infoRes != GSOk)
    return infoRes;

  if (sceMcOpen(port, slot, name, SCE_RDONLY) != 0)
    if (sceMcOpen(port, slot, name, SCE_RDONLY) != 0)
      return GSErrorOther;

  if (sceMcSync(0, &cmd, &result) != 1)
  {
    if (sceMcOpen(port, slot, name, SCE_RDONLY) != 0)
    {
      if (sceMcOpen(port, slot, name, SCE_RDONLY) != 0)
      {
        return GSErrorOther;
      }
      else
      {
        if (sceMcSync(0, &cmd, &result) != 1)
          return GSErrorOther;
      }
    }
    else
    {
      if (sceMcSync(0, &cmd, &result) != 1)
        return GSErrorOther;
    }
  }

  if (result < 0)
    return GSErrorOther;

  fp = result;

  if (sceMcRead(fp, data, maxSize) != 0)
    if (sceMcRead(fp, data, maxSize) != 0)
      return GSErrorOther;

  while (sceMcSync(1, &cmd, &result)==0)
  {
    bytes +=result;
  }

  if (sceMcClose(fp) != 0)
    if (sceMcClose(fp) != 0)
      return GSErrorOther;

  sceMcSync(0, &cmd, &result);


  return bytes;
#endif /* TARGET_XBOX JIV DEBUG */
}

int GenericGameSaver::isFileValid( int port, int slot, char *shortname )
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::isFileValid" );

  return false;
#else
  char iconname[100];
  sceMcTblGetDir tble[1] __attribute__((aligned (64)));;
//  bool worked;
  int free, formatted, type, cmd, result;

  shortname[7] = '\0';

  if (getInfo(port, slot, &type, &free, &formatted) < 0)
    return false;
  strcpy(iconname, "/");
  strcat(iconname, PRODUCT_CODE);
  strcat(iconname, shortname);
  strcat(iconname, "/");
  strcat(iconname, PRODUCT_CODE);
  strcat(iconname, shortname);
  strcat(iconname, ".icn");


  if (sceMcGetDir(port, slot, iconname , 0, 1, tble) != 0)
    if (sceMcGetDir(port, slot, iconname , 0, 1, tble) != 0)
      return GSErrorOther;

  if (sceMcSync(0, &cmd, &result) != 1)
    return GSErrorOther;
  if (result == -4)
    return false;
  else if (result < 0)
    return GSErrorOther;


  return (tble[0].AttrFile & sceMcFileAttrClosed);
#endif /* TARGET_XBOX JIV DEBUG */
}



void GenericGameSaver::saveFileHelper( void *data )
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::saveFile");
  return;
#else
  GameSaverThreadArgs *args = (GameSaverThreadArgs *)data;

  char mainName[21];
  int retVal;//, count;
  int BP = -1, len, halflen, i, j, k, cmd, result;
//  char index[200];
  bool worked = false;
  int infoRes, type, free, formatted;
  bool exists = false;

  WaitSema(MemoryCardSema);

  // Setup the base filename
  strcpy(mainName, PRODUCT_CODE);
  strcat(mainName, args->info->shortname);

  char dirName[strlen(mainName) + 3];
  char fullName[ strlen(mainName) * 2 + 3 ];
  char iconName[ strlen(mainName) + 5 ];
  char sysName[ strlen(mainName) + 3 + 8 ];
  char indexName[ strlen(mainName) + 3 + 9 ];

  strcpy(dirName, "/");
  strcat(dirName, mainName);
  strcat(dirName, "/");




  strcpy(fullName, "/");
  strcat(fullName, mainName);
  strcat(fullName, "/");
  strcat(fullName, mainName);

  strcpy(iconName, mainName);
  strcat(iconName, ".icn");

  strcpy(sysName, "/");
  strcat(sysName, mainName);
  strcat(sysName, "/");
  strcat(sysName, "icon.sys");


  strcpy(indexName, "/");
  strcat(indexName, mainName);
  strcat(indexName, "/");
  strcat(indexName, "index.nfo");



  infoRes = getInfo(args->port, args->slot, &type, &free, &formatted);

  if (infoRes != GSOk)
  {
    if (args->progressCallback)
      args->progressCallback(args->userCallbackData, infoRes);
    SignalSema(MemoryCardSema);
    ExitDeleteThread();
    return;
  }

  // Does this dir exist?
  if (sceMcGetDir(args->port, args->slot, mainName, 0, 0, NULL) != 0)
    if (sceMcGetDir(args->port, args->slot, mainName, 0, 0, NULL) != 0)
    {
      if (args->progressCallback)
        args->progressCallback(args->userCallbackData, GSErrorOther);
      SignalSema(MemoryCardSema);
      ExitDeleteThread();
      return;
    }

  if (sceMcSync(0, &cmd, &result) != 1)
  {
    if (sceMcChdir(args->port, args->slot, mainName, 0) != 0)
    {
      if (sceMcChdir(args->port, args->slot, mainName, 0) != 0)
      {
        if (args->progressCallback)
          args->progressCallback(args->userCallbackData, GSErrorOther);
        SignalSema(MemoryCardSema);
        ExitDeleteThread();
        return;
      }
      else
      {
        if (sceMcSync(0, &cmd, &result) != 1)
        {
          if (args->progressCallback)
            args->progressCallback(args->userCallbackData, GSErrorOther);
          SignalSema(MemoryCardSema);
          ExitDeleteThread();
          return;
        }
      }
    }
    else
    {
      if (sceMcSync(0, &cmd, &result) != 1)
      {
        if (args->progressCallback)
          args->progressCallback(args->userCallbackData, GSErrorOther);
        SignalSema(MemoryCardSema);
        ExitDeleteThread();
        return;
      }
    }
  }

  if (result > 0)
  {
    if (args->overWrite) exists = true;
    else
    {
      if (args->progressCallback)
        args->progressCallback(args->userCallbackData, GSErrorFileExists);
      SignalSema(MemoryCardSema);
      ExitDeleteThread();
      return;
    }
  }
  else if (result == -4)
  {
    exists = false;
  }
  else if (result == 0)
  {
    exists = false;
  }
  else
  {
    if (args->progressCallback)
      args->progressCallback(args->userCallbackData, GSErrorOther);
    SignalSema(MemoryCardSema);
    ExitDeleteThread();
    return;
  }





  // Lets delete it!
  if (exists && args->overWrite)
  {
    // We're in the dir
    // Delete the 3 files

    //**************************************************
    // First the icon.sys
    worked = (sceMcDelete(args->port, args->slot, sysName) == 0);
    worked &= (sceMcSync(0, &cmd, &result) == 1);
    if (!worked)
    {
      worked = (sceMcDelete(args->port, args->slot, sysName) == 0);
      worked &= (sceMcSync(0, &cmd, &result) == 1);
      if (!worked)
      {
        if (args->progressCallback)
          args->progressCallback(args->userCallbackData, GSErrorOther);
        SignalSema(MemoryCardSema);
        ExitDeleteThread();
        return;
      }
    }
    if ((result!=0) && (result != -4))
    {
      if (args->progressCallback)
          args->progressCallback(args->userCallbackData, GSErrorOther);
      SignalSema(MemoryCardSema);
      ExitDeleteThread();
      return;
    }

    //**************************************************
    // Now the main file
    worked = (sceMcDelete(args->port, args->slot, fullName) == 0);
    worked &= (sceMcSync(0, &cmd, &result) == 1);
    if (!worked)
    {
      worked = (sceMcDelete(args->port, args->slot, fullName) == 0);
      worked &= (sceMcSync(0, &cmd, &result) == 1);
      if (!worked)
      {
        if (args->progressCallback)
          args->progressCallback(args->userCallbackData, GSErrorOther);
        SignalSema(MemoryCardSema);
        ExitDeleteThread();
        return;
      }
    }
    if ((result!=0) && (result != -4))
    {
      if (args->progressCallback)
        args->progressCallback(args->userCallbackData, GSErrorOther);
      SignalSema(MemoryCardSema);
      ExitDeleteThread();
      return;
    }

    //**************************************************
    // now the index file
    worked = (sceMcDelete(args->port, args->slot, indexName) == 0);
    worked &= (sceMcSync(0, &cmd, &result) == 1);
    if (!worked)
    {
      worked = (sceMcDelete(args->port, args->slot, indexName) == 0);
      worked &= (sceMcSync(0, &cmd, &result) == 1);
      if (!worked)
      {
        if (args->progressCallback)
          args->progressCallback(args->userCallbackData, GSErrorOther);
        SignalSema(MemoryCardSema);
        ExitDeleteThread();
        return;
      }
    }
    if ((result!=0) && (result != -4))
    {
      if (args->progressCallback)
        args->progressCallback(args->userCallbackData, GSErrorOther);
      SignalSema(MemoryCardSema);
      ExitDeleteThread();
      return;
    }

    //**************************************************
    // Now the icon File
    worked = (sceMcDelete(args->port, args->slot, iconName) == 0);
    worked &= sceMcSync(0, &cmd, &result);
    if (!worked)
    {
      worked = (sceMcDelete(args->port, args->slot, iconName) == 0);
      worked &= (sceMcSync(0, &cmd, &result) == 1);
      if (!worked)
      {
        if (args->progressCallback)
          args->progressCallback(args->userCallbackData, GSErrorOther);
        SignalSema(MemoryCardSema);
        ExitDeleteThread();
        return;
      }
    }
    if ((result!=0) && (result != -4))
    {
      if (args->progressCallback)
        args->progressCallback(args->userCallbackData, GSErrorOther);
      SignalSema(MemoryCardSema);
      ExitDeleteThread();
      return;
    }

    //**************************************************
    // Delete the dir
    worked = (sceMcDelete(args->port, args->slot, mainName) == 0);
    worked &= (sceMcSync(0, &cmd, &result) == 1);
    if (!worked)
    {
      worked = (sceMcDelete(args->port, args->slot, mainName) == 0);
      worked &= (sceMcSync(0, &cmd, &result) == 1);
      if (!worked)
      {
        if (args->progressCallback)
          args->progressCallback(args->userCallbackData, GSErrorOther);
        SignalSema(MemoryCardSema);
        ExitDeleteThread();
        return;;
      }
    }
    if ((result!=0) && (result != -4))
    {
      if (args->progressCallback)
        args->progressCallback(args->userCallbackData, GSErrorOther);
      SignalSema(MemoryCardSema);
      ExitDeleteThread();
      return;
    }

  }
  else if (exists & !args->overWrite)
  {
    if (args->progressCallback)
      args->progressCallback(args->userCallbackData, GSErrorOther);
    SignalSema(MemoryCardSema);
    ExitDeleteThread();
    return;
  }
  if (args->progressCallback)
    args->progressCallback(args->userCallbackData, 10);

  worked = (sceMcMkdir(args->port, args->slot, mainName) == 0);
  worked &= (sceMcSync(0, &cmd, &result) == 1);
  if (!worked)
  {
    worked = (sceMcMkdir(args->port, args->slot, mainName) == 0);
    worked &= (sceMcSync(0, &cmd, &result) == 1);
    if (!worked)
    {
      if (args->progressCallback)
        args->progressCallback(args->userCallbackData, GSErrorOther);
      SignalSema(MemoryCardSema);
      ExitDeleteThread();
      return;
    }
  }
  if ((result!=0) && (result != -4))
  {
    if (args->progressCallback)
      args->progressCallback(args->userCallbackData, GSErrorOther);
    SignalSema(MemoryCardSema);
    ExitDeleteThread();
    return;
  }

  if (args->progressCallback)
    args->progressCallback(args->userCallbackData, 15);
  /*********************************************
   Write the main data
  ***********************************************/
  retVal = saveData(args->port, args->slot, fullName, args->buffer, args->size, args->overWrite);
  if (retVal != GSOk)
  {
    if (args->progressCallback)
      args->progressCallback(args->userCallbackData, retVal);
    SignalSema(MemoryCardSema);
    ExitDeleteThread();
    return;
  }

  if (args->progressCallback)
    args->progressCallback(args->userCallbackData, 80);
  /*********************************************
   Write the index file
  ***********************************************/
  sceCdCLOCK cloc;

  sceCdReadClock(&cloc);

  // May not be perfectly right, but its good enough for sorting
  args->info->timestamp = cloc.second + cloc.minute*60 +
                          cloc.hour*3600 + cloc.day*3600*24 +
                          cloc.month*31*24*3600 + cloc.year*12*31*24*3600;
  retVal = saveData(args->port, args->slot, indexName, (void *)args->info, sizeof(saveInfo), args->overWrite);
  if (retVal != GSOk)
  {
    if (args->progressCallback)
      args->progressCallback(args->userCallbackData, retVal);
    SignalSema(MemoryCardSema);
    ExitDeleteThread();
    return;
  }

  if (args->progressCallback)
    args->progressCallback(args->userCallbackData, 85);
  /************************************************************
    We now have good data.. so we write the icons
  ************************************************************/

  // Find breakpoint in the description
//  BP = len;	// "len" is uninitialized at this point! (dc 01/29/02)
  len = strlen((char *)args->info->desc);
  halflen = len/2;
  i = halflen; j = halflen + 1;
  for (k=0; k < halflen+1; k++)
  {
    if (i >= 0)
    {
      if (args->info->desc[i] == ' ')
      {
        BP = i;
        break;
      }
    }
    i--;
    if (j < len)
    {
      if (args->info->desc[j] == ' ')
      {
        BP = j;
        break;
      }
    }
    j++;
  }

  assert(BP != -1);	// else uninitialized (dc 01/29/02)

  // Write the icon stuff
  int retval = setupIcons(args->port, args->slot, (unsigned char *)args->info->desc, BP, dirName, iconName, args->iconData, args->iconSize);
  if (retval != GSOk)
  {
    if (args->progressCallback)
      args->progressCallback(args->userCallbackData, retval);
    SignalSema(MemoryCardSema);
    ExitDeleteThread();
    return;
  }
  if (args->progressCallback)
    args->progressCallback(args->userCallbackData, 100);
  SignalSema(MemoryCardSema);
  ExitDeleteThread();

#endif
  return;
}

void GenericGameSaver::releaseIconData()
{
  if (icon.Buf)
    KSReleaseFile(&icon);
  icon.Buf = NULL;
}
int GenericGameSaver::getIconData()
{
  if (icon.Buf)
    releaseIconData();
  KSReadFile("KS.icn", &icon, 1);

  return icon.Size;
}
void GenericGameSaver::saveFile(int port, int slot,
                               void *data, int size,
                               bool overWrite,
                               void (*progressCallback)(void *, int),
                               void *userCallbackData)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::saveFile" );
#else
  ThreadParam th_param;
  int dathread;

  th_param.entry = GenericGameSaver::saveFileHelper;
  th_param.stack = this->dastack;
  th_param.stackSize = MC_STACK_SIZE;
  th_param.initPriority = 2;

  th_param.gpReg = &_gp;
  th_param.option = 0;

  arg->buffer = data;
  arg->size = size;
  arg->port = port;
  arg->slot = slot;
  arg->iconData = icon.Buf;
  arg->iconSize = icon.Size;
  arg->overWrite = overWrite;
  arg->info = &fInfo;
  arg->progressCallback = progressCallback;
  arg->userCallbackData = userCallbackData;


  dathread = CreateThread(&th_param);
  StartThread(dathread, arg);

  return;
#endif /* TARGET_XBOX JIV DEBUG */
}



int GenericGameSaver::saveData(int port, int slot, char *name, void *data, int size, bool overWrite)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::saveData");
  return GSErrorOther;
#else
  int fp;
  int infoRes;
  int cmd, result, type, free, formatted;
//  bool exists = false;
  bool worked = 0;
//  char iconName[33];
  int count =2;

  infoRes = getInfo(port, slot, &type, &free, &formatted);

  if (infoRes != GSOk)
    return infoRes;

  // Do we have enough free space?
  result = -1;
  // Now start writing
  // Open file
  count = 2; result = -1;
  while ((result != 0) && (count > 0))
  {
    worked = (sceMcOpen(port, slot, name, SCE_WRONLY | SCE_CREAT) == 0);
    worked &= (sceMcSync(0, &cmd, &result) == 1);
    if (!worked)
    {
      worked = (sceMcOpen(port, slot, name, SCE_WRONLY | SCE_CREAT) == 0);
      worked &= (sceMcSync(0, &cmd, &result) == 1);
      if (!worked)
        return GSErrorOther;
    }
    count --;
  }
  switch (result)
  {
    case -2: return GSErrorUnformatted;
    case 0: break;
    default: return GSErrorOther;
  }

  fp = result;

  if (sceMcWrite(fp, data, size) != 0)
    if (sceMcWrite(fp, data, size) != 0)
      return GSErrorOther;

  // Write data out
  while (!sceMcSync(1, &cmd, &result))
  {
    if (result == -2)
      return GSErrorUnformatted;
    else if (result == -3)
      return GSErrorNotEnoughSpace;
    else if (result < 0)
      return GSErrorOther;

  }

  // Close this file
  count = 2; result = -1;
  while ((result != 0) && (count > 0))
  {
    worked = (sceMcClose(fp) == 0);
    worked &= (sceMcSync(0, &cmd, &result) == 1);
    if (!worked)
    {
      worked = (sceMcClose(fp) == 0);
      worked &= (sceMcSync(0, &cmd, &result) == 1);
      if (!worked)
        return GSErrorOther;
    }
    count --;
  }

  switch (result)
  {
    case -2: return GSErrorUnformatted;
    case 0: break;
    default: return GSErrorOther;
  }


  return GSOk;
#endif /* TARGET_XBOX JIV DEBUG */
}

void GenericGameSaver::setFileInfo(saveInfo s)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB("GenericGameSaver::setFileInfo");
#else
  WaitSema(MemoryCardSema);
  fInfo = s;
  SignalSema(MemoryCardSema);
#endif /* TARGET_XBOX JIV DEBUG */
};


int GenericGameSaver::setupIcons(int port, int slot, unsigned char gameTitle[33], int breakPoint, char *directory, char *iconName, void *iconData, int iconDataSize)
{
#if defined(TARGET_XBOX) || defined(TARGET_GC)
  STUB( "GenericGameSaver::setupIcons");
  return GSErrorOther;
#else
//  int mode = SCE_RDWR | SCE_CREAT;
//  int cmd, result;
//  int iconfp;
	static sceVu0FVECTOR ambient = { 0.50, 0.50, 0.50, 0.00 };
  unsigned short SJISTitle[33];
  char fullSysName[100];
  char fullIconName[100];

  strcpy(fullSysName, directory);
  strcat(fullSysName, "icon.sys");

  strcpy(fullIconName, directory);
  strcat(fullIconName, iconName);


  static sceVu0IVECTOR bgcolor[4] = {
		{ 0x80,    0,    0, 0 },
		{    0, 0x80,    0, 0 },
		{    0,    0, 0x80, 0 },
		{ 0x80, 0x80, 0x80, 0 },
	};
	static sceVu0FVECTOR lightdir[3] = {
		{ 0.5, 0.5, 0.5, 0.0 },
		{ 0.0,-0.4,-0.1, 0.0 },
		{-0.5,-0.5, 0.5, 0.0 },
	};
	static sceVu0FVECTOR lightcol[3] = {
		{ 0.48, 0.48, 0.03, 0.00 },
		{ 0.50, 0.33, 0.20, 0.00 },
		{ 0.14, 0.14, 0.38, 0.00 },
	};

  sceMcIconSys sysfile;

  memset((void *)&sysfile, 0, sizeof(sysfile));
  // Setup the icon.sys file
  // Convert the title name
  AsciiString2Sjis(gameTitle, SJISTitle);

  sysfile.Head[0] = 'P';
  sysfile.Head[1] = 'S';
  sysfile.Head[2] = '2';
  sysfile.Head[3] = 'D';

  memcpy(sysfile.TitleName, SJISTitle, sizeof(SJISTitle));

  sysfile.OffsLF = breakPoint*sizeof(unsigned short);
  sysfile.TransRate = 0x60;

  memcpy(sysfile.BgColor, bgcolor, sizeof(bgcolor));
	memcpy(sysfile.LightDir, lightdir, sizeof(lightdir));
	memcpy(sysfile.LightColor, lightcol, sizeof(lightcol));
	memcpy(sysfile.Ambient, ambient, sizeof(ambient));

  strcpy((char *)sysfile.FnameView, iconName);
  strcpy((char *)sysfile.FnameCopy, iconName);
  strcpy((char *)sysfile.FnameDel, iconName);

  sysfile.Reserv1 = sysfile.Reserv2 = 0;
  memset(sysfile.Reserve3, 0, 512);

  if (saveData(port, slot, fullSysName, (void *)&sysfile, sizeof(sysfile), true) < 0)
    return GSErrorOther;

  if (saveData(port, slot, fullIconName, (void *)iconData, iconDataSize, true) < 0)
    return GSErrorOther;

  return GSOk;


#endif /* TARGET_XBOX JIV DEBUG */
}

