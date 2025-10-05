#include "shell.h"

namespace core
{
    ShellCommandResult start_comment(Shell *shell, shell_function_argument &arg)
    {
        arg.state->ignore = true;
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult end_comment(Shell *shell, shell_function_argument &arg)
    {
        arg.state->ignore = false;
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult set_option(Shell *shell, shell_function_argument &arg)
    {
        vector<string> s = core::split(arg.argument, "=", true);
        if (s.size() == 1)
        {
            (*arg.optset)[s[0]] = "";
        }
        else if (s.size() > 1)
        {
            (*arg.optset)[s[0]] = s[1];
        }
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult remove_option(Shell *shell, shell_function_argument &arg)
    {
        arg.optset->erase(arg.argument);
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult clear_option(Shell *shell, shell_function_argument &arg)
    {
        arg.optset->clear();
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult system_open(Shell *shell, shell_function_argument &arg)
    {
        shell_executive_state state;
        state.enable_printing = false;

        return shell->openInternal(arg.argument, &state);
    }

    ShellCommandResult system_cls(Shell *shell, shell_function_argument &arg)
    {
        arg.state->enable_printing = !arg.state->enable_printing;
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult system_wait(Shell *shell, shell_function_argument &arg)
    {
        getchar();
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult system_exit(Shell *shell, shell_function_argument &arg)
    {
        return SHELL_COMMAND_EXIT;
    }

    ShellCommandResult system_clear(Shell *shell, shell_function_argument &arg)
    {
#if defined _WIN32
        system("cls");
#elif defined(__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
        system("clear");
#elif defined(__APPLE__)
        system("clear");
#endif
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult system_repeat(Shell *shell, shell_function_argument &arg)
    {
        arg.state->repeat_max = stoi(arg.argument);
        arg.state->repeat_index = arg.state->command_index + 1;
        arg.state->repeat_count = 0;
        arg.state->repeat_mode = true;
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult system_end_repeat(Shell *shell, shell_function_argument &arg)
    {
        if (arg.state->repeat_mode)
        {
            arg.state->repeat_count++;
            if (arg.state->repeat_count < arg.state->repeat_max)
            {
                arg.state->command_index = arg.state->repeat_index;
            }
            else
            {
                arg.state->repeat_count = 0;
                arg.state->repeat_max = 0;
                arg.state->repeat_mode = false;
            }
            return SHELL_COMMAND_SUCCESS;
        }
        else
        {
            return SHELL_COMMAND_IGNORE;
        }
    }

    ShellCommandResult system_print(Shell *shell, shell_function_argument &arg)
    {
        *arg.output << arg.argument << endl;
        return SHELL_COMMAND_SUCCESS;
    }

    ShellCommandResult system_help(Shell *shell, shell_function_argument &arg)
    {
        for (auto &&item : shell->_commands)
        {
            *arg.output << item.first << endl;
        }
        return SHELL_COMMAND_SUCCESS;
    }

}

using namespace core;

Shell::Shell(ostream *output, void *context)
{
    if (output == nullptr)
        _output = &cout;
    else
        _output = output;

    _context = context;

    addCommand("/*", start_comment);
    addCommand("*/", end_comment);
    addCommand("set_option", set_option);
    addCommand("remove_option", remove_option);
    addCommand("clear_option", clear_option);
    addCommand("@cls", system_cls);
    addCommand("wait", system_wait);
    addCommand("pause", system_wait);
    addCommand("exit", system_exit);
    addCommand("clear", system_clear);
    addCommand("repeat", system_repeat);
    addCommand("end_repeat", system_end_repeat);
    addCommand("print", system_print);
    addCommand("open", system_open);
    addCommand("help", system_help);
}

Shell::~Shell()
{
    if (_output != nullptr)
    {
        _output->flush();
    }
}

void Shell::addCommand(string name, shell_function function)
{
    if (!CONTAINS_SET(this->_commands, name))
    {
        this->_commands.insert({name, function});
    }
}

void Shell::clearCommands()
{
    this->_commands.clear();
}

void Shell::removeCommand(string &name)
{
    if (CONTAINS_SET(this->_commands, name))
    {
        this->_commands.erase(name);
    }
}

ShellCommandResult Shell::open(string &filename)
{
    shell_executive_state executive_state;
    executive_state.enable_printing = true;
    return openInternal(filename, &executive_state);
}

ShellCommandResult Shell::openInternal(string &filename, shell_executive_state *executive_state)
{
    vector<string> commands;
    ifstream file(filename);
    if (file.is_open())
    {
        string line;
        int line_index = 0;
        while (getline(file, line))
        {
            commands.push_back(line);
        }
        file.close();
    }
    else
    {
        return SHELL_COMMAND_ERROR;
    }

    int commands_size = commands.size();
    while (executive_state->command_index < commands_size)
    {
        string command = commands[executive_state->command_index];
        ShellCommandResult code = invokeInternal(command, executive_state);
        if (code == SHELL_COMMAND_EXIT)
        {
            return SHELL_COMMAND_EXIT;
        }
        else if (code == SHELL_COMMAND_ERROR)
        {
            return SHELL_COMMAND_ERROR;
        }
    }
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult Shell::invokeInternal(string &command, shell_executive_state *state)
{
    try
    {
        if (command[0] == '#' || command[0] == ';' || command.empty())
        {
            state->command_index++;
            return SHELL_COMMAND_IGNORE;
        }

        vector<string> s = split(command, " ", true);
        if (s.size() == 0)
        {
            state->command_index++;
            return SHELL_COMMAND_IGNORE;
        }

        string name = s[0];
        string arg;

        if (s.size() > 1 && s[1][0] != '"')
        {
            arg = s[1];
        }
        else if (s.size() > 1 && s[1][0] == '"')
        {
            stringstream ss;
            for (int i = 1; i < s.size(); i++)
            {
                for (auto &&c : s[i])
                {
                    if (c != '"')
                        ss << c;
                }
                ss << " ";
            }
            arg = ss.str();
        }
        else
        {
            arg = "";
        }

        if (state->enable_printing)
            cout << ">>" << command << endl;

        if (state->ignore)
        {
            state->command_index++;
            return SHELL_COMMAND_IGNORE;
        }
        else if (CONTAINS_MAP(_commands, name))
        {
            state->command_index++;
            shell_function_argument function_argument;
            function_argument.argument = this->formatedString(arg);
            function_argument.output = _output;
            function_argument.state = state;
            function_argument.context = _context;
            return _commands[name](this, function_argument);
        }
        else
        {
            cout << name << " non valido" << endl;
            state->command_index++;
            return SHELL_COMMAND_NOT_FOUND;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        *_output << e.what() << endl;
    }
    catch (const char *err)
    {
        *_output << err << endl;
        *_output << errno << endl;
    }
    catch (int e)
    {
        *_output << "Error" << endl;
    }
    return SHELL_COMMAND_ERROR;
}

ShellCommandResult Shell::invoke(string &command, bool enable_printing)
{
    shell_executive_state state;
    state.enable_printing = enable_printing;

    return invokeInternal(command, &state);
}

int Shell::console()
{
    shell_executive_state state;
    state.enable_printing = false;

    string command;
    while (true)
    {
        cout << "<<";
        std::getline(std::cin, command);
        ShellCommandResult code = invokeInternal(command, &state);
        if (code == SHELL_COMMAND_EXIT)
        {
            return EXIT_SUCCESS;
        }
        else if (code == SHELL_COMMAND_ERROR)
        {
            return EXIT_FAILURE;
        }
    }
}

string createStringByPattern(string pattern, map<string, string> &data, char keySign)
{
    stringstream ss;

    int patterSize = pattern.size();
    int index = 0;
    do
    {
        int startIndex = pattern.find(keySign, index);
        if (startIndex == string::npos)
        {
            break;
        }
        else if (startIndex + 1 >= patterSize)
        {
            return ss.str();
        }

        int endIndex = pattern.find(keySign, startIndex + 1);
        if (startIndex == string::npos)
        {
            return ss.str();
        }

        ss << pattern.substr(index, startIndex - index);

        string key = pattern.substr(startIndex + 1, endIndex - startIndex - 1);
        if (CONTAINS_MAP(data, key))
        {
            ss << data[key];
        }
        else
        {
            ss << keySign << key << keySign;
        }

        index = endIndex + 1;

    } while (index < patterSize);

    ss << pattern.substr(index);
    return ss.str();
}
