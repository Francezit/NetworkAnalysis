#pragma once
#ifndef _H_NETWORK
#define _H_NETWORK

#include "topology.h"
#include "..\core\core.h"

namespace graph
{

#define GET_NODE(id, net) (id >= 0 ? &net->_nodes[id] : (id == ID_NODE_SOURCE ? &net->_s : (id == ID_NODE_TARGET ? &net->_t : throw "Id del nodo non valido")))
#define GET_NODE_SOURCE(net) (&net->_s)
#define GET_NODE_TARGET(net) (&net->_t)
#define COLOR_NODE(id, clr, net) (GET_NODE(id, net)->color = clr)

    enum network_node_color
    {
        NODE_WHITE = 0,
        NODE_GREEN = 1,
        NODE_YELLOW = 2,
        NODE_RED = 3
    };

    typedef struct s_node
    {
    public:
        vector<struct s_node *> next;
        vector<struct s_node *> prev;
        int id;
        network_node_color color;
        int distance_heuristic;

        s_node()
        {
            id = 0;
            color = NODE_WHITE;
            distance_heuristic = -1;
        }
        s_node(int id)
        {
            this->id = id;
            color = NODE_WHITE;
            distance_heuristic = -1;
        }
    } network_node;

    inline string to_string(network_node *node)
    {
        return std::to_string(node->id);
    }
    inline string to_string(network_node &node)
    {
        return std::to_string(node.id);
    }

    using network_path = vector<int> *;

#define createPath() new vector<int>()

#define deletePath(path)   \
    if (path != emptyPath) \
    delete path

#define deletePaths(paths, n)      \
    for (int ii = 0; ii < n; ii++) \
        deletePath(paths[ii]);     \
    delete[] paths

#define isValidPath(path) (path != emptyPath && IS_NODE_SOURCE(path->front()) && IS_NODE_TARGET(path->back()))

#define emptyPath nullptr

    bool containsEdgeInPath(network_path path, const network_edgekey &key);

    string to_string(const network_path &p);

    string to_hash(const network_path &p);

    struct network_path_compare
    {
        bool operator()(const network_path &p1, const network_path &p2)
        {
            int n = min(p1->size(), p2->size());
            for (int i = 1; i < n - 1; i++)
            {
                int a = p1->at(i);
                int b = p2->at(i);
                if (a < b)
                    return true;
                else if (a > b)
                    return false;
            }
            return false;
        }
    };

    typedef struct edgenode
    {
        network_edge *edge;
        network_node *parent;
        network_node *next;
    } edgenode_t;

    bool compareEdgeNodeByCounter(edgenode_t &i, edgenode_t &j);
    bool compareEdgeNodeById(edgenode_t &i, edgenode_t &j);

#define IS_VALID_CUT_CAPACITY(cut_capacity) (cut_capacity >= 0)

    typedef struct s_network_cut
    {
        int *edges;
        bool is_valid;
        int count_size;

        s_network_cut();
        ~s_network_cut();
    } network_cut;

    string to_string(network_cut &cut);
    void copy(network_cut *source, network_cut *target);

    typedef struct s_network_partition
    {
        int *nodes;
        int size_S;
        int size_T;

        s_network_partition();
        ~s_network_partition();

        inline int size()
        {
            return size_S + size_T;
        }

        inline void resize(int splitter)
        {
            int n = size();
            size_S = splitter;
            size_T = n - splitter;
        }

        inline int splitter()
        {
            return size_S;
        }

        inline bool containsInS(int node)
        {
            return core::exists<int>(nodes, 0, size_S, node);
        }

        bool containsInT(int node)
        {
            return core::exists<int>(nodes, size_S + 1, size_S + size_T, node);
        }

        void copyFrom(int *nodes, int size_node, int splitter);
    } network_partition;

    void copy(network_partition *source, network_partition *target);

    int shuffle_partition(network_partition *partition, double mutation_rate, double p_change_size, int min_partition_size, core::RandomNumberGenerator *random);

    typedef struct
    {
        int countNode = 100;
        int countLayer = 20;
        int deltaNodeLayer = 10;
        core::rangeint countEdgeForNode = {2, 5};
        core::rangedouble capacityEdge = {2.0, 20.0};
        double entropy = 0.3;

    } network_generator_option;

    class network
    {
    private:
        topology _topology;
        network_node *_nodes;
        network_node _s, _t;
        int _countNode;
        core::RandomNumberGenerator *_random;

    public:
        network(core::RandomNumberGenerator *rand);

        ~network();

    public:
        typedef network_node *(*select_node_function)(network *net, network_node *, network_path &current_path, void *arg);

        inline int countNodes()
        {
            return _countNode;
        }

        inline int countEdges()
        {
            return _topology.size();
        }

        inline topology &getTopology()
        {
            return _topology;
        }

        inline network_edge &getEdge(const network_edgekey &edge)
        {
            network_edge &e = _topology.get(edge);
            return e;
        }

        inline bool containsEdge(const network_edgekey &edge)
        {
            return _topology.contains(edge);
        }

        inline network_node *getNode(int id)
        {
            network_node *n = GET_NODE(id, this);
            return n;
        }

        inline network_node *getNode(network_path path, int index)
        {
            int id = path->at(index);
            network_node *n = GET_NODE(id, this);
            return n;
        }

        inline int pruning()
        {
            return _topology.pruning(true, true);
        }

        inline int fullconnected()
        {
            return _topology.fullconnected({0.0, this->getFlowUpperbound()}, this->_random);
        }

        void populateNodeIds(int *v, int start, bool include_special_node);

        double getFlowUpperbound();

        double getFlow();

        double getAllowedIncrementFlow(network_path path);

        string generateMATLABScript(bool printCapacity, bool printFlow, bool printColleration = false);

        double computeCutCapacity(network_cut &cut);

        void computeNetworkCut(network_partition *partition, network_cut *cut_computed);

        network_path navigate(select_node_function select_node, void *arg, int max_size = -1);

        int countEdges(network_path path);

        void updateHeuristicDistance(network_path path);

        int edges(network_path path, vector<network_edgekey> &edges);

        double getMinResidualCapacity(network_path path);

        void updateFlow(network_path path, double delta_flow);

        //void updateAllEdges(network_path path, double delta_flow_path, double delta_tau_path, double delta_flow, double delta_tau);

        network_path normalizePath(network_path path);

        void reset(bool flow = true, bool capacity = true, bool status = true, bool color = true, bool heuristic = true, double default_tau = 1);

        void enableAllEdges();

        bool disableEdge(network_edgekey &key);

        bool enableEdge(network_edgekey &key);

        inline int disableRandomEdges(int count_edge)
        {
            int size = _topology.sizeInternalEdges();
            double probs = count_edge / (double)size;
            return disableRandomEdges(SATURATE(probs, 1.0, 0.0));
        }

        int disableRandomEdges(double probs);

        void populateAdjMatrix(vector<vector<double>> &adjMatrix, int *source, int *sink);

        string print();

        friend void buildNetwork(network *net, topology &edges);

        friend void buildNetwork(network *net, network_generator_option &option);
    };
}
#endif