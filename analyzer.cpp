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
    // İlk satırı (Header) atla
    if (!getline(file, line)) return;

    // Satır satır okuma döngüsü
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        // Windows/Linux satır sonu uyumluluğu (\r temizleme)
        if (line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        // --- MANUEL PARSING (OPTIMIZASYON) ---
        // Virgül pozisyonlarını find ile buluyoruz, split yapmıyoruz.
        
        // 1. Virgül
        size_t p1 = line.find(',');
        if (p1 == string::npos) continue;

        // 2. Virgül (Zone, p1 ile p2 arasında)
        size_t p2 = line.find(',', p1 + 1);
        if (p2 == string::npos) continue;

        // Zone bilgisini al (substr mecbur ama sadece gerekli kısmı alıyoruz)
        string zone = line.substr(p1 + 1, p2 - p1 - 1);
        if (zone.empty()) continue;

        // 3. Virgül
        size_t p3 = line.find(',', p2 + 1);
        if (p3 == string::npos) continue;

        // 4. Virgül (DateTime, p3 ile p4 arasında ama bize sadece saati lazım)
        // DateTime stringinin tamamını kopyalamaya gerek yok.
        // Format: YYYY-MM-DD HH:MM:SS
        // DateTime başlangıcı: p3 + 1
        // Saat (HH) konumu: Başlangıçtan itibaren 11. ve 12. karakterler.
        
        size_t dateStart = p3 + 1;
        
        // Satır uzunluğu kontrolü (Segmentation fault yememek için)
        if (line.length() < dateStart + 13) continue;

        // HIZLI SAAT HESAPLAMA (stoi yerine ASCII matematik)
        char h1 = line[dateStart + 11];
        char h2 = line[dateStart + 12];

        // Karakterlerin rakam olup olmadığını kontrol et
        if (h1 >= '0' && h1 <= '9' && h2 >= '0' && h2 <= '9') {
            // ASCII '0' çıkararak int'e çeviriyoruz
            int hour = (h1 - '0') * 10 + (h2 - '0');

            if (hour >= 0 && hour <= 23) {
                // Veriyi işle
                zoneAggregates[zone]++;

                // Slot vektörü yoksa oluştur
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
    results.reserve(zoneAggregates.size()); // Performans için reserve
    for (const auto& pair : zoneAggregates) {
        results.push_back({ pair.first, pair.second });
    }

    // Sıralama Kuralları: Önce sayı (büyükten küçüğe), eşitse Zone ismi (alfabetik)
    sort(results.begin(), results.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if ((int)results.size() > k) results.resize(k);
    return results;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> results;
    // Slotları düzleştir
    for (const auto& entry : slotAggregates) {
        for (int h = 0; h < 24; ++h) {
            if (entry.second[h] > 0) {
                results.push_back({ entry.first, h, entry.second[h] });
            }
        }
    }

    // Sıralama Kuralları: Sayı > Zone İsmi > Saat
    sort(results.begin(), results.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if ((int)results.size() > k) results.resize(k);
    return results;
}
