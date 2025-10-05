#include "network.h"

using namespace core;
using namespace graph;

#define CHECK_NET_LOAD           \
    if (_state == NETWORK_EMPTY) \
    throw "La rete non è stata ancora caricata"
#define CHECK_NET_NO_LOAD        \
    if (_state != NETWORK_EMPTY) \
    throw "La rete non è stata ancora caricata"
#define FILE_DELIMITER ' '

string graph::to_string(const network_path &p)
{
    stringstream ss;
    if (p != emptyPath)
    {
        ss << "[" << p->size() << "](";
        bool first = true;
        for (auto &n : *p)
        {
            if (first)
                first = false;
            else
                ss << ',';
            if (n == ID_NODE_TARGET)
                ss << "t";
            else if (n == ID_NODE_SOURCE)
                ss << "s";
            else
                ss << n;
        }
        ss << ")";
    }
    else
    {
        ss << "empty";
    }
    return ss.str();
}

string graph::to_hash(const network_path &p)
{
    stringstream ss;
    for (auto &n : *p)
    {
        ss << n << ' ';
    }
    return ss.str();
}

bool graph::containsEdgeInPath(network_path path, const network_edgekey &key)
{
    int n = path->size();
    for (int i = 0; i < n; i++)
    {
        int e1 = path->at(i);
        if (e1 == key.id1 && i < n - 1)
        {
            int e2 = path->at(i + 1);
            if (e2 == key.id2)
                return true;
        }
    }
    return false;
}

bool compareEdgeNodeByCounter(edgenode_t &i, edgenode_t &j)
{
    int p1 = i.edge->tau;
    int p2 = j.edge->tau;

    return p1 < p2; //da priorita' all'arco che e' stato visitato di meno
}

bool compareEdgeNodeById(edgenode_t &i, edgenode_t &j)
{
    int p1 = i.parent->id;
    int p2 = j.parent->id;

    if (p1 < p2)
        return true;
    else if (p1 == p2)
        return i.next->id < j.next->id;
    else
        return false;
}

network_cut::s_network_cut()
{
    count_size = 0;
    is_valid = false;
}

network_cut::~s_network_cut()
{
    if (edges != nullptr)
        delete[] edges;
    edges = nullptr;
}

string graph::to_string(network_cut &cut)
{
    stringstream ss;
    ss << "(";
    if (cut.is_valid)
    {
        for (int i = 0; i < cut.count_size; i++)
        {
            if (i > 0)
                ss << ",";
            ss << std::to_string(cut.edges[i]);
        }
    }
    else
    {
        ss << "Cut not valid";
    }
    ss << ")";
    return ss.str();
}

void graph::copy(network_cut *source, network_cut *target)
{
    target->~s_network_cut();

    target->is_valid = source->is_valid;
    if (target->is_valid)
    {
        int n = source->count_size;
        target->count_size = n;
        target->edges = new int[n];
        for (int i = 0; i < n; i++)
            target->edges[i] = source->edges[i];
    }
}

network_partition::s_network_partition()
{
    size_S = 0;
    size_T = 0;
}

network_partition::~s_network_partition()
{
    delete[] this->nodes;
}

void network_partition::copyFrom(int *nodes, int size_node, int splitter)
{
    delete[] this->nodes;

    this->nodes = new int[size_node];
    for (int i = 0; i < size_node; i++)
        this->nodes[i] = nodes[i];
    this->size_S = splitter;
    this->size_T = size_node - splitter;
}

void graph::copy(network_partition *source, network_partition *target)
{
    target->~s_network_partition();

    target->size_S = source->size_S;
    target->size_T = source->size_T;

    int n = source->size();
    target->nodes = new int[n];
    for (int i = 0; i < n; i++)
        target->nodes[i] = source->nodes[i];
}

int graph::shuffle_partition(network_partition *partition, double mutation_rate, double p_change_size, int min_partition_size, RandomNumberGenerator *random)
{
    int n = partition->size();
    int c = random->shuffleInt(partition->nodes, n, mutation_rate);

    if (random->prob(p_change_size))
    {
        int splitter = partition->splitter();
        int min_x = floor((double)(min_partition_size - partition->size_S) / 2.0);
        int max_x = floor((double)(partition->size_T - min_partition_size) / 2.0);
        int x = random->getInt({min_x, max_x});
        splitter += x;
        partition->resize(splitter);
    }

    return c;
}

