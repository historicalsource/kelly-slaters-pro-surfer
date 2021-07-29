// path.cpp
// Copyright (c) 2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

#include "global.h"

#include "path.h"
#include "beam.h"
#include "wds.h"
#include "terrain.h"
#include "collide.h"
#include "random.h"

#include "debug_render.h"

#define _PATH_SEARCH_DIST_MOD   3.0f
#define _EDGE_TO_NODE_RATIO     1.5f

vector<path_graph_edge*>* patrol_edges_workspace;


void path_graph_system_construct()
{
  patrol_edges_workspace = NEW vector<path_graph_edge*>;
}

void path_graph_system_destruct()
{
  delete patrol_edges_workspace;
}

static int compare_path_graph_edge_ptr_by_weight( const void* x1, const void* x2 )
{
  // lower comes first
  rational_t diff = (*(path_graph_edge **)x1)->get_weight() - (*(path_graph_edge **)x2)->get_weight();

  if ( diff > 0.0f )
    return 1;

  if ( diff < 0.0f )
    return -1;

  return 0;
}

static int compare_path_graph_edge_ptr_by_bias( const void* x1, const void* x2 )
{
  // reversed because higher should come first
  rational_t diff = (*(path_graph_edge **)x2)->get_bias() - (*(path_graph_edge **)x1)->get_bias();

  if ( diff > 0.0f )
    return 1;

  if ( diff < 0.0f )
    return -1;

  return 0;
}

static void sort_edges_by_weight(vector<path_graph_edge *> &edges)
{
  if(edges.size() > 1)
    qsort( &(*edges.begin()), edges.size(), sizeof(path_graph_edge *), compare_path_graph_edge_ptr_by_weight );
}

static void sort_edges_by_bias(vector<path_graph_edge *> &edges)
{
  if(edges.size() > 1)
    qsort( &(*edges.begin()), edges.size(), sizeof(path_graph_edge *), compare_path_graph_edge_ptr_by_bias );
}



path_graph_node::path_graph_node()
{
  clear();
}

path_graph_node::~path_graph_node()
{
  clear();
}

void path_graph_node::remove_edge(const path_graph_edge *edge)
{
  vector<path_graph_edge *>::iterator i = edges.begin();
  while(i != edges.end())
  {
    if(*i != NULL && *i == edge)
    {
      i = edges.erase(i);
      return;
    }

    ++i;
  }
}

void path_graph_node::copy(const path_graph_node &b)
{
  pt = b.pt;
  my_region = b.my_region;

  edges.resize(0);
  edges.reserve(b.edges.size());

  vector<path_graph_edge *>::const_iterator i = b.edges.begin();
  while(i != b.edges.end())
  {
    edges.push_back(*i);
    ++i;
  }

  sort_edges();
}

void path_graph_node::clear()
{
  pt = ZEROVEC;
  my_region = NULL;
  flags = 0;
  edges.resize(0);
}

void path_graph_node::add_edge(path_graph_edge *edge, bool sort)
{
  edges.push_back(edge);

  if(sort)
    sort_edges();
}

void path_graph_node::sort_edges_weight()
{
  if(!is_weight_sorted())
  {
    set_flag(_NODE_B_SORTED, false);
    set_flag(_NODE_W_SORTED, true);
    sort_edges_by_weight(edges);
  }
}

void path_graph_node::sort_edges_bias()
{
  if(!is_bias_sorted())
  {
    set_flag(_NODE_W_SORTED, false);
    set_flag(_NODE_B_SORTED, true);
    sort_edges_by_bias(edges);
  }
}




path_graph_edge::path_graph_edge()
{
  clear();
}

path_graph_edge::~path_graph_edge()
{
  clear();
}

void path_graph_edge::copy(const path_graph_edge &b)
{
  clear();

  nodes[0] = b.nodes[0];
  nodes[1] = b.nodes[1];

  flags = b.flags;
  distance = b.distance;
  weight_modifier = b.weight_modifier;
}

void path_graph_edge::clear()
{
  nodes[0] = nodes[1] = NULL;

  flags = 0;
  distance = 0.0f;
  weight_modifier = 1.0f;
  additional_weight_modifier = 0.0f;
  bias = 0.0f;
}

bool path_graph_edge::is_patrol_id(int id) const
{
  assert(id >= 0 && id < _EDGE_MAX_PATROL_ID);
  return(is_flagged(((_EDGE_PATROL_ID_0) << id)));
}

void path_graph_edge::set_patrol_id(int id, bool set)
{
  assert(id >= 0 && id < _EDGE_MAX_PATROL_ID);
  set_flag(((_EDGE_PATROL_ID_0) << id), set);
}

void path_graph_edge::render( camera* camera_link, color32 col, rational_t thickness)
{
  assert(nodes[0] != NULL && nodes[1] != NULL);

  render_beam(nodes[0]->pt, nodes[1]->pt, col, thickness);
}

vector3d path_graph_edge::get_direction(path_graph_node *node) const
{
  if(node == NULL || node == nodes[0])
    return(direction);
  else if(node == nodes[1])
    return(-direction);

  assert(0);
  return(ZEROVEC);
}








path::path()
{
  clear();
}

path::~path()
{
  clear();
}


void path::copy(const path &b)
{
  copy(&b);
}

void path::copy(const path *b)
{
  clear();

  distance = b->distance;
  weight = b->weight;

  nodes.reserve(b->nodes.size());

  vector<path_graph_node *>::const_iterator i = b->nodes.begin();
  while(i != b->nodes.end())
  {
    nodes.push_back((*i));

    ++i;
  }

  reset_waypoints();
}

