#pragma once
#include <utility>

namespace nfs::kernel::lifecycle {
template <class Action> class Cleanup {
  public:
    explicit Cleanup(Action action) : action_(std::move(action)) {}
    Cleanup(const Cleanup &) = delete;
    Cleanup &operator=(const Cleanup &) = delete;
    ~Cleanup() noexcept(false) { run(); }

    void dismiss() noexcept { active_ = false; }
    void run() {
        if (active_) {
            active_ = false;
            action_();
        }
    }

  private:
    Action action_;
    bool active_ = true;
};
} // namespace nfs::kernel::lifecycle
