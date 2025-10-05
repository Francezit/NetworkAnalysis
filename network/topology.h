#pragma once
#ifndef _H_EDGE
#define _H_EDGE

#include "..\core\core.h"

namespace graph
{

#define ID_NODE_SOURCE -2
#define ID_NODE_TARGET -1

#define GET_NODE_NORMAL_ID(nodeId) ((nodeId) + 2)
#define GET_NODE_LABEL_UI(nodeId) (IS_NODE_SOURCE(nodeId) ? string("source") : (IS_NODE_TARGET(nodeId) ? string("target") : to_string(nodeId + 3)))
#define GET_NODE_LABEL(nodeId) std::to_string((nodeId) + 3)

#define IS_DEFAULT_NODE(id) ((id) < 0)
#define IS_NODE_SOURCE(id) ((id) == ID_NODE_SOURCE)
#define IS_NODE_TARGET(id) ((id) == ID_NODE_TARGET)

	typedef struct s_edge
	{
	public:
		double flow;
		double capacity;
		double tau;
		bool enable;
		int counter;

		inline s_edge()
		{
			this->flow = 0;
			this->capacity = 0;
			this->tau = 0;
			this->enable = true;
			this->counter = 1;
		}

		inline s_edge(double capacity, bool enable)
		{
			this->flow = 0;
			this->capacity = capacity;
			this->tau = 0;
			this->enable = enable;
			this->counter = 1;
		}

		inline double getResidualCapacity()
		{
			return flow >= capacity ? 0 : capacity - flow;
		}

		inline bool isSaturated()
		{
			return flow >= capacity;
		}
	} network_edge;

	string to_string(network_edge &edge);

	typedef struct s_edgekey
	{
	public:
		int id1;
		int id2;

		inline s_edgekey() {}

		inline s_edgekey(const int &s, const int &t)
		{
			id1 = s;
			id2 = t;
		}

		inline bool operator==(const struct s_edgekey &other) const
		{
			return (id1 == other.id1 && id2 == other.id2);
		}

		inline bool operator!=(const struct s_edgekey &other) const
		{
			return (id1 != other.id1 || id2 != other.id2);
		}

	} network_edgekey;
	string to_string(network_edgekey &key);

	struct network_edgekey_hasher
	{
		inline std::size_t operator()(const network_edgekey &k) const
		{
			return hash<int>()(k.id1) ^ hash<int>()(k.id2);
		}
	};

	class topology
	{
	private:
		vector<network_edgekey> _keys;
		unordered_map<network_edgekey, network_edge, network_edgekey_hasher> _map;

	public:
		topology();
		~topology();

		inline unordered_map<network_edgekey, network_edge, network_edgekey_hasher>::iterator begin()
		{
			return this->_map.begin();
		}

		inline unordered_map<network_edgekey, network_edge, network_edgekey_hasher>::iterator end()
		{
			return this->_map.end();
		}

		inline int size(bool only_enable) const
		{
			return size(only_enable, false);
		}

		inline int size() const
		{
			return size(false, false);
		}

		inline void clear()
		{
			this->_keys.clear();
			this->_map.clear();
		}

		inline network_edge &get(const network_edgekey &key)
		{
			return this->_map.at(key);
		}

		inline network_edgekey &getKey(int index)
		{
			return this->_keys.at(index);
		}

		inline network_edge &get(int index)
		{
			return this->_map.at(this->_keys.at(index));
		}

		inline bool contains(const network_edgekey &key)
		{
			return CONTAINS_MAP(this->_map, key);
		}

		int size(bool only_enable, bool only_internal) const;

		bool link(int node1, int node2, double capacity, bool increaseIfExist = true);

		int clone(network_edgekey *keys, network_edge *edges);

		void read(string &filename);

		void write(string &filename);

		int sizeInternalEdges();

		int pruning(bool massive, bool compact);

		int fullconnected(core::rangedouble capacity_range, core::RandomNumberGenerator *rdn);

		int uniqueNodes();

	private:
		void updateKeysByMap();
	};
}
#endif