network::network(RandomNumberGenerator *rand)
{
    _s.id = ID_NODE_SOURCE;
    _t.id = ID_NODE_TARGET;
    _countNode = 0;
    _nodes = nullptr;
    _random = rand;
}

network::~network()
{
    _topology.clear();
    delete[] _nodes;
}

double network::getFlowUpperbound()
{
    double c1 = 0, c2 = 0;

    network_node *s = GET_NODE_SOURCE(this);
    for (auto &&n : s->next)
    {
        network_edge &edge = this->getEdge({s->id, n->id});
        c1 += edge.capacity;
    }

    network_node *t = GET_NODE_TARGET(this);
    for (auto &&n : t->prev)
    {
        network_edge &edge = this->getEdge({n->id, t->id});
        c2 += edge.capacity;
    }

    return MIN(c1, c2);
}

double network::getFlow()
{
    network_node *s = GET_NODE_SOURCE(this);
    double c = 0;
    for (auto &&n : s->next)
    {
        network_edge &edge = this->getEdge({s->id, n->id});
        c += edge.flow;
    }
    return c;
}

network_path network::navigate(select_node_function select_node, void *arg, int max_size)
{
    network_path p = createPath();
    network_node *current_node = GET_NODE_SOURCE(this);
    p->push_back(current_node->id);

    int count = 0;
    network_node *last_node;
    while (!IS_NODE_TARGET(current_node->id) && (max_size <= 0 || count <= max_size))
    {
        last_node = current_node;
        current_node = select_node(this, current_node, p, arg);
        if (current_node == nullptr)
        {
            break;
        }

        network_edge &edge = getEdge({last_node->id, current_node->id});
        edge.counter++;
        p->push_back(current_node->id);

        count++;
    }

    return p;
}

int network::countEdges(network_path path)
{
    unordered_set<string> set;
    int size_path = path->size();
    for (int i = 0; i < size_path - 1; i++)
    {
        string s = std::to_string(path->at(i)) + '_' + std::to_string(path->at(i + 1));
        set.insert(s);
    }
    return set.size();
}

void network::updateHeuristicDistance(network_path path)
{
    if (path != emptyPath)
    {
        int size = path->size();
        int end_value = size;

        int lastNodeId = path->back();
        if (!IS_NODE_TARGET(lastNodeId))
        {
            network_node *lastNode = getNode(lastNodeId);

            int upper = countEdges() * 0.7;
            upper = max(upper, size);
            lastNode->distance_heuristic = upper;
            end_value = upper - 1;
        }

        for (int j = 0, i = end_value; j < size - 1; j++, i--)
        {
            int nodeId = path->at(j);
            network_node *node = getNode(nodeId);
            if (node->distance_heuristic < 0 || node->distance_heuristic > i)
                node->distance_heuristic = i;
        }
    }
}

int network::edges(network_path path, vector<network_edgekey> &edges)
{
    unordered_set<string> set;
    int size_path = path->size();
    for (int i = 0; i < size_path - 1; i++)
    {
        string s = std::to_string(path->at(i)) + '_' + std::to_string(path->at(i + 1));
        if (set.insert(s).second)
        {
            edges.push_back({path->at(i), path->at(i + 1)});
        }
    }
    return set.size();
}

double network::getAllowedIncrementFlow(network_path path)
{
    double min_c = INFINITY;
    int size_path = path->size();
    for (int i = 0; i < size_path - 1; i++)
    {
        network_edge &edge = getEdge({path->at(i), path->at(i + 1)});
        double c = edge.getResidualCapacity();
        if (c < min_c)
            min_c = c;
    }
    return min_c;
}

network_path network::normalizePath(network_path path)
{
    if (path == emptyPath)
        return emptyPath;

    network_path new_path = createPath();

    reset(false, false, false, true, false);

    //attraversa il percorso e colara gli archi
    int index_path = 0;
    int size_path = path->size();

    vector<network_node *> temp_path;
    do
    {
        network_node *node = getNode(path, index_path);
        if (node->color == NODE_WHITE)
        {
            node->color = NODE_GREEN;
            temp_path.push_back(node);
            index_path++;
        }
        else if (node->color == NODE_GREEN)
        {
            //rimuove tutto il percorso fatto da questo nodo quando si è passata la prima volta
            network_node *prev_node;
            node->color = NODE_WHITE;
            do
            {
                prev_node = temp_path.back();
                temp_path.pop_back();
                prev_node->color = NODE_WHITE;
            } while (prev_node->id != node->id);
        }
    } while (index_path < size_path);

    for (auto &&n : temp_path)
    {
        new_path->push_back(n->id);
    }
    return new_path;
}

