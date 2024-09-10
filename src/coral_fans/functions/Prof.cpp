#include "coral_fans/functions/Prof.h"
#include "coral_fans/base/Mod.h"
#include "ll/api/i18n/I18n.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/world/level/ChunkPos.h"

#include <algorithm>
#include <format>
#include <numeric>

namespace coral_fans::functions {

// from trapdoor-ll

long long MSPTInfo::mean() const {
    return this->values.empty()
             ? 0
             : std::accumulate(values.begin(), values.end(), 0ll) / static_cast<long long>(values.size());
}

void MSPTInfo::push(long long value) {
    this->values.push_back(value);
    if (this->values.size() > 20) this->values.pop_front();
}

long long MSPTInfo::min() const {
    if (this->values.empty()) return 0;
    auto min = this->values[0];
    for (auto v : this->values)
        if (min > v) min = v;
    return min;
}

std::pair<long long, long long> MSPTInfo::pairs() const {
    long long v1 = 0, v1count = 0;
    long long v2 = 0, v2count = 0;
    for (unsigned long long i = 0; i < values.size(); ++i) {
        if (i % 2 == 0) {
            v1 += values[i];
            v1count++;
        } else {
            v2 += values[i];
            v2count++;
        }
    }
    if (v1count != 0) v1 /= v1count;
    if (v2count != 0) v2 /= v2count;
    if (v1 > v2) std::swap(v1, v2);
    return {v1, v2};
}

long long MSPTInfo::max() const {
    if (values.empty()) return 0;
    auto max = values[0];
    for (auto v : values)
        if (max < v) max = v;
    return max;
}

double micro_to_mill(long long v) { return static_cast<double>(v) / 1000.0; }

void Profiler::reset(Profiler::Type t) {
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

void Profiler::start(long long round, Profiler::Type t) {
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
    const double divide = 1000.0 * static_cast<double>(totalRound);
    auto         mean   = [divide](long long time) { return static_cast<double>(time) / divide; };
    auto         mspt   = mean(gameSessionTickTime);
    int          tps    = mspt <= 50 ? 20 : static_cast<int>(1000.0 / mspt);
    auto         res    = std::format(
        "- MSPT: {:.3f} ms TPS: {} Chunks: {}\n"
                   "- CoralFans: {:.3f} ms\n"
                   "- Redstone: {:.3f} ms\n"
                   " - Signal: {:.3f} ms\n"
                   " - Add: {:.3f} ms\n"
                   " - Update: {:.3f} ms\n"
                   " - Remove: {:.3f} ms\n"
                   "- Chunk (un)load & village: {:.3f} ms\n"
                   "- EntitySystems: {:.3f} ms\n"
                   " - Chunk tick: {:.3f} ms\n"
                   "  - BlockEntities: {:.3f} ms\n"
                   "  - RandomTick: {:.3f} ms\n"
                   "  - PendingTick: {:.3f} ms\n",
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
    retstr = "translate.actorprofiler.total"_tr(totalTime) + retstr;
    return retstr;
}

} // namespace coral_fans::functions