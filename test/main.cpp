#include <single.hpp>
#include <iostream>
#include <optional>


int main(int argc, char **argv) {
    argcpp::Parser parser(argc, argv);
    parser.add_argument("help")
        .takes_value()
        .x_value_range(1, 2)
        .short_name("h");

    parser.add_argument("positional1")
        .position(1)
        .value_name("positional1")
        .help("positional1")
        .required();
    // currently, argument structure looks like this:
    // program_name <positional1> [HELP]

    return 0;
}