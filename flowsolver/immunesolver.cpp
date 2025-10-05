#include "immunesolver.h"

using namespace flowsolver;
using namespace core;
using namespace graph;

ImmuneCandidate::~s_immune_candidate()
{
}

string flowsolver::to_string(ImmuneCandidate &source, int current_age)
{
    stringstream ss;
    if (current_age >= 0 && source.max_age + source.age <= current_age)
    {
        ss << "\t"
           << "Years Old: " << to_string_format(current_age - source.age) << endl;
    }
    ss << "\t"
       << "Age: " << to_string_format(source.age) << endl
       << "\t"
       << "Max Age: " << to_string_format(source.max_age) << endl
       << "\t"
       << "Mutation: " << to_string_format(source.mutation) << endl
       << "\t"
       << "Cut Capacity: " << to_string_format(source.cut_capacity) << endl
       << "\t"
       << "Is Clone: " << to_string_format(source.is_clone) << endl
       << "\t"
       << "Is Mutate: " << to_string_format(source.is_mutate) << endl
       << "\t"
       << "Is Valid: " << to_string_format(source.cut.is_valid) << endl
       << "\t"
       << "Cut: " << to_string(source.cut) << endl;
    return ss.str();
}

void flowsolver::copy(ImmuneCandidate *source, ImmuneCandidate *target)
{
    target->age = source->age;
    target->cut_capacity = source->cut_capacity;
    target->is_clone = source->is_clone;
    target->is_mutate = source->is_mutate;
    target->max_age = source->max_age;
    target->mutation = source->mutation;
    copy(&source->partition, &target->partition);
    copy(&source->cut, &target->cut);
}

ImmuneSolver::ImmuneSolver(/* args */)
{
}

void ImmuneSolver::dispose()
{
    this->internalReset();
    _fitness_buffer.clear();
    delete[] _population;
    delete _network;
}

void ImmuneSolver::initializePopulation()
{
    int size = _option.population_size;

    _population = new ImmuneCandidate[size];
    generateCandidate(_population, 0, size);
    _current_population_size = size;
    _current_interation = 0;
}

void ImmuneSolver::computeFitness()
{
    double avg_fitness = 0;

    statistic *stat = _statistics.back();
    ImmuneCandidate *best_candidate = &_population[0];

    int k = 0;
    for (int i = 0; i < _current_population_size; i++)
    {
        ImmuneCandidate *candidate = &_population[i];
        if (!IS_VALID_CUT_CAPACITY(candidate->cut_capacity))
        {
            candidate->cut_capacity = _network->computeCutCapacity(candidate->cut);
        }

        if (IS_VALID_CUT_CAPACITY(candidate->cut_capacity))
        {
            avg_fitness += candidate->cut_capacity;
            if (candidate->cut_capacity < best_candidate->cut_capacity)
            {
                best_candidate = candidate;
            }
            k++;
        }
    }
    avg_fitness /= (double)k;

    stat->count_edges = _network->countEdges();
    stat->count_nodes = _network->countNodes();
    stat->interations = _current_interation;
    stat->avg_fitness = avg_fitness;
    stat->fitness = best_candidate->cut_capacity;

    ImmuneSolution *solution = new ImmuneSolution();
    solution->extract(best_candidate);
    stat->output = solution;

    if (_best_result == nullptr || stat->fitness < _best_result->fitness)
    {
        _best_result = stat;
    }
}

int ImmuneSolver::generateCandidate(ImmuneCandidate *population, int start, int lenght)
{
    int end = start + lenght;
    int node_size = _network->countNodes();
    int count = 0;

    int *v = new int[node_size];
    _network->populateNodeIds(v, 0, false);

    int k = _option.min_partition_size;

    for (int i = start; i < end; i++)
    {
        population[i].age = _current_interation;
        population[i].is_clone = false;
        population[i].is_mutate = false;
        population[i].max_age = _random->getInt(_option.age_range);
        population[i].mutation = 0;
        population[i].cut_capacity = -1;

        _random->shuffleInt(v, node_size, 0.6);
        int splitter = _random->getInt({k, node_size - k});

        network_partition *partition = &(population[i].partition);
        partition->copyFrom(v, node_size, splitter);

        network_cut *cut = &(population[i].cut);
        _network->computeNetworkCut(partition, cut);

        count++;
    }
    delete[] v;
    return count;
}

