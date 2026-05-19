#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <ostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using f64_t = double;

struct CommunicationLane {
    std::string name;
    uint32_t bandwith;
    uint32_t cost;
    std::vector<uint32_t> conections;

    CommunicationLane() : name(), bandwith(0), cost(0), conections() {}

    CommunicationLane(const CommunicationLane& other) {
        this->name = other.name;
        this->bandwith = other.bandwith;
        this->cost = other.cost;
        this->conections = other.conections;
    }

    CommunicationLane(CommunicationLane&& other) {
        this->name = std::move(other.name);
        this->bandwith = std::move(other.bandwith);
        other.bandwith = 0;
        this->cost = std::move(other.cost);
        this->cost = 0;
        this->conections = std::move(other.conections);
    }

    CommunicationLane& operator=(const CommunicationLane& other) {
        if (this != &other) {
            this->name = other.name;
            this->bandwith = other.bandwith;
            this->cost = other.cost;
            this->conections = other.conections;
        }
        return *this;
    }

    CommunicationLane& operator=(CommunicationLane&& other) {
        if (this != &other) {
            this->name = std::move(other.name);
            this->bandwith = std::move(other.bandwith);
            other.bandwith = 0;
            this->cost = std::move(other.cost);
            this->cost = 0;
            this->conections = std::move(other.conections);
        }
        return *this;
    }
};

enum class PeType {
    PP,
    HC,
};

struct ProcessingElement {
    uint32_t cost;
    uint32_t other;
    PeType type;

    ProcessingElement() : cost(0), other(0), type(PeType::PP) {}
    ProcessingElement(uint32_t c, uint32_t o, uint32_t t) : cost(c), other(o) {
        if (t == 0) {
            type = PeType::HC;
        } else {
            type = PeType::PP;
        }
    }
};

class TaskGraph {
    // Graph Input
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> adj;
    std::vector<ProcessingElement> proc;
    std::vector<std::vector<int32_t>> times;
    std::vector<std::vector<int32_t>> cost;
    std::vector<CommunicationLane> comms;

    // Processed data
    // Stores witch pe is assigned to each task
    std::vector<int32_t> task_pe;
    uint32_t max_time;
    int32_t max_penalty;
    int32_t k;
    int32_t l;
    int32_t p;

    std::vector<size_t> start_times;
    std::vector<size_t> end_times;

    public:
    // Loads task graph from providded file path
    TaskGraph(std::string file_path, uint32_t max_time_to_finish,
              int32_t max_penalty_treshhold, int32_t penalty_k,
              int32_t penalty_l, int32_t penalty_p);

    private:
    // PARSING
    static std::vector<std::vector<std::pair<uint32_t, uint32_t>>>
    parse_tasks(std::fstream& file, uint32_t tc);

    static std::vector<ProcessingElement> parse_proc(std::fstream& file,
                                                     uint32_t pe);

    static std::vector<std::vector<int32_t>>
    parse_times(std::fstream& file, uint32_t tc, uint32_t pe);

    static std::vector<std::vector<int32_t>>
    parse_cost(std::fstream& file, uint32_t tc, uint32_t pe);

    static std::vector<CommunicationLane> parse_comms(std::fstream& file,
                                                      uint32_t cl, uint32_t pe);

    uint32_t calculate_finish_time();
    uint32_t calc_cost_with_optimised_pe();

    public:
    bool penalty_cheapest();
    void display_system();
};

std::vector<std::string> split_string(std::string str, char sep);

auto main(int32_t argc, char** argv) -> int {
    std::string fp;
    uint32_t max_time;

    uint32_t penalty;
    uint32_t k;
    uint32_t l;
    uint32_t p;
    if (argc < 7) {
        std::cerr << "Provide task graph file and max time as argument"
                  << std::endl;
        return 1;
    } else {
        fp = std::string(argv[1]);
        max_time = std::stoul(argv[2]);
        penalty = std::stoul(argv[3]);
        k = std::stoul(argv[4]);
        l = std::stoul(argv[5]);
        p = std::stoul(argv[6]);
    }
    TaskGraph tg = TaskGraph(fp, max_time, penalty, k, l, p);
    auto _ = tg.penalty_cheapest();
    tg.display_system();

    return 0;
}

std::vector<std::string> split_string(std::string str, char sep) {
    std::vector<std::string> res{};
    std::stringstream ss(str);
    std::string space_split_line{};
    while (std::getline(ss, space_split_line, ' ')) {
        std::string line{};
        std::stringstream ss2(space_split_line);
        while (std::getline(ss2, line, '	')) {
            res.push_back(line);
        }
    }

    return res;
}

