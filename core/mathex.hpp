#pragma once
#ifndef _H_MATH_A
#define _H_MATH_A

#include "includes.h"
#include "utility.h"

namespace core
{

#define MATH_PI 3.14159265358979323846
#define MATH_PI_2 1.57079632679489661923
#define MATH_PI_4 0.78539816339744830962
#define MATH_E 2.7182818284590452354

    const int ZEROI = 0;
    const double ZEROD = 0.0;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define AVG(a, b) (((a) + (b)) / 2.0)

#define ABS(a) SIGN(a) * a;
#define SIGN(a) ((a) >= 0 ? 1 : -1)
#define INVD(a) (1.0 / ((double)(a)))
#define INVI(a) (1 / ((int)(a)))
#define INVIF(a) (floor(INVD(a)))
#define INVIC(a) (ceil(INVD(a)))

#define SATURATE(a, max, min) (((a) > (max)) ? (max) : (((a) < (min)) ? (min) : (a)))
#define SATURATE_MIN(a, min) (((a) < (min)) ? (min) : (a))
#define SATURATE_MAX(a, max) (((a) > (max)) ? (max) : (a))

    template <typename T>
    struct range
    {
    private:
        T _min;
        T _max;
        bool _empty;

    public:
        range()
        {
            _empty = true;
        }

        range(const T &min, const T &max)
        {
            if (min > max)
                throw "Range non valido";
            _min = min;
            _max = max;
            _empty = false;
        }

        inline bool isEmpty() { return _empty; }

        inline T getMin() const { return _min; }

        inline T getMax() const { return _max; }

        inline double getAvg() const { return AVG_CAST(double, _max, _min); }

        inline T size() const { return _max - _min; }

        inline bool contains(T &value) { return value >= _min && value <= _max; }

        inline bool containsIn(T &value) { return value > _min && value < _max; };

        inline string toString()
        {
            stringstream ss;
            ss << "[" << std::to_string(_min) << "," << std::to_string(_max) << "]";
            return ss.str();
        }
    };

    using rangeint = range<int>;
    using rangeuint = range<unsigned int>;
    using rangedouble = range<double>;
    using rangelong = range<long>;

    typedef rangeint rangeint_t;
    typedef rangeuint rangeuint_t;
    typedef rangedouble rangedouble_t;
    typedef rangelong rangelong_t;

    rangeint to_rangeint(string s)
    {
        vector<string> v = split(s, ":", true);
        int min = atoi(v[0].c_str());
        int max = atoi(v[1].c_str());
        return rangeint(min, max);
    }

    rangedouble to_rangedouble(string s)
    {
        vector<string> v = split(s, ":", true);
        double min = atof(v[0].c_str());
        double max = atof(v[1].c_str());
        return rangedouble(min, max);
    }

    long factorial(int n)
    {
        if (n < 0)
            throw "Error";
        long fact = 1;
        for (int i = n; i > 1; i--)
            fact = fact * i;
        return fact;
    }

    double binomial(int n, int k, double p)
    {
        if (n >= 0 && n < k)
            return 0;

        long factN = factorial(n);
        long factK = factorial(k);
        long factV = factorial(n - k);
        double w = (double)factN / (double)(factK * factV);

        // p^k * (1-p)^(n-k) part
        return w * pow(p, k) * pow(1.0 - p, n - k);
    }

    void softmax(double *input, size_t size)
    {
        int i;
        double m, sum, constant;

        m = -INFINITY;
        for (i = 0; i < size; ++i)
        {
            if (!isnan(input[i]) && m < input[i])
            {
                m = input[i];
            }
        }

        if (m == 0)
        {
            double p = INVD(size);
            for (i = 0; i < size; i++)
            {
                input[i] = p;
            }
        }
        else if (m > -INFINITY)
        {
            double m_temp = m;
            for (i = 0; i < size; i++)
            {
                if (isnan(input[i]))
                {
                    m_temp = m + 2;
                    input[i] = m_temp;
                }
            }
            m = m_temp;

            sum = 0.0;
            for (i = 0; i < size; ++i)
            {
                sum += exp(input[i] - m);
            }

            constant = m + log(sum);
            for (i = 0; i < size; ++i)
            {
                input[i] = exp(input[i] - constant);
            }
        }
        else
        {
            for (i = 0; i < size; ++i)
            {
                input[i] = 0;
            }
        }
    }

