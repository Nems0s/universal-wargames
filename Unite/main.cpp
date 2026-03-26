#include <iostream>
#include <memory>
#include <vector>
#include "unite_base.hh"
#include "unite_complexe.hh"
#include "rank.hh"

// Utilitaire pour l'affichage des titres de tests
void titre(const std::string& t) {
    std::cout << "\n--- [ " << t << " ] ---" << std::endl;
}

int main() {
    // 1. INITIALISATION DES POSITIONS ET GRADES
    Case origine = {0, 0};
    Case cible_proche = {1, 1};
    Case cible_loin = {10, 10};

    auto rankCmd = std::make_shared<Rank_Commandant>();
    auto rankReg = std::make_shared<Rank_Regulier>();

    // 2. TEST DES UNITES SIMPLES ET GRADES
    titre("TEST DES UNITES ET ROLES");
    auto soldat = std::make_shared<Unite_legere>("Infanterie Alpha", 100, 20, 3, direction::nord_est, origine);

    std::cout << "Unite: " << soldat->name() << " | Grade: ";
    rankReg->get_role(); // Affiche "Unite régulière"
    std::cout << std::endl;
    soldat->affiche();

    // 3. TEST DE LA LOGIQUE DE MOUVEMENT
    titre("TEST DU MOUVEMENT (Portee)");
    std::cout << "Mouvement vers (1,1)... ";
    soldat->movement(cible_proche); // Valide
    std::cout << "Position actuelle : (" << cible_proche.x << "," << cible_proche.y << ")" << std::endl;

    std::cout << "Mouvement vers (10,10) [Portee 3]... ";
    soldat->movement(cible_loin); // Invalide (la position ne doit pas changer)
    std::cout << "Position finale : (" << cible_proche.x << "," << cible_proche.y << ")" << std::endl;

    // 4. TEST DE L'AVANTAGE TACTIQUE (HEXAGONE)
    titre("TEST AVANTAGE ATTAQUE (Logique Hexagone)");
    // Le soldat est en (1,1) et regarde au Nord-Est.
    // L'attaquant arrive par le Sud-Ouest (derrière lui).
    Case pos_attaquant = {1, 0};
    bool avantage = avantage_attaque(pos_attaquant, cible_proche, direction::nord_est);

    std::cout << "Attaque sur " << soldat->name() << " (regarde Nord-Est) depuis (1,0)." << std::endl;
    std::cout << "Avantage d'attaque ? " << (avantage ? "OUI (Pris par surprise)" : "NON (Face-a-face)") << std::endl;

    // 5. TEST DES UNITES COMPLEXES (Héritage Multiple)
    titre("TEST UNITES COMPLEXES");
    auto yamato = std::make_shared<Unite_cuirrasse>("Yamato", 1000, 150, 2, direction::est, origine);
    auto apache = std::make_shared<Unite_gunship>("Apache AH-64", 250, 80, 7, direction::nord_ouest, origine);

    yamato->affiche();
    apache->affiche();

    // 6. TEST DU TRANSPORT
    titre("TEST DU TRANSPORT (Cargo)");
    auto cargo = std::make_shared<Unite_cargo>("C-130 Hercules", 300, 0, 8, direction::nord_est, origine);

    std::cout << "Embarquement de l'infanterie dans le cargo..." << std::endl;
    cargo->ajout_unite(soldat);
    cargo->affiche(); // Affiche la liste des unités transportées

    // 7. TEST DU COMMANDANT (Gestion de liste globale)
    titre("TEST DU RANK COMMANDANT");
    rankCmd->ajout_unite(yamato);
    rankCmd->ajout_unite(apache);
    rankCmd->ajout_unite(cargo);

    std::cout << "Nombre d'unites sous le commandement : " << rankCmd->liste_unites().size() << std::endl;

    std::cout << "Suppression du Yamato de la liste..." << std::endl;
    rankCmd->supprimer_unite(yamato);
    std::cout << "Nombre d'unites restant : " << rankCmd->liste_unites().size() << std::endl;

    for(auto const& u : rankCmd->liste_unites()) {
        std::cout << " -> " << u->name() << " est sous mes ordres." << std::endl;
    }

    std::cout << "\n--- FIN DES TESTS ---" << std::endl;
    return 0;
}
