#include "coral_fans/CoralFans.h"
#include "coral_fans/base/MySchedule.h"


#include "ll/api/i18n/I18n.h"
#include "mc/deps/nbt/CompoundTagVariant.h"
#include "mc/deps/nbt/ListTag.h"
#include "mc/network/packet/TextPacket.h"
#include "mc/network/packet/TextPacketPayload.h"
#include "mc/world/Container.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/ChunkPos.h"
#include <algorithm>
#include <memory>
#include <string>
#include <string_view>


namespace coral_fans::utils {

namespace {

struct PathNode {
    std::string id;
    int         index;
    bool        useIndex;
};

bool parsePath(std::string const& path, std::vector<PathNode>& vec) {
    for (auto key : ll::string_utils::splitByPattern(path, ".")) {
        if (key.ends_with(')')) try {
                int index = (int)key.find('(');
                vec.emplace_back(
                    PathNode{
                        std::string{key.substr(0, index)},
                        std::stoi(std::string{key.substr(index + 1, key.length() - index - 2)}),
                        true
                    }
                );
            } catch (...) {
                return false;
            }
        else vec.emplace_back(PathNode{std::string{key}, 0, false});
    }
    return true;
}

} // namespace

std::pair<std::string, bool> getNbtFromTag(CompoundTag& tag, std::string const& path) {
    using ll::i18n_literals::operator""_tr;
    std::vector<PathNode> nodes;
    if (!parsePath(path, nodes)) return {"translate.data.error.cannotparse"_tr(), false};
    try {
        auto it = tag.mTags.find(nodes[0].id);
        if (it == tag.mTags.end()) return {"translate.data.error.geterror"_tr(), false};
        CompoundTagVariant& tagVariant = it->second;
        if (nodes[0].useIndex) {
            if (tagVariant.is_array()) {
                auto& list = tagVariant.get<ListTag>();
                if (nodes[0].index >= static_cast<int>(list.size()))
                    return {"translate.data.error.geterror"_tr(), false};
                tagVariant = std::move(list[nodes[0].index]);
            }
        }
        for (unsigned long long i = 1; i < nodes.size(); i++) {
            tagVariant = std::move(tagVariant[nodes[i].id]);
            if (nodes[i].useIndex) {
                if (tagVariant.is_array()) {
                    auto& list = tagVariant.get<ListTag>();
                    if (nodes[i].index >= static_cast<int>(list.size()))
                        return {"translate.data.error.geterror"_tr(), false};
                    tagVariant = std::move(list[nodes[i].index]);
                }
            }
        }
        return {tagVariant.toSnbt(SnbtFormat::PrettyChatPrint), true};
    } catch (...) {
        return {"translate.data.error.geterror"_tr(), false};
    }
}

ChunkPos blockPosToChunkPos(BlockPos const& blockPos) {
    return ChunkPos{
        (blockPos.x < 0 ? blockPos.x - 15 : blockPos.x) / 16,
        (blockPos.z < 0 ? blockPos.z - 15 : blockPos.z) / 16
    };
}

std::string removeMinecraftPrefix(std::string const& s) { return s.find("minecraft:") == 0 ? s.substr(10) : s; }

void shortHighligntBlock(int dimid, BlockPos const& blockPos, mce::Color const& color, int time) {
    auto& mod = CoralFans::getInstance();
    auto  s   = mod.getGeometryGroup()->box(
        dimid,
        {
            blockPos,
            {blockPos.x + 1, blockPos.y + 1, blockPos.z + 1}
    },
        color
    );
    my_schedule::MySchedule::getSchedule().add(
        [&, s](int&, int&) {
            mod.getGeometryGroup()->remove(s);
            return false;
        },
        time
    );
}

void swapItemInContainer(Player* player, int slot1, int slot2) {
    if (player && slot1 != slot2) {
        auto& inventory = player->mInventory->mInventory;
        if (slot1 == -1) {
            ItemStack tem = player->getOffhandSlot();
            player->setOffhandSlot(inventory->getItem(slot2));
            inventory->setItem(slot2, tem);
        } else if (slot2 == -1) {
            ItemStack tem = player->getOffhandSlot();
            player->setOffhandSlot(inventory->getItem(slot1));
            inventory->setItem(slot1, tem);
        } else {
            ItemStack tem = inventory->getItem(slot1);
            inventory->setItem(slot1, inventory->getItem(slot2));
            inventory->setItem(slot2, tem);
        }
    }
}

namespace {

constexpr size_t maxTextPacketBodyLength = 1000;

char lowerFormatCode(char code) {
    if (code >= 'A' && code <= 'Z') return static_cast<char>(code + ('a' - 'A'));
    return code;
}

bool isSectionSign(std::string_view text, size_t pos) {
    return pos + 1 < text.size() && static_cast<unsigned char>(text[pos]) == 0xC2
        && static_cast<unsigned char>(text[pos + 1]) == 0xA7;
}

bool isFormatCode(char code) {
    code = lowerFormatCode(code);
    return (code >= '0' && code <= '9') || (code >= 'a' && code <= 'g') || (code >= 'k' && code <= 'o') || code == 'r';
}

struct TextFormatState {
    char color         = 0;
    bool bold          = false;
    bool italic        = false;
    bool underline     = false;
    bool strikethrough = false;
    bool obfuscated    = false;

