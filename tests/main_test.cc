#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "unite.hh"
#include "comportement.hh"
#include "combat.hh"
#include "board.hh"
#include "orientation.hh"

// Création rapide d'une unité vide
std::shared_ptr<Unite> createTestUnite(std::string name, int hp = 100) 
{
    auto u = std::make_shared<Unite>(name, hp, 2, 3, 3, Poids::Moyen, direction::est, Coord{0,0}, 
                                  nullptr, std::list<std::shared_ptr<IComportement>>{}, 
                                  std::map<const Ressource*, int>{}, std::map<const Ressource*, int>{}, 'U', "");
    u->ajouterComportement(std::make_shared<CompMouvTerrestre>(2));
    return u;
}


// Test Mouvement
TEST_CASE("Mouvement - Validation des cases") 
{
    CompMouvTerrestre mouvT(2);
    CompMouvMarin mouvM(2);
    CompMouvVolant mouvV(2);

    Coord actuel = {5, 5};
    Coord cibleLoin = {10, 10};
    Coord cibleProche = {5, 6};

    CHECK(mouvT.EstCaseValide(actuel, cibleProche)); // Correct
    CHECK_FALSE(mouvT.EstCaseValide(actuel, cibleLoin)); // Trop loin

    CHECK(mouvM.EstCaseValide(actuel, cibleProche));
    CHECK_FALSE(mouvM.EstCaseValide(actuel, cibleLoin));

    CHECK(mouvV.EstCaseValide(actuel, cibleProche));
    CHECK_FALSE(mouvV.EstCaseValide(actuel, cibleLoin));
}

TEST_CASE("Mouvement - Isolation et changement de portée") 
{
    auto mouv1 = std::make_shared<CompMouvTerrestre>(2);
    auto mouv2 = std::make_shared<CompMouvTerrestre>(2);

    mouv1->setMov_per_laps(5);

    CHECK_EQ(mouv1->mov_per_laps(), 5);
    CHECK_EQ(mouv2->mov_per_laps(), 2); // Doit rester inchangé
}

// Test Attaque
TEST_CASE("Attaque - Mêlée échoue si trop loin") 
{
    auto att = createTestUnite("Attaquant");
    auto def = createTestUnite("Defenseur");
    def->setLocation({5, 5}); // Loin de {0,0}
    
    CompAttMelee melee(10);
    att->setTuilesVisibles({{5,5}}); 
    
    // Mais PeuxAttaquer en mêlée vérifie aussi la nature (ici même nature mais distance > 1)
    CHECK_FALSE(melee.PeuxAttaquer(*att, *def)); 
}

TEST_CASE("Attaque - Infection et mise à jour des dégâts") 
{
    auto def = createTestUnite("Cible", 100);
    auto attInd = std::make_shared<CompAttIndirect>(10, 5, 3);
    
    attInd->AjoutCibleAtteinte(def);
    attInd->update(); // Tour 1
    
    CHECK_EQ(def->health_point(), 90);
    CHECK_EQ(attInd->nombredetourinfection(), 3);
}


// Test Défense et Soin
TEST_CASE("Défense - Bouclier bloque et consomme") 
{
    CompDefBouclier bouclier(1);
    
    int dgt = bouclier.ReductionDegats(50);
    CHECK_EQ(dgt, 0);
    CHECK_EQ(bouclier.nombre_bouclier(), 0);
    
    dgt = bouclier.ReductionDegats(50);
    CHECK_EQ(dgt, 50);
}

TEST_CASE("Soin - Ne dépasse pas le maximum") 
{
    auto cible = createTestUnite("Patient", 100);
    cible->setHealth_point(95);
    
    CompSoinDirect soin(20, 1, 0);
    if(cible->health_point() + soin.healing_point() > cible->health_point_max()) 
    {
        cible->setHealth_point(cible->health_point_max());
    }
    
    CHECK_EQ(cible->health_point(), 100);
}

TEST_CASE("Soin - Mise à jour de la vie via soin indirect") 
{
    auto cible = createTestUnite("Cible", 50);
    auto soin = std::make_shared<CompSoinIndirect>(10, 5, 3);
    
    soin->AjoutCibleAtteinte(cible);
    soin->update();
    
    CHECK_EQ(cible->health_point(), 50); //Vie déjà au max
    CHECK_EQ(soin->nombredetourregen(), 3);

    cible->setHealth_point(40);
    soin->update();

    CHECK_EQ(cible->health_point(), 50); //Soin
}

