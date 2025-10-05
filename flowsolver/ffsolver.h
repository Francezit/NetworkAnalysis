#pragma once
#ifndef _H_FFS
#define _H_FFS

#include "flowsolverbase.h"

using namespace flowsolver;
using namespace core;
using namespace graph;

class FFSolution : public Solution
{
private:
    double _flow = 0;

public:
    inline FFSolution() {}

    inline FFSolution(double flow)
    {
        _flow = flow;
    }

    inline FFSolution(const FFSolution &s)
    {
        this->_flow = s._flow;
    }
    inline string toString(bool compact)
    {
        stringstream ss;
        ss << "Max Flow " << _flow;
        return ss.str();
    }
    inline bool isBest(Solution *solution)
    {
        return this->_flow > ((FFSolution *)solution)->_flow;
    }
    inline double getFitness()
    {
        return _flow;
    }

    void extract(network *network);
};

class FFSolver : public FlowSolver
{

public:
    FFSolver();

    void dispose();

    void reset();

    void solver();

    void setArguments(map<string, string> &optset);

    void printInput(ostream &stream);

    void printOutput(ostream &stream);

    inline void printStatistic(ostream &stream)
    {
        printStatisticInternal(stream, false, true, true);
    }
};

#endif