void path::clear()
{
  nodes.resize(0);
  distance = 0.0f;
  weight = 0.0f;
  waypoint = 0;
  additional_weight_mod = 0.0f;
}

const path_graph_edge *path::get_edge(const path_graph_node *n1, const path_graph_node *n2) const
{
  assert(n1 != NULL && n2 != NULL && n1 != n2);

  vector<path_graph_edge *>::const_iterator i = n1->edges.begin();
  while(i != n1->edges.end())
  {
    if((*i)->get_other_node(n1) == n2)
      return *i;

    ++i;
  }

  return NULL;
}

void path::render( camera* camera_link, color32 col, rational_t thickness)
{
#ifndef BUILD_BOOTABLE
  vector<path_graph_node *>::iterator i = nodes.begin();
  while(i != nodes.end())
  {
    path_graph_node *n1 = (*i);
    ++i;

    if(n1 != NULL)
    {
      render_marker(camera_link,n1->pt, col, 0.25f);

      if(i != nodes.end())
      {
        path_graph_node *n2 = (*i);

        if(n2 != NULL)
          render_beam(n1->pt, n2->pt, col, thickness);
      }
    }
  }
#endif
}


void path::pop_way_point()
{
//  if(nodes.size() > 0)
//    nodes.erase(nodes.begin());
  waypoint++;
}


bool path::get_cur_way_point( vector3d* vec, region_node** dest_region ) const
{
  *dest_region = NULL;
  if ( !nodes.empty() && waypoint<(int)nodes.size() )
  {
    assert( waypoint<(int)nodes.size() && nodes[waypoint] );
    *vec = nodes[waypoint]->pt;
    *dest_region = nodes[waypoint]->my_region;
    return true;
  }
  return false;
}


// determine next waypoint for pathing; return destination position in <vec>;
// return destination region in <dest_region>
/*
bool path::get_next_way_point( rational_t radius, vector3d* vec, region_node** dest_region )
{
  *dest_region = NULL;
  if ( nodes.size()>0 && waypoint<nodes.size() )
  {
    assert( waypoint<nodes.size() && nodes[waypoint] );

    while ( waypoint < nodes.size()
         && (*vec - nodes[waypoint]->pt).length2() <= radius*radius )
    {
      pop_way_point();
    }

    return get_cur_way_point( vec, dest_region );
  }
  return false;
}
*/
bool crossed_point(const vector3d test_point, const vector3d &cur_pos, const vector3d &last_pos, rational_t radius, bool force_xz);
bool crossed_point(const vector3d test_point, const vector3d &cur_pos, const vector3d &last_pos, rational_t radius, bool force_xz)
{
    vector3d delta = ( test_point - cur_pos );
    vector3d test_delta = ( test_point - last_pos );
    vector3d move = (cur_pos - last_pos);
    rational_t test_len = 0.0f;

    if ( force_xz || __fabs(delta.y) < 1.0f )
    {
      delta.y = 0.0f;
      test_delta.y = 0.0f;
      move.y = 0.0f;
    }

    return(delta.length2() <= radius*radius || (test_len = test_delta.length() - radius) <= 0.0f || move.length2() >= test_len*test_len );
}

bool path::get_next_way_point( const vector3d &cur_pos, const vector3d &last_pos, rational_t radius, vector3d* vec, region_node** dest_region, bool force_xz )
{
  *dest_region = NULL;
  if ( !nodes.empty() && waypoint<(int)nodes.size() )
  {
    assert( waypoint<(int)nodes.size() && nodes[waypoint] );

    if(crossed_point(nodes[waypoint]->pt, cur_pos, last_pos, radius, force_xz))
      pop_way_point();

    return get_cur_way_point( vec, dest_region );
  }

  return false;
}











path path_graph::search_path;

path_graph::path_graph()
{
#if !defined(BUILD_BOOTABLE)
  complexity_warning = false;
  no_find_warning = false;
  patrol_warning = false;
  warned = false;
#endif
}

path_graph::~path_graph()
{
  clear();
}

void path_graph::render( camera* camera_link, color32 col, rational_t thickness, char level)
{
#ifndef BUILD_BOOTABLE
  color32 node_col = col;

  int a = (int)(col.c.a * 1.5f);
  if(a > 255)
    a = 255;

  node_col.c.a = a;

  vector<path_graph_node *>::iterator i = nodes.begin();
  int node_index = 0;
  while(i != nodes.end())
  {
    if((*i) != NULL)
    {
      render_marker(camera_link,(*i)->pt, node_col, 0.25f);
      if(level > 1)
        print_3d_text((*i)->pt, col, "Node %d", node_index);
    }

    ++i;
    ++node_index;
  }

  vector<path_graph_edge *>::iterator j = edges.begin();
  while(j != edges.end())
  {
    if(*j != NULL)
    {
      if((*j)->is_patrol())
        (*j)->render(camera_link,col, thickness);
      else
        (*j)->render(camera_link,color32(col.c.r, col.c.g, 255, col.c.a), thickness);

      vector3d pt1 = (*j)->nodes[0]->pt;
      vector3d pt2 = (*j)->nodes[1]->pt;

      vector3d delta = (pt2 - pt1) * 0.5f;

      if(level > 1)
      {
        stringx patrols = empty_string;

        if((*j)->is_patrol())
        {
          for(int p=0; p<path_graph_edge::_EDGE_MAX_PATROL_ID; ++p)
          {
            if((*j)->is_patrol_id(p))
            {
              if(patrols.length() > 0)
                patrols += ",";
              else
                patrols += "Patrol (";

              patrols += itos(p);
            }
          }

          if(patrols.length() > 0)
            patrols += ")";
        }

        if((*j)->get_weight() != (*j)->distance)
          print_3d_text(pt1 + delta, col, "%.2f (%.2f)%s%s", (*j)->get_weight(), (*j)->distance, patrols.length() > 0 ? "\n" : "", patrols.c_str());
        else
          print_3d_text(pt1 + delta, col, "%.2f%s%s", (*j)->get_weight(), patrols.length() > 0 ? "\n" : "", patrols.c_str());
      }
    }

    ++j;
  }
#endif // !BUILD_BOOTABLE
}





