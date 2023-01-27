// FIXME

#include <werkzeugkiste/files/fileio.h>
#include <werkzeugkiste/files/filesys.h>

#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

// Only needed to query the library version:
#include <werkzeugkiste/version.h>

int main(int /* argc */, char** /* argv */) {
  namespace wkf = werkzeugkiste::files;
  std::cout << "--------------------------------------------------\n"
            << "    Werkzeugkiste v" << werkzeugkiste::Version() << "\n"
            << "    File utilities demo\n"
            << "--------------------------------------------------\n"
            << std::endl;

  // Useful to process *really* large files:
  const std::string file{__FILE__};
  std::cout << "Processing file \"" << file << "\" line-by-line:\n";
  wkf::AsciiFileIterator line_reader{file};
  while (line_reader.HasLine()) {
    std::cout << '#' << std::setw(2) << std::setfill('0')
              << line_reader.LineNumber() << ": " << (*line_reader)
              << std::endl;
    //    ++line_reader;       // equivalent to:
    line_reader.Next();
  }

  return 0;
}
