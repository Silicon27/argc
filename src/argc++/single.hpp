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

namespace argcpp::error {
    struct Add_Argument_Error : public std::exception {
        explicit Add_Argument_Error(const std::string& message) : message(message) {}

        const char* what() const noexcept override {
            return message.c_str();
        }

    private:
        std::string message;
    };
}

namespace argcpp::swib {
    /// argc++ switch library - convert any if to a switch!
    // bool swib_and(std::function<std::variant<bool, int>> f, )
}

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

        /// Single-character (or possibly multi-charactered) shorthand for the argument.
        /// Set to '\0' if no short form is needed.
        std::string short_name;

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

    using nullpair_t = std::nullopt_t;
    inline constexpr nullpair_t nullpair = std::nullopt;

    /// Partial pair, a pair of mutual inclusions, that is, either one or the other may be present,
    /// and it would automatically point to a said object containing both
    template <typename T, typename U>
    struct partial_pair {
        using first_type = T;
        using second_type =  U;

        constexpr partial_pair() = default;
        constexpr ~partial_pair() = default;

        constexpr partial_pair(const T& first, const U& second)
            : first_(first), second_(second), first_has_value_(first_.has_value()), second_has_value_(second_.has_value()) {}
        constexpr partial_pair(std::optional<T> first, std::optional<U> second)
            : first_(first), second_(second), first_has_value_(first.has_value()), second_has_value_(second.has_value()) {}
        constexpr partial_pair(T&& first, U&& second)
            : first_(std::move(first)), second_(std::move(second)), first_has_value_(first_.has_value()), second_has_value_(second_.has_value()) {}

        explicit constexpr partial_pair(const partial_pair& other) = default;
        explicit constexpr partial_pair(partial_pair&& other) = default;

        constexpr bool operator==(const partial_pair& other) const {
            // write a truth table and attempt to write code
            // (F)illed ((E)mpty)
            // assumptions:
            // we assume
            // this.first_  this.second_    other.first_  other.second_
            //     F_1            E                F_1             F          == true  (and its inverse)   x
            //     E              F_1              F               F_1        == true  (and its inverse)   x
            //     E              E                F               F          == false (and its inverse)   x
            //     F_1            F_2              F_1             F_2        == true     x
            //     F_1            F_2              F_1             F_3        special case, where at least one of the pair is not equal to it's opposite on the other pair == false x
            //     E              E                E               E          == true     x

            const bool this_first = first_.has_value();
            const bool this_second = second_.has_value();
            const bool other_first = other.first_.has_value();
            const bool other_second = other.second_.has_value();

            // first perform operations on has_values

            // completely empty
            if (!this_first && !other_first && !this_second && !other_second) {
                return true;
            }

            // 2 cases for all filled:
            //     F_1            F_2              F_1             F_2        == true
            //     F_1            F_2              F_1             F_3        special case == false
            if (this_first && other_first && this_second && other_second) {
                std::pair<T, U> this_pair = {first_.value(), second_.value()};
                std::pair<T, U> other_pair = {other.first_.value(), other.second_.value()};
                return this_pair == other_pair;
            }

            if (
                // first clause for
                //     e              e                f               f          == false
                 (!this_first && !this_second && other_first && other_second)
                 ||
                 // second clause for
                 //     F              F                E               E          == false
                 (this_first && this_second && !other_first && !other_second)
                )
            {
                return false;
            }

            if (
                // first clause for
                //     E              F_1              F               F_1        == true
                (!this_first && this_second && other_first && other_second)
                ||
                // second clause for
                //     F              F_1              E               F_1        == true
                (this_first && this_second && !other_first && other_second)
                )
            {
                return this->second_.value() == other.second_.value();
            }

            if (
                // first clause for
                //     F_1            E                F_1             F          == true
                (this_first && !this_second && other_first && other_second)
                ||
                // second clause for
                //     F_1            F                F_1             E        == true
                (this_first && this_second && !other_first && !other_second)
                )
            {
                return this->first_.value() == other.first_.value();
            }

            // scrapped, value_or requires it's argument to be convertable to the type the optional holds
            return (this->first_ == other.first_) && (this->second_ == other.second_);
        }

        constexpr partial_pair& operator=(const partial_pair& other) {
            this->first_ = other.first_;
            this->second_ = other.second_;
            this->first_has_value_ = other.first_has_value_;
            this->second_has_value_ = other.second_has_value_;
            return *this;
        }

        constexpr partial_pair& operator=(partial_pair&& other) noexcept
            requires(std::is_assignable_v<const first_type&, first_type> && std::is_assignable_v<const second_type&, second_type>)
        {
            // rvalue reference so only decltype(other.first_) (and second_) needs to be the same as first_type (and second_type)
            first_ = std::move(other.first_);
            second_ = std::move(other.second_);
            first_has_value_ = std::move(other.first_has_value_);
            second_has_value_ = std::move(other.second_has_value_);
            return *this;
        }

        constexpr bool first_has_value() const noexcept {
            return this->first_has_value_;
        }

        constexpr bool second_has_value() const noexcept {
            return this->second_has_value_;
        }

        constexpr first_type first() const {
            return this->first_.value();
        }

        constexpr second_type second() const {
            return this->second_.value();
        }

        template <class Y>
        constexpr first_type first_or(Y val) const {
            static_assert(std::is_convertible_v<Y, first_type>, "partial_pair<T>::first_or: Y must be convertible to first_type");
            static_assert(std::is_copy_constructible_v<first_type>, "partial_pair<T>::first_or: first_type must be copy constructible");
            return this->first_has_value_ ? first_.value_or(val) : static_cast<first_type>(std::forward<Y>(val));
        }

        template <class Y>
        constexpr second_type second_or(Y val) const {
            static_assert(std::is_convertible_v<Y, second_type>, "partial_pair<T>::second_or: Y must be convertible to second_type");
            static_assert(std::is_copy_constructible_v<second_type>, "partial_pair<T>::second_or: second_type must be copy constructible");
            return this->second_has_value_ ? second_.value_or(val) : static_cast<second_type>(std::forward<Y>(val));
        }

        void swap(partial_pair& other) noexcept {
            first_.swap(other.first_);
            second_.swap(other.second_);
            first_has_value_ = other.first_has_value_;
            second_has_value_ = other.second_has_value_;
        }


    private:
        std::optional<T> first_;
        std::optional<U> second_;

        bool first_has_value_;
        bool second_has_value_;
    };

    template <typename T, typename U>
    partial_pair(T, U) -> partial_pair<T, U>;
}

