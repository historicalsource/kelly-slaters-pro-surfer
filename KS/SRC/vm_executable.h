// vm_executable.h
#ifndef _VM_EXECUTABLE_H
#define _VM_EXECUTABLE_H


#include "stringx.h"
#include "chunkfile.h"
#include "script_library_class.h"

class symbol;
class script_object;
class script_manager;


class vm_executable
  {
  // Types
  public:
    typedef vector<script_library_class*> parms_list;

  // Data
  protected:
    script_object* owner;
    stringx name;
    stringx fullname;
    // function parameters
    parms_list parameters;
    int parms_stacksize;
    bool static_func;
    bool linked;
    // executable code buffer
    unsigned short* buffer;
    int buffer_len;
    // strings referenced by this code block
    vector<stringx const *> strings;

  // Constructors
  public:
    vm_executable( script_object* _owner );
    vm_executable(const vm_executable& b);
    ~vm_executable();

  // Methods
  public:
    script_object* get_owner() const         { return owner; }
    const stringx& get_name() const          { return name; }
    const stringx& get_fullname() const      { return fullname; }
    const parms_list& get_parameters() const { return parameters; }
    bool is_static() const                   { return static_func; }
    bool is_linked() const                   { return linked; }
    int get_parms_stacksize() const          { return parms_stacksize; }
    const unsigned short* get_start() const  { return buffer; }
    int get_size() const                     { return buffer_len; }


  // Internal Methods
  protected:
    void _destroy();
    void _clear();
    void _build_fullname();
    unsigned short _string_id(const stringx& s);

    // Link all SDR, SFR, and LFR references in the executable code to the
    // appropriate run-time addresses found via the given script manager.
    void _link(const script_manager& sm);

  // Friends
#if !defined(NO_SERIAL_IN)
  friend void serial_in(chunk_file& io,vm_executable* x);
#endif
#if !defined(NO_SERIAL_OUT)
  friend void serial_out(chunk_file& io,const vm_executable& x);
#endif
  friend class script_object;
  };

const chunk_flavor CHUNK_VM_EXECUTABLE    ("xecutabl");
const chunk_flavor CHUNK_PARAMETERS       ("Nparms");
const chunk_flavor CHUNK_PARMS_STACKSIZE  ("parmsize");
const chunk_flavor CHUNK_NUM_STRINGS      ("Nstrings");
const chunk_flavor CHUNK_CODESIZE         ("codesiz");
const chunk_flavor CHUNK_CODE             ("code");


#endif  // _VM_EXECUTABLE_H
