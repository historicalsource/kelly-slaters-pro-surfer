#ifndef ERRHANDLER_H
#define ERRHANDLER_H

class error_handler
  {
  public:
    error_handler() : error(false) {}
    error_handler(const stringx& _message) : message(_message), error(true) {}
    const stringx& get_message() {return message;}
    bool  get_error() {return error;}
  private:
    bool   error;
    stringx message;
  };

#endif