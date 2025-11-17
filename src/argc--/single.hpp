//
// Created by David Yang on 2025-11-08.
//

#ifndef SINGLE_HPP
#define SINGLE_HPP
#include <unordered_map>
#include <iostream>
#include <exception>
#include <ranges>

namespace argcpp::exceptions {
    class add_argument_error : public std::exception {
        std::string msg_;
    public:
        explicit add_argument_error(const std::string& msg) : msg_(msg) {}

        const char* what() const noexcept override {
            return msg_.c_str();
        }

    };

    class push_back_and_replace_error : public std::exception {
        std::string msg_;
    public:
        explicit push_back_and_replace_error(const std::string& msg) : msg_(msg) {}

        const char* what() const noexcept override {
            return msg_.c_str();
        }
    };
}

namespace argcpp::helper {

    /// replaces the value in some vector if position < vector.size()
    ///
    /// adds a value if position == vector.size()
    ///
    /// if position > vector.size() we throw
    template <typename T>
    void push_back_and_replace(std::vector<T>& v, const int position, T& value) {
        if (position < 0) {
            throw exceptions::push_back_and_replace_error("position must be >= 0");
        }
        if (position > v.size()) {
            throw exceptions::push_back_and_replace_error("position out of range, exceeded vector size");
        }

        if (position < v.size()) {
            v[position] = value;
        } else if (position == v.size()) {
            v.push_back(value);
        }
    }
}

namespace argcpp {

    class Value;
    struct Argument;
    struct Positional;
    class Parser;

    class Value {
        bool is_set = false;
        std::vector<Value> stored_values;
        std::string stored_string;
        bool stored_bool;
        double stored_double;
        int stored_int;

    public:
        void reset() noexcept {
            is_set = false;
            stored_values.clear();
            stored_string = "";
            stored_bool = false;
            stored_double = 0.0;
            stored_int = 0;
        }

        explicit operator bool() const noexcept {
            return stored_bool;
        }

        explicit operator double() const noexcept {
            return stored_double;
        }

        explicit operator int() const noexcept {
            return stored_int;
        }

        explicit operator std::string() const noexcept {
            return stored_string;
        }

        explicit operator std::vector<Value>() const noexcept {
            return stored_values;
        }


        Value& operator=(const std::string& s) {
            reset();
            stored_string = s;
            is_set = true;
            return *this;
        }

        Value& operator=(const int i) {
            reset();
            stored_int = i;
            is_set = true;
            return *this;
        }

        Value& operator=(const double d) {
            reset();
            stored_double = d;
            is_set = true;
            return *this;
        }

        Value& operator=(const bool b) {
            reset();
            stored_bool = b;
            is_set = true;
            return *this;
        }

        Value& operator=(const char* s) {
            reset();
            stored_string = s;
            is_set = true;
            return *this;
        }

        Value& operator=(const std::vector<Value> &v) {
            reset();
            stored_values = v;
            is_set = true;
            return *this;
        }



    };

    struct Argument {
    private:
        Parser* parser_ = nullptr; // back-reference to the parser


        /// @brief Primary identifier for the argument in its extended form.
        /// @details Examples: "help", "output", "verbose".
        std::string _canonical_name;

        /// @brief Additional long-form identifiers that resolve to this argument.
        /// @details Useful for maintaining backwards compatibility or providing intuitive alternatives.
        std::vector<std::string> _aliases;

        /// @brief Human-readable explanation of the argument's purpose and behavior.
        /// @details Displayed in help text and documentation.
        std::string _description;

        /// @brief Metavariable name shown in usage examples.
        /// @details Typically uppercase (FILE, NUMBER, PATH) to distinguish from literal values.
        std::string _value_name;

        /// @brief Organizational label for grouping related arguments in help output.
        /// @details Examples: "Input Options", "Network Configuration". By default it falls under a general, unnamed category.
        std::string _category;

        /// @brief Indicates whether the argument expects an associated value.
        /// @details Mutually exclusive with is_flag in most scenarios.
        bool _takes_value = false;

        /// @brief Marks the argument as a boolean switch with no associated value.
        /// @details Presence indicates true, absence indicates false. By default arguments are flags, meaning _min and _max_values are by default set to 0
        bool _is_flag = true;

