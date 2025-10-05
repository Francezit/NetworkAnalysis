#include "utility.h"

vector<string> core::split(const string &s, char delim, bool removeEmpty)
{
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim))
	{
		if (!removeEmpty || !item.empty())
			result.push_back(item);
	}

	return result;
}

vector<string> core::split(string s, string delimiter, bool removeEmpty)
{
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	string token;
	vector<string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
	{
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		if (!removeEmpty || !token.empty())
			res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

string to_string(double &x, const int decDigits)
{
	stringstream ss;
	ss << fixed;
	ss.precision(decDigits);
	ss << x;
	return ss.str();
}

string to_string_format(const double *x, int count)
{
	stringstream s;
	s << '(';
	for (int i = 0; i < count; i++)
	{
		s << core::to_string_format(x[i]);
		if (i < count - 1)
		{
			s << ',';
		}
	}
	s << ')';
	return s.str();
}

string to_string_format(const int *x, int count)
{
	stringstream s;
	s << '(';
	for (int i = 0; i < count; i++)
	{
		s << x[i];
		if (i < count - 1)
		{
			s << ',';
		}
	}
	s << ')';
	return s.str();
}

tuple<string, string> splitCommand(string line, char delimeter)
{
	size_t index = line.find(delimeter);
	if (index == string::npos)
	{
		return {line, ""};
	}
	else
	{
		return {line.substr(0, index), line.substr(index + 1)};
	}
}
