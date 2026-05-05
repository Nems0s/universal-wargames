from stable_baselines3 import PPO
from wargame_gym import SpaceWargamesEnv
import time

def main():
    print("--- PRÉPARATION DE L'ARÈNE ---")
    env = SpaceWargamesEnv()
    
    print("--- TÉLÉCHARGEMENT DU CERVEAU ---")
    # Remplace par le nom d'un fichier dans sauvegardes_ia/ si l'entraînement n'est pas fini
    # Exemple : PPO.load("./sauvegardes_ia/ppo_spacewargames_v2_100000_steps")
    model = PPO.load("ppo_spacewargames_v1") 
    
    obs, info = env.reset()
    terminated = False
    truncated = False
    
    # Noms des actions pour faire un bel affichage
    noms_actions = [
        "Fonder Ville", "Acheter Case", "Ameliorer", "Deplacement", 
        "Attaque", "Rotation", "Recrutement", "Construction", 
        "Soigner", "Camoufler", "Charger", "Decharger", 
        "Enroler", "Detruire", "Fin Tour"
    ]

    print("\n🚀 QUE LE COMBAT COMMENCE 🚀\n")
    
    while not (terminated or truncated):
        # L'IA observe le terrain et prend une décision. 
        # deterministic=True l'oblige à utiliser sa meilleure stratégie, sans hasard.
        action, _states = model.predict(obs, deterministic=True) 
        
        # Le jeu avance
        obs, reward, terminated, truncated, info = env.step(action)
        
        # Affichage
        action_id = action[0] % 15
        print(f"Tour {info['tour']} | L'IA joue : {noms_actions[action_id]:<15} | Récompense reçue : {reward:+.2f}")
        
        # On ralentit un peu pour que ce soit lisible à l'oeil nu
        time.sleep(0.05) 

    print("\n--- PARTIE TERMINÉE ---")
    vainqueur = env.moteur.getNomVainqueur()
    if vainqueur == "IA_Agent":
        print("👑 VICTOIRE DE L'IA !")
    else:
        print("💀 DÉFAITE DE L'IA...")

if __name__ == "__main__":
    main()