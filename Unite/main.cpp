#include "unite_complexe.hh"
#include <iostream>

using namespace std;

int main()
{
    cout << "=====================================================" << endl;
    cout << "       CREATION DES UNITES DE BASE ET SIMPLES" << endl;
    cout << "=====================================================" << endl << endl;

    Unite_legere            UL("Fantassin", 100, 15, 3, direction::est, {0, 0});
    Unite_volante           UV("Hélicoptère", 150, 40, 5, direction::nord_est, {2, 2});
    Unite_maritime          UM("Corvette", 200, 20, 2, direction::ouest, {1, 0});
    Unite_lourde            ULO("Tank Lourd", 300, 50, 2, direction::sud_est, {3, 1});
    Unite_transport         UT("Camion Transport", 250, 5, 3, direction::nord_ouest, {0, 5});

    UL.affiche();
    UV.affiche();
    UM.affiche();
    ULO.affiche();
    UT.affiche();

    cout << endl;
    cout << "=====================================================" << endl;
    cout << "         CREATION DES UNITES HYBRIDES (MULTIPLE)" << endl;
    cout << "=====================================================" << endl << endl;

    Unite_cuirrasse      UC("Cuirasse Royal", 500, 80, 1, direction::sud_est, {4, 4});
    Unite_gunship        UG("Gunship Apache", 250, 90, 6, direction::nord_ouest, {5, 5});
    Unite_amphibi        UA("Soldat Amphibi", 150, 25, 3, direction::sud_ouest, {7, 7});

    // Les unités à triple héritage !
    Unite_cargo_volant   UCV("Cargo Pelican", 400, 30, 7, direction::nord_est, {10, 10});
    Unite_embarcation    UE("Embarcation Zodiac", 180, 40, 5, direction::ouest, {8, 1});

    UC.affiche();
    UG.affiche();
    UA.affiche();
    UCV.affiche();
    UE.affiche();

    cout << endl;
    cout << "=====================================================" << endl;
    cout << "               TEST DU DEPLACEMENT" << endl;
    cout << "=====================================================" << endl << endl;

    cout << "Deplacement du Cargo Pelican (actuellement en 10,10) vers la case (15, 12)..." << endl;
    UCV.movement({15, 12});
    UCV.affiche(); // Les coordonnées doivent avoir changé

    cout << "Deplacement de l'Embarcation (actuellement en 8,1) vers la case (12, 4)..." << endl;
    UE.movement({12, 4});
    UE.affiche();

    return 0;
}
