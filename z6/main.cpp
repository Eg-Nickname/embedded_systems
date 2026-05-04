#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
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
    std::vector<uint32_t> task_pe;
    uint32_t max_time;

    std::vector<size_t> start_times;
    std::vector<size_t> end_times;

    public:
    // Loads task graph from providded file path
    TaskGraph(std::string file_path, uint32_t max_time_to_finish);

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

    public:
    bool refine_fastest_system(uint32_t max_time);
    void display_system();
};

std::vector<std::string> split_string(std::string str, char sep);

auto main(int32_t argc, char** argv) -> int {
    std::string fp;
    uint32_t max_time;
    if (argc == 1) {
        std::cerr << "Provide task graph file and max time as argument"
                  << std::endl;
        return 1;
    } else {
        fp = std::string(argv[1]);
        max_time = std::stoul(argv[2]);
    }
    TaskGraph tg = TaskGraph(fp, max_time);
    auto _ = tg.refine_fastest_system(max_time);
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

TaskGraph::TaskGraph(std::string file_path, uint32_t max_time_to_finish) {
    max_time = max_time_to_finish;
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

    this->task_pe = std::vector<uint32_t>(this->adj.size());
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

// Refining algorithm
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
            start_times[nb] =
                std::max(start_times[nb], start_times[cur] + task_time[cur]);
            end_times[nb] =
                start_times[nb] + this->times[nb][this->task_pe[nb]];
            if (--(indegre[nb]) == 0) {
                q.push(nb);
            }
        }
    }
    return *std::max_element(end_times.begin(), end_times.end());
}

bool TaskGraph::refine_fastest_system(uint32_t max_time) {
    // Get list of avelible procesors for each task and sort them by speed
    std::vector<std::vector<uint32_t>> task_fastest_procesors_list(
        this->adj.size());

    // Create queue with all tasks inside
    std::queue<uint32_t> task_q{};

    for (auto t_idx = 0; t_idx < this->adj.size(); ++t_idx) {
        auto proc_speed_cmp = [&, this, t_idx](uint32_t lhs, uint32_t rhs) {
            return this->times[t_idx][lhs] > this->times[t_idx][rhs];
        };

        for (auto p_idx = 0; p_idx < this->times[t_idx].size(); ++p_idx) {
            if (this->times[t_idx][p_idx] != -1) {
                task_fastest_procesors_list[t_idx].push_back(p_idx);
            }
        }
        std::sort(task_fastest_procesors_list[t_idx].begin(),
                  task_fastest_procesors_list[t_idx].end(), proc_speed_cmp);
        // assign fastest proc to this task at begining
        this->task_pe[t_idx] = task_fastest_procesors_list[t_idx].back();
        task_fastest_procesors_list[t_idx].pop_back();

        if (task_fastest_procesors_list[t_idx].size() > 0) {
            task_q.push(t_idx);
        }
    }
    auto time = calculate_finish_time();
    // std::cout << "Fastest time is " << time << std::endl;
    if (time > this->max_time) {
        std::cout << "Cant find system that meets time constraint:" << max_time
                  << std::endl;
        return false;
    }

    // In queue update proc for this task and check if max time is exceded.
    while (!task_q.empty()) {
        auto cur_t = task_q.front();
        task_q.pop();

        auto cur_t_pe = task_pe[cur_t];
        task_pe[cur_t] = task_fastest_procesors_list[cur_t].back();
        task_fastest_procesors_list[cur_t].pop_back();
        auto time = calculate_finish_time();

        if (time > this->max_time) {
            // If yes go back to old proc
            task_pe[cur_t] = cur_t_pe;
        } else {
            // Else pop from avalible procesors and readd to queue if eny left
            if (task_fastest_procesors_list[cur_t].size() > 0) {
                task_q.push(cur_t);
            }
        }
        // Do until queue empty
    }

    // // Print selected info
    // std::cout << "Slowest time is " << time << std::endl;
    return true;
}

void TaskGraph::display_system() {
    auto end_time = calculate_finish_time();

    uint32_t system_cost = 0;
    std::vector<std::vector<uint32_t>> pe_tasks(this->proc.size());

    // Add cost of each task and create list of tasks that are assigned to each
    // pe
    for (size_t t = 0; t < this->adj.size(); ++t) {
        system_cost += cost[t][this->task_pe[t]];
        pe_tasks[this->task_pe[t]].push_back(t);
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

    std::cout << "Refined system cost " << system_cost << "\n";
    std::cout << "System will perform all calculations after " << end_time
              << " allowed time was " << max_time << std::endl;
}