        /// @brief Enforces that the argument must be present for valid invocation.
        /// @details Parser will fail if this argument is missing.
        bool _required = false;

        /// @brief Fallback value used when the argument is not explicitly provided.
        /// @details Value::empty is a flag for if _default_value is empty
        Value _default_value;

        /// @brief Stores the actual value supplied by the user during parsing.
        /// @details Remains empty if the argument was not provided.
        Value _value;

        /// @brief Collection of values for arguments that accept multiple inputs.
        /// @details Used when min_values/max_values allow more than one value.
        std::vector<Value> _values;

        /// @brief Lower bound on the number of values this argument can accept.
        // if the argument is a flag both _max_values and _min_values would be 0
        int _min_values = 0;

        /// @brief Upper bound on the number of values this argument can accept.
        /// @details Use -1 to allow unlimited values. This means all arguments after this argument would be its values.
        int _max_values = 0;

        /// @brief Restricts input to a predefined set of acceptable values.
        /// @details Parser rejects values not present in this list.
        std::vector<std::string> _allowed_values;

        /// @brief Custom predicate for complex validation logic.
        /// @details Return true if the value meets requirements, false otherwise.
        std::function<bool(const std::string&)> _validator;

        /// @brief Message displayed when validation fails.
        /// @details Provides context-specific guidance to the user.
        std::string _validation_error;

        /// @brief Arguments that cannot be used simultaneously with this one.
        /// @details Parser fails if any conflicting arguments are present together.
        std::vector<std::string> _conflicts_with;

        /// @brief Arguments that must be present if this argument is used.
        /// @details Enforces dependencies between related options.
        std::vector<std::string> _mandated;

        /// @brief At least one argument from this list must be present if this argument is used.
        /// @details Implements "requires any of" dependency semantics.
        std::vector<std::string> _requires_one_of;

        /// @brief Tracks whether the user explicitly provided this argument.
        /// @details Distinguishes between default values and user-supplied values.
        bool _was_provided = false;

        /// @brief Index for positional arguments that don't use flag syntax.
        /// @details Zero indicates this is not a positional argument.
        int _position = 0;

        /// @brief Whether argument is positional
        /// @details implies _position is > 0
        bool _is_positional = false;

        /// @brief Excludes the argument from generated help text.
        /// @details Typically used for deprecated or internal-only options.
        bool _hidden = false;

        /// @brief Marks the argument as outdated but still functional.
        /// @details Parser may emit a warning when deprecated arguments are used.
        bool _deprecated = false;

        /// @brief Informational message shown when a deprecated argument is encountered.
        /// @details Should guide users toward the recommended alternative.
        std::string _deprecated_message;

        /// @brief Character used to split a single value string into multiple values.
        /// @details Common choices: ',' for lists, ':' for paths.
        char _value_delimiter = ',';

        /// @brief Controls whether value matching respects character case.
        /// @details Affects both allowed_values checking and general comparison.
        bool _case_sensitive = true;

        /// @brief Permits values beginning with a hyphen character.
        /// @details Necessary for negative numbers and certain edge cases.
        bool _allow_hyphen_values = false;

        /// @brief Environment variable consulted when the argument is not provided.
        /// @details Offers a secondary source for configuration values.
        std::string _env_var;

        // allow for "attribute-chaining"
    public:
        /// @brief Sets the primary long-form name of the argument.
        /// @details Corresponds to the user-facing identifier used as --name.
        Argument& long_name(const std::string &long_name) {
            this->_canonical_name = long_name;
            this->_value_name = long_name;
            return *this;
        }

        /// @brief Sets the short-form name of the argument.
        /// @details Typically a single character used with a single hyphen (e.g., -v).
        Argument& short_name(const std::string &short_name);

        /// @brief Replaces the alias list with new alternative long-form identifiers.
        /// @details Aliases allow multiple names to refer to the same argument.
        Argument& aliases(const std::vector<std::string> &alias_list);

        /// @brief Sets the argument's description.
        /// @details Shown in generated help and documentation.
        Argument& help(const std::string &description) {
            this->_description = description;
            return *this;
        }

