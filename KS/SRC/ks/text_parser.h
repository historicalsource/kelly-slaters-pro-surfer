#ifndef _TEXT_PARSER_H_
#define _TEXT_PARSER_H_

#define MAXTOKEN 65

class text_parser
{
public:
  text_parser ();
  ~text_parser ();

  void cleanup ();
  bool load_file (stringx& filename);

  bool get_token (bool crossline, bool optional);

  bool get_float (float *f, bool crossline, bool optional)
  {
    if (get_token (crossline, optional))
      return sscanf (token, "%f", f) == 1;
    else
      return false;
  }

  bool get_vector3d (vector3d *vec, bool crossline, bool optional)
  {
    if (!get_float (&vec->x, crossline, optional))
      return false;

    if (!get_float (&vec->y, crossline, optional))
      return false;

    if (!get_float (&vec->z, crossline, optional))
      return false;

    return true;
  }

  char token[MAXTOKEN];

private:
  char *buffer;
  char *script_ptr;
  char last_val;
  char *last_val_pos;
  int script_line;
};

#endif // _TEXT_PARSER_H_