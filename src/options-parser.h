#pragma once

#include <memory>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

/// @brief CLI arguments parser
class OptionsParser {
 public:

  /// @brief key/value type wrapper
  using dualparam_t = std::pair<std::string, bool>;

  /// @brief Arguments description struct
  struct GenOptions
  {
    /// @brief dbc file name
    dualparam_t dbc;

    /// @brief output directory for generated files
    dualparam_t outdir;

    /// @brief main driver name
    dualparam_t drvname;

    /// @brief rewrite previously generated files or generate to next subdirectory
    bool is_rewrite{false};

    /// @brief generate specific utility drivers for each ECU defined in the matrix
    bool is_nodeutils{false};

    /// @brief do not generate configuration file
    bool is_noconfig{false};

    /// @brief do not generate canmonitorutil header
    bool is_nocanmon{false};

    /// @brief do not generate fmon header
    bool is_nofmon{false};

    /// @brief help is requested
    bool is_help{false};
  };

  /// @brief Parses arguments and theirs optional values
  /// @param argc arguments number
  /// @param argv pointer to array with arguments
  /// @return parsed arguments in structured form
  GenOptions GetOptions(int argc, char** argv);

};