void ImmuneSolver::incrementAge()
{
    for (int i = 0; i < _current_population_size; i++)
    {
        ImmuneCandidate &item = _population[i];
        item.is_clone = false;
        item.is_mutate = false;
    }
    _current_interation++;
}

void ImmuneSolver::clone(ImmuneCandidate *source, ImmuneCandidate *target)
{
    target->age = _current_interation;
    target->cut_capacity = source->cut_capacity;
    target->is_clone = true;
    target->is_mutate = false;
    target->max_age = _random->getInt(_option.age_range);
    target->mutation = 0;
    copy(&source->partition, &target->partition);
    copy(&source->cut, &target->cut);
}

void ImmuneSolver::cloningOperator()
{
    double cloning_factor = _option.cloning_factor;
    int sizePC = _current_population_size * (1 + cloning_factor);
    ImmuneCandidate *populationC = new ImmuneCandidate[sizePC];

    for (int i = 0, k = 0; i < _current_population_size; i++)
    {
        copy(&_population[i], &populationC[k++]);
        for (int j = 0; j < cloning_factor; j++)
        {
            clone(&_population[i], &populationC[k++]);
        }
    }

    delete[] _population;

    _current_population_size = sizePC;
    _population = populationC;
}

double ImmuneSolver::computeMutationRate()
{
    statistic *stat = _statistics.back();

    double cut_capacity = 0;
    if (_option.enable_regression && _fitness_buffer.isValid())
    {
        double m = _fitness_buffer.angularCoefficient();
        if (_option.regression_stable_interval.contains(m))
        {
            cut_capacity = _fitness_buffer.mean();
            if (_count_stable_mutation_rate >= _option.delta_prediction)
            {
                cut_capacity += exp2(_count_stable_mutation_rate);
            }

            _count_stable_mutation_rate++;
        }
        else
        {
            cut_capacity = _fitness_buffer.stimate(_option.delta_prediction);
            cut_capacity = SATURATE_MIN(cut_capacity, 0);
            _count_stable_mutation_rate = 0;
        }
    }
    else if (!_fitness_buffer.empty())
    {
        cut_capacity = _best_result->fitness;
    }
    else
    {
        return 0.5;
    }

    stat->alfa = _fitness_buffer.loss();
    stat->beta = cut_capacity;
    stat->gamma = _count_stable_mutation_rate;

    double best_cut_capacity = _best_result->fitness;
    double g = best_cut_capacity / cut_capacity;
    double alfa = pow(MATH_E, -_option.rho * g);
    double mutation_rate = (alfa * _network->countNodes()) + 1;
    return mutation_rate / _network->countNodes();
}

void ImmuneSolver::hypermutationOperator()
{
    statistic *stat = _statistics.back();

    double p = computeMutationRate();

    stat->mutation_probability = SATURATE(p, 0.99, 0.01);
    stat->mutation_count = 0;

    for (int i = 0; i < _current_population_size; i++)
    {
        ImmuneCandidate &source = _population[i];
        if (source.is_clone)
        {
            int mutation_count = 0;
            if (_option.mutation_operator == MUTATION_OPERATOR_SHUFFLE)
            {
                mutation_count = shuffle_partition(&source.partition, p, _option.probs_mutation_partition_size, _option.min_partition_size, _random);
            }
            else if (_option.mutation_operator == MUTATION_OPERATOR_CROSS_OVER)
            {
            }

            if (mutation_count > 0)
            {
                source.mutation += mutation_count;
                source.is_mutate = true;
                source.cut_capacity = -1;
                _network->computeNetworkCut(&source.partition, &source.cut);

                stat->mutation_count += mutation_count;
            }
        }
    }
}

typedef struct
{
    int index;
    double cut_capacity;
    int cut_size;
    int max_age;
    int yearsOld;

} CandidatePointer;

