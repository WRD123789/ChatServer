#ifndef __GROUP__
#define __GROUP__

#include "groupuser.hpp"

#include <string>
#include <vector>

class Group {
public:
    Group(int id = -1, std::string name = "", std::string desc = "")
    {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }

    inline void setId(int id) { this->id = id; }
    inline void setName(std::string name) { this->name = name; }
    inline void setDesc(std::string desc) { this->desc = desc; }

    inline int getId() { return this->id; }
    inline std::string getName() { return this->name; }
    inline std::string getDesc() { return this->desc; }
    inline std::vector<GroupUser>& getUsers() { return this->users; }

private:
    int id;
    std::string name;
    std::string desc;
    std::vector<GroupUser> users;
};

#endif