#include <iostream>
#include <memory>
#include <cassert>
#include "unite.hh"
#include "comportement.hh"
#include "rank.hh"
#include "orientation.hh"

int main() {
    std::cout << "========== DEBUT DES TESTS DU WARGAME ==========" << std::endl << std::endl;

    // 1. INITIALISATION DES RANGS
    auto cmd = std::make_shared<Rank_Commandant>();
    auto reg = std::make_shared<Rank_Regulier>();

    // 2. CRÉATION DES UNITÉS ET COMPOSANTS
    // --- Le Croiseur (Marin + Défense + Transport) ---
    auto croiseur = std::make_shared<Unite>("Croiseur HMS", 500, 50, Poids::Lourd, direction::est, Case{0, 0}, cmd);
    croiseur->ajouterComportement(std::make_shared<CompMouvMarin>(3));
    croiseur->ajouterComportement(std::make_shared<CompDefArmure>(10)); // Réduit de 10
    auto transport = std::make_shared<CompTransport>(2);
    croiseur->ajouterComportement(transport);

    // --- Le Chasseur (Volant + Attaque Distance) ---
    auto chasseur = std::make_shared<Unite>("Rafale", 150, 40, Poids::Leger, direction::nord_est, Case{2, 2}, reg);
    chasseur->ajouterComportement(std::make_shared<CompMouvVolant>(5));
    chasseur->ajouterComportement(std::make_shared<CompAttDistance>(30, 4, 10, 2)); // Dégâts 30, Portée 4, 10 Mun, Min 2

    // --- L'Infecteur (Terrestre + Attaque Indirecte) ---
    auto maraudeur = std::make_shared<Unite>("Maraudeur Toxique", 120, 15, Poids::Moyen, direction::ouest, Case{1, 0}, reg);
    maraudeur->ajouterComportement(std::make_shared<CompMouvTerrestre>(2));
    auto poison = std::make_shared<CompAttIndirect>(15, 2, 3); // 15 dmg, portée 2, 3 tours
    maraudeur->ajouterComportement(poison);

    // --- La Cible (Terrestre + Bouclier) ---
    auto cible = std::make_shared<Unite>("Tank Test", 200, 20, Poids::Lourd, direction::sud_est, Case{1, 1}, reg);
    cible->ajouterComportement(std::make_shared<CompMouvTerrestre>(1));
    cible->ajouterComportement(std::make_shared<CompDefBouclier>(2)); // 2 charges d'annulation

    // ---------------------------------------------------------
    // 3. TESTS DE LA GÉOMÉTRIE (orientation.cc)
    // ---------------------------------------------------------
    std::cout << "[TEST GRILLE] Verification des voisins de (1,1)..." << std::endl;
    auto v = Voisins(Case{1,1});
    std::cout << "Nombre de voisins trouves : " << v.size() << " (Attendu: 6)" << std::endl;

    std::cout << "[TEST GRILLE] Verification portee rayon 2 autour de (0,0)..." << std::endl;
    auto portee2 = case_adjascentes(Case{0,0}, 2);
    std::cout << "Cases dans le rayon 2 : " << portee2.size() << std::endl;

    // ---------------------------------------------------------
    // 4. TESTS DE MOUVEMENT (comportement.cc)
    // ---------------------------------------------------------
    std::cout << std::endl << "--- TESTS MOUVEMENT ---" << std::endl;
    // Le croiseur (Marin) tente d'aller en (2,2)
    auto mouvMarin = croiseur->Mobilite().front();
    if (mouvMarin->EstCaseValide(croiseur->location(), Case{2,2})) {
        std::cout << "Mouvement Marin (0,0 -> 2,2) : VALIDE" << std::endl;
    } else {
        std::cout << "Mouvement Marin (0,0 -> 2,2) : REFUSE (Trop loin ou mauvais terrain)" << std::endl;
    }

    // ---------------------------------------------------------
    // 5. TESTS DE COMBAT (comportement.cc)
    // ---------------------------------------------------------
    std::cout << std::endl << "--- TESTS COMBAT ---" << std::endl;

    // Test Attaque Distance (Chasseur (2,2) vs Cible (1,1))
    // Distance (2,2) à (1,1) est de 1. L'attaque distance a une portee_mini de 2.
    auto compDist = std::dynamic_pointer_cast<CompAttDistance>(chasseur->liste_comportements().back());
    if (compDist && compDist->PeuxAttaquer(*chasseur, *cible)) {
        std::cout << "Chasseur attaque Cible : OUI" << std::endl;
    } else {
        std::cout << "Chasseur attaque Cible : NON (Trop proche ! Zone morte de portee_mini=2)" << std::endl;
    }

    // Test Attaque Indirecte (Maraudeur (1,0) vs Cible (1,1))
    if (poison->PeuxAttaquer(*maraudeur, *cible)) {
        std::cout << "Maraudeur peut infecter Cible : OUI" << std::endl;
        poison->AjoutCibleAtteinte(cible);
        std::cout << "Infection appliquee." << std::endl;
    }

    // ---------------------------------------------------------
    // 6. TESTS DE DÉFENSE (comportement.cc)
    // ---------------------------------------------------------
    std::cout << std::endl << "--- TESTS DEFENSE ---" << std::endl;
    int degats_initiaux = 50;

    // Test Bouclier sur la cible
    auto it_comp = cible->liste_comportements();
    for(auto const& c : it_comp) {
        if(auto def = std::dynamic_pointer_cast<CompDef>(c)) {
            int restants = def->ReductionDegats(degats_initiaux);
            std::cout << "Degats apres passage dans " << typeid(*def).name() << " : " << restants << std::endl;
            degats_initiaux = restants;
        }
    }

    // ---------------------------------------------------------
    // 7. TESTS DE TRANSPORT (comportement.cc)
    // ---------------------------------------------------------
    std::cout << std::endl << "--- TESTS TRANSPORT ---" << std::endl;
    // Maraudeur en (1,0) est voisin du Croiseur en (0,0) ? Oui selon orientation.cc
    if (transport->MonterUnite(*croiseur, maraudeur)) {
        std::cout << "[OK] Maraudeur Toxique a embarque sur le Croiseur." << std::endl;
    } else {
        std::cout << "[ECHEC] Trop loin pour embarquer." << std::endl;
    }

    // ---------------------------------------------------------
    // 8. TESTS D'AFFICHAGE ET UPDATE
    // ---------------------------------------------------------
    std::cout << std::endl << "--- AFFICHAGE FINAL DE L'ETAT ---" << std::endl;
    croiseur->affiche();
    chasseur->affiche();
    maraudeur->affiche();
    cible->affiche();

    std::cout << std::endl << "[UPDATE] Passage au tour suivant..." << std::endl;
    maraudeur->update(); // Devrait afficher l'état de l'infection
    cible->update();    // Devrait afficher l'état du bouclier

    std::cout << std::endl << "========== FIN DES TESTS ==========" << std::endl;

    return 0;
}
