#include <iostream>
#include <memory>
#include "unite.hh"
#include "comportement.hh"
#include "orientation.hh"
#include "rank.hh"

int main() {
    // 1. Création des Grades
    auto gradeCmd = std::make_shared<Rank_Commandant>();
    auto gradeReg = std::make_shared<Rank_Regulier>();

    // 2. Création des Unités
    // Une unité de cavalerie (Terrestre) à la position (2,2)
    Unite cavalier("Chevalier Noir", 100, 20, Poids::Leger, direction::est, {2, 2}, gradeReg);

    // Un Griffon (Volant) à la position (3,3)
    Unite griffon("Griffon Royal", 80, 15, Poids::Moyen, direction::nord_ouest, {3, 3}, gradeReg);

    // Un Commandant (Terrestre) à la position (2,1)
    auto cmd = std::make_shared<Unite>("General Maximus", 150, 25, Poids::Lourd, direction::sud_est, Case{2, 1}, gradeCmd);

    // 3. Ajout des comportements
    // Le cavalier peut bouger sur terre et attaquer au corps à corps
    cavalier.ajouterComportement(std::make_shared<CompMouvTerrestre>(3)); // Vitesse 3
    cavalier.ajouterComportement(std::make_shared<CompAttMelee>(20));

    // Le griffon vole et attaque à distance
    griffon.ajouterComportement(std::make_shared<CompMouvVolant>(4)); // Vitesse 4
    griffon.ajouterComportement(std::make_shared<CompAttDistance>(15, 2, 5)); // Dégâts 15, Portée 2, 5 Munitions

    // 4. Test du système de Grade (Commandant)
    gradeCmd->ajout_unite(std::make_shared<Unite>(cavalier));
    gradeCmd->ajout_unite(std::make_shared<Unite>(griffon));

    std::cout << "--- Test des Grades ---" << std::endl;
    std::cout << "Le grade du General est : "; cmd->rank()->get_role(); std::cout << std::endl;
    std::cout << "Le General commande " << gradeCmd->liste_unites().size() << " unites." << std::endl;

    // 5. Test de la Mobilite (Polymorphisme & Dynamic Cast)
    std::cout << "\n--- Test de Mobilite ---" << std::endl;
    auto movGriffon = griffon.Mobilite();
    for(auto* m : movGriffon) {
        std::cout << "Le griffon a un mouvement de nature : " << (int)m->Nature() << " (2 = AIR)" << std::endl;
    }

    // 6. Test de l'Attaque (Portee et Hexagones)
    std::cout << "\n--- Test d'Attaque Distance (Griffon -> Cavalier) ---" << std::endl;

    // On récupère le comportement d'attaque à distance du griffon
    for(auto const& comp : griffon.liste_comportements()) {
        if(auto attDist = std::dynamic_pointer_cast<CompAttDistance>(comp)) {
            bool possible = attDist->PeuxAttaquer(griffon, cavalier);
            std::cout << "Griffon en {3,3} peut attaquer Cavalier en {2,2} ? "
                      << (possible ? "OUI" : "NON") << std::endl;
        }
    }

    // 7. Test des voisins et rayons (Logic Hexagonale)
    std::cout << "\n--- Test Logique Hexagonale ---" << std::endl;
    Case centre = {2, 2};
    int rayon = 1;
    auto zone = case_adjascentes(centre, rayon);

    std::cout << "Cases adjacentes a {2,2} (Rayon 1) : " << std::endl;
    for(const auto& c : zone) {
        std::cout << "  (" << c.x << "," << c.y << ")" << std::endl;
    }

    // 8. Test de l'avantage d'attaque (Orientation)
    std::cout << "\n--- Test Orientation (Backstab) ---" << std::endl;
    // Si le cavalier regarde à l'Est {2,2} et que l'attaquant est à l'Ouest {1,2}
    Case attaquantPos = {1, 2};
    bool avantage = avantage_attaque(attaquantPos, cavalier.location(), cavalier.regarde());
    std::cout << "Attaque par l'arriere ({1,2} sur {2,2} regarde EST) ? "
              << (avantage ? "OUI (Avantage)" : "NON (Face a face)") << std::endl;

    std::cout << "\n--- Affichage Global ---" << std::endl;
    cmd->affiche();
    cavalier.affiche();

    return 0;
}
