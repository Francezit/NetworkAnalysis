#include "ffsolver.h"

using namespace flowsolver;
using namespace core;
using namespace graph;

//See https://www.srcmake.com/home/cpp-ford-fulkerson-max-flow

bool BFS(vector<vector<double>> &resAdjMatrix, int &source, int &sink, vector<int> &parent)
{
    // Create an array for all nodes we visited. Initialized to false.
    int n = resAdjMatrix.size();
    bool visited[n] = {false};

    // Create a queue to check each node.
    queue<int> q;

    // Push our source into the queue and mark it as visited. It has no parent.
    q.push(source);
    visited[source] = true;
    parent[source] = -1;

    // Keep visiting vertices.
    while (q.empty() == false)
    {
        int u = q.front();
        q.pop();

        // Check all of u's friends.
        for (int i = 0; i < n; i++)
        {
            int v = i;
            double capacity = resAdjMatrix[u][v];

            // We find a neighbor that hasn't been visited, and the capacity is bigger than 0.
            if (visited[v] == false && capacity > 0)
            {
                // Push the neighbor onto the queue, mark it's parent, and mark it as visited.
                q.push(v);
                parent[v] = u;
                visited[v] = true;
            }
        }
    }

    // If the sink got visited, then we found a path to it.
    if (visited[sink] == true)
    {
        return true;
    }

    return false;
}

int FordFulkerson(vector<vector<double>> &adjMatrix, int &source, int &sink)
{
    double maxflow = 0;

    // 1. Create the residual graph. (Same as the original graph.)
    vector<vector<double>> resAdjMatrix;
    int n = adjMatrix.size();
    for (int i = 0; i < n; i++)
    {
        vector<double> row;
        for (int j = 0; j < adjMatrix[i].size(); j++)
        {
            double c = adjMatrix[i][j];
            row.push_back(c);
        }
        resAdjMatrix.push_back(row);
    }

    // 2. Create an empty parent array for BFS to store the augmenting path.
    vector<int> parent;
    for (int i = 0; i < n; i++)
    {
        parent.push_back(-1);
    }

    // 3. Keep calling BFS to check for an augmenting path (from the source to the sink...
    while (BFS(resAdjMatrix, source, sink, parent) == true)
    {
        // 4. Find the max flow through the path we just found.
        double pathflow = DBL_MAX;

        // Go through the path we just found. Iterate through the path.
        int v = sink;
        while (v != source)
        {
            int u = parent[v]; // The parent.

            // Update the pathflow to this capacity if it's smaller
            double capacity = resAdjMatrix[u][v];
            pathflow = MIN(pathflow, capacity);

            // Setup for the next edge in the path.
            v = u;
        }

        // 5. Update the residual capacities of the edges and reverse edges.
        v = sink;
        while (v != source)
        {
            int u = parent[v]; // The parent.

            // Update the capacities.

            resAdjMatrix[u][v] -= pathflow;
            resAdjMatrix[v][u] += pathflow;

            // Setup for the next edge in the path.
            v = u;
        }

        // 6. Add this path's flow to our total max flow so far.
        maxflow += pathflow;
    }

    return maxflow;
}

FFSolver::FFSolver()
{
}

void FFSolver::dispose()
{
    this->internalReset();
    delete _network;
}

void FFSolver::reset()
{
    this->internalReset();
}

void FFSolver::printInput(ostream &stream)
{
    vector<vector<double>> adjMatrix;
    int source, sink;
    _network->populateAdjMatrix(adjMatrix, &source, &sink);

    stringstream ss;
    ss << "AdjMatrix" << endl
       << "Source: " << source << endl
       << "Sink: " << sink << endl
       << "Matrix size: " << adjMatrix.size() << "x" << adjMatrix.size() << endl
       << endl;

    for (int i = 0; i < adjMatrix.size(); i++)
    {
        vector<double> &row = adjMatrix[i];
        for (int j = 0; j < row.size(); j++)
        {
            ss << row[j] << "\t";
        }
        ss << endl;
    }
    print(ss.str(), true);
}

void FFSolver::printOutput(ostream &stream)
{
    stream << "Max Flow: " << _statistics.back()->fitness << endl;
}

void FFSolver::setArguments(map<string, string> &optset)
{
    int opt;
    int temp1, temp2;

    topology edges;

    for (auto &&pair : optset)
    {
        string opt = pair.first;
        string &optarg = pair.second;

        int temp1, temp2;

        if (opt == "enable_log")
        {
            this->_enable_log = true;
            setLogFile(optarg);
        }
    }

    this->_state = SOLVER_READY;
}

void FFSolver::solver()
{
    if (this->_state != SOLVER_READY)
        throw "state is not valid";

    this->_state = SOLVER_COMPUTING;

    vector<vector<double>> adjMatrix;
    int source, sink;
    _network->populateAdjMatrix(adjMatrix, &source, &sink);

    this->_start_timestamp = GET_CURRENT_TIMESTAMP;
    double flow = FordFulkerson(adjMatrix, source, sink);
    this->_end_timestamp = GET_CURRENT_TIMESTAMP;

    statistic *stat = new statistic();
    stat->fitness = flow;
    stat->avg_fitness = flow;
    stat->interations = 0;
    stat->end_timestamp = this->_end_timestamp;
    stat->start_timestamp = this->_start_timestamp;
    stat->count_edges = this->_network->countEdges();
    stat->count_nodes = this->_network->countNodes();
    stat->output = new FFSolution(flow);

    _statistics.push_back(stat);
    this->_best_result = stat;

    this->_state = SOLVER_COMPUTED;

    EXECUTIVE_LOGGER(endl << "Statistiche" << endl
                          << to_string(stat),
                     false);

    double executive_time;
    TIMESTAMP_ELAPSED_MS(this->_start_timestamp, this->_end_timestamp, executive_time);
    EXECUTIVE_LOGGER("Total time: " << executive_time << "ms", true);

    for (auto &&row : adjMatrix)
    {
        row.clear();
    }
    adjMatrix.clear();
}
