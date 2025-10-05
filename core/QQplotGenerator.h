#pragma once
#ifndef _H_TTP
#define _H_TTP

#include "includes.h"
#include "utility.h"

namespace core
{

    typedef struct
    {
        double fitness_upperbound;
        int index;
        int argc;
        char **argv;
        bool enable_statistic;

        double fitness_output;
        int iteration_output;

    } executive_state;

    typedef void (*solver_function)(executive_state *state);

    class QQplotGenerator
    {
    private:
        solver_function function;

        int runs_count;
        string statistic_fullname;
        int run_argc;
        char **run_argv;
        double run_maxfintess;
        bool enable_statistic = false;

    public:
        QQplotGenerator(solver_function function, int argc, char **argv);
        ~QQplotGenerator();

        void runscript(string &filename);

    protected:
        void executive();
    };
}
#endif