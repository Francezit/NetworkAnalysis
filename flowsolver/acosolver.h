#pragma once
#ifndef _H_ACOS
#define _H_ACOS

#include "flowsolverbase.h"

namespace flowsolver
{
    typedef struct s_aco_option
    {
        double pheromone = 5;
        int max_interations = 100;
        int colony_size = 100;
        double alfa = 3.0;
        double beta1 = 5.0;
        double beta2 = 5.0;
        double rho = 0.5;
        double ni = 1;
        bool remove_edge = false;
        double remove_edge_probs = 0.2;
        bool advanced_log = false;

    } AcoOption;

    class AcoSolution : public Solution
    {
    private:
        double *_flows = nullptr;
        int _count_edges = 0;
        double _max_flow = 0;

    public:
        inline AcoSolution() {}

        inline AcoSolution(const AcoSolution &s)
        {
            this->_count_edges = s._count_edges;
            this->_max_flow = s._max_flow;
            this->_flows = new double[s._count_edges];
            for (int i = 0; i < s._count_edges; i++)
            {
                this->_flows[i] = s._flows[i];
            }
        }

        inline ~AcoSolution()
        {
            delete[] this->_flows;
        }

        string toString(bool compact);
        inline bool isBest(Solution *solution)
        {
            return this->_max_flow > ((AcoSolution *)solution)->_max_flow;
        }
        inline double getFitness()
        {
            return _max_flow;
        }

        void extract(graph::network *network);
    };

    class ACOSolver : public FlowSolver
    {
    private:
        AcoOption _option;

    public:
        ACOSolver(/* args */);

        inline AcoOption &getOption()
        {
            return this->_option;
        }

        void dispose();

        void reset();

        void solver();

        void setArguments(map<string, string> &optset);

        void printInput(ostream &stream);

        void printOutput(ostream &stream);

        inline void printStatistic(ostream &stream)
        {
            printStatisticInternal(stream, true, false, true);
        }
    };
}
#endif