bool path_graph::node_in_graph(const vector3d &pt) const
{
  return get_node(pt) != NULL;
}

bool path_graph::node_in_graph(const path_graph_node *node) const
{
  vector<path_graph_node *>::const_iterator i = nodes.begin();
  while(i != nodes.end())
  {
    if(*i != NULL && (*i) == node)
      return(true);

    ++i;
  }

  return(false);
}

bool path_graph::edge_in_graph(const int node1, const int node2) const
{
  assert(node1 >= 0 && node2 >= 0 && node1 < (int)nodes.size() && node2 < (int)nodes.size());

  return(get_edge(node1, node2) != NULL);
}

bool path_graph::edge_in_graph(const path_graph_node *node1, const path_graph_node *node2) const
{
  return(get_edge(node1, node2) != NULL);
}

bool path_graph::edge_in_graph(const vector3d &pt1, const vector3d &pt2) const
{
  return(get_edge(get_node(pt1), get_node(pt2)) != NULL);
}

bool path_graph::edge_in_graph(const path_graph_edge *edge) const
{
  vector<path_graph_edge *>::const_iterator j = edges.begin();
  while(j != edges.end())
  {
    path_graph_edge *edge = (*j);

    if((*j) != NULL && (*j) == edge)
      return(true);

    ++j;
  }

  return false;
}



void path_graph::add_node(const vector3d &pt, unsigned short flags)
{
  if ( !node_in_graph(pt) )
  {
    path_graph_node *node = NEW path_graph_node();
    node->pt = pt;
    sector* sec = g_world_ptr->get_the_terrain().find_sector( pt );
    if ( sec )
      node->my_region = sec->get_region();
    else
      warning( id + ": a node has been placed outside of the world" );
    node->flags = flags;
    nodes.push_back(node);
  }
}

void path_graph::add_edge(const int node1, const int node2, unsigned short flags, rational_t modifier)
{
  assert(node1 >= 0 && node2 >= 0 && node1 < (int)nodes.size() && node2 < (int)nodes.size());

  add_edge(nodes[node1], nodes[node2], flags, modifier);
}

void path_graph::add_edge(path_graph_node *node1, path_graph_node *node2, unsigned short flags, rational_t modifier)
{
  assert(node1 != NULL && node2 != NULL);

  if(!edge_in_graph(node1, node2))
  {
    path_graph_edge *edge = NEW path_graph_edge();

    edge->nodes[0] = node1;
    edge->nodes[1] = node2;
    edge->direction = (edge->nodes[1]->pt - edge->nodes[0]->pt);
    edge->distance = edge->direction.length();

    assert(edge->distance > 0.0f);

    edge->direction *= (1.0f / edge->distance);

    edge->flags = flags;
    edge->weight_modifier = modifier;

    edge->nodes[0]->add_edge(edge, true);
    edge->nodes[1]->add_edge(edge, true);

    edges.push_back(edge);
  }
}


void path_graph::remove_node(const vector3d &pt)
{
  vector<path_graph_node *>::iterator i = nodes.begin();
  while(i != nodes.end())
  {
    if(*i != NULL && (*i)->pt == pt)
    {
      remove_node((*i));
      return;
    }

    ++i;
  }
}

void path_graph::remove_node(const path_graph_node *node)
{
  if(node == NULL)
    return;

  vector<path_graph_edge *>::iterator j = edges.begin();
  while(j != edges.end())
  {
    path_graph_edge *edge = (*j);

    if(edge != NULL && (edge->nodes[0] == node || edge->nodes[1] == node))
    {
      if(edge->nodes[0] != NULL)
        edge->nodes[0]->remove_edge(edge);

      if(edge->nodes[1] != NULL)
        edge->nodes[1]->remove_edge(edge);

      delete edge;

      j = edges.erase(j);
    }
    else
      ++j;
  }

  vector<path_graph_node *>::iterator i = nodes.begin();
  while(i != nodes.end())
  {
    if(*i != NULL && (*i) == node)
    {
      path_graph_node *n = (*i);
      nodes.erase(i);
      delete n;

      return;
    }

    ++i;
  }
}



void path_graph::remove_edge(const int node1, const int node2)
{
  assert(node1 >= 0 && node2 >= 0 && node1 < (int)nodes.size() && node2 < (int)nodes.size());

  path_graph_node *n0 = nodes[node1];
  path_graph_node *n1 = nodes[node2];

  remove_edge(n0, n1);
}