void network::updateFlow(network_path path, double delta_flow)
{
    int n = path->size();
    for (int i = 0; i < n - 1; i++)
    {
        network_edge &edge = getEdge({path->at(i), path->at(i + 1)});
        edge.flow += delta_flow;
    }
}

double network::getMinResidualCapacity(network_path path)
{
    double min = +INFINITY;
    int n = path->size();
    for (int i = 0; i < n - 1; i++)
    {
        network_edge &edge = getEdge({path->at(i), path->at(i + 1)});
        double c = edge.getResidualCapacity();
        if (c < min)
            min = c;
    }
    return min;
}

/*
void network::updateAllEdges(network_path path, double delta_flow_path, double delta_tau_path, double delta_flow, double delta_tau)
{
    int path_size = path->size();
    for (int i = 0; i < path_size - 1; i++)
    {
        network_edge &edge = getEdge({path->at(i), path->at(i + 1)});
        edge.tau += delta_tau_path;
        edge.flow += delta_flow_path;
    }

    for (auto &&edge : this->_edges)
    {
        if (!containsEdgeInPath(path, edge.first.id1, edge.first.id2))
        {
            edge.second.tau += delta_tau;
            edge.second.flow += delta_flow;
        }
        edge.second.tau = SATURATE_MIN(edge.second.tau, 0);
    }
}*/

void network::enableAllEdges()
{

    for (auto &&edge : this->_topology)
    {
        edge.second.enable = true;
    }
}

bool network::disableEdge(network_edgekey &key)
{
    network_edge &edge = this->_topology.get(key);
    if (edge.enable)
    {
        edge.enable = false;
        return true;
    }
    return false;
}

bool network::enableEdge(network_edgekey &key)
{
    network_edge &edge = this->_topology.get(key);
    if (!edge.enable)
    {
        edge.enable = true;
        return true;
    }
    return false;
}

int network::disableRandomEdges(double prob)
{
    int removed = 0;
    for (auto &&item : this->_topology)
    {
        if (!IS_NODE_SOURCE(item.first.id1) && !IS_NODE_TARGET(item.first.id2))
        {
            if (this->_random->prob(prob))
            {
                item.second.enable = false;
                removed++;
            }
        }
    }
    return removed;
}

void network::populateNodeIds(int *v, int start, bool include_special_node)
{
    int i = start;
    if (include_special_node)
    {
        v[i++] = _s.id;
        v[i++] = _t.id;
    }

    for (int j = 0; j < _countNode; j++)
        v[i++] = _nodes[j].id;
}

void network::computeNetworkCut(network_partition *partition, network_cut *cut_computed)
{
    cut_computed->~network_cut();

    int i = 0;
    int n = partition->size();
    vector<int> *edges = new vector<int>();

    unordered_set<int> hash_s;
    for (i = 0; i < partition->size_S; i++)
        hash_s.insert(partition->nodes[i]);
    unordered_set<int> hash_t;
    for (; i < n; i++)
        hash_t.insert(partition->nodes[i]);
    /*
    for (auto &&s : hash_s)
    {
        for (auto &&t : hash_t)
        {
            network_edgekey key(s, t);
            if (_topology.contains(key))
            {
                edges->push_back(key);
            }
        }
    }*/

    i = 0;
    for (auto &&edge : _topology)
    {
        int s = edge.first.id1;
        int t = edge.first.id2;

        if (CONTAINS_SET(hash_s, s) && CONTAINS_SET(hash_t, t))
        {
            edges->push_back(i);
        }
        i++;
    }

    int cut_size = edges->size();
    if (cut_size <= 1)
    {
        cut_computed->edges = nullptr;
        cut_computed->is_valid = false;
        cut_computed->count_size = 0;
    }
    else
    {
        cut_computed->edges = &(edges->front());
        cut_computed->count_size = cut_size;
        cut_computed->is_valid = true;
    }
}

#define GET_NODE_LINK(id) id >= 0 ? &target._nodes[id] : (id == ID_NODE_SOURCE ? &target._s : (id == ID_NODE_TARGET ? &target._t : throw "Id del nodo non valido"))
#define COPY_NODE_LINK(t, s) \
    for (auto item : s)      \
        t.push_back(GET_NODE(item->id, this));

