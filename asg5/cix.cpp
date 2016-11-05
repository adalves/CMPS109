// $Id$
// Ana Carolina Alves - adalves

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", CIX_EXIT},
   {"help", CIX_HELP},
   {"ls"  , CIX_LS  },
   {"get" , CIX_GET },
   {"put" , CIX_PUT },
   {"rm"  , CIX_RM  },
};

void cix_help() {
   static vector<string> help = {
      "exit         - Exit the program.  Equivalent to EOF.",
      "get filename - Copy remote file to local host.",
      "help         - Print help summary.",
      "ls           - List names of files on remote server.",
      "put filename - Copy local file to remote host.",
      "rm filename  - Remove file from remote server.",
   };
   for (const auto& line: help) cout << line << endl;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = CIX_LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != CIX_LSOUT) {
      log << "sent CIX_LS, server did not return CIX_LSOUT" << endl;
      log << "server returned " << header << endl;
   }else {
      char buffer[header.nbytes + 1];
      recv_packet (server, buffer, header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer;
   }
}

void cix_get (client_socket& server, string& filename) {
   cix_header header;
   header.command = CIX_GET;
   filename.copy (header.filename, filename.size());
   header.nbytes = 0;

   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;

   if (header.command != CIX_FILE) {
      log << "sent CIX_GET, server did not return CIX_FILE" << endl;
      log << "server returned " << header << endl;
      log << "get: " << filename << ": " 
          << strerror(header.nbytes) << endl;
   }else {
      char buffer[header.nbytes + 1];
      recv_packet (server, buffer, header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      ofstream file_stream (filename);
      file_stream.write (buffer, header.nbytes);
      file_stream.close();
      log << "get: " << filename << ": OK" << endl;
   }
}

void cix_put (client_socket& server, string& filename) {
   cix_header header;
   header.command = CIX_PUT;
   filename.copy (header.filename, filename.size());

   ifstream file_stream (filename);
   if (file_stream.fail()) {
      log << "put: " << header.filename 
          << ": " << strerror (errno) << endl;
      return;
   }
   string put_file;
   char buffer[0x1000];
   file_stream.read (buffer, sizeof buffer);
   put_file.append (buffer);
   file_stream.close();
   header.nbytes = put_file.size();

   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   send_packet (server, put_file.c_str(), put_file.size());
   log << "sent " << put_file.size() << " bytes" << endl;
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;

   if (header.command != CIX_ACK) {
      log << "sent CIX_PUT, server did not return CIX_ACK" << endl;
      log << "server returned " << header << endl;
      log << "put: " << filename << ": " 
          << strerror(header.nbytes) << endl;
   } else log << "put: " << filename << ": OK" << endl;
}

void cix_rm (client_socket& server, string& filename) {
   cix_header header;
   header.command = CIX_RM;
   header.nbytes = 0;
   filename.copy (header.filename, filename.size());

   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;

   if (header.command != CIX_ACK) {
      log << "sent CIX_PUT, server did not return CIX_ACK" << endl;
      log << "server returned " << header << endl;
      log << "rm: " << filename << ": " 
          << strerror(header.nbytes) << endl;
   } else log << "rm: " << filename << ": OK" << endl;
}

void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

bool check_filename (string& filename) {
   bool filename_ok = true;
   if (filename.size() >= FILENAME_SIZE) {
      cerr << "get: " << filename << ": filename too large" << endl;
      filename_ok = false;
   } else if (filename.find ("/") != string::npos) {
      cerr << "get: " << filename 
           << ": filename cannot contain the \'/\' character" << endl;
      filename_ok = false;
   }
   return filename_ok;
}

int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host;
   in_port_t port;
   if (args.size() == 1) {
      try {
         int is_port = stoi (args[0]);
         host = get_cix_server_host (args, args.size());
         port = is_port;
      } catch (invalid_argument& error) {
         host = args[0];
         port = get_cix_server_port (args, args.size());
      }
   } else {
      host = get_cix_server_host (args, 0);
      port = get_cix_server_port (args, 1);
   }
   log << to_string (hostinfo()) << endl;
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         log << "command " << line << endl;
         string key = line;
         string filename;
         size_t found = line.find (" ");
         if (found != string::npos) {
            key = line.substr (0, found);
            filename = line.substr (found + 1);
            if (!check_filename(filename)) continue;
         }
         const auto& itor = command_map.find (key);
         cix_command cmd = itor == command_map.end()
                         ? CIX_ERROR : itor->second;
         switch (cmd) {
            case CIX_EXIT:
               throw cix_exit();
               break;
            case CIX_HELP:
               cix_help();
               break;
            case CIX_LS:
               cix_ls (server);
               break;
            case CIX_GET:
               if (filename.empty()) {
                  log << "get: missing filename" << endl;
                  break;
               }
               cix_get (server, filename);
               break;
            case CIX_PUT:
               if (filename.empty()) {
                  log << "put: missing filename" << endl;
                  break;
               }
               cix_put (server, filename);
               break;
            case CIX_RM:
               if (filename.empty()) {
                  log << "rm: missing filename" << endl;
                  break;
               }
               cix_rm (server, filename);
               break;
            default:
               log << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

