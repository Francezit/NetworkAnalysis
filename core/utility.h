#pragma once
#ifndef _H_UTILITY
#define _H_UTILITY

#include "includes.h"

namespace core
{

	string to_string(double &x, const int decDigits);

	vector<string> split(const string &s, char delim, bool removeEmpty = true);

	vector<string> split(string s, string delimiter, bool removeEmpty = true);

	inline string to_string_format(const double x)
	{
		if (isnan(x))
			return "NaN";
		if (isinf(x))
			return "Inf";

		string temp = std::to_string(x);
		remove(temp.begin(), temp.end(), ',');
		//replace(temp.begin(), temp.end(), '.', ',');
		return temp;
	}

	inline string to_string_format(const int x)
	{
		return std::to_string(x);
	}

	inline string to_string_format(const bool x)
	{
		if (x)
		{
			return "true";
		}
		else
		{
			return "false";
		}
	}

	string to_string_format(const double *x, int count);

	string to_string_format(const int *x, int count);

	tuple<string, string> splitCommand(string line, char delimeter);

	template <typename T>
	string to_string_format(vector<T> &v)
	{
		stringstream s;
		int count = v.size();
		s << '(';
		for (int i = 0; i < count; i++)
		{
			s << to_string(v.at(i));
			if (i < count - 1)
			{
				s << ',';
			}
		}
		s << ')';
		return s.str();
	}

	template <typename T>
	int getIndex(vector<T> *v, T K)
	{
		auto it = find(v->begin(), v->end(), K);
		if (it != v->end())
		{
			int index = it - v->begin();
			return index;
		}
		else
		{
			return -1;
		}
	}

	template <typename T>
	int getIndex(T *array, int size, T &value)
	{
		for (int i = 0; i < size; i++)
		{
			if (array[i] == value)
				return i;
		}
		return -1;
	}

	template <typename T>
	int exists(T *array, int start, int size, T &value)
	{
		for (int i = start; i < size; i++)
		{
			if (array[i] == value)
				return true;
		}
		return false;
	}

	template <typename T>
	double average(T *array, int size)
	{
		T sum = 0;
		for (int i = 0; i < size; i++)
			sum += array[i];
		return (double)sum / (double)size;
	}

	template <typename T>
	void setAll(T *array, int size, T &def)
	{
		for (int i = 0; i < size; i++)
			array[i] = def;
	}

	template <typename T>
	void divAll(T *array, int size, T &value)
	{
		for (int i = 0; i < size; i++)
			array[i] /= value;
	}

}
#endif