bool compareCandidatePointerSize(CandidatePointer &i1, CandidatePointer &i2)
{
    return i1.cut_size < i2.cut_size;
}

bool compareCandidatePointerMin(CandidatePointer &i1, CandidatePointer &i2)
{
    return i1.cut_capacity < i2.cut_capacity;
}

void ImmuneSolver::selectionOperator()
{
    statistic *stat = _statistics.back();

    int list_size = 0;
    CandidatePointer *list = new CandidatePointer[_current_population_size];
    for (int i = 0; i < _current_population_size; i++)
    {
        if (_population[i].cut.is_valid)
        {
            CandidatePointer &candidate = list[list_size];
            candidate.index = i;
            candidate.yearsOld = _current_interation - _population[i].age;
            candidate.max_age = _population[i].max_age;
            candidate.cut_capacity = _population[i].cut_capacity;
            candidate.cut_size = _population[i].cut.count_size;
            list_size++;
        }
        else
        {
            stat->invalid++;
        }
    }
    sort(list, list + list_size, compareCandidatePointerSize);

    int count_population = 0;
    int population_size = _option.population_size;
    ImmuneCandidate *population = new ImmuneCandidate[population_size];
    for (int i = 0; i < list_size && count_population < population_size; i++)
    {
        CandidatePointer &pointer = list[i];
        if (pointer.yearsOld < pointer.max_age)
        {
            ImmuneCandidate &item = _population[pointer.index];
            copy(&item, &(population[count_population++]));
        }
    }

    //genero nuovi candidati
    if (count_population < population_size)
    {
        int new_candidate = generateCandidate(population, count_population, population_size - count_population);
        count_population += new_candidate;
    }

    delete[] list;
    delete[] _population;

    _population = population;
    _current_population_size = count_population;
}

void ImmuneSolver::reset()
{
    this->internalReset();
    delete[] _population;
    _population = nullptr;
    _current_population_size = 0;
    _fitness_buffer.clear();
    _count_stable_mutation_rate = 0;
}

void ImmuneSolver::solver()
{
    if (this->_state != SOLVER_READY)
        throw "state is not valid";

    this->_start_timestamp = GET_CURRENT_TIMESTAMP;
    this->_state = SOLVER_COMPUTING;

    print("Start solver", false);
    printInput(_logger);

    double executive_time = 0;
    double global_executive_time = 0;

    statistic *stat;
    {
        stat = new statistic();
        _statistics.push_back(stat);

        stat->interations = 0;
        stat->start_timestamp = GET_CURRENT_TIMESTAMP;

        initializePopulation();
        computeFitness();

        stat->end_timestamp = GET_CURRENT_TIMESTAMP;
    }
    while (_current_interation < _option.max_interation)
    {
        statistic *last_stat = stat;

        stat = new statistic();
        _statistics.push_back(stat);

        stat->start_timestamp = GET_CURRENT_TIMESTAMP;

        EXECUTIVE_LOGGER(endl << endl
                              << "---------------------------",
                         false);

        EXECUTIVE_OPERATION(incrementAge, executive_time);
        EXECUTIVE_LOGGER("incrementAge: " << executive_time << "ms", false);

        EXECUTIVE_OPERATION(cloningOperator, executive_time);
        EXECUTIVE_LOGGER("cloningOperator: " << executive_time << "ms", false);

        EXECUTIVE_OPERATION(hypermutationOperator, executive_time);
        EXECUTIVE_LOGGER("hypermutationOperator: " << executive_time << "ms", false);

        EXECUTIVE_OPERATION(computeFitness, executive_time);
        EXECUTIVE_LOGGER("computeFitness: " << executive_time << "ms", false);

        EXECUTIVE_OPERATION(selectionOperator, executive_time);
        EXECUTIVE_LOGGER("selectionOperator: " << executive_time << "ms", false);

        stat->end_timestamp = GET_CURRENT_TIMESTAMP;

        double avg_increment = 100.0 * ((stat->avg_fitness - last_stat->avg_fitness) / last_stat->avg_fitness);
        double best_increment = 100.0 * ((stat->fitness - last_stat->fitness) / last_stat->fitness);
        ImmuneSolution *best = (ImmuneSolution *)stat->output;

        this->_fitness_buffer.insert(stat->fitness, true);
        stat->interations = _current_interation;

        double executive_time = 0;
        TIMESTAMP_ELAPSED_MS(stat->start_timestamp, stat->end_timestamp, executive_time);
        global_executive_time += executive_time;

        EXECUTIVE_LOGGER_CONSOLE(endl
                                 << "AGE: " << to_string_format(_current_interation) << endl
                                 << "AVG CUT CAPACITY: " << to_string_format(stat->avg_fitness) << " (" << to_string_format(avg_increment) << "%)" << endl
                                 << "CUT CAPACITY: " << to_string_format(stat->fitness) << " (" << to_string_format(best_increment) << "%)" << endl
                                 << "MUTATION RATE: " << to_string_format(stat->mutation_probability) << endl
                                 << "TIME: " << to_string_format(executive_time) << "/" << to_string_format(global_executive_time) << endl
                                 << "BEST CANDIDATE" << endl
                                 << to_string(best->getCandidate(), _current_interation));

        EXECUTIVE_LOGGER(endl << "Statistiche" << endl
                              << to_string(stat),
                         false);
    }

    this->_end_timestamp = GET_CURRENT_TIMESTAMP;
    this->_state = SOLVER_COMPUTED;

    TIMESTAMP_ELAPSED_MS(this->_start_timestamp, this->_end_timestamp, executive_time)
    EXECUTIVE_LOGGER("Total time: " << executive_time << "ms", true);
}

