#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

struct TLBEntry {
    int counter = -1;
    int VPN = -1;
    int PPN = -1;
};

int main(int argc, char* argv[]) {
    if (argc < 5) {
        cerr << "Usage: " << argv[0] << " <page_table_file> <va_file> <LRU|FIFO> <ways>\n";
        return 1;
    }

    string pageTableFile   = argv[1];
    string tlbAccessesFile = argv[2];
    string replacementPolicy = argv[3];
    string waysStr         = argv[4];

    int associativity = stoi(waysStr);

    // LRU: evict entry with highest counter (counter == associativity-1)
    // FIFO: same structure, but counters only increment on miss (not on hit)
    int evictThreshold = associativity - 1;

    string outputFile = string("23335_") + argv[1] + "_" + argv[2] + "_" + replacementPolicy + "_" + waysStr;

    // --- Read virtual addresses ---
    ifstream vaStream(tlbAccessesFile);
    if (!vaStream) { cerr << "Cannot open " << tlbAccessesFile << "\n"; return 1; }

    int pagesize;
    vaStream >> pagesize;
    pagesize = (int)pow(2, pagesize);

    vector<int> virtualAddresses;
    int va;
    while (vaStream >> va)
        virtualAddresses.push_back(va);
    vaStream.close();

    int totalAccesses = (int)virtualAddresses.size();

    // --- Read page table ---
    ifstream ptStream(pageTableFile);
    if (!ptStream) { cerr << "Cannot open " << pageTableFile << "\n"; return 1; }

    // Skip header line
    string line;
    getline(ptStream, line); // "VPN      PFN"

    vector<pair<int,int>> pageTable; // {VPN, PFN}
    int vpn, pfn;
    while (ptStream >> vpn >> pfn)
        pageTable.push_back({vpn, pfn});
    ptStream.close();

    // --- TLB simulation ---
    vector<TLBEntry> tlb(associativity);
    int misses = 0;

    ofstream out(outputFile);

    for (int i = 0; i < totalAccesses; i++) {
        int VPN       = virtualAddresses[i] / pagesize;
        int offset    = virtualAddresses[i] % pagesize;
        int PPN       = -1;
        bool miss     = false;
        int hitIdx    = -1;

        // Search TLB
        for (int j = 0; j < associativity; j++) {
            if (tlb[j].VPN == VPN) { hitIdx = j; break; }
        }

        if (hitIdx != -1) {
            // HIT
            if (replacementPolicy == "LRU") {
                int x = tlb[hitIdx].counter;
                tlb[hitIdx].counter = 0;
                for (int z = 0; z < associativity; z++) {
                    if (z != hitIdx && tlb[z].counter >= 0 && tlb[z].counter < x)
                        tlb[z].counter++;
                }
            }
            PPN = tlb[hitIdx].PPN;
        } else {
            // MISS
            miss = true;
            misses++;

            // Look up PPN in page table
            for (auto& entry : pageTable) {
                if (entry.first == VPN) { PPN = entry.second; break; }
            }

            // Find insertion slot: first empty, or evict
            int insertIdx = -1;
            for (int j = 0; j < associativity; j++) {
                if (tlb[j].VPN == -1) { insertIdx = j; break; }
            }

            if (insertIdx == -1) {
                // Evict entry with counter == evictThreshold
                for (int j = 0; j < associativity; j++) {
                    if (tlb[j].counter == evictThreshold) { insertIdx = j; break; }
                }
            }

            tlb[insertIdx] = {0, VPN, PPN};

            // Age all other valid entries
            for (int z = 0; z < associativity; z++) {
                if (z != insertIdx && tlb[z].VPN != -1)
                    tlb[z].counter++;
            }
        }

        int PA = (PPN * pagesize) + offset;
        out << PA << (miss ? " MISS\n" : " HIT\n");
    }

    out.close();

    // Prepend summary to output file
    ifstream inFile(outputFile);
    string body((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    ofstream outFile(outputFile);
    outFile << "TOTAL_ACCESSES = " << totalAccesses << "\n";
    outFile << "TOTAL_MISSES = "   << misses << "\n";
    outFile << "TOTAL_HITS = "     << (totalAccesses - misses) << "\n";
    outFile << body;
    outFile.close();

    return 0;
}
