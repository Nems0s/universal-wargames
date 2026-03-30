#pragma once
#include <vector>
#include "batiment.hh"

class City {
    private:
        size_t _nbBatiments;
        bool _estCapitale;
        std::vector<std::unique_ptr<Batiment>> _batiments;

    public:
        City(int max, bool capitale=false) : _nbBatiments(max), _estCapitale(capitale) {}

        bool peutAjouterBatiment() const;
        void creeBatiment(std::unique_ptr<Batiment> b);
        bool estCapitale() const;
        void product(Joueur & j);
};