void ImmuneSolver::setArguments(map<string, string> &optset)
{
    if (this->_state != SOLVER_INIT)
        throw "state is not valid";

    int opt;
    int temp1, temp2;

    for (auto &&pair : optset)
    {
        string opt = pair.first;
        string &optarg = pair.second;

        int temp1, temp2;
        if (opt == "age_range")
        {
            temp1 = stoi(optarg);
            temp2 = floor(temp1 / 2.0);
            temp1 = (temp1 * 2) - temp2;
            _option.age_range = rangeint(temp2, temp1);
        }
        else if (opt == "cloning_factor")
        {
            _option.cloning_factor = stoi(optarg);
        }
        else if (opt == "delta_prediction")
        {
            _option.delta_prediction = stoi(optarg);
        }
        else if (opt == "enable_regression")
        {
            _option.enable_regression = true;
        }
        else if (opt == "max_interation")
        {
            _option.max_interation = stoi(optarg);
        }
        else if (opt == "population_size")
        {
            _option.population_size = stoi(optarg);
        }
        else if (opt == "probs_mutation_partition_size")
        {
            _option.probs_mutation_partition_size = stod(optarg);
        }
        else if (opt == "rho")
        {
            _option.rho = stod(optarg);
        }
        else if (opt == "min_partition_size")
        {
            _option.min_partition_size = stoi(optarg);
        }
        else if (opt == "enable_log")
        {
            this->_enable_log = true;
            setLogFile(optarg);
        }
    }

    this->_state = SOLVER_READY;
}

void ImmuneSolver::printInput(ostream &stream)
{
    stream << "PARAMETERS" << endl
           << "Age range: " << _option.age_range.toString() << endl
           << "Cloning factor: " << _option.cloning_factor << endl
           << "Delta prediction: " << _option.delta_prediction << endl
           << "Max interation: " << _option.max_interation << endl
           << "Population size: " << _option.population_size << endl
           << "Probs mutation partition size: " << _option.probs_mutation_partition_size << endl
           << "Rho: " << _option.rho << endl
           << "Mutation operator: " << _option.mutation_operator << endl
           << "Min partition size: " << _option.min_partition_size << endl;

    stream << "TOPOLOGY" << endl
           << _network->generateMATLABScript(true, false, false) << endl;
}

void ImmuneSolver::printOutput(ostream &stream)
{
    stream << "BEST OUTPUT: " << endl
           << to_string(this->_best_result) << endl;
}