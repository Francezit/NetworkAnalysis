#include "acosolver.h"

using namespace flowsolver;
using namespace core;
using namespace graph;

void AcoSolution::extract(network *network)
{
    if (_flows != nullptr)
        delete[] _flows;

    topology &edges = network->getTopology();
    _count_edges = edges.size();
    _flows = new double[_count_edges];
    _max_flow = network->getFlow();
    for (int i = 0; i < _count_edges; i++)
        _flows[i] = edges.get(i).flow;
}

string AcoSolution::toString(bool compact)
{
    if (compact)
        return to_string_format(_flows, _count_edges);
    else
    {

        stringstream ss;
        ss << "Count edges: " << to_string_format(_count_edges) << endl
           << "\t"
           << "Max flow: " << to_string_format(_max_flow) << endl
           << "\t"
           << "Flows: " << to_string_format(_flows, _count_edges) << endl;
        return ss.str();
    }
}

ACOSolver::ACOSolver(/* args */)
{
}

void ACOSolver::dispose()
{
    this->internalReset();
    delete _network;
}

void ACOSolver::reset()
{
    this->internalReset();
}

network_node *selectNextNode(network *net, network_node *source, network_path &current_path, void *arg)
{
    network_node *dest = nullptr;

    ACOSolver *solver = (ACOSolver *)arg;
    AcoOption &option = solver->getOption();
    RandomNumberGenerator *random = solver->getRandomGenerator();
    int current_size = current_path->size();

    //compute propability
    int n = source->next.size();
    if (n > 0)
    {
        double *scores = new double[n];
        double sum = 0;
        for (int i = 0; i < n; i++)
        {
            network_node *node = source->next[i];
            network_edge &e = net->getEdge({source->id, node->id});
            if (!e.enable)
            {
                scores[i] = 0;
            }
            else if (IS_NODE_TARGET(node->id) && !e.isSaturated())
            {
                scores[i] = DBL_MAX;
            }
            else
            {
                //double distance_score = 1;
                //if (node->distance_heuristic >= 0)
                //{
                //    distance_score = (node->distance_heuristic + current_size);
                //}

                scores[i] = pow(e.tau, option.alfa) * pow(e.getResidualCapacity(), option.beta1) / pow((double)e.counter, option.beta2);
                //scores[i] = ((pow(e.tau, option.alfa) + 1.0) * pow(e.getResidualCapacity(), option.beta)) / exp((e.counter + 1.0) / option.gamma);
                //scores[i] = (pow(e.tau, option.alfa) * pow(INVD(e.getResidualCapacity()), option.beta)) / (1.0 + (double)e.counter);
            }

            sum += scores[i];
        }

        if (sum > 0)
        {
            divAll<double>(scores, n, sum);

            int next_node_index = random->argmax(scores, n, false);
            dest = source->next.at(next_node_index);

            if (option.advanced_log)
            {
                stringstream ss;
                ss << endl
                   << "size: " << to_string_format(current_size) << endl;
                for (int i = 0; i < n; i++)
                {
                    network_edgekey key(source->id, source->next[i]->id);
                    ss << "edge: " << to_string(key) << endl
                       << "prob: " << to_string_format(scores[i]) << endl
                       << "distance: " << to_string_format(dest->distance_heuristic) << endl
                       << "is choose: " << to_string_format(next_node_index == i) << endl
                       << to_string(net->getEdge(key)) << endl;
                }
                cout << ss.str() << endl;
            }
        }

        delete[] scores;
    }
    return dest;
}

