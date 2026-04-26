# space-wargames

dans l'environnement linux :
- sudo apt update
- sudo apt install libsfml-dev

dans le dossier UI :
- git clone https://github.com/ocornut/imgui.git
- cd imgui && git checkout v1.90.4 && cd ..

- git clone https://github.com/SFML/imgui-sfml.git
- cd imgui-sfml && git checkout v2.6 && cd ../..

---

### Créer l'installateur Windows depuis linux :

**1. Préparer le compilateur croisé**
- Commande pour la compilation Windows sous Linux : `sudo apt install mingw-w64`

**2. Préparer la SFML**
- Télécharger la version 2.6.2 de SFML pour Windows **GCC MinGW (64-bit)** : https://www.sfml-dev.org/download/sfml/2.6.2/
- L'extraire et la mettre dans le dossier `windows/` de ce projet en renommant le dossier obtenu en `sfml-win` (il doit contenir les sous-dossiers `bin`, `lib` et `include`).

**3. Ajouter les DLLs MinGW manquantes (CRITIQUE)**
La SFML nécessite certaines DLLs du compilateur MinGW pour fonctionner sur une machine qui ne l'a pas installé.
- Récupérez ces 3 fichiers depuis une installation MinGW sur Windows (généralement dans `C:\msys64\mingw64\bin\`) :
  - `libstdc++-6.dll`
  - `libgcc_s_seh-1.dll`
  - `libwinpthread-1.dll`
- Placez impérativement ces 3 fichiers dans le dossier `windows/sfml-win/bin/` de ce projet (à côté des fichiers `sfml-graphics-2.dll`, etc.).

**4. Compiler et packager**
- Depuis la racine du projet sous Linux, lancer : `bash windows/package.sh`
- Sous Windows, exécuter le script `create-installateur.iss` via Inno Setup Compiler en pointant vers le dossier généré.

---

### Commandes utiles

**Lister les DLL nécessaires du jeu Windows (commande Linux) :**
- `x86_64-w64-mingw32-objdump -p windows/bin/main.exe windows/sfml-win/bin/*.dll 2>/dev/null | grep "DLL Name" | sort -u`




Améliorations à faire sur le jeu :
- Quand je zoom sur le jeu, cela zoom vers le curseur
- Les ressources sont affichés (principales comme l'or, etc (défini dans les options avec path d'un logo)) en haut sous la barre du haut
- Afficher la production/tour de chaque ressource avec infos dans un tooltip de chaque batiment/ville
- Problème où on ne voit pas les troupes ou les villes ennemis sur la map
- Menu avec une liste des joueurs et des infos sur eux 
- Il faut pouvoir interagir avec les troupes et les villes ennemis (attaquer, capturer, etc)
- Actuellement quand c'est notre tour et qu'on clique sur une tuile du jeu cela affiche les informations de la tuile (comme fonder ville ou autre), il faudrait que cela affiche les informations de la tuile si on la voit, sinon rien.
- Ne pas pouvoir construire plusieurs batiment spéciale sur la même tuile ou affiché les différents batiments dans un panel avec la quantité produite/tour
- Voir la production de chaque batiment de chaque ressource dans un panneau spécifique
- Il faut un menu spécifique où on peut voir toutes les informations de notre empire (productions, ce qu'on possède, troupes, villes, etc)
- Le nom des joueurs doit pouvoir être modifié après avoir appuyé sur rejoindre, pareil pour host pas avant
- Correctement save et load la game avec des boutons continuer/new game etc.
- Gérer le fait de se déconnecter de l'host (sans devoir leave le client)
- Ne pas spawn la cam au début sur l'ennemi mais sur sa capitale, et à chaque tour ne pas remettre la cam sur sa capitale mais la laissez où l'on est
- Fix le fait que ce soit impossible de recruter/build sur sa capitale
- Fix le fait que ce soit impossible de détruire ses propres unités
- Quand je modifie le prix des colonies ou autre dans les options, cela modifie pour soi et pas pour les autres, alors que cela doit juste être défini au début au moment de la création de la partie. Donc en modifiant dans mes options je peux payer 0 pour chaque colonie, ce qui n'est pas normal.
- Fix le fait d'avoir les infos des villes ennemis quand cliqué dessus (pareil pour batiment et unité)
- Pouvoir revenir sur le jeu après avoir appuyé sur menu principal ou modifier les options du jeu
- Message de confirmation de save les options
- Quand on ajoute 3 joueurs au jeu mais qu'en multi il y a que 2 joueurs, il faut que le 3ème joueur soit retiré de la partie, sinon pendant la partie on attends pour rien, il faut juste que ce soit joueur_max et non pas le nombre de joueurs (potentiellement plus tard quand il y aura une IA ou autre, mais pas pour l'instant)
- persistance des options sur le pc de chacun même après avoir quitté le launcher
- Fix les bugs sur les tuiles qui ont un contours verts pour la zone de la ville mais qui ne s'actualise pas bien quand j'achête une tour ou autre
- Fix le bug sur les tuiles qui sont toujours sombre en arrière plan au lieu de directement affiché la tuile.



Nouveaux bugs :
1. Colonie qui coute 0
2. Cela n'affiche pas ma prod correctement car ma capitale me produit de l'or et ce n'est pas affiché dans la prod
3. Bug sur les tuiles vertes qui s'update mal quand j'achete une nouvelle colonie etc
4. Les colonies ne produisent plus rien alors que la config doit mettre que si
5. Mauvaise update des cases autour de l'unité en jaune où elles peuvent se déplacer les cases 
6. Le client peut save alors que cela doit être juste l'hôte
7. Le client peut continuer même sans l'host après avoir leave et fait continuer
8. Les boutons Game Settings et Graphics afficher dans la config des joueurs au niveau des factions
9. Délimitation des planetes mal affichés (couleur correct mais affiché un point de couleur)
10. On ne voit pas les troupes ennemis (point de couleur)
11. Afficher message que la ville est au level max
12. Les troupes ennemis peuvent aller sur la même case sans engager un combat, qui doit être vu par les deux joueurs
13. Panneau de combat pour voir les stats et historique des actions
14. Différence entre les boutons Continuer et Reprendre la partie
15. Quand le client essaye de créer une nouvelle partie en étant dans ma partie :

Program: E:\eux\Space Wargames\main.exe
File: Ul/imgui/imgui.cpp
Line: 9777

Expression: SizeOfDisabledStack == g.DisabledStackSize &&
"BeginDisabled/EndDisabled Mismatch!"

For information on how your program can cause an assertion
failure, see the Visual C++ documentation on asserts

(Press Retry to debug the application - JIT must be enabled)


1) Bug des boutons games settings et graphics qui sont affichés au moment de la sélection des factions
2) json config_rules add factions path vers l'image de la faction correspondant stocké dans le dossier assets et afficher cette image dans l'information de faction avant le détails des caractéristiques
3) afficher configuration de la partie au lieu de configurer la carte, après que les joueurs aient rejoints et afficher pour celui qui a join au moment du click et non pas au moment de choisir une des factions
4) Celui qui join peut faire back au moment du choix des factions, désactiver le bouton ou que cela affiche un message de confirmation pour demander s'il veut vraiment quitter et sinon l'enlevé des joueurs et remettre l'host sur la page d'attente des joueurs avec un message qui dit que tel joueurs s'est déconnecté.
5) Celui qui join peut save la partie alors que cela ne devrait pas être le cas, uniquement l'host car c'est lui qui a la partie.
6) afficher le nom de la ressource s'il n'y a pas d'image, et bloquer ça à un nombre de ressource pour éviter de prendre toute la page.
7) contours de couleur des villes qui est mal affiché
8) ajout de regénération des villes
9) Ajout d'un cout d'entretien des unités
Cout en ressource à chaque tour défini de base dans rules et pour chaque unité (Perte de HP si impossible de payer le cout d'entretien)
10) Refonte de la gestion des conditions de victoire et de la save
