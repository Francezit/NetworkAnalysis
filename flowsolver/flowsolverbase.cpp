#include "flowsolverbase.h"

using namespace flowsolver;
using namespace core;
using namespace graph;

FlowSolver::FlowSolver()
{
    this->_random = new core::RandomNumberGenerator();
    this->_network = new graph::network(_random);
    this->_state = SOLVER_INIT;
    this->_current_interation = 0;
    this->_enable_log = false;
    this->_end_timestamp = GET_EMPTY_TIMESTAMP;
    this->_start_timestamp = GET_EMPTY_TIMESTAMP;
    this->_best_result = nullptr;
}

FlowSolver::~FlowSolver()
{
    if (_logger.is_open())
        _logger.close();
}

void FlowSolver::internalReset()
{
    this->_network->reset();
    this->_state = SOLVER_READY;
    this->clearStatistics();
    this->_end_timestamp = GET_EMPTY_TIMESTAMP;
    this->_start_timestamp = GET_EMPTY_TIMESTAMP;
    this->_best_result = nullptr;
    this->_current_interation = 0;
}

void FlowSolver::clearStatistics()
{
    for (auto &&i : this->_statistics)
    {
        delete i;
    }
    this->_statistics.clear();
}

void FlowSolver::printStatisticInternal(ostream &out, bool print_mutation, bool print_topology, bool print_generic)
{
    out << "Interation;Time (ms);Best Fitness;Increment Best Fitness(%);Avg Fitness;Discarted";
    if (print_topology)
        out << ";RemoveEdges;CountEdges;CountFlows";
    if (print_mutation)
        out << ";Mutation Probability;Mutation Count";
    out << ";Solution";
    if (print_generic)
        out << ";Alfa;Beta;Gamma";
    out << endl;

    int collection_size = this->_statistics.size();

    for (int i = 0; i < collection_size; i++)
    {
        statistic *stat = this->_statistics.at(i);
        double deltaFitness = 0;
        double deltaTime = 0;

        if (collection_size > 1)
        {
            if (i > 1)
            {
                statistic *last = this->_statistics.at(i - 1);

                deltaFitness = last->fitness != 0 ? ((stat->fitness - last->fitness) / (double)last->fitness) : 0;
            }
        }

        TIMESTAMP_ELAPSED_MS(stat->start_timestamp, stat->end_timestamp, deltaTime);

        out << core::to_string_format(stat->interations)
            << ";" << core::to_string_format(deltaTime)
            << ";" << core::to_string_format(stat->fitness)
            << ";" << core::to_string_format(deltaFitness)
            << ";" << core::to_string_format(stat->avg_fitness)
            << ";" << core::to_string_format(stat->invalid);
        if (print_topology)
        {
            out
                << ";" << core::to_string_format(stat->count_edges < this->_network->countEdges())
                << ";" << core::to_string_format(stat->count_edges)
                << ";" << core::to_string_format(stat->count_nodes);
        }
        if (print_mutation)
        {
            out
                << ";" << core::to_string_format(stat->mutation_probability)
                << ";" << core::to_string_format(stat->mutation_count);
        }
        out << ";" << stat->output->toString(true);
        if (print_generic)
        {
            out
                << ";" << core::to_string_format(stat->alfa)
                << ";" << core::to_string_format(stat->beta)
                << ";" << core::to_string_format(stat->gamma);
        }
        out << endl;
    }
}

string to_string(statistic *source)
{
    stringstream ss;
    ss << "Interation: " << core::to_string_format(source->interations) << endl
       << "AVG Fitness: " << core::to_string_format(source->avg_fitness) << endl
       << "Fitness: " << core::to_string_format(source->fitness) << endl
       << "Invalid count: " << core::to_string_format(source->invalid) << endl
       << "Mutation rate: " << core::to_string_format(source->mutation_probability) << endl
       << "Mutation count: " << core::to_string_format(source->mutation_count) << endl
       << "Count edges: " << core::to_string_format(source->count_edges) << endl
       << "Count nodes: " << core::to_string_format(source->count_nodes) << endl
       << "Alfa: " << core::to_string_format(source->alfa) << endl
       << "Beta: " << core::to_string_format(source->beta) << endl
       << "Gamma: " << core::to_string_format(source->gamma) << endl
       << "BEST" << endl
       << source->output->toString() << endl;
    return ss.str();
}