void path_graph::remove_edge(const path_graph_node *node1, const path_graph_node *node2)
{
  vector<path_graph_edge *>::iterator i = edges.begin();
  while(i != edges.end())
  {
    path_graph_edge *edge = *i;
    if(edge != NULL && (edge->nodes[0] == node1 && edge->nodes[1] == node2) || (edge->nodes[0] == node2 && edge->nodes[1] == node1))
    {
      edge->nodes[0]->remove_edge(edge);
      edge->nodes[1]->remove_edge(edge);

      i = edges.erase(i);
      delete edge;

      return;
    }

    ++i;
  }
}

void path_graph::remove_edge(const path_graph_edge *edge)
{
  if(edge == NULL)
    return;

  vector<path_graph_edge *>::iterator i = edges.begin();
  while(i != edges.end())
  {
    path_graph_edge *edgeX = *i;
    if(edgeX != NULL && edgeX == edge)
    {
      edgeX->nodes[0]->remove_edge(edgeX);
      edgeX->nodes[1]->remove_edge(edgeX);

      i = edges.erase(i);
      delete edgeX;

      return;
    }

    ++i;
  }
}




path_graph_node *path_graph::get_node(const vector3d &pt) const
{
  vector<path_graph_node *>::const_iterator i = nodes.begin();
  while(i != nodes.end())
  {
    if(*i != NULL && (*i)->pt == pt)
      return((*i));

    ++i;
  }

  return NULL;
}

path_graph_node *path_graph::get_node(const int node) const
{
  if(node >= 0 && node < (int)nodes.size())
    return nodes[node];
  return NULL;
}
/*
inline static rational_t distance_point_to_line(const vector3d &line_start, const vector3d &line_dir, const vector3d &point)
{
  return(cross((point - line_start), line_dir).length());
}

inline static rational_t distance_point_to_line2(const vector3d &line_start, const vector3d &line_dir, const vector3d &point)
{
  return(cross((point - line_start), line_dir).length2());
}

inline static bool same_direction(const vector3d &dir1, const vector3d &dir2)
{
  return(dir1.x*dir2.x >= 0.0f && dir1.y*dir2.y >= 0.0f && dir1.z*dir2.z >= 0.0f);
}
*/

path_graph_node *path_graph::get_nearest_node(const vector3d &pt, bool smart) const
{
  path_graph_node *nearest_node = NULL;

  if(!nodes.empty())
  {
    if(smart)
    {
      static vector<path_graph_edge *> edge_list;
      edge_list.reserve(edges.size());
      edge_list.resize(0);

      vector3d hit, hit_norm;

      vector<path_graph_edge *>::const_iterator i = edges.begin();
      vector<path_graph_edge *>::const_iterator i_end = edges.end();
      while(i != i_end)
      {
        path_graph_edge *edge = (*i);
        ++i;

        if(edge != NULL)
        {
          rational_t y0 = edge->nodes[0]->pt.y - pt.y;
          rational_t y1 = edge->nodes[1]->pt.y - pt.y;
          rational_t y_mod = __fabs(y0) + __fabs(y1);

          edge->bias = -dist_point_segment(pt, edge->nodes[0]->pt, edge->nodes[1]->pt, hit) * ((y_mod >= 1.0f) ? y_mod : 1.0f);

          edge_list.push_back(edge);
        }
      }

      sort_edges_by_bias(edge_list);

      i = edge_list.begin();
      i_end = edge_list.end();
      while(i != i_end)
      {
        path_graph_edge *edge = (*i);
        ++i;

        rational_t len2_0 = (pt - edge->nodes[0]->pt).length2();
        rational_t len2_1 = (pt - edge->nodes[1]->pt).length2();

        if ( i == edge_list.begin() )
          nearest_node = (len2_0 < len2_1) ? edge->nodes[0] : edge->nodes[1];

        if ( !find_intersection( pt, (len2_0 < len2_1) ? edge->nodes[0]->pt : edge->nodes[1]->pt,
                                 NULL,
                                 FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                                 &hit, &hit_norm ) )
        {
          return (len2_0 < len2_1) ? edge->nodes[0] : edge->nodes[1];
        }
        else if ( !find_intersection( pt, (len2_0 < len2_1) ? edge->nodes[1]->pt : edge->nodes[0]->pt,
                                      NULL,
                                      FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                                      &hit, &hit_norm ) )
        {
          return (len2_0 < len2_1) ? edge->nodes[1] : edge->nodes[0];
        }
      }
    }

    if(!smart || nearest_node == NULL)
    {
      vector<path_graph_node *>::const_iterator i = nodes.begin();
      vector<path_graph_node *>::const_iterator i_end = nodes.end();

      assert((*i) != NULL);

      nearest_node = (*i);

      ++i;

      rational_t min_d2 = (pt - nearest_node->pt).length2();
      while(i != i_end)
      {
        if(*i != NULL)
        {
          rational_t d2 = (pt - (*i)->pt).length2();
          if(d2 < min_d2)
          {
            nearest_node = (*i);
            min_d2 = d2;
          }
        }

        ++i;
      }
    }
  }

  return nearest_node;
}

path_graph_node *path_graph::get_farthest_node(const vector3d &pt) const
{
  path_graph_node *farthest_node = NULL;

  if(!nodes.empty())
  {
    vector<path_graph_node *>::const_iterator i = nodes.begin();

    assert((*i) != NULL);

    farthest_node = (*i);

    ++i;

    rational_t max_d2 = (pt - farthest_node->pt).length2();
    while(i != nodes.end())
    {
      if(*i != NULL)
      {
        rational_t d2 = (pt - (*i)->pt).length2();
        if(d2 > max_d2)
        {
          farthest_node = (*i);
          max_d2 = d2;
        }
      }

      ++i;
    }
  }

  return farthest_node;
}




