#ifndef FILE_FINDER_H
#define FILE_FINDER_H



class file_finder
{
private:
  // path list to search when finding files
  vector<stringx> path_list;

public:
  file_finder();
  virtual ~file_finder();

  stringx find_file( const stringx &filename, const stringx &extension, bool fail_ok = false ) const;
  bool exists( const stringx &filename, const stringx &extension, stringx* found_name=NULL );

  void push_path_front( const stringx &path ) { path_list.insert( path_list.begin(), path ); }
//  void pop_path_front() { path_list.pop_front(); }
  void push_path_back( const stringx &path ) { path_list.push_back( path ); }
  void pop_path_back() { path_list.pop_back(); }
  void init_path_list();
  void clear_path_list() { path_list.resize(0); }
};

bool file_finder_exists(const stringx &filename, const stringx &extension, stringx* found_name=NULL);


extern file_finder *g_file_finder;


#endif // FILE_FINDER_H