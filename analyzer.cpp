#pragma once
#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

void TripAnalyzer::ingestFile(const string& csvPath) {
    ifstream file(csvPath);
    if (!file.is_open()) return;

    string line;
    if (!getline(file, line)) return;

    while (getline(file, line)) {
        if (line.empty()) continue;

        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }
        if (line.empty()) continue;

        stringstream ss(line);
        string field;
        vector<string> row;

        while (getline(ss, field, ',')) {
            row.push_back(field);
        }

        if (row.size() != 6) continue;

        const string& zone = row[1];
        const string& dateTime = row[3];

        if (zone.empty() || dateTime.length() < 13) continue;

        try {
            int hour = stoi(dateTime.substr(11, 2));
            if (hour >= 0 && hour <= 23) {
                zoneAggregates[zone]++;

                if (slotAggregates.find(zone) == slotAggregates.end()) {
                    slotAggregates[zone] = vector<long long>(24, 0);
                }
                slotAggregates[zone][hour]++;
            }
        }
        catch (...) {
            continue;
        }
    }
    file.close();
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> results;
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
