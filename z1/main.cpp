#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <random>
#include <unordered_map>
#include <vector>

namespace DistConfig {
// Graph creation
constexpr uint32_t EDGE_WEIGHT[2] = {0, 50};
constexpr uint32_t NODE_MAX_INDEGREE = 3;

// Cost of PE
constexpr uint32_t PROC_COST[2] = {100, 600};

// Times of PE
constexpr int32_t PP_TIMES[2] = {100, 400};
constexpr int32_t HC_TIMES[2] = {10, 80};

// Chance that PP cant do the task
constexpr uint32_t PROC_SKIP_CHANCE = 10;

// Cost of PE
constexpr int32_t PP_COST[2] = {10, 80};
constexpr int32_t HC_COST[2] = {100, 400};

constexpr int32_t CON_COST[2] = {10, 90};
constexpr int32_t CON_BANDWITH[2] = {5, 50};
constexpr int32_t CON_CHANCE = 50;

} // namespace DistConfig

static_assert(DistConfig::NODE_MAX_INDEGREE > 1,
              "Node has to have at least one indegree");

static_assert(DistConfig::PROC_SKIP_CHANCE > 0 &&
                  DistConfig::PROC_SKIP_CHANCE < 100,
              "Chance can only be 0-100%");

static_assert(DistConfig::PP_TIMES[0] > 0, "Cost has to be greater than 0");

static_assert(DistConfig::PP_TIMES[1] > 0 &&
                  DistConfig::PP_TIMES[1] > DistConfig::PP_TIMES[0],
              "Cost has to be greater than 0 and min dist");

static_assert(DistConfig::CON_CHANCE > 0 && DistConfig::CON_CHANCE < 100,
              "Chance can only be 0-100%");

void get_input_data(int32_t& tc, int32_t& pp, int32_t& hc, int32_t& cl,
                    bool& random_edge_w, std::string& output_file);

std::vector<std::unordered_map<uint32_t, uint32_t>> gen_dag(uint32_t node_count,
                                                            bool random_edge_w);

void gen_file_structure(int32_t tc, int32_t pp, int32_t hc, int32_t cl,
                        bool random_edge_w, std::string& output_file);

void save_to_file(std::vector<std::unordered_map<uint32_t, uint32_t>> adj,
                  std::vector<std::vector<uint32_t>>& proc,
                  std::vector<std::vector<int32_t>>& times,
                  std::vector<std::vector<int32_t>>& cost,
                  std::vector<std::vector<int32_t>>& comms,
                  std::string& output_file);

int main() {
    int32_t tc = -1;
    int32_t pp = -1;
    int32_t hc = -1;
    int32_t cl = -1;
    bool random_edge_w = false;
    std::string output_file = "";

    get_input_data(tc, pp, hc, cl, random_edge_w, output_file);
    gen_file_structure(tc, pp, hc, cl, random_edge_w, output_file);

    std::cout << "Tc: " << tc << " PP: " << pp << " HC: " << hc << " CL: " << cl
              << " Random Edge Weights: " << random_edge_w
              << " Output File: " << output_file << "\n";
}

