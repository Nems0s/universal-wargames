#pragma once
#include <string>
#include <map>
#include <fstream>
#include <memory>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Ressource {
    private:
        std::string _name;
        std::string _symbole;
        std::string _iconPath;
    public:
        Ressource(std::string n, std::string s, std::string i);

        const std::string & getName() const { return _name; }
        const std::string & getSymbole() const { return _symbole; }
        const std::string & getIconPath() const { return _iconPath; }
};

class RessourceConfigReader {
public:
    virtual ~RessourceConfigReader() = default;
    virtual void load(const std::string & chemin, std::map<std::string, std::unique_ptr<Ressource>> & catalogue) = 0;
};


class TxtRessourceReader : public RessourceConfigReader {
public:
    void load(const std::string& chemin, std::map<std::string, std::unique_ptr<Ressource>>& catalogue) override;
};

class JsonRessourceReader : public RessourceConfigReader {
public:
    void load(const std::string& chemin, std::map<std::string, std::unique_ptr<Ressource>>& catalogue) override;
};

class RessourceFactory {
private:
    std::map<std::string, std::unique_ptr<Ressource>> _catalogue;

public:
    void chargerConfiguration(const std::string & chemin, RessourceConfigReader & lecteur);
    const Ressource* getRessource(const std::string& id) const;
    const std::map<std::string, std::unique_ptr<Ressource>>& getCatalogue() const { return _catalogue; }
    std::map<std::string, const Ressource*> getCataloguePointeurs() const;
};