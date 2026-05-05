
import sys
import os

# 1. On ajoute le dossier build au chemin de recherche de Python
sys.path.append(os.path.join(os.path.dirname(__file__), 'build'))

import wargame_env # Maintenant, Python trouve ton module .so !

def main():
    moteur = wargame_env.MoteurDeJeu()
    
    try:
        # 2. Le chemin part maintenant de la racine, comme le C++ l'attend !
        moteur.chargerConfiguration("configs/config_rules.json")
        
        noms = ["IA_Alpha", "IA_Beta"]
        factions = ["NomFaction1", "NomFaction2"] 
        moteur.initGame(42, noms, factions)
        
        print(f"[OK] Partie initialisée. Tour actuel : {moteur.getTourActuel()}")
        moteur.passerTour()
        print(f"[OK] Fin de tour simulée. Tour actuel : {moteur.getTourActuel()}")
        
    except Exception as e:
        print(f"\n[ERREUR] Un problème est survenu : {e}")

if __name__ == "__main__":
    main()