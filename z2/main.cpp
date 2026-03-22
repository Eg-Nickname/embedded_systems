#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct CommunicationLane {
    std::string name;
    uint32_t bandwith;
    uint32_t cost;
    std::vector<bool> conections;

    CommunicationLane() : name(), bandwith(0), cost(0), conections() {}

    CommunicationLane(const CommunicationLane &other) {
        this->name = other.name;
        this->bandwith = other.bandwith;
        this->cost = other.cost;
        this->conections = other.conections;
    }

    CommunicationLane(CommunicationLane &&other) {
        this->name = std::move(other.name);
        this->bandwith = std::move(other.bandwith);
        other.bandwith = 0;
        this->cost = std::move(other.cost);
        this->cost = 0;
        this->conections = std::move(other.conections);
    }

    CommunicationLane &operator=(const CommunicationLane &other) {
        if (this != &other) {
            this->name = other.name;
            this->bandwith = other.bandwith;
            this->cost = other.cost;
            this->conections = other.conections;
        }
        return *this;
    }

    CommunicationLane &operator=(CommunicationLane &&other) {
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

class TaskGraph {
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> adj;
    std::vector<std::vector<uint32_t>> proc;
    std::vector<std::vector<int32_t>> times;
    std::vector<std::vector<int32_t>> cost;
    std::vector<CommunicationLane> comms;

    public:
    // Loads task graph from providded file path
    TaskGraph(std::string file_path);

    // Calculates cost of most time optimal system
    uint32_t calculate_cost_for_optimal_system();

    private:
    static std::vector<std::vector<std::pair<uint32_t, uint32_t>>>
    parse_tasks(std::fstream &file, uint32_t tc);

    static std::vector<std::vector<uint32_t>> parse_proc(std::fstream &file,
                                                         uint32_t pe);

    static std::vector<std::vector<int32_t>>
    parse_times(std::fstream &file, uint32_t tc, uint32_t pe);

    static std::vector<std::vector<int32_t>>
    parse_cost(std::fstream &file, uint32_t tc, uint32_t pe);

    static std::vector<CommunicationLane> parse_comms(std::fstream &file,
                                                      uint32_t cl, uint32_t pe);
};

std::vector<std::string> split_string(std::string str, char sep);

auto main(int32_t argc, char **argv) -> int {
    std::string fp;
    if (argc == 1) {
        std::cout << "Input task graph file path: " << std::endl;
        std::cin >> fp;
    } else {
        fp = std::string(argv[1]);
    }
    TaskGraph tg = TaskGraph(fp);
    uint32_t cost = tg.calculate_cost_for_optimal_system();
    std::cout << std::endl;
    std::cout << "Cost of fastest system is " << cost << " units";
    return 0;
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

            if (line_params[0] == "@tasks") {
                tc = std::stoi(line_params[1]);
                this->adj = this->parse_tasks(file, tc);
            } else if (line_params[0] == "@proc") {
                pe = std::stoi(line_params[1]);
                this->proc = this->parse_proc(file, pe);
            } else if (line_params[0] == "@times") {
                this->times = this->parse_times(file, tc, pe);
            } else if (line_params[0] == "@cost") {
                this->cost = this->parse_cost(file, tc, pe);
            } else if (line_params[0] == "@comm") {
                cl = std::stoi(line_params[1]);
                this->comms = this->parse_comms(file, cl, pe);
            }
        }
    }
}

