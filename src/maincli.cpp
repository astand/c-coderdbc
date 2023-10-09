
#include <stdlib.h>
#include <memory>
#include "app.h"
#include "options-parser.h"

int main(int argc, char* argv[])
{
  OptionsParser parser;
  auto opts = parser.GetOptions(argc, argv);
  CoderApp app(opts);
  app.Run();
}
