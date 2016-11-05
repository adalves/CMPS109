// $Id: graphics.h,v 1.5 2016-02-29 15:10:05-08 - - $
// Ana Carolina Alves - adalves

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <memory>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "rgbcolor.h"
#include "shape.h"

class object {
   private:
      shared_ptr<shape> pshape;
      vertex center;
      rgbcolor color;
   public:
      // Default copiers, movers, dtor all OK.
      object();
      object (const shared_ptr<shape>&, vertex&, rgbcolor&);
      void draw() { pshape->draw (center, color); }
      void move (GLfloat, GLfloat);
      shared_ptr<shape> get_shape () const { return pshape; }
      vertex get_center() const { return center; }
      void set_shape (shared_ptr<shape> pshape_) { pshape = pshape_; }
      void set_center (vertex center_) { center = center_; }
};

class mouse {
      friend class window;
   private:
      int xpos {0};
      int ypos {0};
      int entered {GLUT_LEFT};
      int left_state {GLUT_UP};
      int middle_state {GLUT_UP};
      int right_state {GLUT_UP};
   private:
      void set (int x, int y) { xpos = x; ypos = y; }
      void state (int button, int state);
      void draw();
};

class window {
      friend class mouse;
   private:
      static int width;         // in pixels
      static int height;        // in pixels
      static vector<object> objects;
      static vector<object> numbers;
      static object border;
      static size_t selected_obj;
      static mouse mus;
      static rgbcolor border_color;
      static bool is_border_c_set;
      static int border_thickness;
      static int moveby;
   private:
      static void move_selected_object (GLfloat, GLfloat);
      static void select_object (size_t);
      static void close();
      static void entry (int mouse_entered);
      static void display();
      static void reshape (int width, int height);
      static void keyboard (GLubyte key, int, int);
      static void special (int key, int, int);
      static void motion (int x, int y);
      static void passivemotion (int x, int y);
      static void mousefn (int button, int state, int x, int y);
   public:
      static void push_back (const object&);
      static void setwidth (int width_) { width = width_; }
      static int get_width() { return width; }
      static void setheight (int height_) { height = height_; }
      static int get_height() { return height; }
      static void main();
      static void set_border_color (rgbcolor& color) 
                  { border_color = color; }
      static void set_border_thickness (int thickness) 
                  { border_thickness = thickness; }
      static void set_moveby (int moveby_) { moveby = moveby_; }
};

#endif

