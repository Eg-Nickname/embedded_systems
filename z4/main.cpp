#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
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

    // Stores how long each task will take
    std::vector<f64_t> task_time{};
    // Stores witch pe is assigned to each task
    std::vector<uint32_t> task_pe{};

    // Stores each task assigned to pe
    std::vector<std::vector<uint32_t>> pe_tasks{};

    // Maps cl to another map that tells how many of each PE were
    // connected to it
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>>
        cl_usage;

    public:
    // Loads task graph from providded file path
    TaskGraph(std::string file_path);

    // Calculates cost of most time optimal system
    // uint32_t calculate_cost_for_optimal_system();

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
    // ASSIGN PP
    public:
    uint32_t assign_pp();
    std::vector<uint32_t> calculate_priorities(uint32_t pp);
    // uint32_t calc_prioirty();
    uint32_t calc_prioirty(uint32_t pp, uint32_t node,
                           std::vector<uint32_t>& visited,
                           std::vector<uint32_t>& cost);

    public:
    uint32_t calculate_cost_of_list_select_system();
    // std::vector<f64_t> calculate_start_times();
};

std::vector<std::string> split_string(std::string str, char sep);

auto main(int32_t argc, char** argv) -> int {
    std::string fp;
    if (argc == 1) {
        std::cout << "Input task graph file path: " << std::endl;
        std::cin >> fp;
    } else {
        fp = std::string(argv[1]);
    }
    TaskGraph tg = TaskGraph(fp);
    auto _ = tg.calculate_cost_of_list_select_system();
    return 0;
}

std::vector<std::string> split_string(std::string str, char sep) {
    std::vector<std::string> res{};

    std::stringstream ss(str);
    std::string line{};
    while (std::getline(ss, line, sep)) {
        res.push_back(line);
    }

    return res;
}

TaskGraph::TaskGraph(std::string file_path) {
    std::fstream file;
    file.open(file_path, std::ios::in);
    if (!file) {
        std::cerr << "Cant open file to save TGFF File data: " << file_path
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
                // Emplace pair of connecting edge and weight to adj
                adj[t].emplace_back(
                    std::stoi(line_components[i].substr(0, weight_start_pos)),
                    // We have to offset start of substr to skip '(' and
                    // then calculate len of number in ()
                    std::stoi(line_components[i].substr(
                        weight_start_pos + 1,
                        line_components[i].size() - weight_start_pos - 2)));
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

uint32_t TaskGraph::assign_pp() {
    for (uint32_t pe_id = 0; pe_id < this->proc.size(); pe_id++) {
        if (this->proc[pe_id].type == PeType::PP) {
            // check if pp can do all tasks
            bool can_use = true;
            for (uint32_t task_id = 0; task_id < this->times.size();
                 task_id++) {
                if (this->times[task_id][pe_id] == -1) {
                    can_use = false;
                    break;
                }
            }
            if (can_use) {
                return pe_id;
            }
        }
    }
    return -1;
}

std::vector<uint32_t> TaskGraph::calculate_priorities(uint32_t pp) {
    std::vector<uint32_t> priorities(this->times.size(), 0);
    std::vector<uint32_t> visited(this->times.size(), 0);

    calc_prioirty(pp, 0, visited, priorities);

    return priorities;
}

uint32_t TaskGraph::calc_prioirty(uint32_t pp, uint32_t node,
                                  std::vector<uint32_t>& visited,
                                  std::vector<uint32_t>& cost) {
    if (visited[node] == 0) {
        visited[node] = 1;
        // calc priority of this node
        for (auto& [child, c_cost] : adj[node]) {
            auto dfs =
                times[child][pp] + calc_prioirty(pp, child, visited, cost);
            cost[node] = std::max(cost[node], dfs);
        }
    }
    return cost[node];
}

uint32_t TaskGraph::calculate_cost_of_list_select_system() {
    auto pp = this->assign_pp();
    if (pp == -1) {
        return -1;
    }

    auto priorities = this->calculate_priorities(pp);

    std::vector<size_t> indegre(this->adj.size());
    // Count indegree for topological sorting
    for (auto& nb_list : adj) {
        for (auto& n : nb_list) {
            ++indegre[n.first];
        }
    }

    auto cmp = [&priorities](const uint32_t& lhs_id, const uint32_t& rhs_id) {
        return priorities[lhs_id] < priorities[rhs_id];
    };

    // std::vector<uint32_t> processed(this->times.size(), 0);
    std::vector<uint32_t> start_time(this->times.size(), 0);

    std::priority_queue<uint32_t, std::vector<uint32_t>, decltype(cmp)> pq(cmp);
    pq.push(0);

    uint32_t time_passed = 0;
    std::cout << "Assigned PE:\nPP" << pp << ": ";
    while (!pq.empty()) {
        uint32_t cur = pq.top();
        pq.pop();

        for (auto& [n, _] : adj[cur]) {
            --indegre[n];
            if (indegre[n] == 0) {
                pq.push(n);
                // std::cout << " INSERTING " << n << " INTO PQ\n";
            }
        }
        start_time[cur] = time_passed;
        time_passed += this->times[cur][pp];
        std::cout << "T" << cur << " (" << start_time[cur] << ") ";
    }
    std::cout << std::endl;

    std::cout << "System will finish all tasks after: " << time_passed << "\n";
    uint32_t cost = 0;
    cost +=
        proc[pp].cost; // We only have one PE and all tasks are assigned to it
    for (auto& c : this->cost) {
        cost += c[pp]; // add task cost of selected pp
    }

    std::cout << "System will cost: " << cost << std::endl;

    return 0;
}