void network::reset(bool flow, bool capacity, bool status, bool color, bool heuristic, double default_tau)
{
    for (auto &edge : _topology)
    {
        if (capacity)
            edge.second.capacity = 0;
        if (flow)
            edge.second.flow = 0;
        if (status)
        {
            edge.second.tau = default_tau;
            edge.second.counter = 1;
        }
    }

    if (color)
    {
        for (int i = 0; i < _countNode; i++)
        {
            _nodes[i].color = NODE_WHITE;
        }
        _s.color = NODE_WHITE;
        _t.color = NODE_WHITE;
    }

    if (heuristic)
    {
        for (int i = 0; i < _countNode; i++)
        {
            _nodes[i].distance_heuristic = -1;
        }
        _s.distance_heuristic = -1;
        _t.distance_heuristic = 0;
    }
}

double network::computeCutCapacity(network_cut &cut)
{
    if (cut.is_valid)
    {
        double cut_capacity = 0;
        for (int i = 0; i < cut.count_size; i++)
        {
            int &edge_key = cut.edges[i];
            network_edge &edge = _topology.get(edge_key);
            cut_capacity += edge.capacity;
        }
        return cut_capacity;
    }
    else
    {
        return -1;
    }
}

string network::print()
{
    stringstream ss;
    ss << "Nodes: " << countNodes() << endl
       << "Edges: " << countEdges() << endl
       << "UpperBoundFlow: " << getFlowUpperbound() << endl;
    return ss.str();
}

string network::generateMATLABScript(bool printCapacity, bool printFlow, bool printColleration)
{
    int i = 0, n = _topology.size();
    edgenode_t *items = new edgenode_t[n];
    for (auto &x : _topology)
    {
        items[i].parent = GET_NODE(x.first.id1, this);
        items[i].next = GET_NODE(x.first.id2, this);
        items[i].edge = &x.second;
        i++;
    }
    sort(items, items + n, compareEdgeNodeById);

    stringstream ss1;
    stringstream ss2;
    stringstream ss3;

    ss1 << "s=[ ";
    ss2 << "t=[ ";
    ss3 << "code={ ";

    for (i = 0; i < n; i++)
    {
        ss1 << GET_NODE_LABEL(items[i].parent->id) << " ";
        ss2 << GET_NODE_LABEL(items[i].next->id) << " ";
        if (printCapacity && printFlow)
            ss3 << "'" << core::to_string(items[i].edge->flow, 4) << "/" << core::to_string(items[i].edge->capacity, 4) << "' ";
        else if (printCapacity)
            ss3 << "'" << core::to_string(items[i].edge->capacity, 4) << "' ";
        else if (printFlow)
            ss3 << "'" << core::to_string(items[i].edge->flow, 4) << "' ";
        if (printColleration)
            ss3 << "[" << core::to_string(items[i].edge->tau, 4) << "]";
    }
    ss1 << "];";
    ss2 << "];";
    ss3 << "};";

    delete[] items;

    stringstream ss;
    ss << ss1.str() << endl;
    ss << ss2.str() << endl;
    ss << ss3.str() << endl;
    ss << "G = digraph(s, t);" << endl;
    ss << "figure();" << endl;
    ss << "h=plot(G,'EdgeLabel',code);" << endl;
    ss << "labelnode(h, [1 2], { 'source' 'target' });" << endl;
    ss << "highlight(h, [1 2]);" << endl;
    return ss.str();
}

void populateAdjRow(vector<double> &adjMatrixRow, network_node *node, topology &edges)
{
    for (auto &&x : node->next)
    {
        double capacity = edges.get({node->id, x->id}).capacity;
        int target_id = GET_NODE_NORMAL_ID(x->id);

        adjMatrixRow[target_id] = capacity;
    }
}

void network::populateAdjMatrix(vector<vector<double>> &adjMatrix, int *source, int *sink)
{
    //inizialize matrix
    int count_node = _countNode + 2;
    for (int i = 0; i < count_node; i++)
    {
        vector<double> row(count_node);
        for (int j = 0; j < count_node; j++)
        {
            row[j] = 0.0;
        }
        adjMatrix.push_back(row);
    }

    *source = GET_NODE_NORMAL_ID(ID_NODE_SOURCE);
    *sink = GET_NODE_NORMAL_ID(ID_NODE_TARGET);

    populateAdjRow(adjMatrix[*source], &_s, _topology);
    for (int i = 0; i < _countNode; i++)
    {
        network_node *node = &_nodes[i];
        int node_id = GET_NODE_NORMAL_ID(node->id);
        populateAdjRow(adjMatrix[node_id], node, _topology);
    }
    populateAdjRow(adjMatrix[*sink], &_t, _topology);
}