        /// @brief Assigns the metavariable name used when displaying usage examples.
        /// @details Helps visually distinguish user-supplied values from literal tokens.
        Argument& value_name(const std::string &value_name) {
            this->_value_name = value_name;
            return *this;
        }

        /// @brief Groups this argument under a named help section.
        /// @details Useful for organizing large sets of arguments.
        Argument& category(const std::string &category) {
            this->_category = category;
            return *this;
        }

        /// @brief Marks the argument as one that accepts a value.
        /// @details Automatically disables flag mode and ensures appropriate min/max values.
        Argument& takes_value() {
            this->_takes_value = true;
            this->_is_flag = false;
            if (this->_min_values == 0) this->_min_values = 1;
            if (this->_max_values == 0) this->_max_values = 1;
            return *this;
        }

        /// @brief Declares the argument as a flag.
        /// @details Flags do not accept values and always have min/max values set to 0.
        Argument& is_flag() {
            this->_is_flag = true;
            this->_takes_value = false;
            this->_min_values = 0;
            this->_max_values = 0;
            return *this;
        }

        /// @brief Marks the argument as required for valid invocation.
        /// @details Absence of this argument during parsing results in an error.
        Argument& required() {
            this->_required = true;
            return *this;
        }

        Argument& optional();

        /// @brief Defines a default value that applies when the user does not supply one.
        /// @details The default is used only if no environment variable or user input overrides it.
        Argument& default_value(const Value& default_value) {
            this->_default_value = default_value;
            return *this;
        }

        /// @brief Sets both minimum and maximum number of values for the argument.
        /// @details Flags must always have min/max values of zero. Use -1 for max_values to allow unlimited values.
        Argument& x_value_range(const int min_values, const int max_values) {
            if (_is_flag) {
                if (min_values != 0 || max_values != 0)
                    throw exceptions::add_argument_error("Flags cannot have min_values or max_values > 0.");
                _min_values = 0;
                _max_values = 0;
                return *this;
            }

            if (min_values < 0)
                throw exceptions::add_argument_error("min_values cannot be negative.");

            if (max_values <= 0 && max_values != -1)
                throw exceptions::add_argument_error("max_values must be > 0 or -1 for unlimited.");

            if (max_values != -1 && min_values > max_values)
                throw exceptions::add_argument_error("min_values cannot exceed max_values.");

            _min_values = min_values;
            _max_values = max_values;
            return *this;
        }

        /// @brief Restricts acceptable values to the provided set.
        /// @details The parser rejects input not contained in this list.
        Argument& allowed_values(const std::vector<std::string> &allowed_values) {
            this->_allowed_values = allowed_values;
            return *this;
        }

        /// @brief Assigns a custom validation predicate for argument values.
        /// @details The function must return true to indicate validity.
        Argument& validate(const std::function<bool(const std::string&)>& validator) {
            this->_validator = validator;
            return *this;
        }

        /// @brief Sets the error message shown when validation fails.
        /// @details Should guide the user toward acceptable input.
        Argument& validation_error_message(const std::string& error_message) {
            this->_validation_error = error_message;
            return *this;
        }

        /// @brief Specifies arguments that cannot appear alongside this one.
        /// @details Enforces mutual exclusivity.
        Argument& conflicts_with(const std::vector<std::string> &conflicts_with) {
            this->_conflicts_with = conflicts_with;
            return *this;
        }

        /// @brief Specifies arguments that must also be present when this argument is used.
        /// @details Implements strict dependency enforcement.
        Argument& mandated(const std::vector<std::string> &mandated) {
            this->_mandated = mandated;
            return *this;
        }

        /// @brief Ensures that at least one argument from the given list is present.
        /// @details Useful for alternatives such as (--tcp | --udp).
        Argument& requires_one_of(const std::vector<std::string> &requires_one_of) {
            this->_requires_one_of = requires_one_of;
            return *this;
        }

        /// @brief Assigns a positional index to the argument.
        /// @details Zero indicates a non-positional (named) argument.
        /// @return Reference to the created/inserted Positional object.
        Positional& position(const int position);

        /// @brief Hides the argument from help output.
        /// @details Used for deprecated or private options.
        Argument& hidden() {
            this->_hidden = true;
            return *this;
        }

