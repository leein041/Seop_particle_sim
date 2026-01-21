#include "message.hpp"

namespace seop::msg
{
    


void Message_queue::Process()
{
    if (msg_que_.empty())
        return;
    Message* msg = msg_que_.front();
    msg_que_.pop();

    auto handlers = msg_handlers_.find(typeid(*msg));
    if (handlers != msg_handlers_.end()) {
        for (auto& handler : handlers->second) {
            handler(*msg); // excute
        }
    }
    delete(msg);
}
} // namespace seop::msg


