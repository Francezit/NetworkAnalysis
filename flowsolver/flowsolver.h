#pragma once
#ifndef _H_FSG
#define _H_FSG

#include "acosolver.h"
#include "immunesolver.h"
#include "ffsolver.h"

namespace flowsolver
{
    enum FlowSolverMethod
    {
        IMMUNE = 0,
        ACO = 1,
        FORD_FUKERSON = 2
    };

    FlowSolver *create_solver(FlowSolverMethod method)
    {
        FlowSolver *solver;
        switch (method)
        {
        case IMMUNE:
            solver = new ImmuneSolver();
            break;
        case ACO:
            solver = new ACOSolver();
            break;
        case FORD_FUKERSON:
            solver = new FFSolver();
            break;
        }
        return solver;
    }
}
#endif