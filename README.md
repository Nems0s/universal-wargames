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