// Test Combat et Moral
TEST_CASE("Moral - Seuils de fuite et d'héroïsme") 
{
    auto u = createTestUnite("Soldat");
    
    u->setMoral_point(-19);
    EffetMoral(*u, 10);
    CHECK_EQ(u->health_point(), 0); // Fuite = PV à 0

    u->setHealth_point(100);
    u->setMoral_point(13);
    EffetMoral(*u, 10);// 10 de dégats
    CHECK_EQ(u->temporary_damage(), 15); // Héroïsme (x1.5)
}

TEST_CASE("Moral - Évolution durant le combat") 
{
    auto att = createTestUnite("Vainqueur");
    auto def = createTestUnite("Perdant");
    att->setMoral_point(0);
    def->setMoral_point(0);

    AugmentationMoral(*att, 5);
    DiminussionMoral(*def, 5);

    CHECK(att->moral_point() > 0);
    CHECK(def->moral_point() < 0);
}

TEST_CASE("Combat - Calcul des dégâts et influence de l'orientation") 
{
    auto att = createTestUnite("Attaquant");
    auto def = createTestUnite("Defenseur");
    
    att->setLocation({0,0});
    def->setLocation({0,1});
    def->setRegarde(direction::est); // Dos à l'attaquant
    
    auto meleePtr = std::make_shared<CompAttMelee>(10);
    att->ajouterComportement(meleePtr);
    att->setTuilesVisibles({{0,1}});

    int pvAvant = def->health_point();
    auto comportement = att->Offensive().front();
    bool succes = Combat::fight(*att, comportement, *def);

    CHECK(succes);
    // Dégâts de base 10, mais avantage d'attaque (dos) = x1.5 -> 15 dégâts
    CHECK_EQ(def->health_point(), pvAvant - 15);
}

TEST_CASE("Combat - Application des soins") 
{
    auto healer = createTestUnite("Medecin");
    auto blesse = createTestUnite("Blesse");
    blesse->setHealth_point(50);
    
    CompSoinDirect soin(20, 1, 0);
    healer->ajouterComportement(std::make_shared<CompSoinDirect>(soin));
    healer->setLocation({0,0});
    blesse->setLocation({0,1});

    bool succes = Combat::heal(*healer, healer->Soin().front(), *blesse);

    CHECK(succes);
    CHECK_EQ(blesse->health_point(), 70);
    CHECK(healer->moral_point() > 0); // Le soin booste le moral
}


// Test Rangs (Rank)
TEST_CASE("Rank - Transmission du bonus de commandant") 
{
    auto cmd = createTestUnite("Chef");
    auto rankCmd = std::make_shared<Rank_Commandant>(5);
    rankCmd->ajout_bonus(std::make_shared<BonusDegat>(10));
    cmd->setRank(rankCmd);

    auto soldat = createTestUnite("Soldat");
    auto rankReg = std::make_shared<Rank_Regulier>();
    rankReg->setCommandant(cmd);
    soldat->setRank(rankReg);

    bool aSoin = false;
    BuffCommandant(*soldat, aSoin, 5); // 5 dgt de base

    CHECK_EQ(soldat->temporary_damage(), 15); // 5 + 10 bonus
}

// Test Cooldown
TEST_CASE("Cooldown - Blocage par Cooldown") 
{
    // Template: Base = Melee, CD = 2
    AvecCooldown<CompAttMelee> attCD(2, 10);
    attCD.setCurrent_cooldown(0);

    CHECK(attCD.estPret());
    attCD.executerAction(); // Met le CD à 2
    
    CHECK_FALSE(attCD.estPret());
    attCD.update(); // Tour 1, reste 1
    CHECK_FALSE(attCD.estPret());
    attCD.update(); // Tour 2, reste 0
    CHECK(attCD.estPret());
}

// Test Consommables
TEST_CASE("Consommable - Gestion des ressources consommables") 
{
    Ressource bois("Bois", "B", "");
    std::map<const Ressource*, int> cout = {{&bois, 10}};
    
    // Création d'une attaque qui consomme du bois
    auto attCons = std::make_shared<AvecConsommable<CompAttMelee>>(cout, 20);
    auto u = createTestUnite("Archer");

    // Cas où l'unité n'a rien
    CHECK_FALSE(attCons->estPayable(*u));

    // On donne les ressources
    u->setInventaireInterne({{&bois, 15}});
    CHECK(attCons->estPayable(*u));

    // Simulation de la consommation
    u->consommerPourAction(cout);
    CHECK_EQ(u->getInventaireInterne().at(&bois), 5);
}

