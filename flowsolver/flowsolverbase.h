#pragma once
#ifndef _H_MPS
#define _H_MPS

#include "..\core\core.h"
#include "..\network\network.h"

namespace flowsolver
{
    enum SolverState
    {
        SOLVER_INIT,
        SOLVER_READY,
        SOLVER_COMPUTING,
        SOLVER_COMPUTED
    };

#define SOLVER_START_LOG(op) \
    if (enable_log)          \
    cout << endl             \
         << "START: " op << endl
#define SOLVER_POPULATION_LOG \
    if (enable_log)           \
    printOutput()
#define SOLVER_END_LOG(op) \
    if (enable_log)        \
    cout << "END: " << op << endl
#define EXECUTIVE_LOGGER(cmd, in_console) \
    {                                     \
        stringstream __ss;                \
        __ss << cmd;                      \
        print(__ss.str(), in_console);    \
    }
#define EXECUTIVE_LOGGER_CONSOLE(cmd) \
    {                                 \
        cout << cmd;                  \
    }

    class Solution
    {
    public:
        virtual double getFitness() = 0;
        virtual string toString(bool compact = false) = 0;
        virtual bool isBest(Solution *solution) = 0;
    };

    typedef struct s_statistic
    {
        double fitness = 0;
        double avg_fitness = 0;
        int interations = 0;
        TIMESTAMP end_timestamp;
        TIMESTAMP start_timestamp;
        Solution *output = nullptr;
        int count_edges = 0;
        int count_nodes = 0;
        double mutation_probability = 0;
        int mutation_count = 0;
        double alfa = 0;
        double beta = 0;
        double gamma = 0;
        int invalid = 0;

        inline ~s_statistic()
        {
            if (output != nullptr)
                delete output;
            output = nullptr;
        }

    } statistic;

    string to_string(statistic *stat);

    class FlowSolver
    {
    protected:
        SolverState _state;
        core::RandomNumberGenerator *_random;

        //Input
        graph::network *_network;
        bool _enable_log;
        ofstream _logger;

        vector<statistic *> _statistics;
        int _current_interation;
        TIMESTAMP _start_timestamp;
        TIMESTAMP _end_timestamp;

        //Output
        statistic *_best_result;

    public:
        FlowSolver();
        ~FlowSolver();

        inline SolverState getState() const
        {
            return this->_state;
        }

        inline core::RandomNumberGenerator *getRandomGenerator()
        {
            return this->_random;
        }

        inline void setTopology(graph::topology &topology)
        {
            if (this->_state != SOLVER_READY)
                throw "error";
            buildNetwork(this->_network, topology);
        }

        inline void setLogFile(string &filename)
        {
            if (_logger.is_open())
                _logger.close();
            _logger.open(filename, ios::out | ios::app);
            _logger << endl
                    << endl
                    << "NEW INSTANCE" << endl
                    << "----------------------" << endl;
        }

        inline graph::network *getNetwork()
        {
            return this->_network;
        }

        virtual void setArguments(map<string, string> &optset) = 0;

        virtual void solver() = 0;

        virtual void reset() = 0;

        virtual void printInput(ostream &stream) = 0;

        virtual void printOutput(ostream &stream) = 0;

        virtual void printStatistic(ostream &stream) = 0;

        virtual void dispose() = 0;

        inline double getExecutionTime()
        {
            if (this->_state == SOLVER_COMPUTED)
            {
                double ret;
                TIMESTAMP_ELAPSED_MS(this->_start_timestamp, this->_end_timestamp, ret);
                return ret;
            }
            return NAN;
        }

        inline Solution *getBestSolution()
        {
            return this->_best_result->output;
        }

        inline void printOutput()
        {
            printOutput(cout);
        }

        inline void printInput()
        {
            printInput(cout);
        }

        inline void print(const string &s, bool in_console)
        {
            if (in_console)
                cout << s << endl;

            if (_enable_log && _logger.is_open())
                _logger << s << endl;
        }

    protected:
        void internalReset();
        void clearStatistics();
        void printStatisticInternal(ostream &stream, bool print_mutation, bool print_topology, bool print_generic);
    };
}
#endif