namespace graph
{
    void buildNetwork(network *net, network_generator_option &option)
    {
        net->_nodes = new network_node[option.countNode];
        net->_countNode = option.countNode;

        //inizializza i nodi
        for (int i = 0; i < option.countNode; i++)
        {
            net->_nodes[i].id = i;
        }

        //inizializza i layer
        struct layer
        {
            network_node *vectors = nullptr;
            int count = 0;

            ~layer()
            {
                vectors = nullptr;
            }
        };

        RandomNumberGenerator *rdn = net->_random;

        //determino il range di nodi contenuti in ciascuno layer
        int l = (int)round((double)option.countNode / option.countLayer);
        if (l <= option.deltaNodeLayer)
        {
            l = option.deltaNodeLayer + 1;
        }
        rangeint countNodeForLayer = rangeint(l - option.deltaNodeLayer, l + option.deltaNodeLayer);

        //calcola i nodi presenti in ciascuno layer
        struct layer *firstlayers = new layer[option.countLayer];
        int start = 0, end = 0, nNode = 0;
        int countLayerRelative = 0;

        for (int i = 0; i < option.countLayer && nNode < option.countNode; i++)
        {
            int n;
            if (i == option.countLayer - 1)
                n = option.countNode - nNode;
            else
                n = rdn->getInt(countNodeForLayer);
            start = end;
            end = start + n;
            if (end > option.countNode)
            {
                end = option.countNode;
                n = end - start;
            }
            firstlayers[i].vectors = &net->_nodes[start];
            firstlayers[i].count = n;

            nNode += n;
            countLayerRelative = i;
        }

        double capacityAvg = option.capacityEdge.getAvg();
        rangedouble rangeD = {-(capacityAvg / 2), (capacityAvg / 2)};

        //crea i collegamenti con il nodo src
        for (int i = 0; i < firstlayers->count; i++)
        {

            net->_s.next.push_back(&firstlayers->vectors[i]);
            firstlayers->vectors[i].prev.push_back(&net->_s);

            double c = rdn->getDouble(option.capacityEdge);
            if (c < capacityAvg)
                c = capacityAvg + rdn->getDouble(rangeD);

            net->_topology.link(ID_NODE_SOURCE, firstlayers->vectors[i].id, c);
        }

        //crea collegamenti tra i nodi di un livello e un altro
        for (int i = 0; i < countLayerRelative - 1; i++)
        {
            for (int j = 0; j < firstlayers[i].count; j++)
            {

                int ned = rdn->getInt(option.countEdgeForNode);
                if (ned == 0)
                    ned = 1;

                network_node *e1 = &firstlayers[i].vectors[j];
                for (int k = 0; k < ned; k++)
                {

                    //calcola il layer in avanti con cui deve essere collegato questo nodo
                    int offset = 1;
                    rangeint roffset = {1, countLayerRelative - i};
                    if (rdn->prob(0.4))
                    {
                        offset = rdn->getInt(roffset);
                    }

                    network_node *e2;
                    //si sceglie il nodo finale dell'arco
                    if (offset < roffset.getMax())
                    {
                        //calcolo il nodo finale dall'arco appartenente al layer scelto
                        rangeint rnext = {0, firstlayers[i + offset].count - 1};
                        int e2index = rdn->getInt(rnext);
                        e2 = &firstlayers[i + offset].vectors[e2index];
                    }
                    else
                    {
                        //si collega il nodo al nodo dest
                        e2 = &net->_t;
                    }

                    //si crea l'arco
                    double c = rdn->getDouble(option.capacityEdge);

                    e1->next.push_back(e2);
                    e2->prev.push_back(e1);

                    net->_topology.link(e1->id, e2->id, c);
                }
            }
        }

        //crea collegamenti tra i nodi appartenenti allo stesso livello
        for (int i = 0; i < countLayerRelative; i++)
        {
            rangeint r = {1, countNodeForLayer.getMin()};
            int ned = rdn->getInt(r);
            rangeint rnext = {0, firstlayers[i].count - 1};
            for (int k = 0; k < ned; k++)
            {

                int e1index = rdn->getInt(rnext);
                int e2index = rdn->getInt(rnext);

                if (e1index != e2index)
                {
                    double c = rdn->getDouble(option.capacityEdge);

                    network_node *e1 = &firstlayers[i].vectors[e1index];

                    if (e1->next.size() + 1 <= option.countEdgeForNode.getMax())
                    {
                        network_node *e2 = &firstlayers[i].vectors[e2index];

                        e1->next.push_back(e2);
                        e2->prev.push_back(e1);

                        net->_topology.link(e1->id, e2->id, c);
                    }
                }
            }
        }

        //crea i collegamenti con il nodo dest
        layer *lastlayer = &firstlayers[countLayerRelative - 1];
        for (int i = 0; i < lastlayer->count; i++)
        {
            lastlayer->vectors[i].next.push_back(&net->_t);
            net->_t.prev.push_back(&lastlayer->vectors[i]);

            double c = rdn->getDouble(option.capacityEdge);
            net->_topology.link(lastlayer->vectors[i].id, ID_NODE_TARGET, c);
        }

        //creo collegamenti a caso
        if (option.entropy > 0)
        {
            int n = floor(option.entropy * 30.0);
            for (int j = 0; j < n; j++)
            {
                for (int i = 0; i < option.countNode; i++)
                {
                    network_node *source = &net->_nodes[i];

                    int index = rdn->getInt({0, option.countNode});
                    if (index == i)
                        continue;

                    network_node *dest = index == option.countNode ? &net->_t : &net->_nodes[index];

                    double c = rdn->getDouble(option.capacityEdge);
                    net->_topology.link(source->id, dest->id, c);
                }
            }
        }

        //correge gli errori in caso di nodi isolati
        for (int i = 0; i < option.countNode; i++)
        {
            network_node *node = &net->_nodes[i];
            if (node->prev.size() == 0)
            {
                //sceglie un node precedente a cui collegare questo nodo
                rangeint rangeNode = {-1, node->id - 1};
                int nindex = rdn->getInt(rangeNode);
                network_node *nde;
                if (nindex == -1)
                    nde = &net->_s;
                else
                    nde = &net->_nodes[nindex];

                nde->next.push_back(node);
                node->prev.push_back(nde);

                double c = rdn->getDouble(option.capacityEdge);
                net->_topology.link(nde->id, node->id, c);
            }
            if (node->next.size() == 0)
            {
                //sceglio un nodo successivo a cui collegare questo nodo
                rangeint rangeNode = {node->id + 1, option.countNode};
                int nindex = rdn->getInt(rangeNode);
                network_node *nde;
                if (nindex == option.countNode)
                    nde = &net->_t;
                else
                    nde = &net->_nodes[nindex];

                node->next.push_back(nde);
                nde->prev.push_back(node);

                double c = rdn->getDouble(option.capacityEdge);
                net->_topology.link(node->id, nde->id, c);
            }
        }

        delete[] firstlayers;
    }