        /// @brief Marks the argument as deprecated.
        /// @details The parser may emit warnings when this argument is used.
        Argument& deprecated() {
            this->_deprecated = true;
            return *this;
        }

        /// @brief Provides a guidance message when the deprecated argument is used.
        /// @details Should indicate the recommended replacement argument.
        Argument& deprecated_message(const std::string& deprecated_message) {
            this->_deprecated_message = deprecated_message;
            return *this;
        }

        /// @brief Sets the delimiter used to split a single value into multiple entries.
        /// @details Useful for comma-separated lists or path variables.
        Argument& value_delimiter(const char delimiter) {
            this->_value_delimiter = delimiter;
            return *this;
        }

        /// @brief Allows values that begin with a hyphen.
        /// @details Required for negative numbers and file names such as "-foo".
        Argument& allow_hyphen_value(const bool allow_hyphen_value) {
            this->_allow_hyphen_values = allow_hyphen_value;
            return *this;
        }

        /// @brief Specifies an environment variable to use when no argument value is provided.
        /// @details Useful for configuration defaults and secret propagation (e.g., tokens).
        Argument& env_var(const std::string &env_var) {
            this->_env_var = env_var;
            return *this;
        }

        friend class Parser;
    };

    struct Positional {
        // --- identity ---
        std::string canonical_name_;      // internal name used by Parser
        std::string value_name_;          // shown in help text (e.g., FILE, PATH)

        // --- metadata ---
        std::string description_;         // human-readable explanation

        // --- positional semantics ---
        int position_index_ = 0;          // 0-based physical order
        bool required_ = true;            // required unless declared optional
        bool variadic_ = false;           // whether it consumes unlimited remaining args

        // --- values ---
        int min_values_ = 1;              // usually 1 for simple positionals
        int max_values_ = 1;              // -1 for unlimited (only allowed for last positional)
        Value default_value_;              // default if omitted AND optional
        std::vector<Value> values_;        // collected values after parsing

        // --- validation ---
        std::vector<std::string> allowed_values_;
        std::function<bool(const std::string&)> validator_;
        std::string validation_error_;

        // --- formatting ---
        char value_delimiter_ = ',';      // allows “a,b,c” as value lists

        // --- environment integration ---
        std::string env_var_;             // optional: fallback source

        // --- parser bookkeeping ---
        bool was_provided_ = false;       // track whether user gave it

        // --- builder-style member functions ---
        Positional& help(const std::string& description) {
            this->description_ = description;
            return *this;
        }

        Positional& name(const std::string& name) {
            this->canonical_name_ = name;
            this->value_name_ = name;
            return *this;
        }

        Positional& value_name(const std::string& value_name) {
            this->value_name_ = value_name;
            return *this;
        }
        Positional& default_value(const Value& default_value) {
            this->default_value_ = default_value;
            return *this;
        }
        Positional& allowed_values(const std::vector<std::string>& allowed_values) {
            this->allowed_values_ = allowed_values;
            return *this;
        }
        Positional& validate(const std::function<bool(const std::string&)>& validator) {
            this->validator_ = validator;
            return *this;
        }
        Positional& validation_error_message(const std::string& error_message) {
            this->validation_error_ = error_message;
            return *this;
        }
        Positional& value_delimiter(const char delimiter) {
            this->value_delimiter_ = delimiter;
            return *this;
        }
        Positional& env_var(const std::string& env_var) {
            this->env_var_ = env_var;
            return *this;
        }
        Positional& required() {
            this->required_ = true;
            return *this;
        }
        Positional& optional() {
            this->required_ = false;
            return *this;
        }
        // Positional& min_values(int minval) {
        //     this->min_values_ = minval;
        //     return *this;
        // }
        // Positional& max_values(int maxval) {
        //     this->max_values_ = maxval;
        //     return *this;
        // }

        // TODO make this one range function ^
        Positional& position_index(int idx) {
            this->position_index_ = idx;
            return *this;
        }
        Positional& variadic(bool is_variadic = true) {
            this->variadic_ = is_variadic;
            return *this;
        }
    };

    class Parser {
        std::unordered_map<std::string, std::shared_ptr<Argument>> argument_map_;
        std::vector<std::shared_ptr<Argument>> arguments_;

