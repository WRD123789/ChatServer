#ifndef __USER__
#define __USER__

#include <string>

class User {
public:
    User(int id = -1, std::string name = "", std::string password = "", 
        std::string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = password;
        this->state = state;
    }

    inline void setId(int id) { this->id = id; }
    inline void setName(std::string name) { this->name = name; }
    inline void setPassword(std::string password) { this->password = password; }
    inline void setState(std::string state) { this->state = state; }

    inline int getId() { return this->id; }
    inline std::string getName() { return this->name; }
    inline std::string getPassword() { return this->password; }
    inline std::string getState() { return this->state; }

protected:
    int id;
    std::string name;
    std::string password;
    std::string state;
};

#endif