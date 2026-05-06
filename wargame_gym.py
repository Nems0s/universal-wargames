import gymnasium as gym
from gymnasium import spaces
import numpy as np
import sys
import os
import cv2

sys.path.append(os.path.join(os.path.dirname(__file__), 'build'))
import wargame_env

class SpaceWargamesEnv(gym.Env):
    metadata = {"render_modes": ["console"]}

    def __init__(self, render_mode=None):
        super().__init__()
        self.render_mode = render_mode
        self.moteur = wargame_env.MoteurDeJeu()
        
        # --- CHARGEMENT CONFIGURATION ---
        configs = wargame_env.GameConfigFiles()
        configs.rulesPath = "configs/pirate/config_rules.json"
        configs.ressourcesPath = "configs/pirate/config_ressources.json"
        configs.batimentsPath = "configs/pirate/config_batiments.json"
        configs.villesPath = "configs/pirate/config_villes.json"
        configs.winsPath = "configs/pirate/config_wins.json"
        configs.unitesPath = "configs/pirate/config_unites.json"
        configs.tuilesPath = "configs/pirate/config_tuiles.json"
        self.moteur.chargerConfiguration(configs)
        
        # --- ARCHITECTURE DES DIMENSIONS ---
        self.CHANNELS = 8          # Nombre de couches renvoyées par le C++
        self.MAX_MAP_SIZE = 100    # Limite technique absolue gérée
        self.WINDOW_SIZE = 30      # La "Caméra" de l'IA (Vision Haute Définition)
        self.MINIMAP_SIZE = 20     # La "Minimap" (Vision Globale dézoomée)
        
        # Coordonnées de la caméra
        self.cam_x = 0
        self.cam_y = 0

        # --- ACTION SPACE ---
        # 0-14: Commandes Jeu | 15: Caméra Nord | 16: Sud | 17: Est | 18: Ouest
        self.action_space = spaces.MultiDiscrete([
            19,                 # Action ou Déplacement Caméra
            self.WINDOW_SIZE,   # Coord X source (relative à la caméra)
            self.WINDOW_SIZE,   # Coord Y source
            self.WINDOW_SIZE,   # Coord X cible
            self.WINDOW_SIZE    # Coord Y cible
        ])

        # --- OBSERVATION SPACE (MULTI-INPUT) ---
        # L'IA reçoit un dictionnaire avec 3 entrées distinctes. PPO utilisera un MultiInputPolicy.
        self.observation_space = spaces.Dict({
            "camera": spaces.Box(low=-2.0, high=2.0, shape=(self.CHANNELS, self.WINDOW_SIZE, self.WINDOW_SIZE), dtype=np.float32),
            "minimap": spaces.Box(low=-2.0, high=2.0, shape=(self.CHANNELS, self.MINIMAP_SIZE, self.MINIMAP_SIZE), dtype=np.float32),
            "cam_pos": spaces.Box(low=0.0, high=1.0, shape=(2,), dtype=np.float32) # Position X, Y normalisée
        })

        self.nb_tours_joues = 0
        self.noms_joueurs = ["IA_Agent", "Adversaire_Bot"]
        self.compteur_erreurs = 0
        self.tuiles_explorees = 0

    def set_model(self, model):
        """Permet d'activer le Self-Play"""
        self.model = model

    def load_adversary(self, model_path):
        """Les processus clones chargent l'ennemi sur leur propre CPU"""
        from stable_baselines3 import PPO
        import os
        
        # On vérifie si l'ancien cerveau existe
        if os.path.exists(model_path + ".zip"):
            # CRUCIAL : L'adversaire est forcé sur le CPU pour laisser le GPU libre !
            self.model = PPO.load(model_path, device="cpu")
        else:
            self.model = None # S'il n'y a pas de sauvegarde, l'ennemi passera son tour

    def _get_obs_dict(self, pIdx, cam_x, cam_y):
        flat_obs = self.moteur.get_state_ai(pIdx)
        map_w = self.moteur.getLogicConfig().getPlateauX()
        map_h = self.moteur.getLogicConfig().getPlateauY()
        
        full_obs = np.array(flat_obs, dtype=np.float32).reshape(self.CHANNELS, map_w, map_h)
        
        # 1. CAMERA (Taille fixe WINDOW_SIZE x WINDOW_SIZE avec padding)
        camera_obs = np.full((self.CHANNELS, self.WINDOW_SIZE, self.WINDOW_SIZE), -1.0, dtype=np.float32)
        slice_w = min(self.WINDOW_SIZE, map_w - cam_x)
        slice_h = min(self.WINDOW_SIZE, map_h - cam_y)
        if slice_w > 0 and slice_h > 0:
            camera_obs[:, :slice_w, :slice_h] = full_obs[:, cam_x:cam_x+slice_w, cam_y:cam_y+slice_h]
            
        # 2. MINIMAP DYNAMIQUE (Toujours compressée en MINIMAP_SIZE x MINIMAP_SIZE)
        minimap_obs = np.zeros((self.CHANNELS, self.MINIMAP_SIZE, self.MINIMAP_SIZE), dtype=np.float32)
        for i in range(self.CHANNELS):
            # cv2.resize redimensionne n'importe quelle matrice (ex: 200x200) vers (20x20)
            minimap_obs[i] = cv2.resize(full_obs[i], (self.MINIMAP_SIZE, self.MINIMAP_SIZE), interpolation=cv2.INTER_AREA)
        
        # 3. POSITION CAMERA (Normalisée de 0 à 1, peu importe la taille de la map)
        pos_obs = np.array([cam_x / max(1, map_w), cam_y / max(1, map_h)], dtype=np.float32)
        
        return { "camera": camera_obs, "minimap": minimap_obs, "cam_pos": pos_obs }

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        
        if seed is not None:
            game_seed = int(seed) % 100000 
        else:
            game_seed = int(np.random.randint(0, 10000))
        
        # Init C++
        self.moteur.initGame(game_seed, self.noms_joueurs, ["Pirates", "Pirates"])
        self.moteur.setActiveVictorySet(2) 
        
        # Reset variables Python
        self.nb_tours_joues = 0
        self.compteur_erreurs = 0
        self.cam_x, self.cam_y = 0, 0
        
        # On calcule le nombre initial de tuiles explorées
        obs_dict = self._get_obs_dict(0, self.cam_x, self.cam_y)
        self.tuiles_explorees = np.sum(obs_dict["minimap"][0] == 1.0)
        
        return obs_dict, {"tour": 0}

    def step(self, action):
        pIdx = 0 
        act_type = int(action[0])
        reward = 0.0
        code_resultat = 1 # Erreur par défaut
        
        # --- 1. GESTION DES MOUVEMENTS DE CAMÉRA ---
        if act_type >= 15:
            move_speed = 5 # La caméra saute de 5 cases
            map_w = self.moteur.getLogicConfig().getPlateauX()
            map_h = self.moteur.getLogicConfig().getPlateauY()
            
            if act_type == 15: self.cam_x = max(0, self.cam_x - move_speed)
            elif act_type == 16: self.cam_x = min(map_w - self.WINDOW_SIZE, self.cam_x + move_speed)
            elif act_type == 17: self.cam_y = min(map_h - self.WINDOW_SIZE, self.cam_y + move_speed)
            elif act_type == 18: self.cam_y = max(0, self.cam_y - move_speed)
            
            code_resultat = 0
            reward -= 0.02 # Micro-pénalité pour éviter que l'IA ne fasse que bouger l'écran à l'infini
        else:
            # --- 2. GESTION DES ACTIONS DE JEU ---
            # Conversion : Coordonnées Caméra (0-30) -> Coordonnées Monde Réel (0-100)
            real_action = [
                act_type,
                int(action[1]) + self.cam_x,
                int(action[2]) + self.cam_y,
                int(action[3]) + self.cam_x,
                int(action[4]) + self.cam_y
            ]
            
            map_w = self.moteur.getLogicConfig().getPlateauX()
            map_h = self.moteur.getLogicConfig().getPlateauY()
            
            # Anti-Crash : On vérifie que le clic n'est pas en dehors des limites réelles de la carte
            if real_action[1] >= map_w or real_action[2] >= map_h or real_action[3] >= map_w or real_action[4] >= map_h:
                code_resultat = 5 
            else:
                code_resultat = self.moteur.step_ai(pIdx, real_action)

        # --- 3. CALCUL DU REWARD SHAPING (Totalement géré en Python) ---
        new_obs_dict = self._get_obs_dict(pIdx, self.cam_x, self.cam_y)
        
        if code_resultat == 0:
            self.compteur_erreurs = 0
            
            # A. Récompense d'Exploration (Couche 0)
            nouvelles_explorees = np.sum(new_obs_dict["minimap"][0] == 1.0)
            if nouvelles_explorees > self.tuiles_explorees:
                reward += (nouvelles_explorees - self.tuiles_explorees) * 0.1 # +0.1 par nouvelle case
                self.tuiles_explorees = nouvelles_explorees
                
            # B. Autres récompenses basées sur le type d'action (act_type)
            if act_type == 4:  # Attaque
                reward += 2.0  # Encourage l'agressivité
            elif act_type == 6: # Recrutement
                reward += 1.0
                
        elif code_resultat in [2, 3, 4, 5]: 
            reward -= 0.1
            self.compteur_erreurs += 1

        # Coupe-circuit (Reward Hacking)
        if self.compteur_erreurs > 20:
            act_type = 14 # Force Fin de tour
            reward -= 1.0
            self.compteur_erreurs = 0
            
        # --- 4. GESTION DU TOUR DE L'ENNEMI (SELF-PLAY) ---
        if act_type == 14:
            self.nb_tours_joues += 1
            reward -= 0.2 # On pénalise la durée pour la forcer à gagner vite
            self.compteur_erreurs = 0
            
            # Boucle tant que c'est le tour de l'ennemi
            while self.moteur.getCurrentPlayerTurn() != 0 and not self.moteur.isPartieTerminee():
                adv_idx = self.moteur.getCurrentPlayerTurn()
                
                if hasattr(self, 'model') and self.model is not None:
                    # L'adversaire a sa propre caméra (ici on la centre sur 0,0 pour simplifier le bot adverse)
                    obs_ennemi = self._get_obs_dict(adv_idx, 0, 0) 
                    action_ennemi, _ = self.model.predict(obs_ennemi, deterministic=True) 
                    
                    act_e_list = [int(a) for a in action_ennemi]
                    
                    # Si l'adversaire déplace sa caméra, on l'ignore côté serveur pour aller plus vite, ou on applique
                    if act_e_list[0] >= 15 or act_e_list[0] == 14:
                        self.moteur.step_ai(adv_idx, [14, 0, 0, 0, 0]) # Force fin de tour
                    else:
                        code_e = self.moteur.step_ai(adv_idx, act_e_list)
                        if code_e in [2, 3, 4, 5]: 
                            self.moteur.step_ai(adv_idx, [14, 0, 0, 0, 0]) # Sécurité anti-boucle infinie
                else:
                    self.moteur.step_ai(adv_idx, [14, 0, 0, 0, 0])

        # --- 5. CONDITIONS DE FIN ---
        terminated = self.moteur.isPartieTerminee()
        truncated = False
        
        if terminated:
            if self.moteur.getNomVainqueur() == self.noms_joueurs[0]:
                reward += 500.0
                print(f"👑 VICTOIRE IA ({self.nb_tours_joues} tours)")
            else:
                reward -= 50.0   
                
        if self.nb_tours_joues > 150:
            truncated = True

        # Attention : On redemande l'observation car le tour de l'ennemi a changé le plateau !
        final_obs_dict = self._get_obs_dict(0, self.cam_x, self.cam_y)
        return final_obs_dict, reward, terminated, truncated, {"tour": self.moteur.getTourActuel()}

    def _get_info(self):
        """Retourne les infos de debug à Gymnasium"""
        return {"tour": self.moteur.getTourActuel()}
