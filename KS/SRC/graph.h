// graph.h
#ifndef _GRAPH_H
#define _GRAPH_H


#include <list>
#include <map>
#include <set>


// TEMPLATE CLASS graph
//
// Defines a graph ADT that takes a type for node data and a type for edge data.
// These data types are distinct from the structure of the graph itself, making
// this ADT a container class for the two types of contained objects (node and
// edge).
//
// Note that this implementation uses a map to contain the node list, so that
// inserted nodes are unique under a comparison of their key data.

template
  <
  class _NodeKey,
  class _NodeData,
  class _EdgeData,
  class _KeyComp = less<_NodeKey>
  >
  class graph
  {
  // Types
  public:
    class node;
    class edge
      {
      private:
        _EdgeData data;
        node* dest;
      public:
        edge() : data(),dest(NULL) {}
      protected:
        edge(const _EdgeData& _data,node* _dest) : data(_data),dest(_dest) {}
      public:
        _EdgeData& get_data() { return data; }
        node* get_dest() { return dest; }
      friend class graph;
      };

    class node : public list<edge>
      {
      public:
        typedef list<edge> _myL;
      private:
        _NodeData data;
      public:
        node() : _myL(),data() {}
        node(const _NodeData& _data) : _myL(),data(_data) {}
        bool operator<(const node& b) const { return data<b.data; }
        _NodeData& get_data() { return data; }
      friend class graph;
      };
    typedef map<_NodeKey,node,_KeyComp> node_list;

    typedef node_list::iterator iterator;
    typedef pair<iterator,bool> _Pairib;

  // Data
  private:
    node_list nodes;

  // Constructors
  public:
    graph() : nodes() {}
    graph(const graph& b) : nodes(b.nodes) {}

  // Methods
  public:
    iterator begin() { return nodes.begin(); }
    iterator end()   { return nodes.end(); }

    // attempt to insert a node containing the given data
    _Pairib insert_node(const _NodeKey& k,const _NodeData& nd)
      {
      return nodes.insert(node_list::value_type(k,nd));
      }

    // find a node using the given key
    iterator find(const _NodeKey& k)
      {
      return nodes.find(k);
      }

    // insert an edge containing the given data between the given nodes
    void insert_edge(const _EdgeData& ed,node& a,node& b)
      {
      a.push_back(edge(ed,&b));
      b.push_back(edge(ed,&a));
      }
  };


#endif  // _GRAPH_H