TaskGraph::TaskGraph(std::string file_path, uint32_t max_time_to_finish,
                     int32_t max_penalty_treshhold, int32_t penalty_k,
                     int32_t penalty_l, int32_t penalty_p) {
    this->max_time = max_time_to_finish;
    this->max_penalty = max_penalty_treshhold;
    this->k = penalty_k;
    this->l = penalty_l;
    this->p = penalty_p;

    std::fstream file;
    file.open(file_path, std::ios::in);
    if (!file) {
        std::cerr << "Cant open file to read TGFF File data: " << file_path
                  << std::endl;
        exit(1);
    }

    int32_t tc = -1;
    int32_t pe = -1;
    int32_t cl = -1;

    std::string line;
    while (std::getline(file, line)) {
        if (line.size() > 0 && line.at(0) == '@') {
            std::vector<std::string> line_params = split_string(line, ' ');

            if (line_params[0].starts_with("@tasks")) {
                tc = std::stoi(line_params[1]);
                this->adj = this->parse_tasks(file, tc);
                // std::cout << "tasks size " << adj.size() << "\n";
            } else if (line_params[0].starts_with("@proc")) {
                pe = std::stoi(line_params[1]);
                this->proc = this->parse_proc(file, pe);
                // std::cout << "proc size " << proc.size() << "\n";
            } else if (line_params[0].starts_with("@times")) {
                this->times = this->parse_times(file, tc, pe);
                // std::cout << "times size " << times.size() << "\n";
            } else if (line_params[0].starts_with("@cost")) {
                this->cost = this->parse_cost(file, tc, pe);
                // std::cout << "cost size " << cost.size() << "\n";
            } else if (line_params[0].starts_with("@comm")) {
                cl = std::stoi(line_params[1]);
                this->comms = this->parse_comms(file, cl, pe);
            }
        }
    }

    this->task_pe = std::vector<int32_t>(this->adj.size(), -1);
    this->start_times = std::vector<size_t>(this->adj.size());
    this->end_times = std::vector<size_t>(this->adj.size());
}

std::vector<std::vector<std::pair<uint32_t, uint32_t>>>
TaskGraph::parse_tasks(std::fstream& file, uint32_t tc) {
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> adj(tc);

    std::string line{};
    // Parse adj list of every task
    for (uint32_t t = 0; t < tc; t++) {
        if (std::getline(file, line)) {
            // parse line
            auto line_components = split_string(line, ' ');
            uint32_t succesors = std::stoi(line_components[1]);
            if (line_components.size() < 2 &&
                line_components.size() < succesors + 2) {
                // invalid adj structure
                std::cerr << "Task " << t << " has wrong adj list declaration"
                          << std::endl;
                continue;
            }

            for (uint32_t i = 2; i < succesors + 2; i++) {
                // component structure 12(9)
                auto weight_start_pos =
                    std::distance(line_components[i].begin(),
                                  std::find(line_components[i].begin(),
                                            line_components[i].end(), '('));

                auto weight_len =
                    weight_start_pos +
                    std::distance(std::find(line_components[i].begin(),
                                            line_components[i].end(), '('),
                                  std::find(line_components[i].begin(),
                                            line_components[i].end(), ')'));

                // Emplace pair of connecting edge and weight to adj
                adj[t].emplace_back(
                    std::stoi(line_components[i].substr(0, weight_start_pos)),
                    // We have to offset start of substr to skip '(' and
                    // then calculate len of number in ()
                    std::stoi(line_components[i].substr(weight_start_pos + 1,
                                                        weight_len - 2)));
            }
        } else {
            std::cerr << "@tasks section has to many tasks declared."
                      << std::endl;
            throw std::invalid_argument("File structure not valid");
        }
    }

    return adj;
}

std::vector<ProcessingElement> TaskGraph::parse_proc(std::fstream& file,
                                                     uint32_t pe_count) {
    std::vector<ProcessingElement> proc{};

    std::string line{};
    // Parse params of every PE and HC
    for (uint32_t p = 0; p < pe_count; p++) {
        if (std::getline(file, line)) {
            // parse line
            auto line_components = split_string(line, ' ');
            if (line_components.size() < 3) {
                // invalid proc structure
                std::cerr << "Proc " << p << " has wrong param declaration"
                          << std::endl;
                continue;
            }
            proc.push_back(ProcessingElement(std::stoi(line_components[0]),
                                             std::stoi(line_components[1]),
                                             std::stoi(line_components[2])));
        } else {
            std::cerr << "@proc section has to many procesors declared."
                      << std::endl;
            throw std::invalid_argument("File structure not valid");
        }
    }

    return proc;
}