    class NumericBuffer
    {
    private:
        int _count_sample;
        double _offset;
        double _m;
        bool _isEnable;

        double *_buffer;
        double _cache_avg_n;
        double _cache_sum_s;
        int _buffer_pointer;

    public:
        inline NumericBuffer()
        {
            this->_buffer = nullptr;
            this->resize(20);
        }

        inline NumericBuffer(int count_samples)
        {
            this->_buffer = nullptr;
            this->resize(count_samples);
        }

        inline ~NumericBuffer()
        {
            this->disposeBuffer();
        }

        inline void insert(double sample, bool auto_update = true)
        {
            _cache_sum_s += (sample - _buffer[_buffer_pointer]);
            _buffer[_buffer_pointer] = sample;
            _buffer_pointer = (_buffer_pointer + 1) % _count_sample;
            _isEnable = _isEnable || (!_isEnable && _buffer_pointer == 0);
            if (auto_update)
                calculate();
        }

        void clear()
        {
            if (this->_buffer != nullptr)
            {
                for (int i = 0; i < this->_count_sample; i++)
                {
                    this->_buffer[i] = 0;
                }
            }
            this->_buffer_pointer = 0;
            this->_cache_sum_s = 0;
            this->_isEnable = false;

            this->_offset = 0;
            this->_m = 0;
        }

        void resize(int count_samples)
        {
            this->disposeBuffer();

            this->_count_sample = count_samples;
            this->_buffer = new double[count_samples];
            this->_buffer_pointer = 0;
            this->_cache_avg_n = 0;
            for (int i = 0; i < count_samples; i++)
            {
                this->_buffer[i] = 0;
                this->_cache_avg_n += (i + 1);
            }
            this->_cache_avg_n = this->_cache_avg_n / count_samples;
            this->_cache_sum_s = 0;
            this->_isEnable = false;

            this->_offset = 0;
            this->_m = 0;
        }

        void calculate()
        {
            if (!this->_isEnable)
                return;

            int n = this->_count_sample;
            double avg_s = this->_cache_sum_s / n;
            double avg_n = this->_cache_avg_n;
            double num_b1 = 0, det_b1 = 0;
            int i = 1, pointer = this->_buffer_pointer;
            do
            {
                num_b1 += (i - avg_n) * (this->_buffer[pointer] - avg_s);
                det_b1 += pow(i - avg_n, 2);

                i++;
                pointer = (pointer + 1) % n;
            } while (i <= n);

            _m = num_b1 / det_b1;
            _offset = avg_s - _m * avg_n;
        }

        inline bool isValid()
        {
            return _isEnable;
        }

        inline int count()
        {
            return _isEnable ? _count_sample : _buffer_pointer;
        }

        inline double mean()
        {
            int n = count();
            return n > 0 ? _cache_sum_s / n : 0.0;
        }

        inline double angularCoefficient()
        {
            return _isEnable ? _m : 0.0;
        }

        inline double offset()
        {
            return _offset;
        }

        inline int samples()
        {
            return _count_sample;
        }

        inline double last()
        {
            int pointer = _buffer_pointer > 0 ? _buffer_pointer - 1 : _count_sample - 1;
            return _buffer[pointer];
        }

        inline bool empty()
        {
            return !_isEnable && _buffer_pointer == 0;
        }

        inline double stimate(int delta_sample)
        {
            return _isEnable ? (_m * (delta_sample + _count_sample) + _offset) : 0.0;
        }

        double loss()
        {
            double sum = 0;
            int n = count();
            if (n > 0)
            {
                int i = 1, pointer = this->_buffer_pointer;
                do
                {
                    sum = pow(this->_buffer[pointer] - _m * i + _offset, 2);

                    i++;
                    pointer = (pointer + 1) % n;
                } while (i <= n);
            }
            return sum;
        }

    private:
        void disposeBuffer()
        {
            if (this->_buffer != nullptr)
            {
                delete[] this->_buffer;
            }
            this->_isEnable = false;
            this->_buffer = nullptr;
            this->_count_sample = -1;
            this->_buffer_pointer = -1;
        }
    };
}
#endif