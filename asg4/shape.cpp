// $Id: shape.cpp,v 1.5 2016-03-01 00:13:42-08 - - $
// Ana Carolina Alves - adalves

#include <cmath>
#include <typeinfo>
#include <unordered_map>
using namespace std;

#include "shape.h"
#include "util.h"

unordered_map<void*,string> font::fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

unordered_map<string,void*> font::fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font, const string& textdata):
      glut_bitmap_font(glut_bitmap_font), textdata(textdata) {
   DEBUGF ('c', this);
}

ellipse::ellipse (const GLfloat width, const GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (const GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}

polygon::polygon (const vertex_list& vertices): vertices (vertices) {
   DEBUGF ('c', this);
}

rectangle::rectangle (const GLfloat width, const GLfloat height):
            polygon ({{0, 0}, {width, 0}, 
                     {width, height}, {0, height}}) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (const GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

diamond::diamond (const GLfloat width, const GLfloat height): 
            polygon ({{0, 0}, {width/2, height/2},
                     {0, height}, {-width/2, height/2}}) {
   DEBUGF ('c', this);
}

triangle::triangle (const vertex_list& vertices): polygon (vertices) {
   DEBUGF ('c', this);
}

equilateral::equilateral (const GLfloat width): 
            triangle ({{0, 0}, {width, 0}, {width/2, width}}) {
   DEBUGF ('c', this);
}

void text::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   glEnd();
   glColor3ubv(color.ubvec);
   int width = glutBitmapLength (glut_bitmap_font, 
                                 reinterpret_cast<const unsigned char*>
                                 (textdata.c_str()));
   int height = glutBitmapHeight (glut_bitmap_font);
   glRasterPos2f(center.xpos - width / 2.0, center.ypos - height / 4.0);
   glutBitmapString(glut_bitmap_font, 
      reinterpret_cast<const GLubyte*> (textdata.c_str())); 
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   glEnable (GL_LINE_SMOOTH);
   glColor3ubv (color.ubvec);
   const float delta = 2 * M_PI / 32;
   for (float theta = 0; theta < 2 * M_PI; theta += delta) {
      float xpos = dimension.xpos/2 * cos (theta) + center.xpos;
      float ypos = dimension.ypos/2 * sin (theta) + center.ypos;
      glVertex2f (xpos, ypos);
   }
   glEnd();
}

void polygon::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   GLfloat x_sum;
   GLfloat y_sum;

   for (vertex v: vertices) {
      x_sum += v.xpos;
      y_sum += v.ypos;
   }

   GLfloat x_avg = x_sum / vertices.size();
   GLfloat y_avg = y_sum / vertices.size();

   glEnable (GL_LINE_SMOOTH);
   glColor3ubv (color.ubvec);

   for (vertex v: vertices) {
      GLfloat x = center.xpos + v.xpos - x_avg;
      GLfloat y = center.ypos + v.ypos - y_avg;
      glVertex2f (x, y);
   }
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << font::fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

