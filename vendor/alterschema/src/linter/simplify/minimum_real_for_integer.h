class MinimumRealForInteger final : public Rule {
public:
  MinimumRealForInteger()
      : Rule{"minimum_real_for_integer",
             "If an instance is guaranteed to be an integer, setting a real "
             "number lower bound is the same as a ceil of that lower bound"} {};

  [[nodiscard]] auto condition(const sourcemeta::jsontoolkit::JSON &schema,
                               const std::string &,
                               const std::set<std::string> &vocabularies,
                               const sourcemeta::jsontoolkit::Pointer &) const
      -> bool override {
    return contains_any(
               vocabularies,
               {"https://json-schema.org/draft/2020-12/vocab/validation",
                "https://json-schema.org/draft/2019-09/vocab/validation",
                "http://json-schema.org/draft-07/schema#",
                "http://json-schema.org/draft-06/schema#",
                "http://json-schema.org/draft-04/schema#",
                "http://json-schema.org/draft-03/schema#",
                "http://json-schema.org/draft-02/hyper-schema#",
                "http://json-schema.org/draft-01/hyper-schema#"}) &&
           schema.is_object() && schema.defines("type") &&
           schema.at("type").is_string() &&
           schema.at("type").to_string() == "integer" &&
           schema.defines("minimum") && schema.at("minimum").is_real();
  }

  auto transform(Transformer &transformer) const -> void override {
    const auto current{transformer.schema().at("minimum").to_real()};
    const auto new_value{static_cast<std::int64_t>(std::ceil(current))};
    transformer.assign("minimum", sourcemeta::jsontoolkit::JSON{new_value});
  }
};