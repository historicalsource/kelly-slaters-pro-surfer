#ifndef _PATH_H_
#define _PATH_H_

#include "global.h"

#include "color.h"

#if _ENABLE_WORLD_EDITOR
  #include <fstream.h>
#endif


#include "graph.h"
#include "region_graph.h"

//class region;
//class portal;
//typedef graph<stringx,region*,portal*> region_graph;
//typedef region_graph::node region_node;
class camera;


class path_graph_edge;

class path_graph_node
{
protected:
  vector3d pt;
  region_node* my_region;
  unsigned short flags;

  // sorted list of connecting edges, shortest to longest
  vector<path_graph_edge *>edges;

  void remove_edge(const path_graph_edge *edge);

  friend class path_graph_edge;
  friend class path_graph;
  friend class path;
  friend class PathsDialog;

  enum
  {
    _NODE_VISITED      = 0x0001, // internal use for path finding only!
    _NODE_W_SORTED     = 0x0002, // internal use for path finding only!
    _NODE_B_SORTED     = 0x0004, // internal use for path finding only!
  };

public:
  enum
  {
    _NODE_HIDE_DUCK    = 0x0008, // duck, stand, fire, duck, etc...
    _NODE_HIDE_COVER   = 0x0010, // hide, come out, fire, hide, etc
    _NODE_HIDE_SEEK    = 0x0020, // spot to come out to when cover hiding (not necessary)
    _NODE_PATROL_DELAY = 0x0040, // wait at this node for a random period of time (determined by the brain)
    _NODE_HIDE         = 0x0018, // both hiding flags set brain decides which acion to take
    _NODE_MAX_FLAGS
  };

  path_graph_node();
  path_graph_node(const path_graph_node &b) { copy( b ); }
  virtual ~path_graph_node();

  path_graph_node& operator=(const path_graph_node &b) { copy( b ); return *this; }
  void copy(const path_graph_node &b);
  void clear();

  void add_edge(path_graph_edge *edge, bool sort = true);
  void sort_edges()                   { sort_edges_weight(); }
  void sort_edges_weight();
  void sort_edges_bias();

  const vector3d &get_abs_position() const       { return pt; }
  path_graph_edge *get_edge(int i) const { return edges[i]; }
  int get_num_edges() const           { return edges.size(); }
  unsigned short get_flags() const    { return flags; }

  bool is_flagged(unsigned short flag) const          { return (flags & flag) != 0; }
  void set_flag(unsigned short flag, bool on=true)    { if(on) flags |= flag; else flags &= ~flag; }

  void set_duck_spot(bool on=true)      { set_flag(_NODE_HIDE_DUCK, on); }
  bool is_duck_spot() const             { return(is_flagged(_NODE_HIDE_DUCK)); }

  void set_cover_spot(bool on=true)     { set_flag(_NODE_HIDE_COVER, on); }
  bool is_cover_spot() const            { return(is_flagged(_NODE_HIDE_COVER)); }

  void set_hiding_spot(bool on=true)    { set_flag(_NODE_HIDE, on); }
  bool is_hiding_spot() const           { return(is_flagged(_NODE_HIDE)); }

  void set_seek_spot(bool on=true)      { set_flag(_NODE_HIDE_SEEK, on); }
  bool is_seek_spot() const             { return(is_flagged(_NODE_HIDE_SEEK)); }

  void set_patrol_delay(bool on=true)   { set_flag(_NODE_PATROL_DELAY, on); }
  bool is_patrol_delay() const          { return(is_flagged(_NODE_PATROL_DELAY)); }

  region_node* get_my_region() const    { return(my_region); }

  friend bool operator==(const path_graph_node &lhs, const path_graph_node &rhs );

protected:
  void visit()    { assert(!visited()); set_flag(_NODE_VISITED, true); }
  void unvisit()  { assert(visited()); set_flag(_NODE_VISITED, false); }
  bool visited() const { return is_flagged(_NODE_VISITED); }

  bool is_weight_sorted() const { return is_flagged(_NODE_W_SORTED); }
  bool is_bias_sorted() const   { return is_flagged(_NODE_B_SORTED); }
  void clear_sorting()          { set_flag(_NODE_W_SORTED, false); set_flag(_NODE_B_SORTED, false); }

};

