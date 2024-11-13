#include <sourcemeta/jsontoolkit/json.h>
#include <sourcemeta/jsontoolkit/jsonschema.h>

#include <cstdlib>  // EXIT_SUCCESS
#include <iostream> // std::cout

#include "command.h"
#include "utils.h"

auto sourcemeta::jsonschema::cli::bundle(
    const std::span<const std::string> &arguments) -> int {
  const auto options{
      parse_options(arguments, {"h", "http", "w", "without-id"})};

  if (options.at("").size() < 1) {
    std::cerr
        << "error: This command expects a path to a schema. For example:\n\n"
        << "  jsonschema bundle path/to/schema.json\n";
    return EXIT_FAILURE;
  }

  auto schema{sourcemeta::jsontoolkit::from_file(options.at("").front())};

  if (options.contains("w") || options.contains("without-id")) {
    log_verbose(options) << "Bundling without using identifiers\n";
    sourcemeta::jsontoolkit::bundle(
        schema, sourcemeta::jsontoolkit::default_schema_walker,
        resolver(options, options.contains("h") || options.contains("http")),
        sourcemeta::jsontoolkit::BundleOptions::WithoutIdentifiers);
  } else {
    sourcemeta::jsontoolkit::bundle(
        schema, sourcemeta::jsontoolkit::default_schema_walker,
        resolver(options, options.contains("h") || options.contains("http")),
        sourcemeta::jsontoolkit::BundleOptions::Default);
  }

  sourcemeta::jsontoolkit::prettify(
      schema, std::cout, sourcemeta::jsontoolkit::schema_format_compare);
  std::cout << "\n";
  return EXIT_SUCCESS;
}
