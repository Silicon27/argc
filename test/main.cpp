#include <single.hpp>
#include <iostream>
#include <optional>


int main(int argc, char **argv) {
    argcpp::Parser parser(argc, argv);
    parser.add_argument("help")
        .takes_value()
        .x_value_range(1, 2)
        .short_name("h");


    return 0;
}