        // required positionals, comes before optionals
        std::vector<Positional> required_positionals_;

        // optional positionals, always comes last in the command
        std::vector<Positional> optional_positionals_;


        // results
        std::unordered_map<std::string, Value> results_;

        int argc_;
        char **argv_;

        // members for iteration counts, program specified values, users don't specify these
        std::size_t argv_index;

        std::string next() {
            return argv_[argv_index++];
        }

        void register_alias(const std::string& alias, const Argument& arg) {
            argument_map_[alias] = argument_map_.at(arg._canonical_name);
        }

        static void remove_prefix(std::string& str) {
            if (str.starts_with("-")) {
                str.erase(0, 1);
            } else if (str.starts_with("--")) {
                str.erase(0, 2);
            }
        }

        void parse_positional_arguments() {
            // if empty then do nothing
            if (optional_positionals_.empty() && required_positionals_.empty()) {
                return;
            }
            if (required_positionals_.empty()) {
                goto optional_positionals_loop;
            }

            // split into 2 parts, one for loop for required, and one for optional
            for (auto p : required_positionals_) {
                if (argv_index == argc_) {
                    display_help("There are less than required number of positionals");
                }

                results_[p.canonical_name_] = argv_[argv_index++];
            }

            if (optional_positionals_.empty()) {
                return;
            }

            optional_positionals_loop:

            for (auto p : optional_positionals_) {

            }

        }

        friend struct Argument;

    public:
        Parser(const int argc, char** argv)
            : argc_(argc), argv_(argv), argv_index(0)
        {}

        /// Make argument visible to the parser
        ///
        /// @param name would be implicitly used as long name for argument unless set explicitly. Otherwise, it acts as a unique indexing identifier to distinguish between arguments.
        Argument& add_argument(const std::string &name) {
            const auto arg = std::shared_ptr<Argument>();
            arg->long_name(name);

            arguments_.push_back(arg);
            argument_map_[name] = arg;

            arg->parser_ = this;
            return *arg;
        }

        void display_help(
            std::string condition_message = "" // A helpful message to display alongside the help, empty for no message
        ) {

        }

        void parse() {
            // NOTE before parsing anything, either validate the program name or ignore it
            argv_index++; // skip

            // NOTE make a solution for positional arguments
            parse_positional_arguments();

            for (; argv_index < this->argc_; argv_index++) {
                std::string arg = next();

                remove_prefix(arg);

                auto it = argument_map_.find(arg);

                // if it does not match any allowed arguments
                if (it == argument_map_.end()) { display_help(); return; }

                std::shared_ptr<Argument> argument = it->second;


            }
        }

    };

    inline Argument& Argument::short_name(const std::string &short_name) {
        this->_aliases.push_back(short_name);
        if (parser_) parser_->register_alias(short_name, *this);
        return *this;
    }

    inline Argument& Argument::aliases(const std::vector<std::string> &alias_list) {
        this->_aliases = alias_list;
        for (auto &alias : alias_list) {
            if (parser_) parser_->register_alias(alias, *this);
        }
        return *this;
    }

    inline Positional& Argument::position(const int position) {
        if (position < 0) {
            throw exceptions::add_argument_error("positional arguments must have non-negative positions.");
        }

        _is_flag = false;
        _takes_value = false;
        _required = true;
        _is_positional = true;

        // add positional argument to vector
        if (parser_) {
            Positional p;
            p.canonical_name_ = this->_canonical_name;
            helper::push_back_and_replace(parser_->required_positionals_, position, p);
            this->_position = position;
            // Return reference to the inserted/updated Positional
            return parser_->required_positionals_[position];
        } else {
            throw exceptions::add_argument_error("No parser associated with this Argument for position().");
        }
    }

    inline Argument &Argument::optional() {
        // if there exists a parser object and `this` is positional,
        // then we remove the last index of required_positionals and
        // move it to optional_positionals
        if (parser_ && this->_is_positional) {
            const Positional p = parser_->required_positionals_[parser_->optional_positionals_.size() - 1];
            parser_->optional_positionals_.push_back(p);
            parser_->required_positionals_.pop_back();
        }
        this->_required = false;


        return *this;
    }

}

#endif //SINGLE_HPP