std::vector<std::vector<int32_t>>
TaskGraph::parse_times(std::fstream& file, uint32_t tc, uint32_t pe) {
    std::vector<std::vector<int32_t>> times(tc, std::vector<int32_t>(pe, 0));

    std::string line{};
    // Parse params of every PE and HC
    for (uint32_t t = 0; t < tc; t++) {
        if (std::getline(file, line)) {
            // parse line
            auto line_components = split_string(line, ' ');
            if (line_components.size() < pe) {
                // invalid proc structure
                std::cerr << "Times " << t << " has wrong param declaration"
                          << std::endl;
                continue;
            }

            for (uint32_t p = 0; p < pe; p++) {
                times[t][p] = std::stoi(line_components[p]);
            }

        } else {
            std::cerr << "@times section has not enough times declared."
                      << std::endl;
            throw std::invalid_argument("File structure not valid");
        }
    }
    return times;
}

std::vector<std::vector<int32_t>>
TaskGraph::parse_cost(std::fstream& file, uint32_t tc, uint32_t pe) {
    std::vector<std::vector<int32_t>> cost(tc, std::vector<int32_t>(pe, 0));

    std::string line{};
    // Parse params of every PE and HC
    for (uint32_t t = 0; t < tc; t++) {
        if (std::getline(file, line)) {
            // parse line
            auto line_components = split_string(line, ' ');
            if (line_components.size() < pe) {
                // invalid proc structure
                std::cerr << "Cost " << t << " has wrong param declaration"
                          << std::endl;
                continue;
            }

            for (uint32_t p = 0; p < pe; p++) {
                cost[t][p] = std::stoi(line_components[p]);
            }

        } else {
            std::cerr << "@cost section has not enough costs declared."
                      << std::endl;
            throw std::invalid_argument("File structure not valid");
        }
    }
    return cost;
}

std::vector<CommunicationLane>
TaskGraph::parse_comms(std::fstream& file, uint32_t cl, uint32_t pe) {
    std::vector<CommunicationLane> comm(cl);

    std::string line{};
    // Parse params of every CL
    for (uint32_t c = 0; c < cl; c++) {
        if (std::getline(file, line)) {
            // parse line
            auto line_components = split_string(line, ' ');
            if (line_components.size() < pe + 3) {
                //             // invalid proc structure
                std::cerr << "Cl " << c << " has wrong param declaration"
                          << std::endl;
                continue;
            }

            comm[c].name = line_components[0];
            comm[c].cost = std::stoi(line_components[1]);
            comm[c].bandwith = std::stoi(line_components[2]);

            for (uint32_t p = 0; p < pe; p++) {
                comm[c].conections.push_back(std::stoi(line_components[3 + p]));
            }
        } else {
            std::cerr << "@comm section has not enough CLs declared."
                      << std::endl;
            throw std::invalid_argument("File structure not valid");
        }
    }
    return comm;
}

uint32_t TaskGraph::calculate_finish_time() {
    // Stores Indegree of node
    std::vector<size_t> indegre(this->adj.size());
    for (auto& nb_list : adj) {
        for (auto& n : nb_list) {
            ++indegre[n.first];
        }
    }

    std::vector<uint32_t> task_time(this->adj.size(), 0);
    std::iota(task_time.begin(), task_time.end(), 0);
    std::transform(task_time.begin(), task_time.end(), task_time.begin(),
                   [&, this](uint32_t t_id) {
                       if (this->task_pe[t_id] == -1) {
                           return 0;
                       }
                       return this->times[t_id][this->task_pe[t_id]];
                   });

    start_times.assign(this->adj.size(), 0);
    end_times.assign(this->adj.size(), 0);

    std::queue<uint32_t> q{};

    q.push(0);
    while (!q.empty()) {
        uint32_t cur = q.front();
        q.pop();
        for (auto& [nb, _] : adj[cur]) {
            if (this->task_pe[cur] != -1 && this->task_pe[nb] != -1) {
                start_times[nb] = std::max(start_times[nb],
                                           start_times[cur] + task_time[cur]);

                end_times[nb] =
                    start_times[nb] + this->times[nb][this->task_pe[nb]];
            }
            if (--(indegre[nb]) == 0) {
                q.push(nb);
            }
        }
    }
    return *std::max_element(end_times.begin(), end_times.end());
}

