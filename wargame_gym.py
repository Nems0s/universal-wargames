import gymnasium as gym
from gymnasium import spaces
import numpy as np
import math # NOUVEAU
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), 'build'))
import wargame_env

class SpaceWargamesEnv(gym.Env):
    metadata = {"render_modes": ["console"]}

    def __init__(self, render_mode=None):
        super().__init__()
        self.render_mode = render_mode
        self.moteur = wargame_env.MoteurDeJeu()
        
        configs = wargame_env.GameConfigFiles()
        configs.rulesPath = "configs/pirate/config_rules.json"
        configs.ressourcesPath = "configs/pirate/config_ressources.json"
        configs.batimentsPath = "configs/pirate/config_batiments.json"
        configs.villesPath = "configs/pirate/config_villes.json"
        configs.winsPath = "configs/pirate/config_wins.json"
        configs.unitesPath = "configs/pirate/config_unites.json"
        configs.tuilesPath = "configs/pirate/config_tuiles.json"

        self.moteur.chargerConfiguration(configs)
        
        self.map_width = self.moteur.getLogicConfig().getPlateauX()
        self.map_height = self.moteur.getLogicConfig().getPlateauY()
        
        self.action_space = spaces.MultiDiscrete([15, self.map_width, self.map_height, self.map_width, self.map_height])
        
        self.observation_space = spaces.Box(
            low=-10.0, 
            high=100.0, 
            shape=(4, self.map_width, self.map_height), 
            dtype=np.float32
        )

        self.nb_tours_joues = 0
        self.noms_joueurs = ["IA_Agent", "Adversaire_Bot"]
        self.ancienne_distance = 999.0 # Pour le système de guidage
        self.compteur_erreurs = 0

    def set_model(self, model):
        """Permet de brancher le cerveau pour que l'ennemi s'en serve"""
        self.model = model

    def _get_obs_pour_joueur(self, pIdx):
        """Récupère la vision du plateau du point de vue de n'importe quel joueur"""
        flat_obs = self.moteur.get_state_ai(pIdx)
        return np.array(flat_obs, dtype=np.float32).reshape(4, self.map_width, self.map_height)

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        game_seed = seed if seed is not None else np.random.randint(0, 10000)
        self.moteur.initGame(game_seed, self.noms_joueurs, ["Humains", "Extraterrestres"])
        
        self.moteur.setActiveVictorySet(2) # Victoire par conquête
        self.nb_tours_joues = 0
        self.ancienne_distance = 999.0
        return self._get_obs(), self._get_info()

    def step(self, action):
        pIdx = 0 
        action_list = [int(a) for a in action]
        
        # --- ETAT AVANT ACTION ---
        old_obs = self._get_obs()
        old_allies = np.sum(old_obs[0])   
        old_enemies = np.sum(old_obs[1])  
        
        # --- EXECUTION ---
        code_resultat = self.moteur.step_ai(pIdx, action_list)
        
        # --- ETAT APRES ACTION ---
        new_obs = self._get_obs()
        new_allies = np.sum(new_obs[0])
        new_enemies = np.sum(new_obs[1])
        
        reward = 0.0
        
        # 1. RECOMPENSES DE BASE
        if code_resultat == 0:
            self.compteur_erreurs = 0
            if new_enemies < old_enemies: 
                reward += 10.0 # Tuer est très bien
            if new_allies > old_allies: 
                reward += 2.0  # Recruter est bien
                
            # 2. REWARD SHAPING : LE RADAR (Guidage vers l'ennemi)
            # On cherche les coordonnées (X, Y) de toutes les unités
            coords_allies = np.argwhere(new_obs[0] == 1.0)
            coords_enemies = np.argwhere(new_obs[1] == 1.0)
            
            if len(coords_allies) > 0 and len(coords_enemies) > 0:
                # Centre de gravité moyen des armées
                barycentre_allie = np.mean(coords_allies, axis=0)
                barycentre_ennemi = np.mean(coords_enemies, axis=0)
                
                # Distance euclidienne
                distance_actuelle = math.dist(barycentre_allie, barycentre_ennemi)
                
                # Si l'IA s'est rapprochée de l'ennemi, on la récompense !
                if distance_actuelle < self.ancienne_distance:
                    reward += 0.5
                elif distance_actuelle > self.ancienne_distance:
                    reward -= 0.2 # On pénalise la fuite
                    
                self.ancienne_distance = distance_actuelle
                
        elif code_resultat in [2, 3, 4, 5]: 
            reward -= 0.1 # Action invalide
            self.compteur_erreurs += 1
        
        # Si elle fait 30 actions invalides de suite, on la force à passer son tour.
        if self.compteur_erreurs > 30:
            action_list[0] = 14 # On force l'action "Fin de Tour"
            reward -= 2.0       # Grosse claque pour avoir gaspillé le temps
            self.compteur_erreurs = 0
            
        # 3. GESTION DES TOURS ET SELF-PLAY
        if action_list[0] == 14: # CmdFinTour
            self.nb_tours_joues += 1
            reward -= 0.5 # Le temps presse !
            self.compteur_erreurs = 0
            
            # --- LE SELF-PLAY (Tour de l'ennemi) ---
            while self.moteur.getCurrentPlayerTurn() != 0 and not self.moteur.isPartieTerminee():
                adversaire_idx = self.moteur.getCurrentPlayerTurn()
                
                # Si le cerveau est branché, l'ennemi l'utilise !
                if hasattr(self, 'model') and self.model is not None:
                    obs_ennemi = self._get_obs_pour_joueur(adversaire_idx)
                    action_ennemi, _ = self.model.predict(obs_ennemi, deterministic=True) 
                    
                    action_ennemi_list = [int(a) for a in action_ennemi]
                    code_ennemi = self.moteur.step_ai(adversaire_idx, action_ennemi_list)
                    
                    # SÉCURITÉ VITALE : Si l'ennemi génère une erreur ou passe son tour
                    if code_ennemi in [2, 3, 4, 5] or action_ennemi_list[0] == 14:
                        # On le force à terminer son tour pour éviter une boucle infinie
                        self.moteur.step_ai(adversaire_idx, [14, 0, 0, 0, 0])
                else:
                    # Sécurité si pas de modèle : l'ennemi passe son tour
                    self.moteur.step_ai(adversaire_idx, [14, 0, 0, 0, 0])

        # 4. CONDITIONS DE FIN
        terminated = self.moteur.isPartieTerminee()
        truncated = False
        
        if terminated:
            vainqueur = self.moteur.getNomVainqueur()
            if vainqueur == self.noms_joueurs[0]:
                reward += 1000.0
                print(f"\n[VICTOIRE] L'IA a écrasé l'ennemi en {self.nb_tours_joues} tours ! 🚀")
            else:
                reward -= 50.0   
                
        if self.nb_tours_joues > 150:
            truncated = True
            reward -= 20.0

        return new_obs, reward, terminated, truncated, self._get_info()

    def _get_obs(self):
        flat_obs = self.moteur.get_state_ai(0)
        np_obs = np.array(flat_obs, dtype=np.float32).reshape(4, self.map_width, self.map_height)
        return np_obs

    def _get_info(self):
        return {"tour": self.moteur.getTourActuel()}