inline bool operator==(const path_graph_node &lhs, const path_graph_node &rhs )
{
  bool retval = false;

  if(lhs.pt == rhs.pt)
  {
    if(lhs.edges.size() == rhs.edges.size())
    {
      vector<path_graph_edge *>::const_iterator i = lhs.edges.begin();
      vector<path_graph_edge *>::const_iterator j = rhs.edges.begin();

      while(i != lhs.edges.end())
      {
        if(*i != *j)
          break;

        ++i;
        ++j;
      }

      if(i == lhs.edges.end() && j == rhs.edges.end())
        retval = true;
    }
  }

  return retval;
}



class path_graph_edge
{
protected:
  path_graph_node *nodes[2];
  unsigned short flags;

  rational_t distance;
  rational_t weight_modifier;
  rational_t additional_weight_modifier;

  friend class path_graph_node;
  friend class path_graph;
  friend class path;
  friend class PathsDialog;

  // used to help searching, higher = more likely choice
  rational_t bias;

  vector3d direction;

public:
  enum
  {
    _EDGE_NONE          = 0x0000,
    _EDGE_NO_PATROL     = 0x0001,

    // patrol ID's
    _EDGE_PATROL_ID_0   = 0x0100,
    _EDGE_PATROL_ID_1   = 0x0200,
    _EDGE_PATROL_ID_2   = 0x0400,
    _EDGE_PATROL_ID_3   = 0x0800,
    _EDGE_PATROL_ID_4   = 0x1000,
    _EDGE_PATROL_ID_5   = 0x2000,
    _EDGE_PATROL_ID_6   = 0x4000,
    _EDGE_PATROL_ID_7   = 0x8000,
    _EDGE_HAS_PATROL_ID = 0xFF00,

    _EDGE_MAX_FLAGS,
    _EDGE_MAX_PATROL_ID = 8
  };

  path_graph_edge();
  path_graph_edge(const path_graph_edge &b)  { copy( b ); }
  virtual ~path_graph_edge();

  path_graph_edge& operator=(const path_graph_edge &b) { copy( b ); return *this; }
  void copy(const path_graph_edge &b);
  void clear();

  bool is_flagged(unsigned short flag) const          { return (flags & flag) != 0; }
  void set_flag(unsigned short flag, bool on=true)    { if(on) flags |= flag; else flags &= ~flag; }

  void set_patrol(bool on=true)     { set_flag(_EDGE_NO_PATROL, !on); }
  void set_no_patrol(bool on=true)  { set_flag(_EDGE_NO_PATROL, on); }

  bool is_patrol() const            { return !is_flagged(_EDGE_NO_PATROL); }

  bool is_patrol_id(int id) const;
  void set_patrol_id(int id, bool set = true);
  bool has_patrol_id() const                    { return(is_flagged(_EDGE_HAS_PATROL_ID)); }
  unsigned short patrol_id_flags() const        { return(flags & _EDGE_HAS_PATROL_ID); }

  friend bool operator==(const path_graph_edge &lhs, const path_graph_edge &rhs );

  path_graph_node *get_node(int i) const    { assert(i >= 0 && i < 2); return nodes[i]; }
  path_graph_node *get_other_node(const path_graph_node *n) const { return((nodes[0] == n) ? nodes[1] : nodes[0]); }
  unsigned short get_flags() const    { return flags; }
  rational_t get_distance() const     { return distance; }
  rational_t get_weight() const       { return (distance * (weight_modifier + additional_weight_modifier)); }
  rational_t get_weight_modifier() const { return(weight_modifier); }
  void set_weight_modifier(const rational_t w)    { assert(w > 0.0f); weight_modifier = w; }
  rational_t get_additional_weight_modifier() const { return additional_weight_modifier; }
//  void set_additional_weight_modifier(rational_t w)    { additional_weight_modifier = w; }

  void render( camera* camera_link, color32 col, rational_t thickness = 0.05f);

  rational_t get_bias() const         { return bias; }
  void set_bias(rational_t b)         { bias = b; }

  vector3d get_direction(path_graph_node *node = NULL) const;
};

