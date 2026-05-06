import os
from stable_baselines3 import PPO
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.vec_env import SubprocVecEnv
from stable_baselines3.common.callbacks import CheckpointCallback
from wargame_gym import SpaceWargamesEnv

def main():
    print("--- 🐉 RÉVEIL DE LA RTX 2070 SUPER 🐉 ---")

    # 1. On pousse le Ryzen dans ses retranchements pour générer les données
    # Ton Ryzen 5 3600 a 12 threads. 10 est très agressif mais laissera 2 threads pour l'OS.
    n_envs = 8

    env = make_vec_env(SpaceWargamesEnv, n_envs=n_envs, vec_env_cls=SubprocVecEnv)

    checkpoint_callback = CheckpointCallback(
        save_freq=max(100_000 // n_envs, 1),
        save_path='./sauvegardes_ia/',
        name_prefix='ppo_rtx_master'
    )

    model_path = "ppo_universel_master.zip"
    
    # 2. ON FORCE L'UTILISATION DE LA CARTE GRAPHIQUE
    mon_device = "cuda" 

    if os.path.exists(model_path):
        print(f"🧠 Reprise du cerveau sur {mon_device.upper()}...")
        model = PPO.load(
            model_path[:-4],
            env=env,
            device=mon_device,
            tensorboard_log="./wargame_tensorboard/"
        )
    else:
        print(f"👶 Nouveau cerveau sur {mon_device.upper()}...")
        model = PPO(
            "MultiInputPolicy", 
            env, 
            verbose=1, 
            learning_rate=0.0003,
            n_steps=2048,      
            
            # 3. LE SECRET DU GPU : DES BATCHS ÉNORMES
            # Au lieu de 64, on envoie 1024 données d'un coup à la VRAM.
            # La RTX 2070 va les traiter en parallèle instantanément.
            batch_size=1024,    
            
            ent_coef=0.01,
            device=mon_device, 
            tensorboard_log="./wargame_tensorboard/"
        )

    print("Branchement du Self-Play Asynchrone...")
    # On n'envoie plus l'objet Python. On envoie juste le chemin du fichier !
    # Chaque processus va charger la sauvegarde silencieusement sur son CPU.
    env.env_method("load_adversary", "ppo_universel_master")

    print("--- DÉBUT DE L'APPRENTISSAGE EXTRÊME ---")
    try:
        model.learn(total_timesteps=2_000_000, callback=checkpoint_callback, reset_num_timesteps=False)
        model.save("ppo_universel_master")
        print("Entraînement terminé !")
    except KeyboardInterrupt:
        print("\nArrêt manuel détecté. Sauvegarde en cours...")
        model.save("ppo_universel_master")
        print("Sauvegarde terminée !")

if __name__ == "__main__":
    main()