void ACOSolver::solver()
{

    if (this->_state != SOLVER_READY)
        throw "state is not valid";

    this->_start_timestamp = GET_CURRENT_TIMESTAMP;
    this->_state = SOLVER_COMPUTING;

    int max_size_path = this->_network->countEdges() * 0.6;
    double flow_upperbound = this->_network->getFlowUpperbound();
    int colony_size = this->_option.colony_size;
    unordered_set<network_edgekey, network_edgekey_hasher> visited_edges;

    _network->reset(false, false, true, true, true, _option.pheromone / 2.0);

    while (this->_current_interation < this->_option.max_interations &&
           (this->_best_result == nullptr || this->_best_result->fitness < flow_upperbound))
    {
        statistic *stat = new statistic();
        this->_statistics.push_back(stat);

        stat->start_timestamp = GET_CURRENT_TIMESTAMP;

        network_path min_path = emptyPath;
        //fa navigare una formica alla volta
        for (int k = 0; k < colony_size; k++)
        {
            //un ant naviga per la rete di flusso
            network_path l = this->_network->navigate(selectNextNode, this, max_size_path);
            int size_path = l->size();
            for (int i = 0; i < size_path - 1; i++)
            {
                network_edgekey e = {l->at(i), l->at(i + 1)};
                visited_edges.insert(e);
            }

            //this->_network->updateHeuristicDistance(l);

            if (isValidPath(l))
            {
                if (!isValidPath(min_path) || size_path < min_path->size())
                {
                    min_path = l;
                }
                else
                {
                    deletePath(l);
                }
            }
            else
            {
                deletePath(l);
                stat->invalid++;
            }
        }

        //aggiorno il ferormone
        int edges_traversed = 0, edges_not_traversed = 0;
        for (auto &&edge : _network->getTopology())
        {
            double delta_tau = 0;
            if (CONTAINS_SET(visited_edges, edge.first))
            {
                delta_tau = (double)colony_size * _option.pheromone / (double)edge.second.counter;
                edges_traversed++;
            }
            else
            {
                edges_not_traversed++;
            }

            edge.second.tau = ((1 - _option.rho) * edge.second.tau) + delta_tau;
        }

        //calcolo il cammino incrementale
        network_path augmenting_path = this->_network->normalizePath(min_path);
        double increment_flow = 0;
        if (augmenting_path != emptyPath)
        {
            //aggiorno il flusso lungo il cammino incrementale
            double max_increment_flow = this->_network->getMinResidualCapacity(augmenting_path);
            if (max_increment_flow > 0)
            {
                increment_flow = max_increment_flow * _option.ni;
                this->_network->updateFlow(augmenting_path, increment_flow);
                stat->alfa += increment_flow;
            }
        }
        else
        {
            stat->beta = 1;
        }

        stat->end_timestamp = GET_CURRENT_TIMESTAMP;

        //ottengo il flusso
        double current_flow = this->_network->getFlow();

        EXECUTIVE_LOGGER_CONSOLE(endl
                                 << "Iteration: " << _current_interation << endl
                                 << "Best Path: " << graph::to_string(min_path) << endl
                                 << "Invalid Path: " << std::to_string(stat->invalid) << endl
                                 << "Augmenting Path: " << graph::to_string(augmenting_path) << endl
                                 << "Edges traversed: " << std::to_string(edges_traversed) << endl
                                 << "Edges not traversed: " << std::to_string(edges_not_traversed) << endl
                                 << "All edges: " << std::to_string(this->_network->countEdges()) << endl
                                 << "Increment flow: " << core::to_string(increment_flow, 5) << endl
                                 << "Total flow: " << core::to_string(current_flow, 5) << endl);

        deletePath(min_path);
        deletePath(augmenting_path);

        stat->fitness = current_flow;
        stat->avg_fitness = current_flow / colony_size;
        stat->interations = this->_current_interation;
        /*{
            int removed_edges = 0;
            if (this->_option.remove_edge)
            {
                this->_network->enableAllEdges();
                for (auto &&edge : visited_edges)
                {
                    if (this->_random->prob(_option.remove_edge_probs))
                    {
                        if (this->_network->disableEdge(edge))
                        {
                            removed_edges++;
                        }
                    }
                }
            }

            stat->count_edges = this->_network->countEdges() - removed_edges;
            stat->count_nodes = this->_network->countNodes();
        }*/
        stat->count_edges = this->_network->countEdges();
        stat->count_nodes = this->_network->countNodes();

        AcoSolution *solution = new AcoSolution();
        solution->extract(this->_network);
        stat->output = solution;

        this->_current_interation++;
        if (this->_best_result == nullptr || stat->fitness > this->_best_result->fitness)
        {
            this->_best_result = stat;
        }

        visited_edges.clear();

        EXECUTIVE_LOGGER(endl << "Statistiche" << endl
                              << to_string(stat),
                         false);
    }

    this->_end_timestamp = GET_CURRENT_TIMESTAMP;
    this->_state = SOLVER_COMPUTED;

    double executive_time;
    TIMESTAMP_ELAPSED_MS(this->_start_timestamp, this->_end_timestamp, executive_time);
    EXECUTIVE_LOGGER("Total time: " << executive_time << "ms", true);
}

void ACOSolver::printInput(ostream &stream)
{
    stream << "PARAMETERS" << endl
           << "Colony Size: " << _option.colony_size << endl
           << "Ant Pheromone: " << _option.pheromone << endl
           << "Alfa: " << _option.alfa << endl
           << "Beta1: " << _option.beta1 << endl
           << "Beta2: " << _option.beta2 << endl
           << "Rho: " << _option.rho << endl
           << "Ni: " << _option.ni << endl
           << "Enable Remove Edges: " << _option.remove_edge << endl
           << "Remove Edges Probability: " << _option.remove_edge_probs << endl
           << "MaxInterations: " << _option.max_interations << endl;

    stream << "TOPOLOGY" << endl
           << _network->generateMATLABScript(false, false, false) << endl;
}

void ACOSolver::printOutput(ostream &stream)
{
    stream << "Interation: " << this->_current_interation << endl
           << "BEST OUTPUT: " << endl
           << to_string(this->_best_result) << endl;
}

void ACOSolver::setArguments(map<string, string> &optset)
{
    if (this->_state != SOLVER_INIT)
        throw "state is not valid";

    int opt;
    int temp1, temp2;

    for (auto &&pair : optset)
    {
        string opt = pair.first;
        string &optarg = pair.second;

        if (opt == "pheromone")
        {
            _option.pheromone = stof(optarg);
        }
        else if (opt == "max_interations")
        {
            _option.max_interations = stoi(optarg);
        }
        else if (opt == "colony_size")
        {
            _option.colony_size = stoi(optarg);
        }
        else if (opt == "alfa")
        {
            _option.alfa = stof(optarg);
        }
        else if (opt == "beta1")
        {
            _option.beta1 = stof(optarg);
        }
        else if (opt == "beta2")
        {
            _option.beta2 = stof(optarg);
        }
        else if (opt == "ni")
        {
            _option.ni = stof(optarg);
        }
        else if (opt == "rho")
        {
            _option.rho = stof(optarg);
        }
        else if (opt == "remove_edge")
        {
            _option.remove_edge = true;
        }
        else if (opt == "remove_edge_probs")
        {
            _option.remove_edge_probs = stof(optarg);
        }
        else if (opt == "enable_log")
        {
            this->_enable_log = true;
            setLogFile(optarg);
        }
    }
    this->_state = SOLVER_READY;
}
