// $Id: commands.cpp,v 1.22 2016-01-31 22:09:08-08 - - $
// Ana Carolina Alves - adalves

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"#"     , fn_hash  },
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": No such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_hash(inode_state&, const wordvec&) { }

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if (words.size() == 1) 
      throw command_error ("cat: No file specified");
   else {
      inode_ptr ptr;
      auto itor = ++words.cbegin();
      string pathname;
      wordvec file;
   
      for (; itor != words.end(); ++itor){
         pathname = *itor;
         ptr = state.pathname_to_inode_ptr (pathname);

         if (ptr != nullptr) {
            if (ptr -> get_type() ==  file_type::DIRECTORY_TYPE)
               throw command_error ("cat: " + pathname 
                                    + ": Is a directory");
            else file = state.get_content (ptr) -> readfile();
         } else throw command_error ("cat: " + pathname 
                                     + ": No such file");

         cout << word_range (file.begin(), file.end()) << endl;
      }
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() == 1) {
      state.set_cwd(state.get_root());
   } else if (words.size() > 2) {
      throw command_error ("cd: More than one operand given");
   } else {
      inode_ptr ptr = state.pathname_to_inode_ptr (words.at(1));
      if (ptr != nullptr) {
         if (ptr -> get_type() ==  file_type::PLAIN_TYPE)
            throw command_error ("cd: " + words.at(1)
                                 + ": Is a file");
         else state.set_cwd(ptr);
      } else throw command_error ("cd: " + words.at(1)
                                  + ": No such directory");
   }
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}

void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   int exit_status = 0;

   inode_ptr ptr = state.get_root();
   state.set_cwd (ptr);
   state.get_content (ptr) -> empty(ptr);

   if (words.size() > 1) {
      try {
         exit_status = stoi (words.at(1));
      } catch (...) {
         exit_status = 127;
      }
   }

   exit_status::set (exit_status);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   inode_ptr ptr = state.get_cwd();

   if (words.size() > 1) {
      auto itor = ++words.cbegin();
      string word;

      for (; itor != words.cend(); ++itor) {
         word = *itor;
         ptr = state.pathname_to_inode_ptr (word);
         if (ptr == nullptr) {
            throw command_error ("ls: " + word 
                                 + ": No such file or directory");
            break;
         }
         if (ptr -> get_type() == file_type::PLAIN_TYPE)
            print_file_ls (state, word);
         else
            print_dir_ls (state, ptr);
      }
   } else print_dir_ls (state, ptr);
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   inode_ptr ptr = state.get_cwd();
   vector<inode_ptr> all_directories;
   all_directories.push_back (ptr);
   
   if (words.size() > 1) {
      auto itor = ++words.cbegin();
      string word;

      for (; itor != words.cend(); ++itor) {
         word = *itor;
         ptr = state.pathname_to_inode_ptr (word);
         if (ptr == nullptr) {
            throw command_error ("lsr: " + word 
                                 + ": No such file or directory");
            break;
         }
         if (ptr -> get_type() == file_type::PLAIN_TYPE)
            print_file_ls (state, word);
         else
            all_directories = 
               state.get_subdirectories (ptr, all_directories);
      }
      ptr = state.pathname_to_inode_ptr (words.at(1));
   } else {
      all_directories = state.get_subdirectories (ptr, all_directories);
      ptr = state.get_cwd();
   }


   //print_dir_ls (state, ptr);
   for (inode_ptr temp_ptr: all_directories) {
      print_dir_ls (state, temp_ptr);
   }
}

void print_file_ls (inode_state& state, string& pathname) {   
   wordvec path = state.pathname_to_wordvec (pathname);
   string file = path.at (path.size() - 1);

   cout << file << endl;
}

