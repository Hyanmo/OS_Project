/**
 * @file main.c
 * @brief Point d'entrée du système de fichiers virtuel
 *
 * Ce fichier contient la fonction principale qui lance
 * l'interface de commande du système de fichiers.
 *
 * @author BAKHOUCHE Rachel|HUANG Yanmo|ANAGONOU Hervé
 * @date 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include "file_manager.h"

/**
 * @brief Fonction principale du programme
 * 
 * @return int Code de retour (0 pour succès)
 * 
 * @details
 * - Initialise le système de fichiers
 * - Lance l'interface de commande interactive
 * - Gère la fermeture propre du système
 */
int main() {
    handle_command();
    return 0;
}
