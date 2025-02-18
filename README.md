# Gestionnaire de fichiers - Guide d'utilisation

## Installation

1. Clonez le dépôt du projet.
2. Utilisez la commande `make` pour compiler le programme.
3. Utilisez `make install` pour installer le programme dans `/usr/local/bin/`.

## Exemple d'utilisation

- **Excuter le programme : `./main`
- **Créer un fichier** : `create fichier1.txt 755 1024`
- **Supprimer un fichier** : `delete fichier1.txt`
- **Copier un fichier** : `copy fichier1.txt fichier3.txt`
- **Déplacer un fichier** : `move fichier2.txt fichier4.txt`
- **Modifier les permissions** : `chmod 777 fichier1.txt`

## Liste des fichiers

Vous pouvez lister tous les fichiers présents dans le système avec la commande `list`.

## Permissions

Les permissions suivent le format classique UNIX :

- 7 : Lecture, écriture et exécution (rwx)
- 6 : Lecture et écriture (rw-)
- 5 : Lecture et exécution (r-x)
- 4 : Lecture seule (r--)

Par exemple, `chmod 755 fichier1.txt` donne à `fichier1.txt` des permissions `rwxr-xr-x`.
