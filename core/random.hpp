#pragma once
#ifndef _H_RAND
#define _H_RAND

#include "includes.h"
namespace core
{

    class RandomNumberGenerator
    {
    private:
        std::mt19937 m_generator;

    public:
        RandomNumberGenerator() : m_generator(time(0))
        {
        }

        inline std::mt19937 &generator()
        {
            return m_generator;
        }

        inline int getInt(const rangeint &range)
        {
            uniform_int_distribution<int> uni(range.getMin(), range.getMax());
            int v = uni(m_generator);
            return v;
        }

        int getIntEsclude(const rangeint &range, const int &esclude);

        int getIntEsclude(const rangeint &range, const int *escludes, int &size);

        inline int getInt(vector<double> &distribution)
        {
            std::discrete_distribution<int> d(distribution.begin(), distribution.end());
            return d(this->m_generator);
        }

        int getInt(const rangeint &r, int *&v, const double factor, const double expanse, bool recycle);

        inline int getInt(const rangeint &r, int *&v, const double factor, const double expanse)
        {
            return getInt(r, v, factor, expanse, false);
        }

        int shuffleInt(int *&v, int size, double p);

        int shuffleDouble(double *&v, int size, double p);

        inline bool prob(double p)
        {
            if (p <= 0)
                return false;
            else if (p >= 1)
                return true;

            rangeint rn(0, 100000);
            int r = getInt(rn);

            int v = p * 100000;
            return r <= v;
        }

        int argmax(double *v, int size, bool deterministic);

        inline unsigned int getUInt(const rangeuint &range)
        {
            uniform_int_distribution<unsigned int> uni(range.getMin(), range.getMax());
            unsigned int v = uni(m_generator);
            return v;
        }

        inline double getDouble(const rangedouble &range)
        {
            uniform_real_distribution<double> uni(range.getMin(), range.getMax());
            double v = uni(m_generator);
            return v;
        }

        inline long getLong(const rangelong &range)
        {
            uniform_int_distribution<long> uni(range.getMin(), range.getMax());
            long v = uni(m_generator);
            return v;
        }
    };

    int RandomNumberGenerator::getIntEsclude(const rangeint &range, const int &esclude)
    {
        int count = range.size();
        for (int i = 0; i < count; i++)
        {
            int v = getInt(range);
            if (v != esclude)
                return v;
        }
        throw "error";
    }

    template <typename T>
    class RandomNumberGeneratorByDistribution
    {
    private:
        using generic_value = T;
        struct internal_item
        {
            generic_value value;
            double probability;

            internal_item(double probability, generic_value value)
            {
                this->value = value;
                this->probability = probability;
            }
        };

        vector<internal_item *> _items;
        RandomNumberGenerator *_random;
        double _totalPossibility;

    public:
        inline RandomNumberGeneratorByDistribution(RandomNumberGenerator *random)
        {
            _random = random;
            _totalPossibility = 0;
        }
        inline RandomNumberGeneratorByDistribution()
        {
            _random = new RandomNumberGenerator();
            _totalPossibility = 0;
        }
        inline ~RandomNumberGeneratorByDistribution()
        {
            clear();
        }

        inline void add(double &probability, generic_value &value)
        {
            internal_item *item = new internal_item(probability, value);
            _items.push_back(item);
            _totalPossibility += probability;
        }

        inline void clear()
        {
            for (auto &&e : _items)
            {
                delete e;
            }
            _items.clear();
        }

        inline generic_value next()
        {
            double rand = _random->getDouble({0.0, 1.0}) * _totalPossibility;
            double value = 0;
            for (auto &&e : _items)
            {
                value += e->probability;
                if (rand <= value)
                    return e->value;
            }
            return _items.back()->value;
        }

        inline generic_value max()
        {
            internal_item *candidate = nullptr;
            for (auto &&item : _items)
            {
                if (candidate == nullptr || item->probability > candidate->probability)
                {
                    candidate = item;
                }
            }
            return candidate->value;
        }
    };

    int RandomNumberGenerator::getIntEsclude(const rangeint &range, const int *escludes, int &size)
    {
        int count = range.size();
        for (int i = 0; i < count; i++)
        {
            int v = getInt(range);
            bool find = false;
            for (int j = 0; j < size; j++)
            {
                if (escludes[j] == v)
                {
                    find = true;
                    break;
                }
            }
            if (!find)
                return v;
        }
        throw "error";
    }

    int RandomNumberGenerator::getInt(const rangeint &r, int *&v, const double factor, const double expanse, bool recycle)
    {
        int min = r.getMin();
        int max = r.getMax();
        int n = (int)ceil(((double)(max - min) + 1.0) * expanse);

        rangeint range(0, n - 1);

        if (!recycle)
        {
            v = new int[n];
            for (int i = 0; i < n; i++)
            {
                v[i] = (min + i) % (max + 1);
            }
        }

        int temp;
        int m = (int)ceil((double)n * factor);
        for (int i = 0; i < m; i++)
        {
            int index1 = getInt(range);
            int index2 = getInt(range);

            temp = v[index1];
            v[index1] = v[index2];
            v[index2] = temp;
        }

        return n;
    }

    int RandomNumberGenerator::shuffleInt(int *&v, int size, double p)
    {
        int c = 0;
        rangeint range = {0, size - 1};
        for (int i = 0; i < size; i++)
        {
            if (prob(p))
            {
                int temp = v[i];
                int j = getInt(range);
                v[i] = v[j];
                v[j] = temp;
                c++;
            }
        }
        return c;
    }

    int RandomNumberGenerator::shuffleDouble(double *&v, int size, double p)
    {
        int c = 0;
        rangeint range = {0, size - 1};
        for (int i = 0; i < size; i++)
        {
            if (prob(p))
            {
                double temp = v[i];
                int j = getInt(range);
                v[i] = v[j];
                v[j] = temp;
                c++;
            }
        }
        return c;
    }

    int RandomNumberGenerator::argmax(double *v, int size, bool deterministic)
    {
        RandomNumberGeneratorByDistribution<int> ds(this);
        for (int i = 0; i < size; i++)
            ds.add(v[i], i);
        if (deterministic)
            return ds.max();
        else
            return ds.next();

        /*int max_index = 0;
	int m = v[0];
	bool is_some_equal = true;

	for (int i = 1; i < size; i++)
	{
		if (is_some_equal && v[i] != m)
			is_some_equal = false;

		if (v[i] > v[max_index])
			max_index = i;
	}

	if (is_some_equal)
	{
		max_index = getInt({0, size - 1});
	}
	return max_index;*/
    }

    template <typename T>
    void swap(T *x1, T *x2)
    {
        T temp = *x1;
        *x1 = *x2;
        *x2 = temp;
    }

    template <typename T>
    void shuffle(T *array, int size, RandomNumberGenerator &random)
    {
        int indexs[size];
        random.getInt({0, size}, indexs, 2, 1);

        for (int i = 0; i < size; i++)
        {
            swap(&array[i], &array[indexs[i]]);
        }
    }
}
#endif