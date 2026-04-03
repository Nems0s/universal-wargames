#pragma once
#include <vector>
#include "batiment.hh"

class City {
    private:
        int _level;
        size_t _nbBatiments;
        bool _estCapitale;
        std::vector<std::unique_ptr<Batiment>> _batiments;

    public:
        City(int max=5, bool capitale=false, int level=1);
        //level à la fin car comme ça pas besoin de le mettre lors de la création

        bool peutAjouterBatiment() const;
        void creeBatiment(std::unique_ptr<Batiment> b);
        bool estCapitale() const;
        void product(Joueur & j);
};