    void apply(char rawCode) {
        char code = lowerFormatCode(rawCode);
        if ((code >= '0' && code <= '9') || (code >= 'a' && code <= 'g')) {
            color         = code;
            bold          = false;
            italic        = false;
            underline     = false;
            strikethrough = false;
            obfuscated    = false;
            return;
        }
        switch (code) {
        case 'r':
            color         = 0;
            bold          = false;
            italic        = false;
            underline     = false;
            strikethrough = false;
            obfuscated    = false;
            break;
        case 'l':
            bold = true;
            break;
        case 'o':
            italic = true;
            break;
        case 'n':
            underline = true;
            break;
        case 'm':
            strikethrough = true;
            break;
        case 'k':
            obfuscated = true;
            break;
        default:
            break;
        }
    }

    void scan(std::string_view text) {
        for (size_t i = 0; i < text.size();) {
            if (isSectionSign(text, i)) {
                if (i + 2 < text.size()) apply(text[i + 2]);
                i += 2;
            } else ++i;
        }
    }

    std::string prefix() const {
        std::string result;
        auto        appendCode = [&result](char code) {
            result.append("\xC2\xA7", 2);
            result += code;
        };
        if (color) appendCode(color);
        if (bold) appendCode('l');
        if (italic) appendCode('o');
        if (underline) appendCode('n');
        if (strikethrough) appendCode('m');
        if (obfuscated) appendCode('k');
        return result;
    }
};

size_t utf8SafeLimit(std::string_view text, size_t limit) {
    if (limit >= text.size()) return text.size();
    while (limit > 0 && (static_cast<unsigned char>(text[limit]) & 0xC0) == 0x80) --limit;
    return limit;
}

size_t formatSafeLimit(std::string_view text, size_t limit) {
    if (limit == 0 || limit >= text.size()) return limit;
    if (limit >= 2 && isSectionSign(text, limit - 2) && isFormatCode(text[limit])) return limit - 2;
    return limit;
}

void sendTextPacketToPlayer(Player& player, TextPacketType textType, std::string const& text) {
    TextPacket packet;
    packet.mBody = TextPacketPayload::MessageOnly(textType, text);
    packet.sendTo(player);
}

void sendTextPacketToClients(TextPacketType textType, std::string const& text) {
    TextPacket packet;
    packet.mBody = TextPacketPayload::MessageOnly(textType, text);
    packet.sendToClients();
}

template <typename Sender>
void segmentAndSend(std::string const& text, TextPacketType textType, Sender&& sender) {
    if (text.empty()) {
        sender(textType, text);
        return;
    }

    std::string_view remaining{text};
    TextFormatState  state;
    while (!remaining.empty()) {
        std::string prefix = state.prefix();
        if (prefix.size() + remaining.size() <= maxTextPacketBodyLength) {
            sender(textType, prefix + std::string{remaining});
            return;
        }

        size_t capacity = prefix.size() >= maxTextPacketBodyLength ? 0 : maxTextPacketBodyLength - prefix.size();
        size_t limit    = std::min(capacity, remaining.size());
        limit           = utf8SafeLimit(remaining, limit);
        limit           = formatSafeLimit(remaining, limit);
        if (limit == 0) limit = std::min(remaining.size(), static_cast<size_t>(1));

        size_t newline        = remaining.substr(0, limit).find_last_of('\n');
        bool   splitAtNewline = newline != std::string_view::npos;
        size_t chunkSize      = splitAtNewline ? newline : limit;

        std::string chunk{remaining.substr(0, chunkSize)};
        sender(textType, prefix + chunk);
        state.scan(chunk);
        remaining.remove_prefix(chunkSize + (splitAtNewline ? 1 : 0));
    }
}

} // namespace

void segmentAndSendToPlayer(std::string const& text, Player* player, TextPacketType textType) {
    if (!player) return;
    segmentAndSend(text, textType, [player](TextPacketType type, std::string const& segment) {
        sendTextPacketToPlayer(*player, type, segment);
    });
}

void segmentAndSendToClients(std::string const& text, TextPacketType textType) {
    segmentAndSend(text, textType, [](TextPacketType type, std::string const& segment) {
        sendTextPacketToClients(type, segment);
    });
}
} // namespace coral_fans::utils