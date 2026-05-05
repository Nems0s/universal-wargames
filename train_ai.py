from stable_baselines3 import PPO
from stable_baselines3.common.env_checker import check_env
from stable_baselines3.common.callbacks import CheckpointCallback
from wargame_gym import SpaceWargamesEnv

def main():
    print("--- INITIALISATION DU CAMP D'ENTRAÎNEMENT ---")
    env = SpaceWargamesEnv()
    check_env(env, warn=False)

    # Sauvegarde le cerveau tous les 100 000 coups
    checkpoint_callback = CheckpointCallback(
        save_freq=100000,
        save_path='./sauvegardes_ia/',
        name_prefix='ppo_spacewargames_v2'
    )

    # ---------------------------------------------------------
    # 1. CREATION DE L'IA
    # ---------------------------------------------------------

    # print("Création du réseau de neurones...")
    # model = PPO(
    #     "MlpPolicy", 
    #     env, 
    #     verbose=1, 
    #     learning_rate=0.0003,
    #     n_steps=2048,
    #     batch_size=64,
    #     ent_coef=0.01, # Encourage l'IA à explorer un peu plus (très utile !)
    #     device="cpu", # Le Ryzen 5 3600 va faire le travail
    #     tensorboard_log="./wargame_tensorboard/"
    # )


    # ---------------------------------------------------------
    # 2. CHARGEMENT DE L'IA
    # ---------------------------------------------------------

    print("Chargement du cerveau existant à 100 000 steps...")
    model = PPO.load(
        "./sauvegardes_ia/ppo_spacewargames_v2_400000_steps",
        env=env,
        tensorboard_log="./wargame_tensorboard/"
    )


    # ---------------------------------------------------------
    # 3. LANCEMENT DE L'IA
    # ---------------------------------------------------------

    print("--- DÉBUT DU MILLION DE STEPS ---")
    model.learn(
        total_timesteps=1_000_000, 
        callback=checkpoint_callback,
        reset_num_timesteps=False 
    )

    model.save("ppo_spacewargames_master")
    print("Entraînement terminé et modèle MASTER sauvegardé !")

if __name__ == "__main__":
    main()

# tensorboard --logdir ./wargame_tensorboard/
# python3 train_ai.py