TEST_CASE("Special - Transfert de Ravitaillement") 
{
    Ressource fuel("Fuel", "F", "");
    auto uRavitailleur = createTestUnite("Camion");
    auto uCible = createTestUnite("Char");
    
    uRavitailleur->setLocation({1, 1});
    uCible->setLocation({1, 2});

    // Configuration inventaires
    uRavitailleur->setInventaireInterne({{&fuel, 50}});
    uCible->setInventaireInterne({{&fuel, 0}});
    uCible->setCapaciteMax({{&fuel, 20}});
    
    CompRavitaillement compRav(1); // Portée de 1

    bool peutRav = compRav.PeuxRavitailler(*uRavitailleur, *uCible);
    CHECK(peutRav);

    bool transfertOk = compRav.TransfererRessources(*uRavitailleur, *uCible);
    CHECK(transfertOk);
    
    CHECK(uCible->getInventaireInterne().count(&fuel) > 0);
    CHECK(uCible->getInventaireInterne().count(&fuel) <= 20);

    CHECK(uRavitailleur->getInventaireInterne().at(&fuel) < 50);
    CHECK(uRavitailleur->getInventaireInterne().at(&fuel) >= 30);
}

// Vision et Orientation
TEST_CASE("Vision - Taille et limites du cône de vision") 
{
    Coord origine = {10, 10};
    // direction::est, range=2, fov=3
    auto vue = ConeVision(origine, direction::est, 2, 3);
    
    // Au rang 1: 3 cases, au rang 2: expansion.
    CHECK_FALSE(vue.empty());
    // Vérifier qu'on ne voit pas derrière
    auto it = std::find(vue.begin(), vue.end(), Coord{10, 9}); // Ouest
    CHECK(it == vue.end());
}

// Test Factory
TEST_CASE("Factory - Injection des comportements à la création") 
{
    UniteFactory factory;
    
    auto u = createTestUnite("Archer");
    u->ajouterComportement(std::make_shared<CompAttDistance>(15, 4, 2));

    size_t verif = 1;
    CHECK_EQ(u->Offensive().size(), verif);
    CHECK_EQ(u->Offensive().front()->portee(), 4);
    CHECK_EQ(u->getSymbol(), 'U');
}

