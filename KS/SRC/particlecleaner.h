#ifndef PARTICLE_CLEANER_H
#define PARTICLE_CLEANER_H

#include <vector>

class particle_generator; // from particle.h

class particle_cleaner : public vector<particle_generator*>
{
public:
  void add_particle_list(particle_generator * _generator);
  void remove_particle_list(particle_generator * _generator);
  void clean_up_particles();
};

extern particle_cleaner* g_particle_cleaner;


#endif