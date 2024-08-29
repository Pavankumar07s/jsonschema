#include <sourcemeta/jsontoolkit/json.h>
#include <sourcemeta/jsontoolkit/jsonl.h>
#include <sourcemeta/jsontoolkit/jsonschema.h>

#include <chrono>   // std::chrono
#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream> // std::cerr
#include <set>      // std::set
#include <string>   // std::string

#include "command.h"
#include "utils.h"

// TODO: Add a flag to emit output using the standard JSON Schema output format
// TODO: Add a flag to collect annotations
// TODO: Add a flag to take a pre-compiled schema as input
auto sourcemeta::jsonschema::cli::validate(
    const std::span<const std::string> &arguments) -> int {
  const auto options{parse_options(arguments, {"h", "http", "b", "benchmark"})};

  if (options.at("").size() < 1) {
    std::cerr
        << "error: This command expects a path to a schema and a path to an\n"
        << "instance to validate against the schema. For example:\n\n"
        << "  jsonschema validate path/to/schema.json path/to/instance.json\n";
    return EXIT_FAILURE;
  }

  if (options.at("").size() < 2) {
    std::cerr
        << "error: In addition to the schema, you must also pass an argument\n"
        << "that represents the instance to validate against. For example:\n\n"
        << "  jsonschema validate path/to/schema.json path/to/instance.json\n";
    return EXIT_FAILURE;
  }

  const auto &schema_path{options.at("").at(0)};
  const auto custom_resolver{
      resolver(options, options.contains("h") || options.contains("http"))};

  const auto schema{sourcemeta::jsontoolkit::from_file(schema_path)};

  if (!sourcemeta::jsontoolkit::is_schema(schema)) {
    std::cerr << "error: The schema file you provided does not represent a "
                 "valid JSON Schema\n  "
              << std::filesystem::canonical(schema_path).string() << "\n";
    return EXIT_FAILURE;
  }

  const auto benchmark{options.contains("b") || options.contains("benchmark")};
  const auto schema_template{sourcemeta::jsontoolkit::compile(
      schema, sourcemeta::jsontoolkit::default_schema_walker, custom_resolver,
      sourcemeta::jsontoolkit::default_schema_compiler,
      sourcemeta::jsontoolkit::SchemaCompilerCompilationMode::Optimized)};

  bool result{true};

  auto iterator{options.at("").cbegin()};
  std::advance(iterator, 1);
  for (; iterator != options.at("").cend(); ++iterator) {
    const std::filesystem::path instance_path{*iterator};
    if (instance_path.extension() == ".jsonl") {
      log_verbose(options)
          << "Interpreting input as JSONL: "
          << std::filesystem::weakly_canonical(instance_path).string() << "\n";
      std::size_t index{0};

      auto stream{sourcemeta::jsontoolkit::read_file(instance_path)};
      try {
        for (const auto &instance : sourcemeta::jsontoolkit::JSONL{stream}) {
          index += 1;
          std::ostringstream error;
          sourcemeta::jsontoolkit::SchemaCompilerErrorTraceOutput output{
              instance};
          bool subresult = true;
          if (benchmark) {
            const auto timestamp_start{
                std::chrono::high_resolution_clock::now()};
            subresult =
                sourcemeta::jsontoolkit::evaluate(schema_template, instance);
            const auto timestamp_end{std::chrono::high_resolution_clock::now()};
            const auto duration_us{
                std::chrono::duration_cast<std::chrono::microseconds>(
                    timestamp_end - timestamp_start)};
            if (subresult) {
              std::cout << "took: " << duration_us.count() << "us\n";
            } else {
              error << "error: Schema validation failure\n";
            }
          } else {
            subresult = sourcemeta::jsontoolkit::evaluate(
                schema_template, instance,
                sourcemeta::jsontoolkit::SchemaCompilerEvaluationMode::Fast,
                std::ref(output));
          }

          if (subresult) {
            log_verbose(options)
                << "ok: "
                << std::filesystem::weakly_canonical(instance_path).string()
                << " (entry #" << index << ")"
                << "\n  matches "
                << std::filesystem::weakly_canonical(schema_path).string()
                << "\n";
          } else {
            std::cerr
                << "fail: "
                << std::filesystem::weakly_canonical(instance_path).string()
                << " (entry #" << index << ")\n\n";
            sourcemeta::jsontoolkit::prettify(instance, std::cerr);
            std::cerr << "\n\n";
            std::cerr << error.str();
            print(output, std::cerr);
            result = false;
            break;
          }
        }
      } catch (const sourcemeta::jsontoolkit::ParseError &error) {
        // For producing better error messages
        throw sourcemeta::jsontoolkit::FileParseError(instance_path, error);
      }

      if (index == 0) {
        log_verbose(options) << "warning: The JSONL file is empty\n";
      }
    } else {
      const auto instance{sourcemeta::jsontoolkit::from_file(instance_path)};
      std::ostringstream error;
      sourcemeta::jsontoolkit::SchemaCompilerErrorTraceOutput output{instance};
      bool subresult{true};
      if (benchmark) {
        const auto timestamp_start{std::chrono::high_resolution_clock::now()};
        subresult =
            sourcemeta::jsontoolkit::evaluate(schema_template, instance);
        const auto timestamp_end{std::chrono::high_resolution_clock::now()};
        const auto duration_us{
            std::chrono::duration_cast<std::chrono::microseconds>(
                timestamp_end - timestamp_start)};
        if (subresult) {
          std::cout << "took: " << duration_us.count() << "us\n";
        } else {
          error << "error: Schema validation failure\n";
          result = false;
        }
      } else {
        subresult = sourcemeta::jsontoolkit::evaluate(
            schema_template, instance,
            sourcemeta::jsontoolkit::SchemaCompilerEvaluationMode::Fast,
            std::ref(output));
      }

      if (subresult) {
        log_verbose(options)
            << "ok: "
            << std::filesystem::weakly_canonical(instance_path).string()
            << "\n  matches "
            << std::filesystem::weakly_canonical(schema_path).string() << "\n";
      } else {
        std::cerr << "fail: "
                  << std::filesystem::weakly_canonical(instance_path).string()
                  << "\n";
        std::cerr << error.str();
        print(output, std::cerr);
        result = false;
      }
    }
  }

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
