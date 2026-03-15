#pragma once
#include <iostream>
#include <string>
#include <CLI/CLI.hpp>

#include "../meta.hpp"

namespace AO {
    using std::cout, std::cerr, std::string, CLI::App, CLI::CallForHelp, CLI::CallForVersion, CLI::ParseError;

    struct Arguments {
        bool printTokens, printAST;
        string inputFile;
    };

    [[nodiscard]] inline Arguments parseArguments(int argc, char** argv) noexcept {
        App app;
        app.set_config();
        app.set_help_all_flag();
        app.allow_extras(false);
        app.allow_config_extras(false);
        app.allow_non_standard_option_names(true);
        app.allow_windows_style_options(false);
        app.set_version_flag("-v,--version", AO_VERSION, "Display program version information and exit.");
        app.set_help_flag("-h,--help", "Show this help message and exit.");

        bool printTokens = false;
        app.add_flag("-pt", printTokens, "Print tokens after lexing.");

        bool printAST = false;
        app.add_flag("-pa", printAST, "Print AST after parsing.");

        string inputFile;
        app.add_option("inputFile", inputFile, "Specify the input file.")->required()->check(CLI::ExistingFile);

        try { app.parse(argc, argv); }
        catch(const CallForHelp& e) {
            cout << app.help() << '\n';
            exit(e.get_exit_code());
        }
        catch(const CallForVersion& e) {
            cout << e.what() << '\n';
            exit(e.get_exit_code());
        }
        catch(const ParseError& e) {
            cerr << e.what() << '\n';
            exit(e.get_exit_code());
        }
        return {
            .printTokens = printTokens,
            .printAST = printAST,
            .inputFile = inputFile
        };
    }
}