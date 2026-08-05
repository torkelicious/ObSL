#pragma once

#include <deque>
#include <memory>
#include <vector>

#include <ObSL/Interpreter.h>

namespace ObSL {
    class ScriptEntry {
    public:
        ScriptEntry() = default;

        ~ScriptEntry() = default;

        ScriptEntry(const ScriptEntry &) = delete;

        ScriptEntry &operator=(const ScriptEntry &) = delete;

        int exec(int argc, char *argv[]);

    private:
        Interpreter m_interpreter;

        // the interpreter does not own the ASTs it executes !!!
        std::deque<std::string> m_repl_sources;
        std::vector<std::vector<std::unique_ptr<Stmt> > > m_repl_asts;

        // everything should go through exec()
        void runFile(const std::string &path);

        void runREPL();

        void run(const std::string &source, bool is_repl = false);

        static void run_lint(const std::string &path);
    };
} // ObSL
