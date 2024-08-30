#ifndef __GROUP_USER__
#define __GROUP_USER__

#include "user.hpp"

class GroupUser : public User {
public:
    inline void setRole(std::string role) { this->role = role; }
    inline std::string getRole() { return this->role; }

private:
    std::string role;
};

#endif