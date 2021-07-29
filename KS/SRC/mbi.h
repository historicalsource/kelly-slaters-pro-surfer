#ifndef MBI_H
#define MBI_H

// just a reminder, because I find it fairly counterintuitive:
//   blur is afterimages of the sword
//   trail is the colored trail

// Motion blur stuff

struct qt {
    quaternion   q;
    vector3d     t;
};


class motion_blur_info
  {
  public:
    motion_blur_info( int max_trail_length );
    ~motion_blur_info();
    int motion_trail_start;
    int motion_trail_end;
    int motion_trail_count;
    int motion_trail_length;
    int buffer_size;
    qt* motion_trail_buffer;
    vector3d motion_trail_head;
    vector3d motion_trail_tail;

    // appearance parameters
    color32 trail_color;
    int trail_min_alpha;
    int trail_max_alpha;
    int blur_min_alpha;
    int blur_max_alpha;
    int num_blur_images;
    float blur_spread;

    time_value_t last_motion_recording;
  };

#endif