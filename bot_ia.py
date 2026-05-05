import numpy as np
from stable_baselines3 import PPO
import os

model = None
map_w = 15
map_h = 15

def init_ia(model_name, w, h):
    global model, map_w, map_h
    try:
        # On charge le cerveau depuis le dossier courant ou sauvegardes_ia/
        chemin = model_name
        if not os.path.exists(chemin + ".zip"):
            chemin = f"sauvegardes_ia/{model_name}"
            
        model = PPO.load(chemin, device="cpu")
        map_w = w
        map_h = h
        print(f"[Python] Cerveau IA '{model_name}' charge avec succes pour map {w}x{h} !")
        return True
    except Exception as e:
        print(f"[Python] Erreur critique chargement IA : {e}")
        return False

def get_action(obs_list):
    # Le C++ nous envoie une liste plate, on la reformate pour PyTorch
    obs_array = np.array(obs_list, dtype=np.float32).reshape(1, 4, map_w, map_h)
    
    # deterministic=True oblige l'IA à jouer son meilleur coup, sans exploration au hasard
    action, _ = model.predict(obs_array, deterministic=True)
    
    # On renvoie une simple liste d'entiers au C++
    return [int(x) for x in action[0]]