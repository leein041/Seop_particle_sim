#pragma once
#include <functional>
#include <memory>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace seop::msg
{
class Message;
using Msg_handler = std::function<bool(Message& msg)>;
using Msg_handler_list = std::vector<Msg_handler>;

class Message
{
  public:
    virtual ~Message() = default;

    bool hanlded_{false};
};

class Message_dispatcher
{
  public:
    Message_dispatcher(Message& msg) : msg_(msg)
    {
    }

    template <typename T, typename F>
    bool Dispatch(const F& func)
    {
        if (typeid(msg_) == typeid(T)) {
            msg_.hanlded_ = func(static_cast<T&>(msg_));
            return true;
        } else {
            return false;
        }
    }

  private:
    Message& msg_;
};

class Message_queue
{
  public:
    template <typename T>
    void Register_handler(const std::function<bool(T& msg)>& handler)
    {
        msg_handlers_[typeid(T)].push_back([handler](Message& msg) -> bool { return handler(static_cast<T&>(msg)); });
    }

    void Push_messag(Message* msg);
    void Process();

  private:
    std::queue<Message*>                                  msg_que_ = {};
    std::unordered_map<std::type_index, Msg_handler_list> msg_handlers_;
};
} // namespace seop::msg