inline bool operator==(const path_graph_edge &lhs, const path_graph_edge &rhs )
{
  return(
    ((lhs.nodes[0] == rhs.nodes[0] && lhs.nodes[1] == rhs.nodes[1]) || (lhs.nodes[0] == rhs.nodes[1] && lhs.nodes[1] == rhs.nodes[0])) &&
    lhs.flags == rhs.flags &&
    lhs.distance == rhs.distance &&
    lhs.weight_modifier == rhs.weight_modifier
    );
}



class path
{
protected:
  vector<path_graph_node *> nodes;
  rational_t distance;
  rational_t weight;
  rational_t additional_weight_mod;
  int waypoint;

  friend class path_graph_edge;
  friend class path_graph_node;
  friend class path_graph;
  friend class PathsDialog;

public:
  path();
  path(const path &b)  { copy( b ); }
  virtual ~path();

  path& operator=(const path &b)  { copy( b ); return *this; }
  void copy(const path &b);
  void copy(const path *b);
  void clear();

  int get_num_waypoints() const { return nodes.size(); }
  void reset_waypoints()    { waypoint = 0; }
  int get_waypoint_index() const { return waypoint; }

//  void set_additional_weight_mod(rational_t mod = 0.5f) { additional_weight_mod = mod; }
  rational_t get_additional_weight_mod() const { return additional_weight_mod; }

  rational_t get_distance() const { return distance; }
  rational_t get_weight() const   { return weight; }

  vector3d get_way_point(int i) const { assert(i >= 0 && i < (int)nodes.size() && nodes[i] != NULL); return nodes[i]->pt; }

  // pop waypoint
  void pop_way_point();

  // just get current waypoint
  bool get_cur_way_point( vector3d* vec, region_node** dest_region ) const;

  // auto pops the waypoints when they are reached
  // must pass in a vector that contains the current position (this will get overwritten with way point!)
  // also, <is_crossing_region> will get the value TRUE if the current edge crosses from one region to another
//  bool get_next_way_point( rational_t radius, vector3d* vec, region_node** dest_region );
  bool get_next_way_point( const vector3d &cur_pos, const vector3d &last_pos, rational_t radius, vector3d* vec, region_node** dest_region, bool force_xz = false );

  void render( camera* camera_link, color32 col = color32_blue, rational_t thickness = 0.05f);

  const path_graph_node *get_node(int n) const
  {
    if(n >= 0 && n < (int)nodes.size())
      return(nodes[n]);
    return NULL;
  }
  const path_graph_node *get_start_node() const         { return get_node(0); }
  const path_graph_node *get_end_node() const           { return get_node(nodes.size()-1); }
  const path_graph_edge *get_edge(int n1, int n2) const { assert(abs(n1-n2) == 1); return get_edge(get_node(n1), get_node(n2)); }
  const path_graph_edge *get_edge(const path_graph_node *n1, const path_graph_node *n2) const;

  vector3d get_start() const  { if(!nodes.empty()) return nodes[0]->pt; else return ZEROVEC; }
  vector3d get_end() const    { if(!nodes.empty()) return nodes[nodes.size() - 1]->pt; else return ZEROVEC; }

  friend bool operator==(const path &lhs, const path &rhs );
};

inline bool operator==(const path &lhs, const path &rhs )
{
  bool retval = false;

  if(lhs.distance == rhs.distance && lhs.weight == rhs.weight && lhs.nodes.size() == rhs.nodes.size())
  {
    vector<path_graph_node *>::const_iterator i = lhs.nodes.begin();
    vector<path_graph_node *>::const_iterator j = rhs.nodes.begin();

    while(i != lhs.nodes.end())
    {
      if(*i != *j)
        break;

      ++i;
      ++j;
    }

    if(i == lhs.nodes.end() && j == rhs.nodes.end())
      retval = true;
  }

  return retval;
}




class path_graph
{
protected:
  stringx id;
  vector<path_graph_node*> nodes;
  vector<path_graph_edge*> edges;

  friend class path_graph_edge;
  friend class path_graph_node;
  friend class path;
  friend class world_dynamics_system;
  friend class PathsDialog;

  static path search_path;

  void unvisit_all();
  void unsort_all();
  bool find_path_recurse(path_graph_node *cur, path_graph_node *dest, path &cur_path, path *best_path, int &depth, int &call_count, unsigned short node_flags = 0, rational_t max_dist = -1.0f, bool retval = false);

