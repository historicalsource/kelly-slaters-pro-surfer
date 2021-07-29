#include "global.h"

#include <algorithm>
#include "particlecleaner.h"
#include "particle.h"

///////////////////////////////////////////////////////////////////////////////
// Particle_cleaner
///////////////////////////////////////////////////////////////////////////////
particle_cleaner *g_particle_cleaner;

void particle_cleaner::clean_up_particles()
{
  particle_cleaner::iterator i = begin();
  while ( i != end() )
  {
    particle_generator* pg = *i;
    assert( pg );
    if ( !pg->is_active() && !pg->is_still_visible() )
    {
      pg->destroy_particles();
      i = erase( i );
    }
    else
      ++i;
  }
}

void particle_cleaner::add_particle_list( particle_generator* _generator )
{
  push_back( _generator );
}

void particle_cleaner::remove_particle_list( particle_generator* _generator )
{
  particle_cleaner::iterator pci = find( begin(), end(), _generator );
  assert( pci != end() );
  erase( pci );
}