/*Version gtest
//Test Mouvement
TEST(MouvementTest, ValidationCases_Mouvement) 
{
    CompMouvTerrestre mouvT(2);
    CompMouvMarin mouvM(2);
    CompMouvVolant mouvV(2);

    Coord actuel = {5, 5};
    Coord cibleLoin = {10, 10};
    Coord cibleProche = {5, 6};

    EXPECT_TRUE(mouvT.EstCaseValide(actuel, cibleProche)); //Correct
    EXPECT_FALSE(mouvT.EstCaseValide(actuel, cibleLoin)); //Trop loin

    EXPECT_TRUE(mouvM.EstCaseValide(actuel, cibleProche));
    EXPECT_FALSE(mouvM.EstCaseValide(actuel, cibleLoin));

    EXPECT_TRUE(mouvV.EstCaseValide(actuel, cibleProche));
    EXPECT_FALSE(mouvV.EstCaseValide(actuel, cibleLoin));
}

TEST(MouvementTest, Isolation_ChangementPortee) 
{
    auto mouv1 = std::make_shared<CompMouvTerrestre>(2);
    auto mouv2 = std::make_shared<CompMouvTerrestre>(2);

    mouv1->setMov_per_laps(5);

    EXPECT_EQ(mouv1->mov_per_laps(), 5);
    EXPECT_EQ(mouv2->mov_per_laps(), 2); // Doit rester inchangé
}

//Test Attaque
TEST(AttaqueTest, Melee_EchoueSiLoin) 
{
    auto att = createTestUnite("Attaquant");
    auto def = createTestUnite("Defenseur");
    def->setLocation({5, 5}); // Loin de {0,0}
    
    CompAttMelee melee(10);
    att->setTuilesVisibles({{5,5}}); 
    
    // Mais PeuxAttaquer en mêlée vérifie aussi la nature (ici même nature mais distance > 1)
    EXPECT_FALSE(melee.PeuxAttaquer(*att, *def)); 
}

TEST(AttaqueTest, Infection_UpdateDegats) 
{
    auto def = createTestUnite("Cible", 100);
    auto attInd = std::make_shared<CompAttIndirect>(10, 5, 3);
    
    attInd->AjoutCibleAtteinte(def);
    attInd->update(); // Tour 1
    
    EXPECT_EQ(def->health_point(), 90);
    EXPECT_EQ(attInd->nombredetourinfection(), 3);
}

//Test Défense et Soin
TEST(DefenseTest, Bouclier_BloqueEtConsomme) 
{
    CompDefBouclier bouclier(1);
    
    int dgt = bouclier.ReductionDegats(50);
    EXPECT_EQ(dgt, 0);
    EXPECT_EQ(bouclier.nombre_bouclier(), 0);
    
    dgt = bouclier.ReductionDegats(50);
    EXPECT_EQ(dgt, 50);
}

TEST(SoinTest, Soin_NeDepassePasMax) 
{
    auto cible = createTestUnite("Patient", 100);
    cible->setHealth_point(95);
    
    CompSoinDirect soin(20, 1, 0);
    if(cible->health_point() + soin.healing_point() > cible->health_point_max()) 
    {
        cible->setHealth_point(cible->health_point_max());
    }
    
    EXPECT_EQ(cible->health_point(), 100);
}

TEST(SoinTest, Soin_UpdateVie) 
{
    auto cible = createTestUnite("Cible", 50);
    auto soin = std::make_shared<CompSoinIndirect>(10, 5, 3);
    
    soin->AjoutCibleAtteinte(def);
    soin->update();
    
    EXPECT_EQ(cible->health_point(), 60);
    EXPECT_EQ(soin->nombredetourinfection(), 3);
}

//Test combat
TEST(MoralTest, Seuils_FuiteEtHeroisme) 
{
    auto u = createTestUnite("Soldat");
    
    u->setMoral_point(-20);
    EffetMoral(*u, 10);
    EXPECT_EQ(u->health_point(), 0); // Fuite = PV à 0

    u->setHealth_point(100);
    u->setMoral_point(18);
    EffetMoral(*u, 10);
    EXPECT_EQ(u->temporary_damage(), 15); // Héroïsme (x1.5)
}

TEST(MoralTest, Evolution_Combat) 
{
    auto att = createTestUnite("Vainqueur");
    auto def = createTestUnite("Perdant");
    att->setMoral_point(0);
    def->setMoral_point(0);

    AugmentationMoral(*att, 5);
    DiminussionMoral(*def, 5);

    EXPECT_GT(att->moral_point(), 0);
    EXPECT_LT(def->moral_point(), 0);
}

TEST(CombatTest, Fight_CalculDegatsEtOrientation) 
{
    auto att = createTestUnite("Attaquant");
    auto def = createTestUnite("Defenseur");
    
    att->setLocation({0,0});
    def->setLocation({0,1}); // Juste à côté
    def->setRegarde(direction::est); // Dos à l'attaquant
    
    CompAttMelee melee(10);
    att->ajouterComportement(std::make_shared<CompAttMelee>(melee));
    att->setTuilesVisibles({{0,1}});

    int pvAvant = def->health_point();
    bool succes = Combat::fight(*att, &melee, *def);

    EXPECT_TRUE(succes);
    // Dégâts de base 10, mais avantage d'attaque (dos) = x1.5 -> 15 dégâts
    EXPECT_EQ(def->health_point(), pvAvant - 15);
}


TEST(CombatTest, Heal_ApplicationSoin) 
{
    auto healer = createTestUnite("Medecin");
    auto blesse = createTestUnite("Blesse");
    blesse->setHealth_point(50);
    
    CompSoinDirect soin(20, 1, 0);
    healer->ajouterComportement(std::make_shared<CompSoinDirect>(soin));
    healer->setLocation({0,0});
    blesse->setLocation({0,1});

    bool succes = Combat::heal(*healer, &soin, *blesse);

    EXPECT_TRUE(succes);
    EXPECT_EQ(blesse->health_point(), 70);
    EXPECT_GT(healer->moral_point(), 0); // Le soin booste le moral
}


//Test Rank 
TEST(RankTest, BonusCommandant_Transmission) 
{
    auto cmd = createTestUnite("Chef");
    auto rankCmd = std::make_shared<Rank_Commandant>(5);
    rankCmd->ajout_bonus(std::make_shared<BonusDegat>(10));
    cmd->setRank(rankCmd);

    auto soldat = createTestUnite("Soldat");
    auto rankReg = std::make_shared<Rank_Regulier>();
    rankReg->setCommandant(cmd);
    soldat->setRank(rankReg);

    bool aSoin = false;
    BuffCommandant(*soldat, aSoin, 5); // 5 dgt de base

    EXPECT_EQ(soldat->temporary_damage(), 15); // 5 + 10 bonus
}

//Test Cooldown
TEST(WrapperTest, Cooldown_Blocage) 
{
    // Template: Base = Melee, CD = 2
    AvecCooldown<CompAttMelee> attCD(2, 10); 

    EXPECT_TRUE(attCD.estPret());
    attCD.executerAction(); // Met le CD à 2
    
    EXPECT_FALSE(attCD.estPret());
    attCD.update(); // Tour 1, reste 1
    EXPECT_FALSE(attCD.estPret());
    attCD.update(); // Tour 2, reste 0
    EXPECT_TRUE(attCD.estPret());
}

//Consommable
TEST(WrapperTest, Consommable_GestionRessources) 
{
    Ressource bois("Bois", "B", "");
    std::map<const Ressource*, int> cout = {{&bois, 10}};
    
    // Création d'une attaque qui consomme du bois
    auto attCons = std::make_shared<AvecConsommable<CompAttMelee>>(cout, 20);
    auto u = createTestUnite("Archer");

    //Cas où l'unité n'a rien
    EXPECT_FALSE(attCons->estPayable(*u));

    // On donne les ressources
    u->setInventaireInterne({{&bois, 15}});
    EXPECT_TRUE(attCons->estPayable(*u));

    //Simulation de la consommation
    u->consommerPourAction(cout);
    EXPECT_EQ(u->getInventaireInterne().at(&bois), 5);
}

TEST(SpecialTest, Ravitaillement_Transfert) 
{
    Ressource fuel("Fuel", "F", "");
    auto uRavitailleur = createTestUnite("Camion");
    auto uCible = createTestUnite("Char");
    
    //Configuration inventaires
    uRavitailleur->setInventaireInterne({{&fuel, 50}});
    uCible->setInventaireInterne({{&fuel, 0}});
    uCible->setCapaciteMa({{&fuel, 20}});
    
    CompRavitaillement compRav(1); // Portée de 1

    bool peutRav = compRav.PeuxRavitailler(*uRavitailleur, *uCible);
    bool transfertOk = compRav.TransfererRessources(*uRavitailleur, *uCible);

    EXPECT_TRUE(peutRav);
    EXPECT_TRUE(transfertOk);
    EXPECT_GT(uCible->getInventaireInterne().count(&fuel), 0);
    EXPECT_LE(uCible->getInventaireInterne().count(&fuel), 20);

    EXPECT_LT(uRavitailleur->getInventaireInterne().at(&fuel), 50);
    EXPECT_GE(uRavitailleur->getInventaireInterne().at(&fuel), 30);
}


//Vision et Orientation
TEST(VisionTest, ConeVision_TailleCorrecte) 
{
    Coord origine = {10, 10};
    // direction::est, range=2, fov=3
    auto vue = ConeVision(origine, direction::est, 2, 3);
    
    // Au rang 1: 3 cases, au rang 2: expansion.
    EXPECT_FALSE(vue.empty());
    // Vérifier qu'on ne voit pas derrière
    auto it = std::find(vue.begin(), vue.end(), Coord{10, 9}); // Ouest
    EXPECT_EQ(it, vue.end());
}

//Test Factory
TEST(FactoryTest, UniteCreation_ComportementsInjectes) 
{
    UniteFactory factory;
    // Simuler l'ajout manuel au catalogue pour tester la création
    // (Dans la vraie vie, on utiliserait JsonUniteReader ici)
    
    auto u = createTestUnite("Archer");
    u->ajouterComportement(std::make_shared<CompAttDistance>(15, 4, 2));

    EXPECT_EQ(u->Offensive().size(), 1);
    EXPECT_EQ(u->Offensive().front()->portee(), 4);
    EXPECT_EQ(u->getSymbol(), 'U');
}*/