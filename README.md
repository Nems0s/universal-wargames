# space-wargames

dans l'environnement linux :
- sudo apt update
- sudo apt install libsfml-dev


dans le dossier UI :
- git clone https://github.com/ocornut/imgui.git
- cd imgui && git checkout v1.90.4 && cd ..

- git clone https://github.com/SFML/imgui-sfml.git
- cd imgui-sfml && git checkout v2.6 && cd ../..


Créer l'installateur Windows depuis linux :
- télécharger la version 2.6.2 de SMFL pour Windows GCC MinGW: https://www.sfml-dev.org/download/sfml/2.6.2/
- l'extraire et mettre dans le dossier windows en le renommant "sfml-win"
- commande pour la compilation windows sous linux : sudo apt install mingw-w64
- bash package.sh
- exécuter le script create-installateur.iss via Inno Setup Compiler avec le bon dossier