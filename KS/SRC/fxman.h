#ifndef FXMAN_H
#define FXMAN_H
////////////////////////////////////////////////////////////////////////////////
/*
  fxman

  Special FX manager
*/
////////////////////////////////////////////////////////////////////////////////
class entity;
class po;
class visual_rep;

////////////////////////////////////////////////////////////////////////////////
enum { MAX_NUM_SPECIAL_FX=64 };

class fx_manager : public singleton
{
  public:
    fx_manager();
    ~fx_manager();

    DECLARE_SINGLETON(fx_manager)

    void      reset();
    void      load_effect_visual( const stringx& visrep_name );
    entity*   play_effect( const stringx& visrep_name,
                           const po& where,
                           unsigned flavor );
    enum 
    {
      NON_LOOPING = 0,
      LOOPING = 1
    };

  private:
    vector<visual_rep*> preloaded_effect_visuals;
    vector<bool>        effect_slot_in_use;
    int                 minimum_size;
};

#endif