//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_AGENT_H
#define RLEXPERIMENTS_AGENT_H

namespace rl {
template <typename AgentId, typename Action, typename State>
class Agent
{
public:
    Agent() = delete;
    Agent(const AgentId& anAgentId) : myId(anAgentId) {}

    const AgentId& GetAgentId() const { return myId; }
    virtual Action GetNextAction(const State& aCurrentState) = 0;

protected:
    AgentId myId;
};
}

#endif //RLEXPERIMENTS_AGENT_H
