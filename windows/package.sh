#!/bin/bash
echo "=== Compilation pour Windows ==="
make -f windows/Makefile.win clean
make -f windows/Makefile.win

echo "=== Création du dossier de distribution ==="
DIR="windows/SpaceWargames_Windows"
rm -rf $DIR
mkdir -p $DIR

echo "=== Copie des fichiers ==="
# 1. L'exécutable
cp windows/bin/main.exe $DIR/

# 2. Les ressources (Assets et JSON)
cp -r assets $DIR/
cp -r configs $DIR/

# 3. TOUTES LES DLLs (SFML + MinGW compatibles)
echo "=== Copie de toutes les DLLs ==="
cp windows/sfml-win/bin/*.dll $DIR/

echo "=== Terminé ! ==="
echo "Le dossier complet se trouve dans : $DIR"