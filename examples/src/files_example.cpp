//FIXME

#include <iostream>
#include <string>
#include <vector>
#include <ostream>

// Only needed to query the library version:
#include <werkzeugkiste/version.h>

#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/files/filesys.h>


int main(int /* argc */, char ** /* argv */) {
  namespace wkf = werkzeugkiste::files;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << WERKZEUGKISTE_VERSION << "\n"
            << "    File utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  //TODO
  return 0;
}