  bool find_all_paths_recurse(path_graph_node *cur, path &cur_path, vector<path> &paths, int &depth, int &call_count, unsigned short node_flags, rational_t max_dist = -1.0f, bool retval = false);

#if !defined(BUILD_BOOTABLE)
  bool complexity_warning;
  bool no_find_warning;
  bool patrol_warning;
  bool warned;
#endif

public:
  path_graph();
  virtual ~path_graph();

  void set_id(const stringx &new_id) { id = new_id; }
  const stringx& get_id() const      { return id; }

  void render( camera* camera_link, color32 col = color32_green, rational_t thickness = 0.05f, char level = 2);

  bool node_in_graph(const vector3d &pt) const;
  bool node_in_graph(const path_graph_node *node) const;
  bool edge_in_graph(const int node1, const int node2) const;
  bool edge_in_graph(const path_graph_node *node1, const path_graph_node *node2) const;
  bool edge_in_graph(const vector3d &pt1, const vector3d &pt2) const;
  bool edge_in_graph(const path_graph_edge *edge) const;

  void add_node(const vector3d &pt, unsigned short flags = 0);
  void add_edge(const int node1, const int node2, unsigned short flags = 0, rational_t modifier = 1.0f);
  void add_edge(path_graph_node *node1, path_graph_node *node2, unsigned short flags = 0, rational_t modifier = 1.0f);

  void remove_node(const vector3d &pt);
  void remove_node(const path_graph_node *node);
  void remove_node(const int node)    { remove_node(get_node(node)); }
  void remove_edge(const int node1, const int node2);
  void remove_edge(const path_graph_node *node1, const path_graph_node *node2);
  void remove_edge(const path_graph_edge *edge);
  void remove_edge(const int edge)    { remove_edge(get_edge(edge)); }

  path_graph_node *get_node(int node) const;
  path_graph_node *get_node(const vector3d &pt) const;
  path_graph_node *get_nearest_node(const vector3d &pt, bool smart = false) const;
  path_graph_node *get_farthest_node(const vector3d &pt) const;
  path_graph_edge *get_edge(const path_graph_node *node1, const path_graph_node *node2) const;
  path_graph_edge *get_edge(int node1, int node2) const;
  path_graph_edge *get_edge(int edge) const;

  int get_node_id(path_graph_node *node) const;
  int get_edge_id(path_graph_edge *edge) const;

  int get_num_edges() const   { return edges.size(); }
  int get_num_nodes() const   { return nodes.size(); }

  inline const vector<path_graph_node *> &get_nodes() const { return(nodes); }

  void read_data(chunk_file &fs, stringx &label);

#if _ENABLE_WORLD_EDITOR
  void write_data(ofstream &out);
  void perpetuate_all_patrol_id(bool set = true);
  void perpetuate_patrol_id(path_graph_edge *edge, int id = -1, bool set = true);
  void perpetuate_patrol_id_recurse(path_graph_edge *edge, unsigned short id_flags, bool set = true);
#endif

  bool valid() const;
  void clear();

  // path finding
  bool find_path(const vector3d &start, const vector3d &dest, path *found_path = NULL, const unsigned short flags = 0);
  bool find_path(path_graph_node *start, path_graph_node *dest, path *found_path = NULL, const unsigned short flags = 0);

  int find_all_paths(const vector3d &start, vector<path> &found_paths, const unsigned short flags, bool sort = true);
  int find_all_paths(path_graph_node *start, vector<path> &found_paths, const unsigned short flags, bool sort = true);

  bool find_path_to_hiding_spot(const vector3d &start, path *found_path = NULL, bool avoid_hero_sight = true);
  bool find_path_to_hiding_spot(path_graph_node *start, path *found_path = NULL, bool avoid_hero_sight = true);

  bool get_next_patrol_point(const vector3d &last_pos, const vector3d &cur_pos, vector3d &next_point, int patrol_id = 0);
  bool get_next_patrol_point(path_graph_node *last, path_graph_node *curr, vector3d &next_point, int patrol_id = 0);
  bool get_nearest_patrol_point(const vector3d &cur_pos, vector3d &patrol_point, int patrol_id = 0);

  void use_path(path *path, rational_t additional_weight = 0.5f);
  void restore_path(path *path);
};

void path_graph_system_construct();
void path_graph_system_destruct();

#endif
