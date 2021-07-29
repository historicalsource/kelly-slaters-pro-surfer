#ifndef APP_H
#define APP_H
////////////////////////////////////////////////////////////////////////////////
/*
  app.h
  application class
  manages the initialization and destruction of stuff that lasts the life
  of the program, and acts as a layer between the operating system and the
  game:  the module diagram looks kind of like this

     operating system (main)
               |
              app
              /  \
           game   \
            |      \
          abstraction layer (ras_ functions)
                |
         input/output APIs  (directX)

*/
////////////////////////////////////////////////////////////////////////////////
#include "singleton.h"
#include "signals.h"

class game;
class instance_render_info;

class app : public singleton, public signaller
{
  public:
    // initializes rasterizer
    app();

    // cleans up
    ~app();

    static void cleanup();
    static void bomb(); // shuts down everything and calls exit(), or may reboot a console

		static void cleanup_stl_memory_dregs( void );

    // the application's main update loop.
    void tick();

    // returns a pointer to the single instance of app
    DECLARE_SINGLETON(app)

    game* get_game() { return the_game; }

  private:
    game* the_game;

    instance_render_info* viri;  // moved from a global into app for memory-leak removal purposes --GT 4/17/01

#ifdef TARGET_XBOX
	float reboot_timer; // keeps track of whether the reboot key combo has been pressed for the appropriate amount of time
#endif

  /////////////////////////////////////////////////////////////////////////////
  // Event signals
  /////////////////////////////////////////////////////////////////////////////
  public:
    // enum of local signal ids (for coding convenience and readability)
    enum signal_id_t
    {
      // a descendant class uses the following line to append its local signal ids after the parent's
      PARENT_SYNC_DUMMY = signaller::N_SIGNALS - 1,
      #define MAC(label,str)  label,
      #include "global_signals.h"
      #undef MAC
      N_SIGNALS
    };

    // This static function must be implemented by every class which can generate
    // signals, and is called once only by the application for each such class;
    // the effect is to register the name and local id of each signal with the
    // signal_manager.  This call must be performed before any signal objects are
    // actually created for this class (via signaller::signal_ptr(); see signal.h).
    static void register_signals();

    static unsigned short get_signal_id( const char *name );

    instance_render_info* get_viri();
    void set_viri(instance_render_info* new_viri);

  private:
    // Every descendant of signaller that expects to generate signals and has
    // defined its own local list of signal ids should implement this virtual
    // function for the construction of the signal list, so that it will reserve
    // exactly the number of signal pointers required, on demand.
    virtual signal_list* construct_signal_list() { return NEW signal_list( N_SIGNALS, (signal*)NULL ); }

  protected:
    // This virtual function, used only for debugging purposes, returns the
    // name of the given local signal
    virtual const char* get_signal_name( unsigned idx ) const;
};

#endif
