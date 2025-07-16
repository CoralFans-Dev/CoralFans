#include "coral_fans/functions/prof/Prof.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/i18n/I18n.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/world/level/ChunkPos.h"

#include <algorithm>
#include <cstdint>
#include <format>
#include <numeric>

namespace coral_fans::functions {

std::vector<std::pair<std::string, uint64>> Profiler::TypeVec = {
    {"normal", Type::normal},
    {"entity", Type::entity},
    {"chunk",  Type::chunk },
    {"pt",     Type::pt    }
};

// from trapdoor-ll

long long MSPTInfo::mean() const {
    uint8_t right = 20;
    if (!values[index]) {
        if (!index) return 0ll;
        right = index;
    }
    return std::accumulate(values, values + right, 0ll) / 20;
}

void MSPTInfo::push(long long value) {
    this->values[index] = value;
    index               = ++index % 20;
}

long long MSPTInfo::min() const {
    uint8_t right = 20;
    if (!values[index]) {
        if (!index) return 0;
        right = index;
    }
    auto min = this->values[0];
    for (int i = 1; i < right; i++)
        if (min > values[i]) min = values[i];
    return min;
}

std::pair<long long, long long> MSPTInfo::pairs() const {
    long long v1 = 0;
    long long v2 = 0;
    if (!values[index]) {
        uint8_t v1count = 0, v2count = 0;
        if (index % 2) {
            v1      += values[0];
            v1count  = 1;
        }
        for (uint8_t i = v1count; i < index; i += 2) {
            v2 += values[i];
            v1 += values[i + 1];
        }
        v1count = (index + 1) % 2;
        v2count = index % 2;
        if (v1count) v1 /= v1count;
        if (v2count) v2 /= v2count;
        return {v1, v2};
    } else {
        for (uint8_t i = 0; i < 20; i += 2) {
            v1 += values[i];
            v2 += values[i + 1];
        }
        v1 /= 10;
        v2 /= 10;
    }
    if (v1 > v2) std::swap(v1, v2);
    return {v1, v2};
}

long long MSPTInfo::max() const {
    uint8_t right = 20;
    if (!values[index]) {
        if (!index) return 0;
        right = index;
    }
    auto max = this->values[0];
    for (int i = 1; i < right; i++)
        if (max < values[i]) max = values[i];
    return max;
}

double micro_to_mill(long long v) { return static_cast<double>(v) / 1000.0; }

void Profiler::reset(uint64 t) {
    this->type = t;
    this->redstoneInfo.reset();
    this->chunkInfo.reset();
    this->gameSessionTickTime = 0;
    this->gameSessionTicksBuffer.clear();
    this->dimensionTickTime    = 0;
    this->entitySystemTickTime = 0;
    for (auto& m : this->actorInfo) m.clear();
    for (auto& pt : this->ptCounter) pt.clear();
}

void Profiler::start(long long round, uint64 t) {
    this->reset(t);
    this->profiling    = true;
    this->currentRound = 0;
    this->totalRound   = round;
}

void Profiler::stop() {
    this->profiling = false;
    this->print();
    this->reset(Profiler::Type::normal);
}

void Profiler::print() const {
    std::string rst;
    switch (this->type) {
    case Profiler::Type::normal:
        rst = this->printBasics();
        break;
    case Profiler::Type::entity:
        rst = this->printActor();
        break;
    case Profiler::Type::chunk:
        rst = this->printChunks();
        break;
    case Profiler::Type::pt:
        rst = this->printPendingTicks();
        break;
    }
    TextPacket::createRawMessage(rst).sendToClients();
    coral_fans::mod().getLogger().info(rst);
}

std::string Profiler::printChunks() const {
    using ll::i18n_literals::operator""_tr;
    const static std::string dims[] =
        {"translate.dimension.overworld"_tr(), "translate.dimension.nether"_tr(), "translate.dimension.theend"_tr()};
    std::string retstr;
    for (int i = 0; i < 3; ++i) {
        auto& dim_data = this->chunkInfo.chunk_counter[i];
        if (!dim_data.empty()) {
            retstr += "§b§l-- " + dims[i] + " --§r\n";
            std::vector<std::pair<ChunkPos, double>> v;
            for (auto& kv : dim_data) {
                if (!kv.second.empty()) {
                    auto time = micro_to_mill(std::accumulate(kv.second.begin(), kv.second.end(), 0ll))
                              / static_cast<double>(kv.second.size());
                    v.emplace_back(kv.first, time);
                }
            }
            long long sort_count = std::min(static_cast<long long>(v.size()), 5ll);
            std::partial_sort(
                v.begin(),
                v.begin() + sort_count,
                v.end(),
                [](const std::pair<ChunkPos, double>& p1, const std::pair<ChunkPos, double>& p2) {
                    return p1.second > p2.second;
                }
            );
            for (int j = 0; j < sort_count; ++j) retstr += std::format(" - §a{}§r  {:.3f}\n", v[j].first, v[j].second);
        }
    }
    return retstr;
}

std::string Profiler::printPendingTicks() const {
    using ll::i18n_literals::operator""_tr;
    const static std::string dims[] =
        {"translate.dimension.overworld"_tr(), "translate.dimension.nether"_tr(), "translate.dimension.theend"_tr()};
    std::string retstr;
    for (int i = 0; i < 3; ++i) {
        auto& pt_data = this->ptCounter[i];
        if (!pt_data.empty()) {
            retstr += "§b§l-- " + dims[i] + " --§r\n";
            std::vector<std::pair<ChunkPos, long long>> v;
            for (auto& kv : pt_data)
                if (kv.second != 0) v.emplace_back(kv);
            std::sort(
                v.begin(),
                v.end(),
                [](const std::pair<ChunkPos, long long>& p1, const std::pair<ChunkPos, long long>& p2) {
                    return p1.second > p2.second;
                }
            );
            if (v.size() > 10) v.resize(10);
            for (auto& item : v) retstr += std::format(" - §a{}§r  {}\n", item.first.toString(), item.second);
        }
    }
    return retstr;
}

std::string Profiler::printBasics() const {
    using ll::i18n_literals::operator""_tr;
    const double divide = 1000.0 * static_cast<double>(totalRound);
    auto         mean   = [divide](long long time) { return static_cast<double>(time) / divide; };
    auto         mspt   = mean(gameSessionTickTime);
    int          tps    = mspt <= 50 ? 20 : static_cast<int>(1000.0 / mspt);
    auto         res    = "translate.profiler.normal"_tr(
        /*summary*/
        mspt,
        tps,
        this->chunkInfo.getChunkNumber(),
        /*coralfans*/
        mean(coralfansSessionTickTime),
        /*redstone*/
        mean(redstoneInfo.sum()),
        mean(redstoneInfo.signalUpdate),
        mean(redstoneInfo.pendingAdd),
        mean(redstoneInfo.pendingUpdate),
        mean(redstoneInfo.pendingRemove),
        /*dimension*/
        mean(dimensionTickTime),
        /*entity system*/
        mean(entitySystemTickTime),
        /*chunks*/
        mean(static_cast<long long>(chunkInfo.totalTickTime)),
        mean(chunkInfo.blockEntitiesTickTime),
        mean(chunkInfo.randomTickTime),
        mean(chunkInfo.pendingTickTime)
    ); //
    return res;
}

std::string Profiler::printActor() const {
    using ll::i18n_literals::operator""_tr;
    const static std::string dims[] =
        {"translate.dimension.overworld"_tr(), "translate.dimension.nether"_tr(), "translate.dimension.theend"_tr()};
    std::string retstr;
    double      totalTime = 0.0;
    for (int i = 0; i < 3; i++) {
        auto& actor_data = this->actorInfo[i];
        if (!actor_data.empty()) {
            retstr += "§b§l-- " + dims[i] + " --§r\n";
            std::vector<std::pair<std::string, EntityInfo>> v;
            for (auto& kv : actor_data) v.emplace_back(kv);
            std::sort(
                v.begin(),
                v.end(),
                [](const std::pair<std::string, EntityInfo>& p1, const std::pair<std::string, EntityInfo>& p2) {
                    return p1.second.time > p2.second.time;
                }
            );
            for (auto& item : v) {
                retstr += std::format(
                    " - §a{}§r  {:.3f} ms ({})\n",
                    item.first,
                    micro_to_mill(item.second.time) / static_cast<double>(this->totalRound),
                    item.second.count / totalRound
                );
                totalTime += micro_to_mill(item.second.time) / static_cast<double>(this->totalRound);
            }
        }
    }
    retstr = "translate.profiler.actor.total"_tr(totalTime) + retstr;
    return retstr;
}

} // namespace coral_fans::functions