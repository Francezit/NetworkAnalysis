#pragma once
#ifndef _H_SHL
#define _H_SHL

#include "includes.h"
#include "utility.h"

namespace core
{

    enum ShellCommandResult
    {
        SHELL_COMMAND_EXIT = -1,
        SHELL_COMMAND_IGNORE = 0,
        SHELL_COMMAND_ERROR = 1,
        SHELL_COMMAND_NOT_FOUND = 2,
        SHELL_COMMAND_SUCCESS = 10,
    };

    typedef struct s_shell_state
    {
        int repeat_count = 0;
        int repeat_max = 1;
        int repeat_index = 0;
        bool repeat_mode = false;
        int command_index = 0;
        bool enable_printing = true;
        bool ignore = false;

    } shell_executive_state;

    string createStringByPattern(string pattern, map<string, string> &data, char keySign);

    typedef struct
    {
        shell_executive_state *state;
        map<string, string> *optset;
        ostream *output;
        string argument;
        void *context;

    } shell_function_argument;

    class Shell
    {
    public:
        typedef ShellCommandResult (*shell_function)(Shell *shell, shell_function_argument &argument);

    private:
        map<string, string> _optset;
        unordered_map<string, shell_function, hash<string>> _commands;
        ostream *_output;
        void *_context;

    public:
        Shell(ostream *output, void *context);
        ~Shell();

        void addCommand(string name, shell_function function);
        void removeCommand(string &name);
        void clearCommands();

        inline void setVariable(string &name, string &value)
        {
            _optset[name] = value;
        }
        inline string getVariable(string &name)
        {
            return _optset[name];
        }
        inline bool containsVariable(string &name)
        {
            return CONTAINS_MAP(_optset, name);
        }
        inline void removeVariable(string &name)
        {
            _optset.erase(name);
        }
        inline void clearVariables()
        {
            _optset.clear();
        }

        int console();
        ShellCommandResult open(string &filename);
        ShellCommandResult invoke(string &command, bool enable_printing = false);

        inline string formatedString(string &original)
        {
            return createStringByPattern(original, this->_optset, '$');
        }

    private:
        shell_function findCommand(string &name);
        ShellCommandResult invokeInternal(string &command, shell_executive_state *state);
        ShellCommandResult openInternal(string &filename, shell_executive_state *executive_state);

    private:
        friend ShellCommandResult start_comment(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult end_comment(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult set_option(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult remove_option(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult clear_option(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult system_cls(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult system_wait(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult system_exit(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult system_clear(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult system_repeat(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult system_end_repeat(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult system_print(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult system_help(Shell *shell, shell_function_argument &arg);
        friend ShellCommandResult system_open(Shell *shell, shell_function_argument &arg);
    };
}
#endif