#include <iostream>
#include <map>

class Ressource;

class Joueur {
private:
    std::string _name;
    std::map<Ressource*, int> _inventaire;

public:
    void ajouterRessource(Ressource* r, int n);

    bool consommerRessource(Ressource* r, int n);

    bool peutPayer(const std::map<Ressource*, int>& cout) const;

    void payer(const std::map<Ressource*, int>& cout);
};