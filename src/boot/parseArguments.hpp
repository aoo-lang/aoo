#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <CLI/CLI.hpp>

#include "Arguments.hpp"

namespace AOO {
    using std::cout, std::cerr, std::vector, std::string, CLI::App, CLI::Option, CLI::CallForHelp, CLI::CallForVersion, CLI::ParseError;

    [[nodiscard]] inline Arguments parseArguments(int argc, char** argv) noexcept {
        App app;
        app.set_config();
        app.set_help_all_flag();
        app.allow_extras(false);
        app.allow_config_extras(false);
        app.allow_non_standard_option_names(true);
        app.allow_windows_style_options(false);
        app.set_version_flag("-v,--version", "0.0.1", "Display program version information and exit.");
        app.set_help_flag("-h,--help", "Show this help message and exit.");

        string dump_lexer_tokens_dummy;
        const Option& dump_lexer_tokens_Option = *app.add_option("--dump-lexer-tokens", dump_lexer_tokens_dummy, "Dump Lexer tokens. Use `--dump-lexer-tokens=<file>` to dump to a file (replaces the file). If not specified, tokens will be dumped to stdout.")->expected(0, 1);

        string inputFile;
        app.add_option("inputFile", inputFile, "Specify the input file.")->required()->check(CLI::ExistingFile);

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

        Arguments result;

        if (dump_lexer_tokens_Option.count() == 0) result.printLexerTokens = false;
        else {
            const vector<string>& optionValues = dump_lexer_tokens_Option.results();
            result.printLexerTokens = true;
            if (!optionValues.empty()) result.printLexerTokensFile = optionValues[0];
        }
        result.inputFile = inputFile;

        return result;
    }
}