path_graph_edge *path_graph::get_edge(const path_graph_node *node1, const path_graph_node *node2) const
{
  if(node1 != NULL && node2 != NULL)
  {
    vector<path_graph_edge *>::const_iterator i = edges.begin();
    while(i != edges.end())
    {
      if(*i != NULL)
      {
        path_graph_edge *edge = *i;
        if((edge->nodes[0] == node1 && edge->nodes[1] == node2) || (edge->nodes[0] == node2 && edge->nodes[1] == node1))
          return(edge);
      }

      ++i;
    }
  }

  return(NULL);
}

path_graph_edge *path_graph::get_edge(const int node1, const int node2) const
{
  assert(node1 >= 0 && node2 >= 0 && node1 < (int)nodes.size() && node2 < (int)nodes.size());

  path_graph_node *n0 = nodes[node1];
  path_graph_node *n1 = nodes[node2];

  return(get_edge(n0, n1));
}

path_graph_edge *path_graph::get_edge(const int edge) const
{
  assert(edge >= 0 && edge < (int)edges.size());

  return(edges[edge]);
}




int path_graph::get_node_id(path_graph_node *node) const
{
  vector<path_graph_node *>::const_iterator i = nodes.begin();
  int id = 0;
  while(i != nodes.end())
  {
    if(*i == node)
      return(id);

    ++id;
    ++i;
  }

  return(-1);
}

int path_graph::get_edge_id(path_graph_edge *edge) const
{
  vector<path_graph_edge *>::const_iterator i = edges.begin();
  int id = 0;
  while(i != edges.end())
  {
    if(*i == edge)
      return(id);

    ++id;
    ++i;
  }

  return(-1);
}

void path_graph::read_data(chunk_file &fs, stringx &label)
{
  set_id(label);
  id.to_upper();

  vector3d pt;
  int node1, node2;
  unsigned short flags;
  rational_t modifier;

  for( serial_in(fs, &label); label != chunkend_label; serial_in(fs, &label) )
  {
//    label.to_lower();

    if(label == "node" || label == "NODE")
    {
      serial_in(fs, &pt);
      serial_in(fs, &flags);

      add_node(pt, flags);
    }
    else if(label == "edge" || label == "EDGE")
    {
      serial_in(fs, &node1);
      serial_in(fs, &node2);
      serial_in(fs, &modifier);
      serial_in(fs, &flags);

      add_edge(node1, node2, flags, modifier);
    }
    else
      error("Bad keyword '%s' in path '%s' section!", label.c_str(), id.c_str());
  }

#if !defined(BUILD_BOOTABLE)
  if(!valid())
    warning("%s: Not a valid path graph!", id.c_str());

//  if(((rational_t)edges.size()) > (((rational_t)nodes.size())*_EDGE_TO_NODE_RATIO))
//    warning("%s: Node to edge ratio exceeded (nodes: %d, edges: %d, max edges: %d)!", id.c_str(), nodes.size(), edges.size(), (int)((((rational_t)nodes.size())*_EDGE_TO_NODE_RATIO) + 0.5f));

  vector<path_graph_edge *> bad_patrol_edges;
  bad_patrol_edges.reserve(edges.size());
  bad_patrol_edges.resize(0);

  vector<path_graph_edge *>::const_iterator i = edges.begin();
  while(i != edges.end())
  {
    if(*i != NULL)
    {
      path_graph_edge *edge = *i;

      if(edge->is_patrol() && !edge->has_patrol_id())
        bad_patrol_edges.push_back(edge);
    }

    ++i;
  }

  if(!bad_patrol_edges.empty())
  {
    stringx error = id + ": The following patrol edges do not contain patrol ID's:\n\n";

    int counter = 0;
    vector<path_graph_edge *>::const_iterator i = bad_patrol_edges.begin();
    while(i != bad_patrol_edges.end())
    {
      if(*i != NULL)
      {
        path_graph_edge *edge = *i;

        error += "( " + itos(get_node_id(edge->get_node(0))) + " -> " + itos(get_node_id(edge->get_node(1))) + " ),   ";
        counter++;

        if(counter >= 5)
        {
          error += "\n";
          counter = 0;
        }
      }

      ++i;
    }

    warning(error);
  }

#endif
}

#if _ENABLE_WORLD_EDITOR

void path_graph::write_data(ofstream &out)
{
  if(out.is_open())
  {
    unvisit_all();
    unsort_all();

    if(!valid())
      warning(stringx("Path '") + id + "' is not valid!");

    out<<id.c_str()<<"\n";

    vector<path_graph_node *>::iterator i = nodes.begin();
    while(i != nodes.end())
    {
      path_graph_node *node = *i;
      ++i;

      if(node != NULL)
        out<<"  node "<<node->pt.x<<" "<<node->pt.y<<" "<<node->pt.z<<" "<<node->flags<<"\n";
    }

    vector<path_graph_edge *>::iterator j = edges.begin();
    while(j != edges.end())
    {
      path_graph_edge *edge = *j;
      ++j;

      if(edge != NULL)
        out<<"  edge "<<get_node_id(edge->nodes[0])<<" "<<get_node_id(edge->nodes[1])<<" "<<edge->weight_modifier<<" "<<edge->flags<<"\n";
    }

    out<<chunkend_label.c_str()<<"\n";
  }
}

