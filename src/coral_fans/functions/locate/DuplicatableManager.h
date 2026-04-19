#pragma once


namespace coral_fans::functions::locate {

class NetherDuplicatableController;
class TheEndDuplicatableController;

class DuplicatableManager {
public:
    enum class ShowType : unsigned int {
        Netherite    = 1 << 0,
        NetherSpring = 1 << 1,
        NetherFire   = 1 << 2,
        GlowStone    = 1 << 3,
        Mushroom     = 1 << 4,
        NetherGold   = 1 << 5,
        NetherQuartz = 1 << 6,
        NetherMagma  = 1 << 7,
        NetherGravel = 1 << 8,
        Blackstone   = 1 << 9,
        SoulSand     = 1 << 10,
        EndIsland    = 1 << 11,
        ChorusFlower = 1 << 12,
        EndGateway   = 1 << 13,
    };

private:
    unsigned int showType                   = 0;
    int          tickCounter                = 1;
    int          cacheDataRemoveTickCounter = 1;

    NetherDuplicatableController* netherController;
    TheEndDuplicatableController* theEndController;

private:
    void removeData();
    void draw();
    void bsciDataRuntimeRemove();

public:
    void tick();
    void setShowType(ShowType, bool);
    bool getShowType(ShowType);
    void clear();

    static void hook(bool);

private:
    DuplicatableManager();
    ~DuplicatableManager();

public:
    [[nodiscard]] static DuplicatableManager& getInstance();

    DuplicatableManager(const DuplicatableManager&)            = delete;
    DuplicatableManager& operator=(const DuplicatableManager&) = delete;
};

} // namespace coral_fans::functions::locate