void print_dir_ls (inode_state& state, inode_ptr& ptr) {
   wordvec content = state.get_content (ptr) -> get_dir_content();
   
   cout << state.inode_ptr_to_pathname (ptr) << ":" << endl;

   for (size_t i = 0; i < content.size() - 2; i += 3) {
      cout << setw(6) << content[i] << setw(6) << content.at (i + 1) 
           << "  " << content.at (i + 2) << endl;
   }
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() <= 1) {
      throw command_error ("make: No filename specified");
   } else {
      wordvec path = state.pathname_to_wordvec (words.at(1));
      inode_ptr ptr = state.wordvec_to_inode_ptr (path);

      if (ptr != nullptr
          && ptr -> get_type() == file_type::DIRECTORY_TYPE) {
         throw command_error ("make: " + words.at(1) 
                              + ": Cannot specify a directory");
         return;
      }

      string name;
      if (path.size() > 1) {
         name = path.at (path.size() - 1);
         path.pop_back();
         ptr = state.wordvec_to_inode_ptr (path);

         if (ptr == nullptr) {
            throw command_error ("make: " + words.at(1) 
                                 + ": Pathname does not exist");
            return;
         }
      } else {
         name = path.at(0);
         ptr = state.get_cwd();
      }

      ptr = state.get_content(ptr) -> mkfile(name);

      if (words.size() > 2) {
         state.get_content(ptr) -> writefile(words);
      }

   }
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if (words.size() == 1) 
      throw command_error ("mkdir: No directory name specified");
   else {
      wordvec path = state.pathname_to_wordvec (words.at(1));
      inode_ptr ptr = state.get_cwd();
      string name;
      if (path.size() > 1) {
         name = path.at (path.size() - 1);
         path.pop_back();
         ptr = state.wordvec_to_inode_ptr (path);
         if (ptr == nullptr) {
            throw command_error ("mkdir: " + words.at(1) 
                                 + ": Pathname does not exist");
            return;
         }
      } else name = path.at(0);
   
      state.get_content (ptr) -> mkdir (name);
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string prompt = "";
   auto itor = ++words.cbegin();

   if (words.size() == 1) prompt = "% ";
   else {
      for (; itor != words.cend(); ++itor){
         prompt += *itor + " ";
      }
   }
   
   state.set_prompt (prompt);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   cout << state.inode_ptr_to_pathname (state.get_cwd()) << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() == 1) 
      throw command_error ("rm: No file or directory specified");
   else {
      auto itor = ++words.cbegin();
      string word;
   
      for (; itor != words.end(); ++itor){
         word = *itor;
         rm_r (state, word, false);
      }
   }
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() == 1) 
      throw command_error ("rmr: No file or directory specified");
   else {
      auto itor = ++words.cbegin();
      string word;
   
      for (; itor != words.end(); ++itor){
         word = *itor;
         rm_r (state, word, true);
      }
   }
}

void rm_r (inode_state& state, const string& pathname, bool recursive){
   wordvec path = state.pathname_to_wordvec (pathname);
   inode_ptr ptr = state.wordvec_to_inode_ptr (path);
   string name = "";

   if (ptr == state.get_root()) {
      throw command_error ("rmr: Cannot remove root");
      return;
   }

   if (ptr != nullptr) {
      if (ptr -> get_type() == file_type::DIRECTORY_TYPE 
          && ptr -> get_size() != 2 && !recursive) {
         throw command_error ("rm: " + pathname 
                               + ": Directory not empty");
         return;
      } else {
         if (ptr -> get_type() == file_type::DIRECTORY_TYPE)
            state.get_content (ptr) -> remove(ptr, name);
         name = path.at (path.size() - 1);
         path.pop_back();
         if (!path.empty())
            ptr = state.wordvec_to_inode_ptr (path);
         else ptr = state.get_cwd();
         state.get_content (ptr) -> remove(ptr, name);
      }
   } else throw command_error ((recursive ? "rmr: " : "rm: ") 
                               + pathname 
                               + ": No such file of directory");
}

void terminate_program (inode_state& state) {
   wordvec words;
   fn_exit (state, words);
}