bool TaskGraph::penalty_cheapest() {
    // std::cout << "Slowest time is " << time << std::endl;
    std::vector<size_t> indegre(this->adj.size());
    for (auto& nb_list : adj) {
        for (auto& n : nb_list) {
            ++indegre[n.first];
        }
    }

    auto q = std::queue<uint32_t>();
    q.push(0);
    while (!q.empty()) {
        auto t = q.front();
        q.pop();
        for (auto& [nb, _] : adj[t]) {
            if (--(indegre[nb]) == 0) {
                q.push(nb);
            }
        }

        // for (auto t = 0; t < this->adj.size(); ++t) {
        // auto cost = std::numeric_limits<uint32_t>::max();
        // this->task_pe[t] = 0;
        std::vector<std::pair<uint32_t, uint32_t>> proc_system_cost{};
        for (auto i = 0; i < this->proc.size(); ++i) {
            this->task_pe[t] = i;
            if (this->times[t][i] != -1) {
                auto new_c = calc_cost_with_optimised_pe();
                proc_system_cost.push_back({new_c, i});
            }
        }
        this->task_pe[t] = -1;
        std::sort(proc_system_cost.begin(), proc_system_cost.end());

        for (auto [cost, p] : proc_system_cost) {
            this->task_pe[t] = p;
            // Now we have cheapest system with task selelected for our T
            auto end_time = calculate_finish_time();

            // Now we can calculate penalty function and check if we are in spec
            auto calced_p =
                k * cost + l * end_time +
                p * (std::max(0, (int32_t)end_time - (int32_t)max_time));

            if (calced_p < max_penalty) {
                auto new_c = calc_cost_with_optimised_pe();
                // std::cout << "Penalty for T" << t << " = " << calced_p
                //           << " C=" << cost << " new_c=" << new_c
                //           << " T=" << end_time << std::endl;
                break;
            } else {
                this->task_pe[t] = -1;
            }
        }

        if (this->task_pe[t] == -1) {
            std::cout << "Failed to find solution that satisfies penalty "
                         "function"
                      << std::endl;
            std::cout << "System finished without assigning T" << t
                      << std::endl;
            return false;
        }
    }
    return true;
}

void TaskGraph::display_system() {
    // this->task_pe[9] = -1;
    // this->task_pe[8] = -1;
    uint32_t system_cost = calc_cost_with_optimised_pe();

    auto end_time = calculate_finish_time();

    std::vector<std::vector<uint32_t>> pe_tasks(this->proc.size());

    // Add cost of each task and create list of tasks that are assigned to
    // each
    // pe
    for (size_t t = 0; t < this->adj.size(); ++t) {
        // system_cost += cost[t][this->task_pe[t]];
        if (this->task_pe[t] != -1) {
            pe_tasks[this->task_pe[t]].push_back(t);
        }
    }

    auto calced_p = k * system_cost + l * end_time +
                    p * (std::max(0, (int32_t)end_time - (int32_t)max_time));

    std::cout << "Task assignment:\n";
    uint32_t pp_count = 0;
    uint32_t hc_count = 0;
    for (size_t i = 0; i < pe_tasks.size(); i++) {
        if (this->proc[i].type == PeType::PP) {
            std::cout << "PP" << pp_count << ":";
            pp_count++;
        } else {
            std::cout << "HC" << hc_count << ":";
            hc_count++;
        }

        for (auto task : pe_tasks[i]) {
            std::cout << " " << task << "(" << start_times[task] << ")";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "No Cl assigned. No data to send between tasks." << std::endl;
    std::cout << std::endl;

    std::cout << "System cost " << system_cost << "\n";
    std::cout << "System will perform all calculations after " << end_time
              << " allowed time was " << max_time << std::endl
              << "\nCalculated penalty " << calced_p << " with max allowed "
              << max_penalty << std::endl;
}

uint32_t TaskGraph::calc_cost_with_optimised_pe() {
    calculate_finish_time();
    uint32_t system_cost = 0;
    std::vector<std::vector<uint32_t>> pe_tasks(this->proc.size());

    // Add cost of each task and create list of tasks that are assigned to
    // each pe
    for (size_t t = 0; t < this->adj.size(); ++t) {
        if (this->task_pe[t] != -1) {
            // std::cout << "COST DBG: " << this->task_pe[t] << std::endl;
            system_cost += cost[t][this->task_pe[t]];
            pe_tasks[this->task_pe[t]].push_back(t);
        }
    }

    // Imporve pp usage to reduce redundant pe
    for (size_t pe = 0; pe < pe_tasks.size(); ++pe) {
        if (proc[pe].type != PeType::PP) {
            continue;
        }
        std::vector<std::pair<uint32_t, int>> events;
        events.reserve(pe_tasks[pe].size() * 2);

        for (uint32_t task_id : pe_tasks[pe]) {
            events.emplace_back(this->start_times[task_id], 1);
            events.emplace_back(this->end_times[task_id], -1);
        }

        std::sort(events.begin(), events.end());

        int32_t current_active = 0;
        int32_t max_active = 0;

        for (const auto& event : events) {
            current_active += event.second;
            max_active = std::max(max_active, current_active);
        }

        system_cost += proc[pe].cost * max_active;
    }

    return system_cost;
}