void path_graph::perpetuate_all_patrol_id(bool set)
{
  unvisit_all();

  vector<path_graph_edge *>::iterator i = edges.begin();
  while(i != edges.end())
  {
    path_graph_edge *edge = *i;
    ++i;

    if(edge != NULL && edge->is_patrol())
      perpetuate_patrol_id_recurse(edge, set ? edge->patrol_id_flags() : path_graph_edge::_EDGE_HAS_PATROL_ID, set);
  }

  unvisit_all();
}

void path_graph::perpetuate_patrol_id(path_graph_edge *edge, int id, bool set)
{
  assert(id == -1 || (id >= 0 && id < path_graph_edge::_EDGE_MAX_PATROL_ID));

  unvisit_all();

  if(edge != NULL && edge->is_patrol())
    perpetuate_patrol_id_recurse(edge, id == -1 ? edge->patrol_id_flags() : (path_graph_edge::_EDGE_PATROL_ID_0) << id, set);

  unvisit_all();
}

void path_graph::perpetuate_patrol_id_recurse(path_graph_edge *edge, unsigned short id_flags, bool set)
{
  if(edge != NULL && edge->is_patrol())
  {
    edge->set_flag(id_flags, set);

    for(int i=0; i<2; ++i)
    {
      path_graph_node *node = edge->get_node(i);
      if(!node->visited())
      {
        node->visit();

        vector<path_graph_edge *>::iterator e = node->edges.begin();
        while(e != node->edges.end())
        {
          if((*e) != NULL && (*e)->is_patrol())
            perpetuate_patrol_id_recurse((*e), id_flags, set);

          ++e;
        }
      }
    }
  }
}

#endif




bool path_graph::valid() const
{
  vector<path_graph_edge *>::const_iterator i = edges.begin();
  while(i != edges.end())
  {
    if(*i != NULL)
    {
      path_graph_edge *edge = *i;

      if(edge->nodes[0] == NULL || edge->nodes[1] == NULL || edge->nodes[0] == edge->nodes[1] || !node_in_graph(edge->nodes[0]) || !node_in_graph(edge->nodes[1]))
        return(false);
    }

    ++i;
  }

  vector<path_graph_node *>::const_iterator j = nodes.begin();
  while(j != nodes.end())
  {
    if(*j != NULL)
    {
      vector<path_graph_edge *>::const_iterator i2 = (*j)->edges.begin();
      while(i2 != (*j)->edges.end())
      {
        if(*i2 != NULL)
        {
          if(!edge_in_graph((*i2)))
            return(false);
        }

        ++i2;
      }
    }

    ++j;
  }

  return true;
}

void path_graph::clear()
{
  vector<path_graph_edge *>::iterator i = edges.begin();
  while(i != edges.end())
  {
    if(*i != NULL)
    {
      path_graph_edge *edge = *i;
      i = edges.erase(i);

      delete edge;
    }
    else
      ++i;
  }

  vector<path_graph_node *>::iterator j = nodes.begin();
  while(j != nodes.end())
  {
    if(*j != NULL)
    {
      path_graph_node *node = *j;
      j = nodes.erase(j);

      delete node;
    }
    else
      ++j;
  }
}

void path_graph::unvisit_all()
{
  vector<path_graph_node *>::iterator j = nodes.begin();
  while(j != nodes.end())
  {
    if(*j != NULL)
      (*j)->set_flag(path_graph_node::_NODE_VISITED, false);

    ++j;
  }
}

void path_graph::unsort_all()
{
  vector<path_graph_node *>::iterator j = nodes.begin();
  while(j != nodes.end())
  {
    if(*j != NULL)
      (*j)->clear_sorting();

    ++j;
  }
}

// this function will need to be optimized more. I have a few ideas for this, to be implemented at a later date... (JDB 7/26/00)
bool path_graph::find_path_recurse(path_graph_node *cur, path_graph_node *dest, path &cur_path, path *best_path, int &depth, int &call_count, const unsigned short flags, rational_t max_dist, bool retval)
{
  assert(cur);

  #if !defined(BUILD_BOOTABLE)
    ++call_count;
    ++depth;

    int power = nodes.size();
    power = power*power*power*power;

    if(!warned && call_count > power)
    {
      if(!complexity_warning)
      {
        warning("%s::find_path_recurse: call_count (call_count %d, depth %d) exceeds nodes^4 (%d, %d)\n\nTry decreasing complexity of connected edges in graph", id.c_str(), call_count, depth, nodes.size(), power);
        complexity_warning = true;
      }

      warned = true;
    }

    if(warned)
      return(false);
  #endif

  cur_path.nodes.push_back(cur);
  cur->visit();

  if((dest != NULL ? (cur == dest) : cur->is_flagged(flags)))
  {
    if(best_path)
    {
      if(best_path->nodes.empty() || cur_path.weight < best_path->weight)
        best_path->copy(cur_path);
    }

    retval = true;
  }
  else
  {
    if(dest != NULL)
    {
      if(!cur->is_bias_sorted())
      {
        vector3d delta = (dest->pt - cur->pt);
        delta.normalize();

        vector<path_graph_edge *>::iterator i = cur->edges.begin();
        vector<path_graph_edge *>::iterator i_end = cur->edges.end();
        while(i != i_end)
        {
          path_graph_edge *edge = (*i);
          ++i;

          if(edge != NULL)
          {
            if(edge->get_other_node(cur) == dest)
              edge->bias = 1000.0f;
            else
              edge->bias = dot(delta, edge->get_direction(cur));
          }
        }

        cur->sort_edges_bias();
      }
    }
    else if(!cur->is_weight_sorted())
      cur->sort_edges_weight();

    vector<path_graph_edge *>::iterator i = cur->edges.begin();
    vector<path_graph_edge *>::iterator i_end = cur->edges.end();
    while(i != i_end)
    {
      path_graph_edge *edge = (*i);
      ++i;

      if(edge != NULL && (max_dist < 0.0f || (cur_path.distance + edge->distance) < max_dist) && (best_path == NULL || best_path->nodes.empty() || (cur_path.weight + edge->get_weight()) < best_path->weight))
      {
        path_graph_node *node = edge->get_other_node(cur);

        if(node && !node->visited())
        {
          cur_path.weight += edge->get_weight();
          cur_path.distance += edge->get_distance();

          if((retval = find_path_recurse(node, dest, cur_path, best_path, depth, call_count, flags, max_dist, retval)) != false && best_path == NULL)
            return(retval);

          cur_path.weight -= edge->get_weight();
          cur_path.distance -= edge->get_distance();
        }
      }
    }
  }

  cur_path.nodes.pop_back();
  cur->unvisit();

  #if !defined(BUILD_BOOTABLE)
    --depth;
  #endif

  return(retval);
}


