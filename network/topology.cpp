#include "topology.h"
#define FILE_DELIMITER " "

using namespace graph;
using namespace core;

string to_string(network_edge &edge)
{
    stringstream ss;
    ss << "{enable=" << to_string_format(edge.enable)
       << ", tau=" << to_string_format(edge.tau)
       << ", counter=" << to_string_format(edge.counter)
       << ", (" << to_string_format(edge.flow) << "/" << to_string_format(edge.capacity) << ")}";
    return ss.str();
}
string to_string(network_edgekey &key)
{
    stringstream ss;
    ss << "(" << to_string_format(key.id1) << "," << to_string_format(key.id2) << ")";
    return ss.str();
}

topology::topology()
{
}

topology::~topology()
{
    this->_keys.clear();
    this->_map.clear();
}

int topology::sizeInternalEdges()
{
    int count = 0;
    for (auto &&item : this->_map)
    {
        if (!IS_NODE_SOURCE(item.first.id1) && !IS_NODE_TARGET(item.first.id2))
            count++;
    }
    return count;
}

int topology::size(bool only_enable, bool only_internal) const
{
    if (!only_enable && !only_internal)
    {
        return this->_keys.size();
    }
    else
    {
        int count = 0;
        for (auto &&edge : _map)
        {
            bool is_enable = edge.second.enable;
            bool is_internal = !IS_NODE_SOURCE(edge.first.id1) && !IS_NODE_TARGET(edge.first.id2);

            if (only_enable == is_enable)
                count++;
        }
        return count;
    }
}

int topology::fullconnected(rangedouble capacity_range, RandomNumberGenerator *rdn)
{
    int inserted = 0;
    for (auto &&key : _keys)
    {
        if (link(key.id2, key.id1, rdn->getDouble(capacity_range), false))
            inserted++;
    }

    if (inserted > 0)
        updateKeysByMap();
    return inserted;
}

int topology::pruning(bool massive, bool compact)
{
    int removed = 0;
    for (auto &&key : _keys)
    {
        if (key.id1 == key.id2 || (massive && (key.id2 == ID_NODE_SOURCE || key.id1 == ID_NODE_TARGET)))
        {
            _map.erase(key);
            removed++;
        }
    }

    if (compact)
    {
        unordered_set<network_edgekey, network_edgekey_hasher> disabledSet;
        for (auto &&pair : _map)
        {
            if (!CONTAINS_SET(disabledSet, pair.first))
            {
                network_edgekey invert(pair.first.id2, pair.first.id2);
                if (contains(invert))
                {
                    network_edge &e2 = get(invert);
                    network_edge &e1 = pair.second;

                    if (e2.capacity > e1.capacity)
                    {
                        e2.capacity -= e1.capacity;
                        disabledSet.insert(pair.first);
                    }
                    else if (e2.capacity < e1.capacity)
                    {
                        e1.capacity -= e2.capacity;
                        disabledSet.insert(invert);
                    }
                    else
                    {
                        disabledSet.insert(invert);
                        disabledSet.insert(pair.first);
                    }
                }
            }
        }
        removed += disabledSet.size();
        for (auto &&key : disabledSet)
        {
            _map.erase(key);
        }
        disabledSet.clear();
    }

    if (removed > 0)
        updateKeysByMap();

    return removed;
}

void topology::updateKeysByMap()
{
    _keys.clear();
    for (auto &&pair : _map)
    {
        _keys.push_back(pair.first);
    }
}

int topology::clone(network_edgekey *keys, network_edge *edges)
{
    int i = 0;
    for (auto &item : this->_map)
    {
        keys[i] = item.first;
        edges[i] = item.second;
        i++;
    }
    return i;
}

int normalize_node_id(int id, int id_source, int id_target)
{
    if (id == id_source)
        return ID_NODE_SOURCE;
    else if (id == id_target)
        return ID_NODE_TARGET;
    return id;
}

void topology::read(string &filename)
{
    ifstream file(filename);
    if (file.is_open())
    {
        string line;
        int line_index = 0;
        int id_source = ID_NODE_SOURCE;
        int id_target = ID_NODE_TARGET;
        while (getline(file, line))
        {
            if (line.empty())
                continue;
            line_index++;

            if (line_index == 3)
            {
                id_source = stoi(line);
            }
            else if (line_index == 4)
            {
                id_target = stoi(line);
            }
            else if (line_index >= 5)
            {
                vector<string> v = split(line, FILE_DELIMITER, true);
                if (v.size() < 3)
                    continue;

                int id1 = stoi(v[0]);
                int id2 = stoi(v[1]);
                double c = stof(v[2]);

                id1 = normalize_node_id(id1, id_source, id_target);
                id2 = normalize_node_id(id2, id_source, id_target);

                link(id1, id2, c);
            }
        }
        file.close();
    }
    else
        throw "File non disponibile";
}

void topology::write(string &filename)
{
    ofstream file(filename);
    if (file.is_open())
    {
        file << uniqueNodes() << endl
             << size() << endl
             << ID_NODE_SOURCE << endl
             << ID_NODE_TARGET << endl;

        for (auto &&e : _map)
        {
            file << e.first.id1 << " " << e.first.id2 << " " << e.second.capacity << endl;
        }
        file.close();
    }
}

bool topology::link(int node1, int node2, double capacity, bool increaseIfExist)
{
    network_edgekey key(node1, node2);
    if (!contains(key))
    {
        network_edge e(capacity, true);

        this->_map.insert({key, e});
        this->_keys.push_back(key);
        return true;
    }
    else if (increaseIfExist)
    {
        network_edge &e = get(key);
        e.capacity += capacity;
    }
    return false;
}

int topology::uniqueNodes()
{
    unordered_set<int> set;
    for (auto &&k : this->_keys)
    {
        set.insert(k.id1);
        set.insert(k.id2);
    }
    return set.size();
}
