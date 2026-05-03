#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <CLI/CLI.hpp>

#include "../manul/splashes.hpp"
#include "../util/platform.hpp"
#include "Arguments.hpp"

namespace AOO {
    using std::cout, std::cerr, std::cin, std::vector, std::string, CLI::App, CLI::Option, CLI::CallForHelp, CLI::CallForVersion, CLI::ParseError, CLI::IsMember;

    [[nodiscard]] inline Arguments parseArguments(int argc, char** argv) noexcept {
    #if AOO_PLATFORM_WINDOWS
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif

    #if 0
        for (int i = 0; i < argc; i++) cout << "Argument " << i << ": " << argv[i] << '\n';
    #endif

        App app;
        app.set_config();
        app.set_help_all_flag();
        app.allow_extras(false);
        app.allow_config_extras(false);
        app.allow_non_standard_option_names(true);
        app.allow_windows_style_options(false);
        
        app.set_version_flag("-v,--version", "0.0.1", "Display version and exit.");
        app.set_help_flag("-h,--help", "Show this help message and exit.");

        vector<string> include_files;
        app.add_option("-I,--include", include_files, "Specify files with modules to include.")->type_name("[FILE_PATH]");

        vector<string> include_paths;
        app.add_option("-L,--include-path", include_paths, "Specify paths to search for included files.")->type_name("[DIR_PATH]");

        string dump_kind;
        const Option& dump_lexer_tokens_Option = *app.add_option("--dump", dump_kind, "Dump Lexer tokens.")->type_name("[DUMP_KIND]")->check(CLI::IsMember({"lexer", "ast", "ir", "asm"}));

        string dump_to;
        app.add_option("--dump-to", dump_to, "Specify the file path to dump the output of --dump. If not, the output will be dumped to stdout.")->type_name("[FILE_PATH]");

        string literal_code;
        app.add_option("--literal-code", literal_code, "Provide the literal code to compile.")->type_name("[CODE]");

        string input_file_path;
        app.add_option("input_file_path", input_file_path, "File to compile.")->type_name("[FILE_PATH]");

        string support_file_path;

        app.footer(string("© 2026 LJM12914. Licensed under the MIT License.\n") + AOO::Manul::getSplash() + "\nhttps://github.com/aoo-lang/aoo");

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
            .debug = {
                .dump_kind = [&dump_kind]() {
                    using enum Arguments::Debug::DumpKind;
                    if (dump_kind == "lexer") return Lexer;
                    else if (dump_kind == "ast") return AST;
                    else if (dump_kind == "ir") return IR;
                    else if (dump_kind == "asm") return ASM;
                    else return None;
                }(),
                .dump_to = dump_to
            },
            .input_file_path = input_file_path,
            .literal_code = literal_code,
            .include_files = include_files,
            .include_paths = include_paths
        };

        if (result.input_file_path.empty() && result.literal_code.empty()) {
            cerr << "Error: No input file or literal code specified.\n";
            exit(1000);
        }

        return result;
    }
}