void get_input_data(int32_t& tc, int32_t& pp, int32_t& hc, int32_t& cl,
                    bool& random_edge_w, std::string& output_file) {
    // Insert task count
    std::cout << "Insert task count: " << std::endl;
    std::cin >> tc;
    while (tc <= 0) {
        std::cout << "Task count must be a positive. Please insert again: "
                  << std::endl;
        std::cin >> tc;
    }
    while (hc + pp <= 0) {
        // Insert Programable Processor count
        std::cout << "Insert Programable Processors (PP) count: " << std::endl;
        std::cin >> pp;
        while (pp < 0) {
            std::cout
                << "Programable Processor count must be a positive. Please "
                   "insert again: "
                << std::endl;
            std::cin >> pp;
        }

        // Insert Hardware Core
        std::cout << "Insert Hardware Core (HC) count: " << std::endl;
        std::cin >> hc;
        while (hc < 0) {
            std::cout << "Hardware Core count must be a positive. Please "
                         "insert again: "
                      << std::endl;
            std::cin >> hc;
        }
        if ((hc + pp) <= 0) {
            std::cout << "Total processor count (PP + HC) must be greater than "
                         "0. Please "
                         "insert again PP and HC counts."
                      << std::endl;
        }
    }

    // Insert Communication Link count
    std::cout << "Insert Communication Link count: " << std::endl;
    std::cin >> cl;
    while (cl <= 0) {
        std::cout << "Communication Link count must be a positive. Please "
                     "insert again: "
                  << std::endl;
        std::cin >> cl;
    }

    // Ask for random weights
    std::string ans = "";
    std::cout << "Do you want random weights (Y)es / (N): " << std::endl;
    std::cin >> ans;

    std::transform(ans.begin(), ans.end(), ans.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (ans == "y" || ans == "yes") {
        random_edge_w = true;
    }

    // Insert output file name
    std::cout << "Insert output graph filename: " << std::endl;
    std::cin >> output_file;
}

std::vector<std::unordered_map<uint32_t, uint32_t>>
gen_dag(uint32_t node_count, bool random_edge_w) {
    // Random numbers
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint32_t> edge_dist;

    // Grpah
    std::vector<std::unordered_map<uint32_t, uint32_t>> adj(node_count);
    if (random_edge_w) {
        edge_dist = std::uniform_int_distribution<std::uint32_t>(
            DistConfig::EDGE_WEIGHT[0], DistConfig::EDGE_WEIGHT[1]);
    } else {
        // 0,0 default 0 weight
        edge_dist = std::uniform_int_distribution<std::uint32_t>(0, 0);
    }

    // Use layered approach for generating dag
    // Node 0 is the source
    // Node n randomly selects one node fron 0 to n-1 range. This ensures that
    // no cycle is formed furing generation. to increce complexity we can
    // randomly get multiple edges.

    //
    std::uniform_int_distribution<std::uint32_t> con_count_dist(
        1, DistConfig::NODE_MAX_INDEGREE);
    for (int32_t n = 1; n < node_count; n++) {
        // Select random nodes from 0 to n-1
        std::uniform_int_distribution<std::uint32_t> con_edge_dist(0, n - 1);
        for (int32_t cc = 0; cc < con_count_dist(gen); cc++) {
            auto con_node =
                con_edge_dist(gen); // if con_node repeat, it will only update
            // edge weight (random and we dont care)
            adj[con_node][n] = edge_dist(gen);
        }
    }
    return adj;
}

void gen_file_structure(int32_t tc, int32_t pp, int32_t hc, int32_t cl,
                        bool random_edge_w, std::string& output_file) {
    int32_t p_count = pp + hc;
    // Random numbers
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint32_t> proc_cost_dist(
        DistConfig::PROC_COST[0], DistConfig::PROC_COST[1]);

    // Gen adj list of tasks
    auto adj = gen_dag(tc, random_edge_w);

    // Gen proc from pp and hc
    // Initialize with zeros. (pp+hc) x 3 matrix
    std::vector<std::vector<uint32_t>> proc(pp + hc,
                                            std::vector<uint32_t>(3, 0));

    for (size_t i = 0; i < p_count; i++) {
        proc[i][0] = proc_cost_dist(gen);
        if (i < pp) {
            proc[i][2] = 1;
        }
    }

    // Gen times for each task on each proc
    std::vector<std::vector<int32_t>> times(tc,
                                            std::vector<int32_t>(p_count, -1));
    std::uniform_int_distribution<std::int32_t> pp_times_dist(
        DistConfig::PP_TIMES[0], DistConfig::PP_TIMES[1]);
    std::uniform_int_distribution<std::int32_t> hc_times_dist(
        DistConfig::HC_TIMES[0], DistConfig::HC_TIMES[1]);

    // Chance of negative
    std::uniform_int_distribution<std::int32_t> skip_chance_dist(0, 100);

    for (auto& t : times) {
        uint32_t skips = 0;
        for (size_t i = 0; i < p_count; i++) {
            if (skip_chance_dist(gen) < DistConfig::PROC_SKIP_CHANCE) {
                skips++;
                continue;
            }
            if (i < pp) {
                t[i] = pp_times_dist(gen);
            } else {
                t[i] = hc_times_dist(gen);
            }
        }
        if (skips == p_count) {
            t[0] = pp_times_dist(gen);
        }
    }

    // gen cost for each task core
    std::vector<std::vector<int32_t>> cost(tc,
                                           std::vector<int32_t>(p_count, 0));
    std::uniform_int_distribution<std::int32_t> pp_cost_dist(
        DistConfig::PP_COST[0], DistConfig::PP_COST[1]);
    std::uniform_int_distribution<std::int32_t> hc_cost_dist(
        DistConfig::HC_COST[0], DistConfig::HC_COST[1]);

    for (auto& c : cost) {
        for (size_t i = 0; i < p_count; i++) {
            if (i < pp) {
                c[i] = pp_cost_dist(gen);
            } else {
                c[i] = hc_cost_dist(gen);
            }
        }
    }

    // gen com
    // Columns 3 + procesor count, By default all connected
    std::vector<std::vector<int32_t>> comms(
        cl, std::vector<int32_t>(p_count + 2, 1));
    std::uniform_int_distribution<std::int32_t> con_cost_dist(
        DistConfig::CON_COST[0], DistConfig::CON_COST[1]);
    std::uniform_int_distribution<std::int32_t> con_bandwith_dist(
        DistConfig::CON_BANDWITH[0], DistConfig::CON_BANDWITH[1]);
    std::uniform_int_distribution<std::int32_t> con_chance_dist(0, 100);
    for (auto& com : comms) {
        com[0] = con_cost_dist(gen);
        com[1] = con_bandwith_dist(gen);
        uint32_t skips = 0;
        for (size_t i = 2; i < comms.size(); i++) {
            if (con_chance_dist(gen) < DistConfig::CON_CHANCE) {
                com[i] = 0;
                skips++;
            }
        }
        if (skips == p_count) {
            com[2] = 1; // ensure at least one connection
        }
    }
    // Check that every processor is connected to at least one Cl
    std::uniform_int_distribution<std::int32_t> con_id(0, cl - 1);
    for (size_t i = 2; i < comms.size(); i++) {
        uint32_t cons = 0;
        for (size_t con_c = 0; con_c < comms.size(); con_c++) {
            cons += comms[con_c][i]; // Only 0 or 1 if not connected sum = 0
        }
        if (cons == 0) {
            comms[con_id(gen)][i] = 1;
        }
    }

    // save to file
    save_to_file(adj, proc, times, cost, comms, output_file);
}

void save_to_file(std::vector<std::unordered_map<uint32_t, uint32_t>> adj,
                  std::vector<std::vector<uint32_t>>& proc,
                  std::vector<std::vector<int32_t>>& times,
                  std::vector<std::vector<int32_t>>& cost,
                  std::vector<std::vector<int32_t>>& comms,
                  std::string& output_file) {
    // Open file for data write
    std::fstream file;
    file.open(output_file, std::ios::out);
    if (!file) {
        std::cerr << "Cant open file to save TGFF File data: " << output_file
                  << std::endl;
        exit(1);
    }

    // SAVE ADJ / @tasks
    file << "@tasks " << adj.size() << std::endl;
    for (size_t task_id = 0; task_id < adj.size(); task_id++) {
        // Row consists of task id | succesor_count | adj list of succesorts
        // with weights
        file << task_id << " " << adj[task_id].size();
        // List all tasks
        for (auto& [succesor, weight] : adj[task_id]) {
            file << " " << succesor << "(" << weight
                 << ")"; // insert space before each entry to spearate them
        }
        file << std::endl;
    }
    file << std::endl;

    // SAVE @proc
    file << "@proc " << proc.size() << std::endl;
    for (auto& p : proc) {
        file << p[0] << " " << p[1] << " " << p[2] << std::endl;
    }
    file << std::endl;

    // SAVE @times
    file << "@times" << std::endl;
    for (auto& t_row : times) {
        for (auto& t : t_row) {
            file << t << " ";
        }
        file << std::endl;
    }
    file << std::endl;

    // SAVE @cost
    file << "@cost" << std::endl;
    for (auto& c_row : cost) {
        for (auto& c : c_row) {
            file << c << " ";
        }
        file << std::endl;
    }
    file << std::endl;

    // SAVE @comm
    file << "@comm " << comms.size() << std::endl;
    for (size_t com_id = 0; com_id < comms.size(); com_id++) {
        file << "chan" << com_id << " ";
        for (auto& ce : comms[com_id]) {
            file << ce << " ";
        }
        file << std::endl;
    }
}