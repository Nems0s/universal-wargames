import os
from stable_baselines3 import PPO
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.vec_env import SubprocVecEnv
from stable_baselines3.common.callbacks import CheckpointCallback
from wargame_gym import SpaceWargamesEnv

def main():
    print("--- 🚀 MULTIPROCESSING: ACTIVATION DU RYZEN 5 3600 🚀 ---")

    # Ton processeur a 12 threads. Lancer 8 environnements est le "Sweet Spot" 
    # pour le saturer sans faire planter ton PC.
    n_envs = 8

    print(f"Création de {n_envs} univers parallèles...")
    # SubprocVecEnv lance de vrais processus séparés. Le C++ va tourner 8x plus vite !
    env = make_vec_env(SpaceWargamesEnv, n_envs=n_envs, vec_env_cls=SubprocVecEnv)

    # La fréquence de sauvegarde doit être divisée par 8, car on génère 8x plus de données à la seconde.
    checkpoint_callback = CheckpointCallback(
        save_freq=max(100_000 // n_envs, 1),
        save_path='./sauvegardes_ia/',
        name_prefix='ppo_multi_opti'
    )

    model_path = "ppo_universel_master.zip"
    
    # On reste sur le CPU. Avec 8 process C++, la charge de transfert RAM->GPU annulerait le gain.
    mon_device = "cpu" 

    if os.path.exists(model_path):
        print(f"🧠 Ancien cerveau trouvé ! Reprise de l'entraînement sur {mon_device.upper()}...")
        model = PPO.load(
            model_path[:-4],
            env=env,
            device=mon_device,
            tensorboard_log="./wargame_tensorboard/"
        )
    else:
        print(f"👶 Nouveau cerveau. Création du réseau Multi-Input sur {mon_device.upper()}...")
        model = PPO(
            "MultiInputPolicy", 
            env, 
            verbose=1, 
            learning_rate=0.0003,
            n_steps=2048,      # L'IA va désormais analyser 2048 * 8 = 16 384 actions avant d'apprendre !
            batch_size=256,    # On augmente le Batch Size car on a beaucoup plus de données
            ent_coef=0.01,
            device=mon_device, 
            tensorboard_log="./wargame_tensorboard/"
        )

    print("Branchement du Self-Play Asynchrone (Chargement local)...")
    # On utilise la fonction 'load_adversary' qu'on a créée dans wargame_gym.py
    env.env_method("load_adversary", "ppo_universel_master")

    print("--- DÉBUT DE L'APPRENTISSAGE ACCÉLÉRÉ ---")
    try:
        model.learn(total_timesteps=1_000_000, callback=checkpoint_callback, reset_num_timesteps=False)
        model.save("ppo_universel_master")
        print("Entraînement terminé et modèle MASTER sauvegardé !")
    except KeyboardInterrupt:
        print("\nArrêt manuel détecté. Sauvegarde du cerveau en cours...")
        model.save("ppo_universel_master")
        print("Sauvegarde de sécurité terminée. À la prochaine !")

if __name__ == "__main__":
    main()