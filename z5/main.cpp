#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using f64_t = double;
namespace Config {
constexpr uint32_t NEW_RESOURCE_CHANCE = 80;
constexpr uint32_t MAX_ITERS = 100000;
} // namespace Config

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
    uint32_t type_uid;

    ProcessingElement() : cost(0), other(0), type(PeType::PP), type_uid(0) {}
    ProcessingElement(uint32_t c, uint32_t o, uint32_t t, uint32_t t_uid)
        : cost(c), other(o), type_uid(t_uid) {
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

    // Maps cl to another map that tells how many of each PE were
    // connected to it
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint32_t>>
        cl_usage;

    public:
    f64_t max_time = 0.0;
    // Loads task graph from providded file path
    TaskGraph(std::string file_path, f64_t max_time);

    // Calculates cost of most time optimal system
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

    public:
    int32_t calculate_cost_of_random_select_system();
};

std::vector<std::string> split_string(std::string str, char sep);

auto main(int32_t argc, char** argv) -> int {
    std::string fp;
    int max_t = 0;
    if (argc <= 2) {
        std::cerr << "Provide file name and max time as arguments" << std::endl;
        return 1;
    } else {
        fp = std::string(argv[1]);
        max_t = std::stoi(argv[2]);
    }
    // TODO ADD ARG PASSING OF THIS
    f64_t max_time = (f64_t)max_t;

    TaskGraph tg = TaskGraph(fp, max_time);
    auto time = -1;
    auto iters = 0;
    while (iters < Config::MAX_ITERS && time == -1) {
        time = tg.calculate_cost_of_random_select_system();
        iters += 1;
        // std::cout << "Iter " << iters << "\n";
    }
    if (iters == Config::MAX_ITERS) {
        std::cout << "\nDone " << iters
                  << " iterations without finding solution that satisfies time "
                  << tg.max_time << "\n";
    }
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

TaskGraph::TaskGraph(std::string file_path, f64_t maximum_time)
    : max_time(maximum_time) {
    std::fstream file;
    file.open(file_path, std::ios::in);
    if (!file) {
        std::cerr << "Cant open TGFF File data: " << file_path << std::endl;
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
    uint32_t pp_count = 0;
    uint32_t hc_count = 0;
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
            int pe_type = std::stoi(line_components[2]);
            if (pe_type == 0) {
                proc.push_back(ProcessingElement(std::stoi(line_components[0]),
                                                 std::stoi(line_components[1]),
                                                 pe_type, hc_count++));
            } else {
                proc.push_back(ProcessingElement(std::stoi(line_components[0]),
                                                 std::stoi(line_components[1]),
                                                 pe_type, pp_count++));
            }
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

struct UsedProcesor {
    uint32_t proc_id;
    std::vector<uint32_t> assigned_tasks;
};

int32_t TaskGraph::calculate_cost_of_random_select_system() {
    uint32_t cost = 0;
    std::vector<size_t> indegre(this->adj.size());
    // Count indegree for topological sorting
    for (auto& nb_list : adj) {
        for (auto& n : nb_list) {
            ++indegre[n.first];
        }
    }
    std::vector<f64_t> start_times(this->times.size(), 0);
    std::vector<f64_t> end_times(this->times.size(), 0);

    std::vector<UsedProcesor> used_processors{};

    // Random setting
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint32_t> select_used_chance_dist =
        std::uniform_int_distribution<std::uint32_t>(0, 100);

    std::uniform_int_distribution<std::uint32_t> select_proc_dist =
        std::uniform_int_distribution<std::uint32_t>(0, this->proc.size() - 1);

    auto iter = 0;
    std::priority_queue<std::pair<uint32_t, uint32_t>> pq{};
    pq.push({0, 0});
    f64_t end_time = 0;
    while (!pq.empty() && end_time <= this->max_time) {
        // We want to assign all avalible tasks
        ++iter;
        for (uint32_t q = pq.size(); q > 0; --q) {
            // we want to do all
            auto [cur, data] = pq.top();
            pq.pop();
            // Randomise if we should get new proc or use already used
            bool random_select =
                Config::NEW_RESOURCE_CHANCE > select_used_chance_dist(gen);
            if (!random_select) {
                // We try to use old resource but if we fail we just skip to
                // using new random

                std::vector<uint32_t> avalible_processors;
                for (uint32_t i = 0; i < used_processors.size(); i++) {
                    if (this->proc[used_processors[i].proc_id].type ==
                            PeType::PP &&
                        this->times[cur][used_processors[i].proc_id] != -1) {
                        // add proc as avalivable
                        avalible_processors.push_back(i);
                    }
                }

                if (avalible_processors.size() == 0) {
                    random_select = true;
                } else {
                    std::uniform_int_distribution<std::uint32_t>
                        select_used_dist =
                            std::uniform_int_distribution<std::uint32_t>(
                                0, avalible_processors.size() - 1);
                    uint32_t used_proc =
                        avalible_processors[select_used_dist(gen)];

                    auto& pe = used_processors[used_proc];
                    used_processors[used_proc].assigned_tasks.push_back(cur);
                    // We start as son as all predecesor end or after our
                    // assigned pp is free
                    start_times[cur] = std::max(
                        start_times[cur], end_times[pe.assigned_tasks.back()]);
                    end_times[cur] =
                        start_times[cur] + this->times[cur][pe.proc_id];
                    cost +=
                        this->cost[cur][pe.proc_id]; // add cost of performing
                                                     // task on used processor
                }
            }

            // We have to check again as prev if could have changed variable
            if (random_select) {
                // Randomly select new proc until
                uint32_t pe = select_proc_dist(gen);
                while (times[cur][pe] == -1) {
                    pe = select_proc_dist(gen);
                }

                used_processors.push_back({pe, {cur}});
                cost += this->proc[pe].cost;
                cost += this->cost[cur][pe];

                end_times[cur] = start_times[cur] + this->times[cur][pe];
            }
            end_time = std::max(end_time, end_times[cur]);
            if (end_time > this->max_time) {
                break;
            }

            // add elements into queue
            for (auto& [c, d] : adj[cur]) {
                --indegre[c];
                if (indegre[c] == 0) {
                    pq.push({c, d});
                }
                // Add start time of task
                if (start_times[c] < end_times[cur]) {
                    start_times[c] = end_times[cur];
                }
            }
        }
    }

    if (end_time > this->max_time) {
        return -1;
    }

    std::cout << "System will cost: " << cost << std::endl;
    std::cout << "System will end calculations at: " << end_time << std::endl;
    std::cout << "Task assignment:";

    std::vector<uint32_t> proc_used(this->proc.size() + 1, 0);
    for (const auto& p : used_processors) {
        if (this->proc[p.proc_id].type == PeType::HC) {
            std::cout << "\n\tHC";
        } else {
            std::cout << "\n\tPP";
        }
        std::cout << this->proc[p.proc_id].type_uid << "_"
                  << proc_used[p.proc_id]++ << ": ";
        for (const auto& t : p.assigned_tasks) {
            std::cout << "T" << t << "(" << start_times[t] << ") ";
        }
    }

    return 0;
}