/* 
   MaBoSS (Markov Boolean Stochastic Simulator)
   Copyright (C) 2011-2018 Institut Curie, 26 rue d'Ulm, Paris, France
   
   MaBoSS is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   MaBoSS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA 
*/

/*
   Module:
     MaBoSS-server.cc

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
 
   Date:
     May 2018
*/

#include <iostream>
#include "Client.h"
#include "Server.h"
#include "Utils.h"

const char* prog;

static int usage(std::ostream& os = std::cerr)
{
  os << "\nUsage:\n\n";
  os << "  " << prog << " [-h|--help]\n\n";
  os << "  " << prog << " [-V|--version]\n\n";
  os << "  " << prog << " --host HOST --port PORT\n";
  return 1;
}

static int help()
{
  //  std::cout << "\n=================================================== " << prog << " help " << "===================================================\n";
  (void)usage(std::cout);
  std::cout << "\nOptions:\n\n";
  std::cout << "  -V --version                            : displays MaBoSS-client version\n";
  std::cout << "  --host HOST                             : uses given host\n";
  std::cout << "  --port PORT                             : uses given PORT (number or filename)\n";
  std::cout << "  -h --help                               : displays this message\n";
  return 0;
}

int main(int argc, char* argv[])
{
  std::string port;
  std::string host;
  prog = argv[0];

  for (int nn = 1; nn < argc; ++nn) {
    const char* opt = argv[nn];
    if (!strcmp(opt, "-version") || !strcmp(opt, "--version") || !strcmp(opt, "-V")) { // keep -version for backward compatibility
      //std::cout << "MaBoSS version " + MaBEstEngine::VERSION << " [networks up to " << MAXNODES << " nodes]\n";
      std::cout << "MaBoSS version <TBD>\n";
      return 0;
    } else if (!strcmp(opt, "--host")) {
      if (checkArgMissing(opt, nn, argc)) {
	return usage();
      }
      host = argv[++nn];
    } else if (!strcmp(opt, "--port")) {
      if (checkArgMissing(opt, nn, argc)) {
	return usage();
      }
      port = argv[++nn];
    } else if (!strcmp(opt, "--help") || !strcmp(opt, "-h")) {
      return help();
    } else {
      std::cerr << '\n' << prog << ": unknown option " << opt << std::endl;
      return usage();
    }
  }

  if (host.length() == 0) {
    std::cerr << '\n' << prog << ": host is missing\n";
    return usage();
  }

  if (port.length() == 0) {
    std::cerr << '\n' << prog << ": port is missing\n";
    return usage();
  }

  Server* server = Server::getServer(host, port);
  server->manageRequests();
  delete server;

  return 0;
}
