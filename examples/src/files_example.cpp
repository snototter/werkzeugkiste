// FIXME

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

// Only needed to query the library version:
#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/files/filesys.h>
#include <werkzeugkiste/version.h>

int main(int /* argc */, char** /* argv */)
{
  namespace wkf = werkzeugkiste::files;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << werkzeugkiste::Version() << "\n"
            << "    File utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  // TODO
  return 0;
}
