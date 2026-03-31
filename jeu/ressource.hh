#pragma once
#include <iostream>
#include <map>
#include <fstream>

class Ressource {
    private:
        std::string _name;
        std::string _symbole;
    
    public:
        Ressource(std::string n, std::string s) : _name(n), _symbole(s) {}
        std::string getName() const { return _name; }
        std::string getSymbole() const { return _symbole; }

};


class RessourceConfigReader {
public:
    virtual ~RessourceConfigReader() = default;
    virtual void load(const std::string& chemin, std::map<std::string, Ressource*>& catalogue) = 0;
};


class TxtRessourceReader : public RessourceConfigReader {
public:
    void load(const std::string& chemin, std::map<std::string, Ressource*>& catalogue);
};

