////////////////////////////////////////////////////////////////////////////////
/*
  msgboard.cpp

  Probably a temporary item.  Built for visual 

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

//!#include "character.h"
#include "msgboard.h"
#include "debug.h"
#include "osdevopts.h"
#include "hwrasterize.h"


message_board::message_board()
{
}



void message_board::post(stringx s, time_value_t time )
{
#if !defined(BUILD_BOOTABLE) && !defined(BUILD_RELEASE)
  unsigned i;
  for (i=0;i<messages.size();++i)
  {
    if (messages[i].time==0) 
      break;
  }
  message msg;
  strncpy(msg.text,s.c_str(),MESSAGE_STRING_LENGTH-1);
  msg.time = time;

  if (i==messages.size())
    messages.push_back(msg);
  else
    messages[i] = msg;
#endif
}


void message_board::frame_advance(time_value_t time_inc)
{
#if !defined(BUILD_BOOTABLE) && !defined(BUILD_RELEASE)
  for (unsigned i=0;i<messages.size();++i)
  {
    messages[i].time -= time_inc;
    if (messages[i].time<0) messages[i].time = 0;
  }
#endif
}

void message_board::render()
{
#if !defined(BUILD_BOOTABLE) && !defined(BUILD_RELEASE)
  if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_MESSAGES) )
  {
    int pix_y = 390;
    for (unsigned i=0;i<messages.size();++i)
    {
      if (messages[i].time>0)
      {
        float intensity = messages[i].time+.25f<1.0f?messages[i].time+.25f:1.0f;
        hw_rasta::inst()->print(messages[i].text,vector2di(0,pix_y),
                  color32(255,255,255,intensity*255));
      }
      pix_y -= 12;
    }
  }
#endif
}