bool path_graph::find_all_paths_recurse(path_graph_node *cur, path &cur_path, vector<path> &best_paths, int &depth, int &call_count, const unsigned short flags, rational_t max_dist, bool retval)
{
  assert(cur);

  ++call_count;
  ++depth;

  cur_path.nodes.push_back(cur);

  cur->visit();

  if(cur->is_flagged(flags))
  {
    bool found = false;

    vector<path>::iterator i = best_paths.begin();
    while(i != best_paths.end())
    {
      if((*i).get_end_node() == cur)
      {
        if(cur_path.weight < (*i).weight)
          (*i).copy(cur_path);

        found = true;
        break;
      }

      ++i;
    }

    if(!found)
      best_paths.push_back(cur_path);

    retval = true;
  }
  else
  {
    vector<path_graph_edge *>::iterator i = cur->edges.begin();
    while(i != cur->edges.end())
    {
      path_graph_edge *edge = (*i);
      ++i;

      if(edge != NULL)
      {
        path_graph_node *node = edge->get_other_node(cur);

        if(node && !node->visited())
        {
          cur_path.weight += edge->get_weight();
          cur_path.distance += edge->get_distance();

          retval = find_all_paths_recurse(node, cur_path, best_paths, depth, call_count, flags, max_dist, retval);

          cur_path.weight -= edge->get_weight();
          cur_path.distance -= edge->get_distance();
        }
      }
    }
  }


  --depth;

  cur_path.nodes.pop_back();

  cur->unvisit();

  return(retval);
}


bool path_graph::find_path(const vector3d &start, const vector3d &dest, path *found_path, const unsigned short flags)
{
  return(find_path(get_nearest_node(start, true), flags == 0 ? get_nearest_node(dest, true) : NULL, found_path, flags));
}

bool path_graph::find_path(path_graph_node *start, path_graph_node *dest, path *found_path, const unsigned short flags)
{
  if(start != NULL && (dest != NULL || flags != 0))
  {
    if(found_path)
      found_path->clear();

    // this may not be necessary, but here to be safe...
    unvisit_all();
    unsort_all();

    int depth = 0;
    int call_count = 0;

    search_path.clear();
    search_path.nodes.reserve(nodes.size());

    // find the path, walk the path, don't stray from the path, um...........
    // 'There is a difference between knowing the path and walking the path' (Morpheus)
    // 'Fear is the path that leads to the darkside' (Yoda)
    #if !defined(BUILD_BOOTABLE)
      warned = false;
    #endif

    // estimate a max distance
    rational_t max_dist = -1.0f;
    if(found_path != NULL && dest != NULL)
    {
      vector3d delta = start->pt - dest->pt;

      if(__fabs(delta.y) >= 1.5f)
      {
        max_dist = -1.0f;
      }
      else
      {
        delta.y *= 10.0f;

        max_dist = delta.length() * _PATH_SEARCH_DIST_MOD;
      }
    }

//    bool use_depth = true; // unused -- remove me?
    bool found = false;
    if((found = find_path_recurse(start, dest, search_path, found_path, depth, call_count, flags, max_dist, false)) == false && max_dist > 0.0f)
    {
    #if !defined(BUILD_BOOTABLE)
/*
      if(!no_find_warning)
      {
        warning("%s::find_path: Failure to find path on first try, attempting second try without distance estimate\n\nSolutions (in preferable order):\n1. Connect nodes %d and %d in the graph (if possible)\n2. Construct graph with a more direct route between previous nodes from solution #1\n3. Increase estimate size in code, currently distance*%.2f", id.c_str(), get_node_id(start), get_node_id(dest), _PATH_SEARCH_DIST_MOD);
        no_find_warning = true;
      }
*/
    #endif

      return(find_path_recurse(start, dest, search_path, found_path, depth, call_count, flags, -1.0f, false));
    }
    else
      return(found);
  }
  else
    return(false);
}


int path_graph::find_all_paths(const vector3d &start, vector<path> &found_paths, const unsigned short flags, bool sort)
{
  return(find_all_paths(get_nearest_node(start, true), found_paths, flags, sort));
}


