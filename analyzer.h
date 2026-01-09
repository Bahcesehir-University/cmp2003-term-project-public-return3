#pragma once

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct ZoneCount {
    string zone;
    long long count;
};

struct SlotCount {
    string zone;
    int hour;
    long long count;
};

class TripAnalyzer {
public:
    void ingestFile(const string& csvPath);
    vector<ZoneCount> topZones(int k) const;
    vector<SlotCount> topBusySlots(int k) const;

private:
    unordered_map<string, long long> zoneAggregates;
    unordered_map<string, vector<long long>> slotAggregates;
};
