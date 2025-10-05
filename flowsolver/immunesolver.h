#pragma once
#ifndef _H_IMUNES
#define _H_IMUNES

#include "flowsolverbase.h"

namespace flowsolver
{
    typedef struct s_immune_candidate
    {
        ~s_immune_candidate();

        graph::network_partition partition;
        graph::network_cut cut;
        int age;             //numero di iterazione in cui e' stato creato
        int max_age;         //numero di iterazione in cui e' presente, all'iterazione max_age+age viene eliminato
        int mutation;        //numero di mutazioni subbite
        double cut_capacity; //capacità del taglio
        bool is_clone;       //indica se questo e un clone nell'iterazione corrente
        bool is_mutate;      //indica se nell'iterazione corrente e' mutato

    } ImmuneCandidate;

    string to_string(ImmuneCandidate &candidate, int current_age);
    void copy(ImmuneCandidate *source, ImmuneCandidate *target);
    void clone(ImmuneCandidate *source, ImmuneCandidate *target);

    class ImmuneSolution : public Solution
    {
    private:
        ImmuneCandidate _candidate;

    public:
        inline ImmuneCandidate &getCandidate()
        {
            return _candidate;
        }
        inline string toString(bool compact = false)
        {
            if (compact)
                return graph::to_string(_candidate.cut);
            else
                return to_string(_candidate, -1);
        }
        inline void extract(ImmuneCandidate *candidate)
        {
            copy(candidate, &_candidate);
        }
        inline bool isBest(Solution *solution)
        {
            return _candidate.cut_capacity < ((ImmuneSolution *)solution)->_candidate.cut_capacity;
        }
        inline double getFitness()
        {
            return _candidate.cut_capacity;
        }
    };

    enum MutationOperatorType
    {
        MUTATION_OPERATOR_SHUFFLE = 0,
        MUTATION_OPERATOR_CROSS_OVER = 1
    };

    typedef struct s_immune_option
    {
        int population_size = 100;
        core::rangeint age_range = core::rangeint(3, 10);
        int cloning_factor = 2;
        int max_interation = 1000;
        bool enable_regression = false;
        double probs_mutation_partition_size = 0.4;
        int delta_prediction = 3;
        double rho = 1;
        int min_partition_size = 2;
        core::rangedouble regression_stable_interval = core::rangedouble(-0.01, 0.01);
        MutationOperatorType mutation_operator = MUTATION_OPERATOR_SHUFFLE;
    } ImmuneOption;

    class ImmuneSolver : public FlowSolver
    {
    private:
        ImmuneCandidate *_population;
        int _current_population_size;
        core::NumericBuffer _fitness_buffer;
        ImmuneOption _option;

        int _count_stable_mutation_rate = 0;

    public:
        ImmuneSolver();

        void reset();

        void solver();

        void setArguments(map<string, string> &optset);

        void printInput(ostream &stream);

        void printOutput(ostream &stream);

        void dispose();

        inline void printStatistic(ostream &stream)
        {
            printStatisticInternal(stream, true, false, true);
        }

    private:
        void initializePopulation();
        void computeFitness();
        void incrementAge();
        void cloningOperator();
        void hypermutationOperator();
        void selectionOperator();

        int generateCandidate(ImmuneCandidate *population, int start, int lenght);
        void clone(ImmuneCandidate *source, ImmuneCandidate *target);
        double computeMutationRate();
    };
}

#endif