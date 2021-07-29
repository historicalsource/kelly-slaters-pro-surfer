#ifndef MSGBOARD_H
#define MSGBOARD_H

#include "ostimer.h"

// These are included because everyone who uses the message board will end up includiong them anyway.
// The class does not depend on them.
#include "app.h"
#include "game.h"


#define MESSAGE_STRING_LENGTH 100

class message_board
{
public:
  message_board();
  void post(stringx s, time_value_t time); // length of time it should stay up
  void frame_advance(time_value_t time_inc);
  void render();
private:
  struct message
  {
    char text[MESSAGE_STRING_LENGTH];
    time_value_t time;
  };
  vector<message> messages;
};

#endif
