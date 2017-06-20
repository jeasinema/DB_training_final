/*-----------------------------------------------------
 File Name : main.cpp
 Purpose :
 Creation Date : 19-06-2017
 Last Modified : Tue 20 Jun 2017 01:29:11 PM CST
 Created By : Jeasine Ma [jeasinema[at]gmail[dot]com]
-----------------------------------------------------*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility>
#include <cassert>

#include "gtree.hpp"
#include "g_plus_tree.hpp"
#include "km.hpp"

using std::unordered_map;
using std::string;
using std::vector;
using std::map;

const int K = 500; // amount for candidates when K-NN
const double share_rate_limit = 0.8;
const string file_drivers = "./test/workers.txt";
const string file_passenger = "./test/jobs.txt";
const string file_result = "BJ_result";

// src_id, dst_id
vector<int> passenger_src;
vector<int> passenger_dst;
vector<vector<int>> driver_candidate;
vector<unordered_map<int, double>> driver_share_rate;  // int - drive_id, double - sharing rate

vector<int> driver_src;
vector<int> driver_dst;

// bi graph
// myid, <id, value>
unordered_map<int, unordered_map<int, double>> driver_to_passenger, passenger_to_driver;
double driver_get_share_rate(int driver_id, int passenger_id) {
    auto driver = driver_to_passenger.find(driver_id);
    if (driver != driver_to_passenger.end()) {
        auto passenger = driver->second.find(passenger_id);
        if (passenger != driver->second.end()) {
            return passenger->second;
        } else {
            goto fail;
        }
    } else {
        goto fail;
    }

fail:
    return 0.0;
}
double passenger_get_share_rate(int passenger_id, int driver_id) {
    auto passenger = passenger_to_driver.find(passenger_id);
    if (passenger != passenger_to_driver.end()) {
        auto driver = passenger->second.find(driver_id);
        if (driver != passenger->second.end()) {
            return driver->second;
        } else {
            goto fail;
        }
    } else {
        goto fail;
    }

fail:
    return 0.0;
}


int main(int argc, char *argv[]) {
    // load g-tree for knn
    gtree::init();
    gtree::gtree_load();
    gtree::hierarchy_shortest_path_load();
    std::cout << "Successfully load g-tree" << std::endl;
    // usage: 
    // pre_query(the point you want to find);
    // vector<gtree::ResultSet> result = gtree::knn_query(locid, K);
	// printf("ID=%d DIS=%d\n", result[i].id, result[i].dis);


    // load g-plus-tree for p2p search
    g_plus_tree::load();
    std::cout << "Successfully load g-plus-tree" << std::endl;
    // usage:
    // int dis = g_plus_tree::tree.search(S,T);
    
    // load drivers and passengers 
    std::fstream f_drivers{file_drivers};
    std::fstream f_passengers{file_passenger};
    int src = 0, dst = 0;
    while (f_drivers >> src >> dst) {
        driver_src.push_back(src);
        driver_dst.push_back(dst);
    }
    while (f_passengers >> src >> dst) {
        passenger_src.push_back(src);
        passenger_dst.push_back(dst);
    }
    assert(passenger_src.size() == passenger_dst.size());
    assert(driver_src.size() == driver_dst.size());
    f_drivers.close();
    f_passengers.close();
    std::cout << "Successfully load drivers and passengers" << std::endl;

    // step1: load drivers' src to g-tree, then do knn on each passenger's src
    gtree::pre_query(driver_src);
    for (int i = 0; i < passenger_src.size(); ++i) {
        auto ret = gtree::knn_query(passenger_src[i], K);
        vector<int> candidate;
        candidate.clear();
        for (auto& j : ret) {
            candidate.push_back(j.id);
        }
        driver_candidate.emplace_back(candidate);
        std::cout << "knn for passenger src:" << i << "/" << passenger_src.size() << std::endl;
    }
    // step2: load drives' dst to g-tree, then fo knn on each passenger's dst
    // step3: cal intersection for drivers found in step1/2 for each passenger
    gtree::pre_query(driver_dst);
    for (int i = 0; i < passenger_dst.size(); ++i) {
        auto ret = gtree::knn_query(passenger_dst[i], K);
        int pre_size = driver_candidate[i].size();
        vector<int> candidate;
        candidate.clear();
        for (auto& j : ret) {
            //driver_candidate[i].push_back(j.id);
            candidate.push_back(j.id);
        }
        vector<int> tmp;
        tmp.clear();
        std::sort(candidate.begin(), candidate.end());
        std::sort(driver_candidate[i].begin(), driver_candidate[i].end());
        std::set_intersection(candidate.begin(), candidate.end(), 
                driver_candidate[i].begin(), driver_candidate[i].end(), std::back_inserter(tmp));
        //driver_candidate.emplace(driver_candidate.begin() + i, tmp);
        driver_candidate[i] = tmp;
        std::cout << "knn for passenger dst:" << i << "/" << passenger_src.size() << std::endl;
        std::cout << "size change(old/real): " << pre_size << "/" << driver_candidate[i].size() << std::endl;
    }
    // step4: using g-plus-tree for cal share-rate
    assert(passenger_src.size() == driver_candidate.size());
    for (int i = 0; i < passenger_src.size(); ++i) {
        // this passenger's normal distance
        double dis_mid = g_plus_tree::tree.search(passenger_src[i], passenger_dst[i]);
        unordered_map<int, double> share_rate;
        share_rate.clear();
        assert(share_rate.size() == 0);
        for (auto& j : driver_candidate[i]) {
            double dis_pre = g_plus_tree::tree.search(driver_src[j], passenger_src[i]);
            double dis_end = g_plus_tree::tree.search(driver_dst[j], passenger_dst[i]);
            double rate = dis_mid / (dis_pre + dis_mid + dis_end);
            std::cout << "cal share rate for passenger:" << i << " driver:" << j << " rate:" << rate;
            if (rate >= share_rate_limit) {
                share_rate[j] = rate;
                std::cout << " accept" << std::endl;
            } else {
                std::cout << " reject" << std::endl;
            }
        }
        driver_share_rate.emplace_back(share_rate);
    }

    // step5: build the bi-partite graph
    // for (int i = 0; i < 10000; ++i) {
    //     unordered_map<int, double> tmp;
    //     tmp[i] = 0.1;
    //     driver_share_rate.emplace_back(tmp);
    // }
    for (int i = 0; i < driver_share_rate.size(); ++i) {
        passenger_to_driver[i] = driver_share_rate[i];
        for (auto j = driver_share_rate[i].begin(); j != driver_share_rate[i].end(); ++j) {
            int driver_id = j->first;
            int passenger_id = i;
            double rate = j->second;

            auto res = driver_to_passenger.find(driver_id);
            if (res != driver_to_passenger.end()) {
                assert(res->first == driver_id);
                (res->second)[passenger_id] = rate;
            } else {
                unordered_map<int, double> tmp;
                tmp[passenger_id] = rate;
                driver_to_passenger[driver_id] = tmp;
            }
        }
    }
    // step6: KM 
    // align the size of passenger and driver
    int node_amount = passenger_src.size() < driver_src.size() ? driver_src.size() :
        passenger_src.size();
    km::init(node_amount);  // alloc mem
    auto edge = km::get_global_edge();
    int share_rate_times = 100000;  // avoid using double
    for (int i = 0; i < node_amount; ++i) {
        for (int j = 0; j < node_amount; ++j) {
            edge[i+1][j+1] = share_rate_times*passenger_get_share_rate(i, j);
            if (edge[i+1][j+1] != 0) {
                std::cout << edge[i+1][j+1] << std::endl;
            }
        }
        //std::cout << "loading graph:" << i << "/" << node_amount << std::endl;
    } 
    std::cout << "load graph finished!" << std::endl;
    auto ret = km::get_perfect_match();
    
    // step7: output results
    std::ofstream f_res{file_result};
    f_res << "passenger_id driver_id share_rate" << std::endl;
    double share_rate_amount = 0.0;
    int match_pair_amount = 0;
    for (auto& i : ret) {
        int passenger = std::get<0>(i);
        int driver = std::get<1>(i);
        double rate = (double)std::get<2>(i)/(double)share_rate_times;
        
        // judge if exist
        if (passenger < passenger_src.size() && driver < driver_src.size() && rate >= share_rate_limit) {
            f_res << passenger << " " << driver << " " << rate << std::endl;
            std::cout << passenger << " " << driver << " " << rate << std::endl;
            share_rate_amount += rate;
            match_pair_amount ++;
        }
    }
    f_res << "Total matched pairs: " << match_pair_amount << std::endl;
    std::cout << "Total matched pairs: " << match_pair_amount << std::endl;
    f_res << "Total share rate: " << share_rate_amount << std::endl;
    std::cout << "Total share rate: " << share_rate_amount << std::endl;
    f_res.close();
    std::cout << "All done, result will be written to " << file_result << std::endl;
    return 0;
}