static int compare_path( const void* x1, const void* x2 )
{
  rational_t diff = ((path *)x1)->get_weight() - ((path *)x2)->get_weight();

  if ( diff > 0.0f )
    return 1;

  if ( diff < 0.0f )
    return -1;

  return 0;
}

int path_graph::find_all_paths(path_graph_node *start, vector<path> &found_paths, const unsigned short flags, bool sort)
{
  found_paths.resize(0);

  if(start != NULL && flags != 0)
  {
    // this may not be necessary, but here to be safe...
    unvisit_all();
    unsort_all();

    int depth = 0;
    int call_count = 0;

    search_path.clear();
    search_path.nodes.reserve(nodes.size());

    // find the path, walk the path, don't stray from the path, um...........
    // 'There is a difference between knowing the path and walking the path' (Morpheus)
    // 'Fear is the path that leads to the darkside' (Yoda)
    #if !defined(BUILD_BOOTABLE)
      warned = false;
    #endif

    find_all_paths_recurse(start, search_path, found_paths, depth, call_count, flags, -1.0f, false);
  }

  if(sort && found_paths.size() > 1)
    qsort( &(*found_paths.begin()), found_paths.size(), sizeof(path), compare_path );

  return(found_paths.size());
}

bool path_graph::find_path_to_hiding_spot(const vector3d &start, path *found_path, bool avoid_hero_sight)
{
  return(find_path_to_hiding_spot(get_nearest_node(start, true), found_path, avoid_hero_sight));
}

bool path_graph::find_path_to_hiding_spot(path_graph_node *start, path *found_path, bool avoid_hero_sight)
{
  // will want this later to find all possible paths to a hiding spot, and see which one is closest and cannot be seen by hero, if specified)
  return(find_path(start, NULL, found_path, path_graph_node::_NODE_HIDE));
}

bool path_graph::get_next_patrol_point(const vector3d &last_pos, const vector3d &cur_pos, vector3d &next_point, int patrol_id)
{
  return(get_next_patrol_point(get_nearest_node(last_pos), get_nearest_node(cur_pos), next_point, patrol_id));
}

bool path_graph::get_next_patrol_point(path_graph_node *last, path_graph_node *curr, vector3d &next_point, int patrol_id)
{
  if(curr)
  {
    path_graph_edge *last_edge = NULL;

    patrol_edges_workspace->resize(0);
    patrol_edges_workspace->reserve(curr->edges.size());

    vector<path_graph_edge *>::iterator i = curr->edges.begin();
    while(i != curr->edges.end())
    {
      path_graph_edge *edge = *i;
      ++i;

      if(edge && edge->is_patrol() && (patrol_id == -1 || edge->is_patrol_id(patrol_id)))
      {
        if(edge->get_other_node(curr) != NULL)
        {
          if(edge->get_other_node(curr) != last)
            patrol_edges_workspace->push_back(edge);
          else if(last != NULL)
            last_edge = edge;
        }
      }
    }

    if(!patrol_edges_workspace->empty())
    {
      path_graph_edge *edge = (*patrol_edges_workspace)[random((int)patrol_edges_workspace->size())];
      next_point = edge->get_other_node(curr)->pt;

      return(true);
    }
    else if(last && last_edge && last_edge->is_patrol() && (patrol_id == -1 || last_edge->is_patrol_id(patrol_id)))
    {
      next_point = last->pt;

      return(true);
    }
    else
    {
      next_point = curr->pt;

      return(false);
    }
  }

  return(false);
}

bool path_graph::get_nearest_patrol_point(const vector3d &cur_pos, vector3d &patrol_point, int patrol_id)
{
  rational_t mind = -1.0f;
  path_graph_node *min_node = NULL;

  vector<path_graph_edge *>::iterator i = edges.begin();
  while(i != edges.end())
  {
    path_graph_edge *edge = *i;
    ++i;

    if(edge && edge->is_patrol() && (patrol_id == -1 || edge->is_patrol_id(patrol_id)))
    {
      for(int j=0; j<2; j++)
      {
        rational_t len = (cur_pos - edge->nodes[j]->pt).length2();
        if(mind < 0.0f || len <= mind)
        {
          mind = len;
          min_node = edge->nodes[j];
        }
      }
    }
  }

  if(min_node != NULL)
  {
    patrol_point = min_node->pt;
    return(true);
  }
  else
    return(false);
}

void path_graph::use_path(path *path, rational_t additional_weight)
{
  assert(path);

  path->additional_weight_mod = additional_weight;

  vector<path_graph_node *>::iterator i = path->nodes.begin();
  while(i != path->nodes.end())
  {
    path_graph_node *n1 = (*i);
    ++i;

    if(i != path->nodes.end())
    {
      path_graph_node *n2 = (*i);

      path_graph_edge *edge = get_edge(n1, n2);
      assert(edge);
      edge->additional_weight_modifier += path->additional_weight_mod;
    }
  }
}

void path_graph::restore_path(path *path)
{
  assert(path);

  vector<path_graph_node *>::iterator i = path->nodes.begin();
  while(i != path->nodes.end())
  {
    path_graph_node *n1 = (*i);
    ++i;

    if(i != path->nodes.end())
    {
      path_graph_node *n2 = (*i);

      path_graph_edge *edge = get_edge(n1, n2);
      assert(edge);
      edge->additional_weight_modifier -= path->additional_weight_mod;
    }
  }
}