    void buildNetwork(network *net, topology &elms)
    {
        if (net->_nodes != nullptr)
        {
            delete[] net->_nodes;
            net->_topology.clear();
        }

        set<int> nodeSet;
        for (auto &e : elms)
        {
            nodeSet.insert(e.first.id1);
            nodeSet.insert(e.first.id2);
        }

        int countNode = nodeSet.size() - 2;
        int countEdge = elms.size();

        //imposta i nodi
        map<int, int> mapids;
        net->_nodes = new network_node[countNode];
        net->_countNode = countNode;

        int i = 0;
        for (auto &&node_id : nodeSet)
        {
            if (!IS_DEFAULT_NODE(node_id))
            {
                net->_nodes[i] = i;
                mapids.insert({node_id, i});
                i++;
            }
            else
            {
                mapids.insert({node_id, node_id});
            }
        }

        //crea i collegmenti
        for (auto &x : elms)
        {
            int id1 = mapids[x.first.id1];
            int id2 = mapids[x.first.id2];

            network_node *nd1 = GET_NODE(id1, net);
            network_node *nd2 = GET_NODE(id2, net);

            nd1->next.push_back(nd2);
            nd2->prev.push_back(nd1);
            net->_topology.link(id1, id2, x.second.capacity);
        }
    }

}