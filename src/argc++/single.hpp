//
// Created by David Yang on 2025-10-21.
//
//

#ifndef SINGLE_HPP
#define SINGLE_HPP
/// Single include header for argc++

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <iostream>



namespace argcpp {

    typedef int argc_t;
    typedef char** argv_t;

    /// @brief Base argument struct
    /// Base argument struct, can be inherited if a custom theme requires additional info.
    ///
    /// This implies the need for one to specify a custom Theme class supporting this new member.
    struct Argument {
    /// Primary identifier for the argument in its extended form.
    /// Examples: "help", "output", "verbose"
    std::string long_name;

    /// Single-character shorthand for the argument.
    /// Set to '\0' if no short form is needed.
    char short_name;

    /// Additional long-form identifiers that resolve to this argument.
    /// Useful for maintaining backwards compatibility or providing intuitive alternatives.
    std::vector<std::string> aliases;

    /// Human-readable explanation of the argument's purpose and behavior.
    /// Displayed in help text and documentation.
    std::string description;

    /// Metavariable name shown in usage examples.
    /// Typically uppercase (FILE, NUMBER, PATH) to distinguish from literal values.
    std::string value_name;

    /// Organizational label for grouping related arguments in help output.
    /// Examples: "Input Options", "Network Configuration"
    std::string category;

    /// Indicates whether the argument expects an associated value.
    /// Mutually exclusive with is_flag in most scenarios.
    bool takes_value;

    /// Marks the argument as a boolean switch with no associated value.
    /// Presence indicates true, absence indicates false.
    bool is_flag;

    /// Enforces that the argument must be present for valid invocation.
    /// Parser will fail if this argument is missing.
    bool required;

    /// Fallback value used when the argument is not explicitly provided.
    /// Empty string indicates no default.
    std::string default_value;

    /// Stores the actual value supplied by the user during parsing.
    /// Remains empty if the argument was not provided.
    std::string value;

    /// Collection of values for arguments that accept multiple inputs.
    /// Used when min_values/max_values allow more than one value.
    std::vector<std::string> values;

    /// Lower bound on the number of values this argument can accept.
    /// Zero permits the argument to be optional.
    int min_values;

    /// Upper bound on the number of values this argument can accept.
    /// Use -1 to allow unlimited values.
    int max_values;

    /// Restricts input to a predefined set of acceptable values.
    /// Parser rejects values not present in this list.
    std::vector<std::string> allowed_values;

    /// Custom predicate for complex validation logic.
    /// Return true if the value meets requirements, false otherwise.
    std::function<bool(const std::string&)> validator;

    /// Message displayed when validation fails.
    /// Provides context-specific guidance to the user.
    std::string validation_error;

    /// Arguments that cannot be used simultaneously with this one.
    /// Parser fails if any conflicting arguments are present together.
    std::vector<std::string> conflicts_with;

    /// Arguments that must be present if this argument is used.
    /// Enforces dependencies between related options.
    std::vector<std::string> mandated;

    /// At least one argument from this list must be present if this argument is used.
    /// Implements "requires any of" dependency semantics.
    std::vector<std::string> requires_one_of;

    /// Tracks whether the user explicitly provided this argument.
    /// Distinguishes between default values and user-supplied values.
    bool was_provided;

    /// Index for positional arguments that don't use flag syntax.
    /// Zero indicates this is not a positional argument.
    int position;

    /// Excludes the argument from generated help text.
    /// Typically used for deprecated or internal-only options.
    bool hidden;

    /// Marks the argument as outdated but still functional.
    /// Parser may emit a warning when deprecated arguments are used.
    bool deprecated;

    /// Informational message shown when a deprecated argument is encountered.
    /// Should guide users toward the recommended alternative.
    std::string deprecated_message;

    /// Character used to split a single value string into multiple values.
    /// Common choices: ',' for lists, ':' for paths.
    char value_delimiter;

    /// Controls whether value matching respects character case.
    /// Affects both allowed_values checking and general comparison.
    bool case_sensitive;

    /// Permits values beginning with a hyphen character.
    /// Necessary for negative numbers and certain edge cases.
    bool allow_hyphen_values;

    /// Environment variable consulted when the argument is not provided.
    /// Offers a secondary source for configuration values.
    std::string env_var;
};

    /// @brief Defines **how** arguments should be parsed, that is, the syntax of the CLI
    class Rule_Set {
    public:
        // Possible inflictable argument types
        enum class Infliction_Arg_T {
            FLAG,
            VALUE,
        };

        virtual ~Rule_Set() = default;
        virtual void apply(std::string x) = 0;
    };

    /// @brief defines a ruleset for UNIX style CLI parsing
    class Simple_Rule_Set : public Rule_Set {
    public:
        void apply(std::string x) override {

        }
    };

    template <
        typename Argument_T = Argument,
        typename Rules = Simple_Rule_Set
    >
    requires std::is_base_of_v<Argument, Argument_T>
    and std::is_base_of_v<Rule_Set, Rules>
    class Parser {
        argc_t argc;
        argv_t argv;
        std::unordered_map<std::pair<char, std::string>, Argument_T> argument_map;
        std::unordered_map<std::string, Argument_T*> long_map;
        std::unordered_map<char, Argument_T*> short_map;
        Rules rules;

        Argument_T get_argument_object(std::string arg) {
            if (arg.size() == 1) {
                return *short_map[arg[0]];
            }

            if (arg.size() > 1) {
                return *long_map[arg];
            }

            return Argument_T();
        }

    public:
        using Arg_T = Argument_T;
        using Rule_T = Rules;

        /// @brief Basic constructor requiring both
        Parser(const argc_t argc, const argv_t argv) : argc(argc), argv(argv) {}

        void add_argument(const Argument_T& argument) {
            argument_map[std::make_pair(argument.short_name, argument.long_name)] = argument;
            long_map[argument.long_name] = &argument_map[std::make_pair(argument.short_name, argument.long_name)];
            short_map[argument.short_name] = &argument_map[std::make_pair(argument.short_name, argument.long_name)];
        }

        void parse() {
            for (auto arg : argv) {

            }
        }
    };

    /// @brief Supports customization backend for argc++
    class Theme {

    };

}


#endif //SINGLE_HPP
