// $Id: file_sys.h,v 1.29 2016-01-31 23:31:54-08 - - $
// Ana Carolina Alves - adalves

#ifndef __INODE_H__
#define __INODE_H__

#include <exception>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <memory>
#include <map>
#include <vector>
using namespace std;

#include "util.h"

// inode_t -
//    An inode is either a directory or a plain file.

enum class file_type {PLAIN_TYPE, DIRECTORY_TYPE};
class inode;
class base_file;
class plain_file;
class directory;
using inode_ptr = shared_ptr<inode>;
using base_file_ptr = shared_ptr<base_file>;
ostream& operator<< (ostream&, file_type);

// inode_state -
//    A small convenient class to maintain the state of the simulated
//    process:  the root (/), the current directory (.), and the
//    prompt.
// getters and setters -
//    for prompt, contents (base_file_ptr in inode), root and cwd

class inode_state {
   friend class inode;
   friend ostream& operator<< (ostream& out, const inode_state&);
   private:
      inode_state (const inode_state&) = delete; // copy ctor
      inode_state& operator= (const inode_state&) = delete; // op=
      inode_ptr root {nullptr};
      inode_ptr cwd {nullptr};
      string prompt_ {"% "};
   public:
      inode_state();
      const string& get_prompt();
      void set_prompt (const string&);
      base_file_ptr get_content (inode_ptr);
      const inode_ptr get_root();
      const inode_ptr get_cwd();
      void set_cwd (inode_ptr);
      // Helper functions
      inode_ptr find_inode_ptr (const string&, inode_ptr&);
      wordvec pathname_to_wordvec (const string&);
      inode_ptr wordvec_to_inode_ptr (const wordvec&);
      inode_ptr pathname_to_inode_ptr (const string&);
      string inode_ptr_to_pathname (const inode_ptr&);
      vector<inode_ptr> get_subdirectories 
                           (inode_ptr&, vector<inode_ptr>&);
};

// class inode -
// inode ctor -
//    Create a new inode of the given type, 
//    and stores the number and type.
// get_inode_nr -
//    Retrieves the serial number of the inode.  Inode numbers are
//    allocated in sequence by small integer.
// size -
//    Returns the size of an inode.  For a directory, this is the
//    number of dirents.  For a text file, the number of characters
//    when printed (the sum of the lengths of each word, plus the
//    number of words.
// getters and setters -
//    for number, contents, name, size and type

class inode {
   friend class inode_state;
   private:
      static int next_inode_nr;
      int inode_nr;
      file_type type;
      string name;
      base_file_ptr contents;
   public:
      inode (file_type);
      int get_inode_nr() const;
      base_file_ptr get_content();
      void set_name (const string&);
      const string& get_name () const;
      int get_size() const;
      file_type get_type() const;
};

// class base_file -
// Just a base class at which an inode can point.  No data or
// functions.  Makes the synthesized members useable only from
// the derived classes.

class file_error: public runtime_error {
   public:
      explicit file_error (const string& what);
};

class base_file {
   protected:
      base_file() = default;
      base_file (const base_file&) = delete;
      base_file (base_file&&) = delete;
      base_file& operator= (const base_file&) = delete;
      base_file& operator= (base_file&&) = delete;
   public:
      virtual ~base_file() = default;
      virtual size_t size() const = 0;
      virtual const wordvec& readfile() const = 0;
      virtual void writefile (const wordvec& newdata) = 0;
      virtual void remove (inode_ptr& ptr, string& name) = 0;
      virtual wordvec get_dir_content() = 0;
      virtual inode_ptr mkdir (const string& dirname) = 0;
      virtual inode_ptr mkfile (const string& filename) = 0;
      virtual void make_root (inode_ptr root_ptr) = 0;
      virtual map<string,inode_ptr> get_dirents() const = 0;
      virtual void empty(inode_ptr&) = 0;
      virtual void insert_dirents 
                   (const inode_ptr&, const inode_ptr&) = 0;
};

// class plain_file -
// Used to hold data.
// If errors are not commented here, they're checked in commands.cpp.
// synthesized default ctor -
//    Default vector<string> is a an empty vector.
// size - 
//    Return the size of the file
//    (Quantity of characters in each string + vector.size() - 1;
// readfile -
//    Returns a copy of the contents of the wordvec in the file.
// writefile -
//    Replaces the contents of a file with new contents.

class plain_file: public base_file {
   private:
      wordvec data;
   public:
      virtual size_t size() const override;
      virtual const wordvec& readfile() const override;
      virtual void writefile (const wordvec& newdata) override;
      virtual void remove (inode_ptr& ptr, string& name) override;
      virtual wordvec get_dir_content() override;
      virtual inode_ptr mkdir (const string& dirname) override;
      virtual inode_ptr mkfile (const string& filename) override;
      virtual void make_root (inode_ptr root_ptr) override;
      virtual map<string,inode_ptr> get_dirents() const override;
      virtual void empty(inode_ptr&) override;
      virtual void insert_dirents 
                   (const inode_ptr&, const inode_ptr&) override;
};

// class directory -
// Used to map filenames onto inode pointers.
// If errors are not commented here, they're checked in commands.cpp.
// dirents -
//    Map that contains all pointers to all files and directories
//    inside this directory.
// size - 
//    Returns the size of the directory
//    (quantity of files/directories inside).
// remove -
//    Calls the empty() method to remove file/directory.
//    If called to remove a directory, sets up the pointer to the 
//    parent directory so that empty() will erase everything inside 
//    the directory and the directory itself.
// get_dir_content - 
//    Returns a wordvec with the contents of a directory 
//    (inode number, size, name - in this order).
// mkdir -
//    Creates a new directory under the current directory and 
//    immediately calls insert_dirents() to add the directories 
//    dot (.) and dotdot (..) to it. Error if directory already exists.
// mkfile -
//    Create a new empty text file with the given name.
// make_root -
//    Sets up the root directory.
// insert_dirents - 
//    Sets up the "." and ".." in a new directory.
// get_dirents - 
//    Getter method for the dirents map.
// empty - 
//    Completely removes a file or directory (as well as
//    everything inside).

class directory: public base_file {
   private:
      // Must be a map, not unordered_map, so printing is lexicographic
      map<string,inode_ptr> dirents;
   public:
      virtual size_t size() const override;
      virtual const wordvec& readfile() const override;
      virtual void writefile (const wordvec& newdata) override;
      virtual void remove (inode_ptr& ptr, string& name) override;
      virtual wordvec get_dir_content() override;
      virtual inode_ptr mkdir (const string& dirname) override;
      virtual inode_ptr mkfile (const string& filename) override;
      virtual void make_root (inode_ptr root_ptr) override;
      virtual void insert_dirents 
                   (const inode_ptr&, const inode_ptr&) override;
      virtual map<string,inode_ptr> get_dirents() const override;
      virtual void empty(inode_ptr&) override;
};

#endif