uint32_t TaskGraph::calculate_cost_for_optimal_system() {
    // For each task get minimum
    std::vector<uint32_t> pe_usage(this->proc.size());
    std::vector<uint32_t> task_pe{};
    task_pe.reserve(adj.size());

    for (auto &task_time : this->times) {
        auto map_to_max = [](uint32_t t) {
            return (t == -1) ? std::numeric_limits<uint32_t>::max() : t;
        };

        // Documentation ref
        // https://en.cppreference.com/w/cpp/ranges/transform_view.html
        auto transformed_view = task_time | std::views::transform(map_to_max);

        auto min_it =
            std::min_element(transformed_view.begin(), transformed_view.end());
        std::cout << "min=" << *min_it << "\n";

        size_t pe_id = std::distance(transformed_view.begin(), min_it);
        task_pe.push_back(pe_id);
        pe_usage[pe_id]++;
    }

    for (auto &p : pe_usage) {
        std::cout << p << " ";
    }
    // Calculate total cost of system
    uint32_t system_cost = 0;
    // First add cost of all PP
    for (size_t i = 0; i < proc.size(); i++) {
        if (proc[i][2] == 1 && pe_usage[i] > 0) {
            system_cost += proc[i][0];
        }
    }
    // Get cost of each task execution
    for (size_t t = 0; t < this->adj.size(); t++) {
        system_cost += cost[t][task_pe[t]];
    }
    // Count connections
    for (size_t c = 0; c < this->comms.size(); c++) {
        for (size_t i = 0; i < this->comms[c].conections.size(); i++) {
            if (proc[i][2] == 1 && pe_usage[i] > 0 &&
                this->comms[c].conections[i]) {
                system_cost += this->comms[c].cost;
            } else {
                system_cost += this->comms[c].cost * pe_usage[i];
            }
        }
    }

    return system_cost;
}

std::vector<std::vector<std::pair<uint32_t, uint32_t>>>
TaskGraph::parse_tasks(std::fstream &file, uint32_t tc) {
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> adj(tc);

    std::string line{};
    // Parse adj list of every task
    for (uint32_t t = 0; t < tc; t++) {
        if (std::getline(file, line)) {
            // parse line
            auto line_components = split_string(line, ' ');
            if (line_components.size() < 2 &&
                line_components.size() != std::stoi(line_components[1]) + 2 !=
                    line_components.size()) {
                // invalid adj structure
                std::cerr << "Task " << t << " has wrong adj list declaration"
                          << std::endl;
                continue;
            }
            for (uint32_t i = 2; i < line_components.size(); i++) {
                // component structure 12(9)
                auto weight_start_pos =
                    std::distance(line_components[i].begin(),
                                  std::find(line_components[i].begin(),
                                            line_components[i].end(), '('));
                // Emplace pair of connecting edge and weight to adj
                adj[t].emplace_back(
                    std::stoi(line_components[i].substr(0, weight_start_pos)),
                    // We have to offset start of substr to skip '(' and then
                    // calculate len of number in ()
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

std::vector<std::vector<uint32_t>> TaskGraph::parse_proc(std::fstream &file,
                                                         uint32_t pe) {
    std::vector<std::vector<uint32_t>> proc(pe, std::vector<uint32_t>(3, 0));

    std::string line{};
    // Parse params of every PE and HC
    for (uint32_t p = 0; p < pe; p++) {
        if (std::getline(file, line)) {
            // parse line
            auto line_components = split_string(line, ' ');
            if (line_components.size() < 3) {
                // invalid proc structure
                std::cerr << "Proc " << p << " has wrong param declaration"
                          << std::endl;
                continue;
            }
            proc[p][0] = std::stoi(line_components[0]);
            proc[p][1] = std::stoi(line_components[1]);
            proc[p][2] = std::stoi(line_components[2]);

        } else {
            std::cerr << "@proc section has to many procesors declared."
                      << std::endl;
            throw std::invalid_argument("File structure not valid");
        }
    }

    return proc;
}

std::vector<std::vector<int32_t>>
TaskGraph::parse_times(std::fstream &file, uint32_t tc, uint32_t pe) {
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
TaskGraph::parse_cost(std::fstream &file, uint32_t tc, uint32_t pe) {
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
TaskGraph::parse_comms(std::fstream &file, uint32_t cl, uint32_t pe) {
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
                comm[c].conections.push_back(std::stoi(line_components[2 + p]));
            }
        } else {
            std::cerr << "@comm section has not enough CLs declared."
                      << std::endl;
            throw std::invalid_argument("File structure not valid");
        }
    }
    return comm;
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
