# Tetris MiniRISC Project

Ce projet est une implémentation du célèbre jeu Tetris, conçue spécifiquement pour fonctionner sur une architecture MiniRISC en utilisant le langage de programmation C. Il s'agit d'un projet académique ou d'apprentissage visant à comprendre et développer des applications pour des architectures RISC (« Reduced Instruction Set Computer ») et tester leur compatibilité avec l'émulateur Harvey.

---

## Prérequis

Avant de commencer, assurez-vous de disposer des éléments suivants :

1. **Langage C** : Une base solide en langage C est nécessaire pour comprendre et modifier le projet.
2. **MiniRISC** : Une architecture RISC simplifiée qui est simulée via un émulateur.
3. **Emulateur Harvey** : Outil requis pour exécuter et tester le programme sur un environnement simulé.

### Installation de l'émulateur Harvey

L'émulateur Harvey est nécessaire pour exécuter le projet sur un environnement MiniRISC. Voici les étapes pour l'installer :

1. **Cloner le répertoire Harvey** :
   ```bash
   git clone https://github.com/<organisation>/harvey.git
   ```

2. **Compiler l'émulateur** :
   ```bash
   cd harvey
   make
   ```

3. **Vérifier l'installation** :
   Exécutez une commande de test comme :
   ```bash
   ./harvey -help
   ```

---

## Structure du Projet

Le projet est organisé de la manière suivante :

```
.
├── FreeRTOS           # Source et configurations liées à FreeRTOS
├── minirisc           # Fichiers liés à l'architecture MiniRISC
├── support            # Bibliothèques de support
├── xprintf            # Utilitaires pour l'affichage et le débogage
├── .gitattributes     # Configuration des attributs Git
├── Makefile           # Script pour compiler le projet
├── README.md          # Documentation du projet
└── main.c             # Code source principal du projet
```

### Description des fichiers principaux

- **FreeRTOS** : Contient les fichiers nécessaires pour l'intégration de FreeRTOS dans le projet.
- **minirisc** : Regroupe les fichiers spécifiques à l'architecture MiniRISC utilisée.
- **support** : Fournit des fonctions utilitaires pour le projet.
- **xprintf** : Implémente des fonctions d'affichage formatées.
- **main.c** : Contient tout le code source du jeu Tetris, incluant la logique des pièces, la gestion du tableau de jeu et l'affichage.
- **Makefile** : Simplifie la compilation en une seule commande.

---

## Compilation et Exécution

1. **Compilation** :
   Assurez-vous d'être dans le répertoire racine du projet, puis exécutez :
   ```bash
   make
   ```

2. **Exécution avec Harvey** :
   L'émulateur Harvey exécutera le projet comme suit :
   ```bash
   ./harvey -run ./tetris.bin
   ```

3. **Nettoyage** :
   Pour supprimer les fichiers compilés :
   ```bash
   make clean
   ```

---

## Fonctionnalités

- **Logique Tetris Complète** :
  - Gestion des pièces en chute.
  - Rotation et placement.
  - Suppression des lignes complètes.
- **Compatible MiniRISC** :
  - Fonctionne de manière optimale sur une architecture MiniRISC.
- **Support Harvey** :
  - Testé et exécuté sur l'émulateur Harvey.

---
##Collaborateurs

-Ali Mantach.
-Shakib YOUSSEF.
-Mohamad Makke.

---

## Améliorations futures

- Ajout d'un mode multijoueur.
- Interface graphique plus riche.
- Optimisation pour d'autres architectures.

---

Si vous avez des questions ou des problèmes, n'hésitez pas à ouvrir une issue dans le dépôt GitHub.
