// $Id: main.cpp,v 1.7 2016-02-11 03:45:06-08 - - $
// Ana Carolina Alves - adalves

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <cerrno>
#include <cstring>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

const string cin_name = "-";

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            traceflags::setflags (optarg);
            break;
         default:
            complain() << "-" << (char) optopt << ": invalid option"
                       << endl;
            break;
      }
   }
} 

string trim (const string& line) {
   size_t first = line.find_first_not_of (" \t");
   if (first == string::npos) return "";
   size_t last = line.find_last_not_of (" \t");
   return line.substr (first, last - first + 1);
}

void parse_commands (str_str_map& map, string& line) {
   size_t equals_pos = line.find_first_of ("=");

   // Line is a comment or blank - ignore
   if (line.size() == 0 || line[0] == '#'){
      return;
   } 

   // Line is "key" - find the key in the map and print the pair
   else if (equals_pos == string::npos) {
      string key = line;
      str_str_map::iterator itor = map.find (key);
      if (itor == map.end())
         cout << key << ": key not found" << endl;
      else
         cout << itor->first << " = " << itor->second << endl;
   } 

   // Line is just "=" - print all pairs
   else if (line.size() == 1) {
      str_str_map::iterator itor = map.begin();
      for (; itor != map.end(); ++itor) {
         cout << itor->first << " = " << itor->second << endl;
      }
   } 

   // Line is "key =" - delete the pair from the map
   else if (equals_pos == line.size() - 1) {
      string key = line.substr (0, equals_pos);
      key = trim (key);
      str_str_map::iterator itor = map.find (key);
      if (itor == map.end()) 
         cout << key << ": key not found" << endl;
      else 
         map.erase (itor);
   } 

   // Line is "= value" - print all pairs that have this value
   else if (equals_pos == 0) {
      string value = line.substr (1, string::npos);
      value = trim (value);
      bool key_found = false;
      str_str_map::iterator itor = map.begin();
      for (; itor != map.end(); ++itor) {
         if (itor->second == value) {
            cout << itor->first << " = " << itor->second << endl; 
            key_found = true;
         }
      }
      if (!key_found)
         cout << value << ": value not found" << endl;
   } 

   // Line is "key = value" - insert pair in the map
   else {
      string key = line.substr (0, equals_pos);
      string value = line.substr (equals_pos + 1, string::npos);
      key = trim (key);
      value = trim (value);
      str_str_pair pair(key, value);
      map.insert (pair);
      cout << pair.first << " = " << pair.second << endl;
   }
}

void read_file (str_str_map& map, istream& infile, 
                const string& filename) {
   string line;
   size_t linenr = 0;

   for (;;) {
      getline (infile, line);

      if (infile.eof()) break;
      line = trim (line);
      cout << filename << ": " << ++linenr << ": " << line << endl;

      parse_commands (map, line);
   }
}

int main (int argc, char** argv) {
   sys_info::set_execname (argv[0]);
   scan_options (argc, argv);
   str_str_map map;

   if (optind == argc) read_file (map, cin, cin_name);
   for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
      string filename = *argp;
      if (filename == cin_name) read_file (map, cin, filename);
      else {
         ifstream infile (filename);

         if (infile.fail()) {
            complain() << sys_info::get_execname() << ": " << filename 
                 << ": " << strerror (errno) << endl;
         } else {
            read_file (map, infile, filename);
            infile.close();
         }
      }
   }

   return sys_info::get_exit_status();
}

