//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_AGENT_H
#define RLEXPERIMENTS_AGENT_H

#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>

namespace RL
{
template <typename AgentId, typename Action, typename State>
class Agent
{
public:
    Agent() = delete;
    Agent(const AgentId& anAgentId) : myId(anAgentId) {}
    virtual ~Agent() {}

    const AgentId& GetAgentId() const { return myId; }
    virtual Action GetNextAction(const State& aCurrentState) = 0;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(myId));
    }

protected:
    AgentId myId;
};
}

#endif //RLEXPERIMENTS_AGENT_H
