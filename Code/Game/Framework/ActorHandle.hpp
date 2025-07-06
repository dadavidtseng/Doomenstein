//----------------------------------------------------------------------------------------------------
// ActorHandle.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------------
struct ActorHandle
{
    ActorHandle();
    ActorHandle(unsigned int uid, unsigned int index);

    static const ActorHandle INVALID;

    bool         IsValid() const;
    unsigned int GetIndex() const;
    bool         operator==(ActorHandle const& other) const;
    bool         operator!=(ActorHandle const& other) const;

private:
    unsigned int m_data;    // used to store 32-bit integers.
};
