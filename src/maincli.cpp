
#include <stdlib.h>
#include <memory>
#include "app.h"
#include "options-parser.h"

int main(int argc, char* argv[])
{
  OptionsParser parser;
  auto opts = parser.GetOptions(argc, argv);
  auto app = std::make_unique<CoderApp>(opts);
  app->Run();
}
