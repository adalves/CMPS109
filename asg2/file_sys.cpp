// $Id: file_sys.cpp,v 1.41 2016-01-31 23:43:09-08 - - $
// Ana Carolina Alves - adalves

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "commands.h"
#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   get_content (root) -> make_root(root);
   cwd = root;
   DEBUGF ('i', "root = " << root -> get_name() << ", cwd = " << cwd
          << ", prompt = \"" << get_prompt() << "\"");
}

const string& inode_state::get_prompt() { return prompt_; }

void inode_state::set_prompt (const string& new_prompt) {
   prompt_ = new_prompt;
}

base_file_ptr inode_state::get_content (inode_ptr ptr) {
   return ptr -> get_content(); 
}

const inode_ptr inode_state::get_root() { return root; }

const inode_ptr inode_state::get_cwd() {return cwd; }

void inode_state::set_cwd (inode_ptr ptr) { cwd = ptr; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode_ptr inode_state::find_inode_ptr (const string& name, 
                                            inode_ptr& curr) {
   map<string,inode_ptr> dirents = get_content (curr) -> get_dirents();
   inode_ptr ptr = nullptr;

   if (dirents.find (name) != dirents.end()) {
      ptr = dirents.find (name) -> second;
   }

   return ptr;
}

wordvec inode_state::pathname_to_wordvec (const string& pathname) {
   wordvec path;
   string delimiter = "/";
   size_t begin = 0, end = 0;

   if (pathname.at(0) == '/') {
      ++begin;
   }
   for (;;) {
      if (begin == pathname.size()) break;
      end = pathname.find_first_of(delimiter, begin);
      if (end == string::npos) {
         end = pathname.size();
         path.push_back (pathname.substr (begin, end - begin));
         DEBUGF ('i', "path " << path);
         break;
      }
      path.push_back (pathname.substr (begin, end - begin));
      DEBUGF ('i', "path " << end);
      begin = end + 1;
   }

   return path;
}

inode_ptr inode_state::wordvec_to_inode_ptr (const wordvec& pathname) {
   inode_ptr ptr = cwd;

   if (pathname.empty()) {
      return root;
   }

   for (string path: pathname) {
      ptr = find_inode_ptr (path, ptr);
      if (ptr == nullptr) break;
   }

   return ptr;
}

inode_ptr inode_state::pathname_to_inode_ptr (const string& pathname) {
   return wordvec_to_inode_ptr (pathname_to_wordvec (pathname));
}

string inode_state::inode_ptr_to_pathname (const inode_ptr& ptr) {
   inode_ptr temp = ptr;
   wordvec path;
   string pathname = "";

   if (ptr != root) {
      while (temp != root) {
         path.push_back (temp -> get_name());
         temp = find_inode_ptr ("..", temp);
      }

      auto itor = path.crbegin();

      for (; itor != path.crend(); ++itor) {
         pathname += "/" + *itor;
      }
   } else pathname = "/";

   return pathname;
}

vector<inode_ptr> inode_state::get_subdirectories 
                  (inode_ptr& ptr, vector<inode_ptr>& all_directories) {
   map<string,inode_ptr> dirents = get_content (ptr) -> get_dirents();
   auto dirents_itor = dirents.begin();
   string name;

   for (; dirents_itor != dirents.end(); ++dirents_itor) {
      ptr = dirents_itor -> second;
      name = dirents_itor -> first;
      if (ptr -> get_type() ==  
          file_type::DIRECTORY_TYPE 
          && name != ".."
          && name != "."
          && find (all_directories.begin(), all_directories.end(), ptr)
                  == all_directories.end()) {
         all_directories.push_back(ptr);
         get_subdirectories (ptr, all_directories);
         DEBUGF ('i', "all_directories " << name);
      }
   }

   return all_directories;
}

inode::inode(file_type type): inode_nr (next_inode_nr++), type (type) {
   switch (type) {
      case file_type::PLAIN_TYPE:
         contents = make_shared<plain_file>();
         break;
      case file_type::DIRECTORY_TYPE:
         contents = make_shared<directory>();
         break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

base_file_ptr inode::get_content() { return contents; }

void inode::set_name (const string& new_name) {
   name = new_name;
}

const string& inode::get_name() const {
   return name;
}

int inode::get_size() const {
   return contents -> size();
}

file_type inode::get_type() const { return type; }

file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
   for (string word: data) {
      size += word.size();
   }
   if (size >= 1) size += data.size() - 1;
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data.clear();
   data.insert (data.end(), words.begin() + 2, words.end());
}

void plain_file::remove (inode_ptr&, string&) {
   throw file_error ("is a plain file");
}

wordvec plain_file::get_dir_content () {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

void plain_file::make_root (const inode_ptr) {
   throw file_error ("is a plain file");
}

map<string,inode_ptr> plain_file::get_dirents() const {
   throw file_error ("is a plain file");
}

void plain_file::empty(inode_ptr&) {
   throw file_error ("is a plain file");
}

void plain_file::insert_dirents (const inode_ptr&, const inode_ptr&) {
   throw file_error ("is a plain file");
}

size_t directory::size() const {
   size_t size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (inode_ptr& ptr, string& name) {
   if (!name.empty()) {
      dirents.erase (dirents.find (name));
   } else empty (ptr);
}

wordvec directory::get_dir_content () {
   inode_ptr ptr;
   wordvec content;
   string name;
   auto itor = dirents.cbegin();

   
   for (; itor != dirents.cend(); ++itor) {
      name = itor -> first;
      ptr = itor -> second;
      content.push_back (to_string (ptr -> get_inode_nr()));
      content.push_back (to_string (ptr -> get_size()));

      if (name == ".." || name == ".") {
         content.push_back (itor -> first);
      } else {
         if (ptr -> get_type() ==  file_type::DIRECTORY_TYPE) {
            content.push_back (ptr -> get_name() + "/");
         }
         else
            content.push_back (ptr -> get_name());
      }
   }

   return content;
}

inode_ptr directory::mkdir (const string& pathname) {
   DEBUGF ('i', pathname);

   inode_ptr ptr;
   if (dirents.find (pathname) == dirents.end()) {
      ptr = make_shared<inode>(file_type::DIRECTORY_TYPE);
      inode_ptr parent = dirents.at (".");
      inode_ptr child = ptr;
      base_file_ptr dir_ptr = ptr -> get_content();
      dir_ptr -> insert_dirents (parent, child);
      dirents.insert (make_pair (pathname, ptr));
      ptr -> set_name (pathname);
      return ptr;
   } else {
      throw command_error ("mkdir: " + pathname
                           + ": File or directory already exists");
   }

   return ptr;
}

inode_ptr directory::mkfile (const string& pathname) {
   DEBUGF ('i', pathname);

   inode_ptr ptr;
   if (dirents.find (pathname) == dirents.end()) {
      ptr = make_shared<inode>(file_type::PLAIN_TYPE);
      ptr -> set_name (pathname);
      dirents.insert (make_pair (pathname, ptr));
   } else {
      ptr = dirents.find (pathname) -> second;
   }

   return ptr;
}

void directory::make_root (const inode_ptr root_ptr) {
   dirents.insert (make_pair (".", root_ptr));
   dirents.insert (make_pair ("..", root_ptr));
   root_ptr -> set_name("/");
}

void directory::insert_dirents 
     (const inode_ptr& parent, const inode_ptr& child) {
   dirents.insert (make_pair (".", child));
   dirents.insert (make_pair ("..", parent));
}

map<string,inode_ptr> directory::get_dirents() const {
   return dirents;
}

void directory::empty (inode_ptr& ptr) {
   auto dirents_itor = dirents.begin();
   base_file_ptr dir_ptr = ptr -> get_content();
   string name;

   for (; dirents_itor != dirents.end(); ++dirents_itor) {
      name = dirents_itor -> first;
      ptr = dirents_itor -> second;
      dir_ptr = ptr -> get_content();
      if (name != ".." && name != ".") {
         if (ptr -> get_type() ==  file_type::DIRECTORY_TYPE) {
            dir_ptr -> empty(ptr);
         }
      }
   }

   dirents.clear();
}

