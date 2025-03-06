namespace coral_fans::functions {

class FuncDropNoCostManager {
public:
    int  _slot, _count;
    bool mutex = false, mutex2 = true;

    static FuncDropNoCostManager& getInstance() {
        static FuncDropNoCostManager instance;
        return instance;
    }
};
void droppernocostHook(bool);
} // namespace coral_fans::functions