# Gestionnaire de fichiers - Guide d'utilisation

## Installation

1. Clonez le dépôt du projet.
2. Utilisez la commande `make` pour compiler le programme.
3. Utilisez `make install` pour installer le programme dans `/usr/local/bin/`.

## Exemple d'utilisation

- **Créer un fichier** : `./file_manager create fichier1.txt 755 1024`
- **Supprimer un fichier** : `./file_manager delete fichier1.txt`
- **Copier un fichier** : `./file_manager copy fichier1.txt fichier3.txt`
- **Déplacer un fichier** : `./file_manager move fichier2.txt fichier4.txt`
- **Modifier les permissions** : `./file_manager chmod 777 fichier1.txt`

## Liste des fichiers

Vous pouvez lister tous les fichiers présents dans le système avec la commande `./file_manager list`.

## Permissions

Les permissions suivent le format classique UNIX :

- 7 : Lecture, écriture et exécution (rwx)
- 6 : Lecture et écriture (rw-)
- 5 : Lecture et exécution (r-x)
- 4 : Lecture seule (r--)

Par exemple, `chmod 755 fichier1.txt` donne à `fichier1.txt` des permissions `rwxr-xr-x`.
