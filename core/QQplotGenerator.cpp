#include "QQplotGenerator.h"

using namespace core;

QQplotGenerator::QQplotGenerator(solver_function function, int argc, char **argv)
{
    this->function = function;
    this->run_argc = argc;
    this->run_argv = argv;
}

QQplotGenerator::~QQplotGenerator()
{
    this->run_argv = nullptr;
}

void QQplotGenerator::runscript(string &filename)
{
    ifstream file(filename);
    if (file.is_open())
    {
        string line;
        bool ignore = false;
        //assegna gli archi e calcola il numero dei nodi
        while (getline(file, line))
        {
            cout << line << endl;
            if (line.empty() || line[0] == '#')
                continue;

            if (line[0] == '/' && line[1] == '*')
                ignore = true;
            else if (line[0] == '*' && line[1] == '/')
                ignore = false;
            else if (ignore)
                continue;

            auto s = splitCommand(line, ' ');
            string name = std::get<0>(s);
            string value = std::get<1>(s);

            if (name == "runs")
            {
                this->runs_count = stoi(value);
            }
            else if (name == "statistic_fullname")
            {
                this->statistic_fullname = value;
            }
            else if (name == "maxfitness")
            {
                this->run_maxfintess = stof(value);
            }
            else if (name == "enable_statistic")
            {
                this->enable_statistic = true;
            }
            else if (name == "wait")
            {
                getchar();
            }
            else if (name == "start")
            {
                executive();
            }
        }
    }
}

void QQplotGenerator::executive()
{
    {
        ofstream file(statistic_fullname);
        if (!file.is_open())
            throw "impossibile aprire il file per la scrittura";
        file << "Index;Time;Fitness Upperbound;Fitness;Age;IsValid;" << endl;
        file.close();
    }

    int i = 0;
    int count = this->runs_count;
    while (i < count)
    {
        executive_state state;
        state.index = i++;
        state.fitness_upperbound = this->run_maxfintess;
        state.argc = this->run_argc;
        state.argv = this->run_argv;
        state.enable_statistic = this->enable_statistic;

        cout << endl
             << "----------------------------------" << endl
             << "[START] ESECUZIONE RUN " << i << "/" << count << endl;

        TIMESTAMP start = GET_CURRENT_TIMESTAMP;
        this->function(&state);
        TIMESTAMP end = GET_CURRENT_TIMESTAMP;

        double exe_time;
        TIMESTAMP_ELAPSED_MS(start, end, exe_time);

        bool valid = state.fitness_output >= state.fitness_upperbound;

        double exe_rest_time = (count - i) * exe_time;

        cout << endl
             << "[END] ESECUZIONE RUN " << i << "/" << count << endl
             << "Fitness: " << state.fitness_output << endl
             << "Upperbound: " << state.fitness_upperbound << endl
             << "Time: " << exe_time << " ms" << endl
             << "Interation: " << state.iteration_output << endl
             << "Valid: " << to_string_format(valid) << endl
             << "Stima: " << to_string_format((exe_rest_time / 1000.0) / 60.0) << " min" << endl
             << "----------------------------------" << endl;

        {
            ofstream file(statistic_fullname, std::ios_base::app);
            if (!file.is_open())
            {
                cout << "Errore di scrittura"
                     << to_string_format(state.index) << ";"
                     << to_string_format(exe_time) << ";"
                     << to_string_format(state.fitness_upperbound) << ";"
                     << to_string_format(state.fitness_output) << ";"
                     << to_string_format(state.iteration_output) << ";"
                     << to_string_format(valid) << ";"
                     << endl;
            }
            else
            {
                file << to_string_format(state.index) << ";"
                     << to_string_format(exe_time) << ";"
                     << to_string_format(state.fitness_upperbound) << ";"
                     << to_string_format(state.fitness_output) << ";"
                     << to_string_format(state.iteration_output) << ";"
                     << to_string_format(valid) << ";"
                     << endl;
                file.close();
            }
        }

        if (!valid)
        {
            count++;
        }
    }
}
