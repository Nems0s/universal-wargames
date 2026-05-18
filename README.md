# Projet Aux Armes/ Universal Wargame

Ce projet a été réalisé dans le cadre de notre troisième année de licence. Il s'agit de la mise en place d'un moteur d'universal Wargame, et de la mise en place d'une instance sur le thème de l'espace.

---
## Fonctionnalités

- **Moteur de jeu 4X modulable** : Jeu au tour par tour intégrant exploration, expansion, exploitation et extermination.
- **Data-Driven (Configuration JSON)** : Les entités du jeu (factions, unités, bâtiments, ressources, règles) sont configurables via les fichiers du dossier `configs/space/`, permettant de créer de nouvelles instances facilement.
- **Multijoueur (Host/Join)** : Jeu en réseau avec système d'hébergement de partie et de sauvegarde.
- **Deux Interfaces** :
  - *Graphique (`wargame_gui`)* : Interface riche basée sur SFML et ImGui.
  - *Console (`wargame_cli`)* : Pour jouer en mode texte.
- **Génération procédurale** : Création de cartes variées (systèmes stellaires, nébuleuses, etc.) utilisant le Bruit de Perlin.
- **Support IA / Reinforcement Learning** : Intégration d'un environnement Python Gym (`wargame_gym.py`) pour l'entraînement d'agents (`train_ai.py`).

---
## Structure du projet

- `jeu/` : Moteur principal gérant la carte (plateau, tuiles), les villes, les bâtiments et les ressources.
- `joueur/` : Gestion des joueurs, des factions et de leurs statistiques.
- `unite/` : Logique, caractéristiques et comportements des unités (déplacement, ravitaillement, etc.).
- `combat/` : Mécaniques de résolution des batailles et gestion du moral.
- `arbitre/` : Système de validation des actions qui garantit le respect des règles du jeu.
- `UI/` : Implémentation de l'interface graphique avec SFML et ImGui.
- `configs/` : Contient les fichiers JSON qui définissent les paramètres des différentes instances du jeu.
- `tests/` : Suite de tests unitaires pour assurer la fiabilité du moteur.

---
## Prérequis
### Dans un environnement linux :
- sudo apt update
- sudo apt install libsfml-dev

dans le dossier UI :
- git clone https://github.com/ocornut/imgui.git
- cd imgui && git checkout v1.90.4 && cd ..

- git clone https://github.com/SFML/imgui-sfml.git
- cd imgui-sfml && git checkout v2.6 && cd ../..


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

### Commandes utiles

**Lister les DLL nécessaires du jeu Windows (commande Linux) :**
- `x86_64-w64-mingw32-objdump -p windows/bin/main.exe windows/sfml-win/bin/*.dll 2>/dev/null | grep "DLL Name" | sort -u`

---
## Compilation et Lancement du jeu
### Compilation du projet
1. En console
  - make cli

2. Pour l'interface graphique
  - make gui

3. Pour les tests
  - make test

4. Pour tout lancer
  - make

**Conseil** : Pour accélerer la compilation vous pouvez utiliser -j4 ou -j8 après la commande. Cela permet dans lancer la compilation en multithreading.

### Lancement des fichiers
1. En console
  - ./bin/wargame_cli

2. Pour l'interface graphique
  - ./bin/wargame_gui

3. Pour les tests
  - ./bin/wargame_test

**Infos** les fichiers de configuration json dans configs peuvent être modifier à tout moment pour changer le contexte du jeu. Vous pouvez également modifier les assets présents dans le dossier du même nom.

---
## Auteurs
Ce projet a été conçu et développé en binôme :

* **Simon Beasse** : Conceptions des unités, des comportements ainsi que du combat et de l'interface console.
* **Naïm Courbois** : Conception du plateau, des bâtiments, du système économique et de l'interface graphique.

---