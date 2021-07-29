//SNDEntry.h
//SND file entry

#ifndef SNDENTRY_H
#define SNDENTRY_H

#include <string.h>

class SNDEntry
{
  public:
    SNDEntry();
    SNDEntry(const SNDEntry& c);
    ~SNDEntry();

    bool parse_line( const char *line ); //NULL or newline terminated
    const char *get_name() { return name; }
    const char *get_snd_entry() { return snd_entry; }

    bool operator==( const char *_name )
    {
      return !stricmp( _name, name );
    }
    bool is_used() { return used;}
    void mark_used() { used = true; }
    void clear_used() { used = false; }

  protected:
    char *name;
    char *snd_entry;
    bool used;

  private:
    bool parse_name();
    const char *find_end_of_valid( const char *line );
    const char *find_whitespace( const char *line );
};

#endif