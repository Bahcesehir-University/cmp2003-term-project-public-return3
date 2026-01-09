#include "analyzer.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

using namespace std;

void TripAnalyzer::ingestFile(const string& csvPath) {
    ifstream file(csvPath);
    if (!file.is_open()) return;

    string line;
    if (!getline(file, line)) return;

    while (getline(file, line)) {
        if (line.empty()) continue;

        if (line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        size_t p1 = line.find(',');
        if (p1 == string::npos) continue;

        size_t p2 = line.find(',', p1 + 1);
        if (p2 == string::npos) continue;

        string zone = line.substr(p1 + 1, p2 - p1 - 1);
        if (zone.empty()) continue;

        size_t p3 = line.find(',', p2 + 1);
        if (p3 == string::npos) continue;

        size_t dateStart = p3 + 1;

        if (line.length() < dateStart + 13) continue;

        char h1 = line[dateStart + 11];
        char h2 = line[dateStart + 12];

        if (h1 >= '0' && h1 <= '9' && h2 >= '0' && h2 <= '9') {
            int hour = (h1 - '0') * 10 + (h2 - '0');

            if (hour >= 0 && hour <= 23) {
                zoneAggregates[zone]++;

                if (slotAggregates.find(zone) == slotAggregates.end()) {
                    slotAggregates[zone] = vector<long long>(24, 0);
                }
                slotAggregates[zone][hour]++;
            }
        }
    }
    file.close();
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> results;
    results.reserve(zoneAggregates.size());
    for (const auto& pair : zoneAggregates) {
        results.push_back({ pair.first, pair.second });
    }

    sort(results.begin(), results.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if ((int)results.size() > k) results.resize(k);
    return results;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> results;
    for (const auto& entry : slotAggregates) {
        for (int h = 0; h < 24; ++h) {
            if (entry.second[h] > 0) {
                results.push_back({ entry.first, h, entry.second[h] });
            }
        }
    }

    sort(results.begin(), results.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if ((int)results.size() > k) results.resize(k);
    return results;
}
