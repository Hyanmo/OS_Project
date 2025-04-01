# Guide d'utilisation du Gestionnaire de Fichiers

## Compilation et Exécution

1. Compilez le programme avec la commande `make`
2. Exécutez le programme avec `./file_manager`

## Commandes Disponibles

1. **Créer un fichier**
   - Commande : `create nom_fichier permissions`
   - Exemple : `create test.txt 644`

2. **Créer un répertoire**
   - Commande : `mkdir nom_repertoire permissions`
   - Exemple : `mkdir documents 755`

3. **Lister les fichiers**
   - Commande : `ls [chemin]`
   - Exemple : `ls` `ls /documents`

4. **Changer de répertoire**
   - Commande : `cd chemin`
   - Exemples :
     - `cd documents` (accéder au sous-répertoire)
     - `cd ..` (retourner au répertoire parent)
     - `cd /` (retourner à la racine)

5. **Copier un fichier**
   - Commande : `copy source destination`
   - Exemple : `copy test.txt copie.txt`

6. **Déplacer un fichier**
   - Commande : `move source destination`
   - Exemple : `move test.txt documents/test.txt`

7. **Supprimer un fichier**
   - Commande : `rm [-r] chemin_fichier`
   - Exemple : `rm test.txt` `rm -r documents`

8. **Modifier les permissions**
   - Commande : `chmod nom_fichier permissions`
   - Exemple : `chmod test.txt 644`

9. **Ouvrir un fichier**
   - Commande : `open nom_fichier mode`
   - Mode : "r" (lecture) ou "w" (écriture)
   - Exemple : `open test.txt r`

10. **Fermer un fichier**
    - Commande : `close nom_fichier`
    - Exemple : `close test.txt`

11. **Lire un fichier**
    - Commande : `read nom_fichier`
    - Exemple : `read test.txt`

12. **Écrire dans un fichier**
    - Commande : `write nom_fichier contenu`
    - Exemple : `write test.txt “Hello, World!”`

13. **Créer un lien dur**
    - Commande : `ln source nom_lien`
    - Exemple : `ln test.txt lien_test`

14. **Créer un lien symbolique**
    - Commande : `ln -s source nom_lien`
    - Exemple : `ln -s test.txt lien_symb_test`

15. **Quitter le programme**
    - Commande : `exit`

## Système de Permissions

Les permissions suivent le format UNIX standard avec trois chiffres :
- 7 (rwx) : Lecture, écriture et exécution
- 6 (rw-) : Lecture et écriture
- 5 (r-x) : Lecture et exécution
- 4 (r--) : Lecture seule
- 3 (-wx) : Écriture et exécution
- 2 (-w-) : Écriture seule
- 1 (--x) : Exécution seule
- 0 (---) : Aucune permission

Permissions courantes :
- 755 : Propriétaire (rwx), Autres (r-x)
- 644 : Propriétaire (rw-), Autres (r--)
- 777 : Tous les droits pour tous
