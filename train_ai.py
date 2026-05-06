import os
from stable_baselines3 import PPO
from stable_baselines3.common.env_checker import check_env
from stable_baselines3.common.callbacks import CheckpointCallback
from wargame_gym import SpaceWargamesEnv

def main():
    print("--- CREATION DU SUPER-CERVEAU (MULTI-INPUT) ---")
    env = SpaceWargamesEnv()

    # Vérification obligatoire pour s'assurer que l'environnement Dict est parfait
    check_env(env, warn=False)

    # 1. Configuration des sauvegardes régulières
    checkpoint_callback = CheckpointCallback(
        save_freq=100_000,
        save_path='./sauvegardes_ia/',
        name_prefix='ppo_multi'
    )

    model_path = "ppo_universel_master.zip"

    my_device = "cuda"
    # my_device = "cpu" # Plus rapide car le jeu tourne sur le CPU : transfert RAM->PCIe puis revenir lent

    # 2. Logique de Reprise Automatique
    if os.path.exists(model_path):
        print("🧠 Ancien cerveau trouvé ! Reprise de l'entraînement...")
        model = PPO.load(
            model_path[:-4],
            env=env,
            device=my_device,
            tensorboard_log="./wargame_tensorboard/"
        )
    else:
        print("👶 Nouveau cerveau. Création du réseau Multi-Input...")
        model = PPO(
            "MultiInputPolicy", # Indique à PyTorch de créer plusieurs CNN qui fusionnent !
            env, 
            verbose=1, 
            learning_rate=0.0003,
            n_steps=2048,
            batch_size=64,
            ent_coef=0.01, # Encourage l'IA à explorer un peu plus
            device=my_device,
            tensorboard_log="./wargame_tensorboard/"
        )

    # Branchement du Self-Play
    print("Branchement du Self-Play (L'IA est son propre adversaire)...")
    env.set_model(model)

    # 3. Lancement (On peut l'arrêter avec Ctrl+C et relancer, ça reprendra !)
    print("--- DÉBUT DE L'APPRENTISSAGE ---")
    try:
        # On demande 1 million, mais tu peux couper quand tu veux.
        model.learn(total_timesteps=1_000_000, callback=checkpoint_callback, reset_num_timesteps=False)
        model.save("ppo_universel_master")
        print("Entraînement terminé et modèle MASTER sauvegardé !")
    except KeyboardInterrupt:
        print("\nArrêt manuel détecté. Sauvegarde du cerveau en cours...")
        model.save("ppo_universel_master")
        print("Sauvegarde de sécurité terminée. À la prochaine !")

if __name__ == "__main__":
    main()

# tensorboard --logdir ./wargame_tensorboard/
# python3 train_ai.py