// pure0.cpp - C++17
//
// a micro language
// Copyright (c) 2026 - 2030 Carmine-Emanuele Cella. All rights reserved.
// Use of this source code is governed by a BSD-style 2

#include "core.h"
#include "dsp.h"

int main(int argc, char** argv) {
    srand(static_cast<unsigned int>(std::time(nullptr)));
    auto env = make_environment();
    add_dsp(env);
    try {
        if (argc > 1) {
            for (int i = 1; i < argc; ++i) {
                std::ifstream in(argv[i]);
                if (!in) {
                    std::cerr << "cannot open input " << argv[i] << "\n";
                    continue;
                }
                TokenStream ts{in};
                while (!ts.eof()) (void)eval(parse_expr(ts), env);
            }
        } else {
             std::cout << "[pure0, v0.2]\n\n";
             std::cout << "Copyright (c) 2026 - 2030 Carmine-Emanuele Cella. All rights reserved.\n\n";
             repl(env);
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

// eof

