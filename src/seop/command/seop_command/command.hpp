#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace seop::command
{
enum class Command_type
{
    // window

    // Graphic

    // Scene
    Resize_particle,
    // Imgui

    // End
    End
};

class Command
{
  public:
    virtual ~Command() = default;
    virtual bool execute() = 0;

  private:
};

class Command_list
{
  public:
    template <typename T, typename... Args> void register_command(Args &&...args)
    {
        commands_[typeid(T)] = std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T> T *get()
    {
        auto it = commands_.find(typeid(T));
        if (it != commands_.end()) {
            return static_cast<T *>(it->second.get());
        } else {
            return nullptr;
        }
    }

    std::unordered_map<std::type_index, std::unique_ptr<Command>> commands_;
};
} // namespace seop::command