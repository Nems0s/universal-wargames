#!/bin/bash
echo "=== Compilation pour Windows ==="
make -f windows/Makefile.win clean
make -f windows/Makefile.win

echo "=== Création du dossier de distribution ==="
DIR="windows/SpaceWargames_Windows"
rm -rf $DIR
mkdir -p $DIR

echo "=== Copie des fichiers ==="
# 1. L'exécutable (CORRIGÉ : on le prend depuis windows/bin/)
cp windows/bin/main.exe $DIR/

# 2. Les ressources (Assets et JSON)
cp -r assets $DIR/
cp -r configs $DIR/

# 3. Les DLLs SFML
cp windows/sfml-win/bin/sfml-graphics-2.dll $DIR/
cp windows/sfml-win/bin/sfml-window-2.dll $DIR/
cp windows/sfml-win/bin/sfml-system-2.dll $DIR/
cp windows/sfml-win/bin/sfml-network-2.dll $DIR/

echo "=== Terminé ! ==="
echo "Le dossier complet se trouve dans : $DIR"