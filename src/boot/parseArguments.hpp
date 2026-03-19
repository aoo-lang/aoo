#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <CLI/CLI.hpp>

#include "../util/platform.hpp"
#include "Arguments.hpp"

namespace AOO {
    using std::cout, std::cerr, std::cin, std::vector, std::string, CLI::App, CLI::Option, CLI::CallForHelp, CLI::CallForVersion, CLI::ParseError;

    [[nodiscard]] inline Arguments parseArguments(int argc, char** argv) noexcept {
    #if AOO_PLATFORM_WINDOWS
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif
        App app;
        app.set_config();
        app.set_help_all_flag();
        app.allow_extras(false);
        app.allow_config_extras(false);
        app.allow_non_standard_option_names(true);
        app.allow_windows_style_options(false);

        app.set_version_flag("-v,--version", "0.0.1", "Display program version information and exit.");
        app.set_help_flag("-h,--help", "Show this help message and exit.");

        bool debugMode = false;
        app.add_flag("--debug", debugMode, "Enable debug mode.")->default_val("false");

        string dump_lexer_tokens_dummy;
        const Option& dump_lexer_tokens_Option = *app.add_option("--dump-lexer-tokens", dump_lexer_tokens_dummy, "Dump Lexer tokens. Specify `[FILE]` to dump to a file, or to stdout, if not.")->expected(0, 1)->type_name("[FILE]");

        string inputFile;
        app.add_option("inputFile", inputFile, "File to compile.")->type_name("[FILE]");

        app.footer("© 2026 LJM12914.\nAOO Compiler is licensed under the MIT License.\nhttps://github.com/aoo-lang/aoo");

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

        Arguments result = {
            .debugMode = debugMode,
        };

        if (debugMode) {
            cout << "Debug mode enabled.\nSpecify input file: ";
            string inputFile2;
            cin >> inputFile2;
            if (inputFile2.empty()) {
                cerr << "Error: No input file specified.\n";
                exit(1000);
            }
            result.inputFile = inputFile2;
        }
        else if (inputFile.empty()) {
            cerr << "Error: No input file specified.\n";
            exit(1000);
        }

        if (dump_lexer_tokens_Option.count() == 0) result.printLexerTokens = false;
        else {
            const vector<string>& optionValues = dump_lexer_tokens_Option.results();
            result.printLexerTokens = true;
            if (!optionValues.empty()) result.printLexerTokensFile = optionValues[0];
        }

        return result;
    }
}