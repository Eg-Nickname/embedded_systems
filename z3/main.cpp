#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>
#include <queue>
#include <ranges>
#include <set>
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
    uint32_t calculate_cost_for_optimal_system();

    private:
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
    std::vector<f64_t> calculate_start_times();
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
    uint32_t _ = tg.calculate_cost_for_optimal_system();
    // auto _ = tg.calculate_start_times();

    // std::cout << std::endl;
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

    //
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
            // std::cout << "Line=\'" << line << "\'" << std::endl;
        }
    }
}

uint32_t TaskGraph::calculate_cost_for_optimal_system() {
    this->task_pe.reserve(adj.size());
    this->task_time.reserve(adj.size());
    this->pe_tasks.resize(this->proc.size());

    for (auto& task_times : this->times) {
        auto map_to_float = [](uint32_t t) {
            return (t == -1) ? std::numeric_limits<f64_t>::max()
                             : static_cast<f64_t>(t);
        };

        // Documentation ref
        // https://en.cppreference.com/w/cpp/ranges/transform_view.html
        auto transformed_view =
            task_times | std::views::transform(map_to_float);

        auto min_it =
            std::min_element(transformed_view.begin(), transformed_view.end());

        size_t pe_id = std::distance(transformed_view.begin(), min_it);
        this->task_pe.push_back(pe_id);
        this->task_time.push_back(*min_it);
        this->pe_tasks[pe_id].push_back(task_pe.size() - 1);
    }

    // Calculate total cost of system
    uint32_t system_cost = 0;

    // TODO Alternativly calculate how many pp of this type we need to achive
    // maximum parallelism First add cost of all PP
    for (size_t i = 0; i < proc.size(); i++) {
        if (proc[i].type == PeType::PP && this->pe_tasks[i].size() > 0) {
            system_cost += proc[i].cost;
        }
    }

    // Get cost of each task execution
    for (size_t t = 0; t < this->adj.size(); t++) {
        system_cost += cost[t][this->task_pe[t]];
    }

    // Get task start times
    auto start_times = this->calculate_start_times();

    std::vector<f64_t> end_times(start_times.size());

    std::transform(start_times.begin(), start_times.end(), task_time.begin(),
                   end_times.begin(), std::plus<f64_t>());

    // Count connections
    // As we are not told how to optimize or chose CL. Assume we want all
    // avalible conections for our FASTEST system
    for (size_t c = 0; c < this->comms.size(); c++) {
        for (size_t i = 0; i < this->comms[c].conections.size(); i++) {
            if (proc[i].type == PeType::PP && this->pe_tasks[i].size() > 0 &&
                this->comms[c].conections[i]) {
                system_cost += this->comms[c].cost;
            } else {
                system_cost += this->comms[c].cost * this->pe_tasks[i].size();
            }
        }
    }

    std::cout << "Task assignment:\n";
    uint32_t pp_count = 0;
    uint32_t hc_count = 0;
    for (uint32_t i = 0; i < pe_tasks.size(); i++) {
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

    std::cout << std::endl << "Communication lane assignment:\n";
    for (auto& cl : this->comms) {
        std::cout << cl.name << ": ";
        uint32_t pp_count = 0;
        uint32_t hc_count = 0;

        for (auto pe_id = 0; pe_id < cl.conections.size(); pe_id++) {
            if (cl.conections[pe_id] == 1 && this->pe_tasks[pe_id].size() > 0) {
                if (this->proc[pe_id].type == PeType::PP) {
                    std::cout << "PP" << pp_count << ", ";
                    pp_count++;
                } else {
                    std::cout << "HC" << hc_count << ", ";
                    hc_count++;
                }
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Fastest system cost " << system_cost << "\n";

    std::cout << "System will perform all calculations after "
              << *std::max_element(end_times.begin(), end_times.end());

    return system_cost;
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

std::vector<f64_t> TaskGraph::calculate_start_times() {
    // Stores Indegree of node
    std::vector<size_t> indegre(this->adj.size());
    // Count indegree for topological sorting
    for (auto& nb_list : adj) {
        for (auto& n : nb_list) {
            ++indegre[n.first];
        }
    }

    std::vector<f64_t> start_times(this->adj.size());
    std::queue<uint32_t> q{};
    q.push(0);
    while (!q.empty()) {
        uint32_t cur = q.front();
        q.pop();

        for (auto& [nb, weight] : adj[cur]) {
            auto start_time_form_cur = start_times[cur] + task_time[cur];
            if (task_pe[cur] != task_pe[nb] ||
                this->proc[this->task_pe[nb]].type == PeType::HC) {
                f64_t bandwith = 0;

                // Check all avilivle cl and pick the fastest that can
                // transfer data between procesors
                for (auto& cl : this->comms) {
                    if (cl.conections[this->task_pe[cur]] == 1 &&
                        cl.conections[this->task_pe[nb]] == 1) {
                        bandwith = std::max(bandwith, (f64_t)cl.bandwith);
                    }
                }

                // As we have to send data to this task we have to chose
                // fastest connection even if time after send wont be
                // the fastest, the process of sending still exists

                start_time_form_cur +=
                    ((f64_t)weight / bandwith); // Add data transfer time if
                                                // tasks are on different PE or
                                                // if one of them is on HC
            }
            // Update max start time
            start_times[nb] = std::max(start_times[nb], start_time_form_cur);

            if (--(indegre[nb]) == 0) {
                q.push(nb);
            }
        }
    }
    return start_times;
}
