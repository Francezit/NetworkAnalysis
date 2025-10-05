#include "core/core.h"
#include "flowsolver/flowsolver.h"
#include "network/network.h"

using namespace core;
using namespace graph;
using namespace flowsolver;

#pragma Commands

typedef struct
{
    network *network;
    FlowSolver *flowsolver;
    RandomNumberGenerator *random;

} shell_context;

ShellCommandResult command_using(Shell *shell, shell_function_argument &arg)
{
    return SHELL_COMMAND_NOT_FOUND;
}

ShellCommandResult command_network_open(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->network != nullptr)
    {
        delete context->network;
        context->network = nullptr;
    }

    topology topology;
    topology.read(arg.argument);

    network *net = new network(context->random);
    buildNetwork(net, topology);

    context->network = net;
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_network_close(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->network != nullptr)
    {
        delete context->network;
        context->network = nullptr;
    }
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_network_generate(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->network != nullptr)
    {
        delete context->network;
        context->network = nullptr;
    }

    network *net = new network(context->random);

    network_generator_option option;
    for (auto &&pair : *arg.optset)
    {
        if (pair.first == "capacityEdge")
        {
            double temp = stof(pair.second);
            option.capacityEdge = rangedouble(0.5, temp);
        }
        else if (pair.first == "countEdgeForNode")
        {
            int temp = stoi(pair.second);
            option.countEdgeForNode = rangeint(1, temp);
        }
        else if (pair.first == "countLayer")
        {
            option.countLayer = stoi(pair.second);
        }
        else if (pair.first == "countNode")
        {
            option.countNode = stoi(pair.second);
        }
        else if (pair.first == "deltaNodeLayer")
        {
            option.deltaNodeLayer = stoi(pair.second);
        }
        else if (pair.first == "entropy")
        {
            option.entropy = stof(pair.second);
        }
    }

    buildNetwork(net, option);
    context->network = net;

    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_network_save(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->network == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    context->network->getTopology().write(arg.argument);
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_network_fullconnected(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->network == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    int n = context->network->fullconnected();
    *arg.output << "Sono stati aggiunti " << n << " archi" << endl;
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_network_pruning(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->network == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    int n = context->network->pruning();
    *arg.output << "Sono stati rimossi " << n << " archi" << endl;
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_network_export(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->network == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    *arg.output << context->network->generateMATLABScript(false, false, false) << endl;
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_network_print(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->network == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    *arg.output << context->network->print() << endl;
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_flowsolver_create(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->network == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }
    if (context->flowsolver != nullptr)
    {
        delete context->flowsolver;
        context->flowsolver = nullptr;
    }

    int m = stoi(arg.argument);
    FlowSolverMethod method = static_cast<FlowSolverMethod>(m);

    FlowSolver *solver = create_solver(method);

    solver->setTopology(context->network->getTopology());
    solver->setArguments(*arg.optset);
    context->flowsolver = solver;
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_flowsolver_run(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->flowsolver == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    context->flowsolver->solver();
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_flowsolver_dispose(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->flowsolver != nullptr)
    {
        delete context->flowsolver;
        context->flowsolver = nullptr;
    }
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_flowsolver_input(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->flowsolver == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    context->flowsolver->printInput(*arg.output);
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_flowsolver_output(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->flowsolver == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    context->flowsolver->printOutput(*arg.output);
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_flowsolver_reset(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->flowsolver == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    context->flowsolver->reset();
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_flowsolver_best(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->flowsolver == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    *arg.output << context->flowsolver->getBestSolution()->toString();
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_flowsolver_export(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->flowsolver == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    *arg.output << context->flowsolver->getNetwork()->generateMATLABScript(true, true, false);
    return SHELL_COMMAND_SUCCESS;
}

ShellCommandResult command_flowsolver_statistic(Shell *shell, shell_function_argument &arg)
{
    shell_context *context = (shell_context *)arg.context;
    if (context->flowsolver == nullptr)
    {
        return SHELL_COMMAND_ERROR;
    }

    string name;
    if (arg.state->repeat_mode)
    {
        name = to_string(arg.state->repeat_count) + "_" + arg.argument + ".csv";
    }
    else
    {
        name = arg.argument + ".csv";
    }

    ofstream file(name);
    if (file.is_open())
    {
        context->flowsolver->printStatistic(file);
        file.close();
        *arg.output << "Saved" << endl;
        return SHELL_COMMAND_SUCCESS;
    }
    else
    {
        return SHELL_COMMAND_ERROR;
    }
}

Shell *create_shell(shell_context *context)
{
    Shell *shell = new Shell(nullptr, context);
    shell->addCommand("using", command_using);

    shell->addCommand("network_load", command_network_open);
    shell->addCommand("network_close", command_network_close);
    shell->addCommand("network_generate", command_network_generate);
    shell->addCommand("network_save", command_network_save);
    shell->addCommand("network_fullconnected", command_network_fullconnected);
    shell->addCommand("network_pruning", command_network_pruning);
    shell->addCommand("network_export", command_network_export);
    shell->addCommand("network_print", command_network_print);

    shell->addCommand("flowsolver_create", command_flowsolver_create);
    shell->addCommand("flowsolver_run", command_flowsolver_run);
    shell->addCommand("flowsolver_dispose", command_flowsolver_dispose);
    shell->addCommand("flowsolver_input", command_flowsolver_input);
    shell->addCommand("flowsolver_output", command_flowsolver_output);
    shell->addCommand("flowsolver_reset", command_flowsolver_reset);
    shell->addCommand("flowsolver_statistic", command_flowsolver_statistic);
    shell->addCommand("flowsolver_best", command_flowsolver_best);
    return shell;
}

#pragma endregion

map<char, string> parseArgument(int argc, char *argv[])
{
    int opt;
    int temp1, temp2;

    map<char, string> optset;
    int i = 1;
    char key = 0;
    while (i < argc)
    {
        char *cmd = argv[i++];
        if (cmd[0] == '-')
        {
            if (key != 0)
            {
                optset[key] = "";
            }
            key = cmd[1];
            optset[key] = "";
        }
        else if (key != 0)
        {
            string line(cmd);
            optset[key] = line;
            key = 0;
        }
    }
    return optset;
}

int main(int argc, char *argv[])
{
    try
    {
        cout << "Solver start" << endl;

        map<char, string> global_optset = parseArgument(argc, argv);

        shell_context *context = new shell_context();
        context->flowsolver = nullptr;
        context->network = nullptr;
        context->random = new RandomNumberGenerator();

        Shell *shell = create_shell(context);

        string script_file;
        if (global_optset.size() > 0)
        {
            for (auto &&pair : global_optset)
            {
                char opt = pair.first;
                string &optarg = pair.second;

                switch (opt)
                {
                case 'f':
                    script_file = optarg;
                    break;
                }
            }
        }

        if (script_file.empty())
        {
            return shell->console();
        }
        else
        {
            shell->open(script_file);
            return shell->console();
        }
    }
    catch (const char *err)
    {
        cout << err << endl;
        cout << errno << endl;
    }
    catch (int e)
    {
        cout << "Error" << endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        cout << e.what() << endl;
    }
    getchar();
    return EXIT_FAILURE;
}