namespace argcpp {

    /// base class for prefix types
    struct Prefix {
        virtual ~Prefix() = 0;

        /// @brief match has to modify arg by removing the prefix (convention). If match does not modify arg, the preceding body parser provided should take this into account otherwise users risk parsing errors
        virtual bool match(Argument& arg) = 0;
    };

    struct Basic_Prefix : Prefix {
        ~Basic_Prefix() override = default;
        bool match(Argument& arg) override {
            return !arg.short_name.empty() ? arg.short_name.starts_with("-") : arg.long_name.starts_with("--");
        }
    };

    struct Body {
        virtual ~Body() = 0;

        /// @brief not allowed to modify the argument, simply checks that the resulted body matches style conventions imposed by the Body class
        virtual bool match(const Argument& arg) = 0;
    };

    struct Basic_Body : Body {
        ~Basic_Body() override = default;
        bool match(const Argument& arg) override {
            return !arg.short_name.empty() ? arg.short_name.length() == 1 : arg.long_name.length() > 1;
        }
    };

    /// base class for argument parsing types
    struct Operate {
        virtual ~Operate() = 0;

        ///
        virtual void operate(const std::unordered_map<partial_pair<std::string, std::string>, Argument>& argument_map, std::string arg) = 0;
    };

    struct Basic_Operate : Operate {
        ~Basic_Operate() override = default;

        void operate(const std::unordered_map<partial_pair<std::string, std::string>, Argument>& argument_map, std::string arg) override {
            // we attempt to first search for arg in argument_map
            partial_pair<std::string, std::string> pp(arg, nullpair);
            // if ()
        }
    };

    template <
        typename Prefix_t = Basic_Prefix,
        typename Body_t = Basic_Body,
        typename Operate_t = Basic_Operate
    >
    requires std::is_base_of_v<Prefix, Prefix_t>
    class Parser {
        argc_t argc;
        argv_t argv;
        std::unordered_map<partial_pair<std::string, std::string>, Argument> argument_map;
    public:
        // public prefix type
        using p_Prefix_t = Prefix_t;
        using p_Body_t = Body_t;
        using p_Operator_t = Operate_t;

        Parser() = delete;
        Parser(const argc_t& argc, const argv_t& argv) : argc(argc), argv(argv) {}
        Parser(const Parser& p) {
            this->argc = p.argc;
            this->argv = p.argv;
            this->argument_map = p.argument_map;
        }
        Parser(Parser&& p) noexcept {
            this->argc = p.argc;
            this->argv = p.argv;
            this->argument_map = std::move(p.argument_map);
        }

        ~Parser() = default;

        void add_argument(const Argument& arg) {
            // add_argument needs to account for how users may specify the name of the argument with a prefixed `-` or `--`

            // verify the prefix with Prefix
            Prefix_t prefix;
            if (!prefix.match(arg)) {
                throw error::Add_Argument_Error("Given argument does not match the prefix type");
            }

            Body_t body;
            if (!body.match(arg)) {
                throw error::Add_Argument_Error("Given argument does not match the body type");
            }

            argument_map[partial_pair(arg.long_name, arg.short_name)] = arg;
        }

        // for parsed argument indexing

        void parse() {
            for (int i = 1; i < argc; i++) {
                // parse the argument
                std::string arg = argv[i];


            }
        }


    };

}

#endif //SINGLE_HPP
