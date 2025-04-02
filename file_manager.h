#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

// Simulation du système de fichiers
#define MAX_FILES 100
#define MAX_PATH_LENGTH 256
#define MAX_NAME_LENGTH 50

typedef enum {
    FILE_TYPE,
    DIRECTORY_TYPE
} FileType;

// Définition des modes d'ouverture de fichier
#define FILE_MODE_READ  1
#define FILE_MODE_WRITE 2
#define FILE_MODE_BOTH  3
#define FS_FILENAME "filesystem.dat"

/**
 * @file file_manager.h
 * @brief Interface du système de fichiers virtuel
 *
 * Ce fichier définit l'interface publique du système de fichiers,
 * y compris les structures de données et les fonctions disponibles.
 * Le système implémente les fonctionnalités suivantes:
 * - Création et suppression de fichiers et répertoires
 * - Gestion des permissions
 * - Lecture et écriture de fichiers
 * - Navigation dans l'arborescence
 * - Liens durs et symboliques
 * - Persistance des données
 *
 * @author BAKHOUCHE Rachel|HUANG Yanmo|ANAGONOU Hervé
 * @date 2024
 */

/** @brief Nombre maximum de fichiers par répertoire */
#define MAX_FILES 100

/** @brief Longueur maximale d'un chemin */
#define MAX_PATH_LENGTH 256

/** @brief Longueur maximale d'un nom de fichier */
#define MAX_NAME_LENGTH 50

/**
 * @brief Types de nœuds dans le système de fichiers
 */
typedef enum {
    FILE_TYPE,      /**< Nœud de type fichier */
    DIRECTORY_TYPE  /**< Nœud de type répertoire */
} FileType;

/** @brief Mode lecture seule */
#define FILE_MODE_READ  1
/** @brief Mode écriture seule */
#define FILE_MODE_WRITE 2
/** @brief Mode lecture et écriture */
#define FILE_MODE_BOTH  3
/** @brief Nom du fichier de stockage persistant */
#define FS_FILENAME "filesystem.dat"

/**
 * @brief Structure représentant un nœud dans le système de fichiers
 *
 * Cette structure est utilisée pour représenter à la fois les fichiers et les répertoires.
 * Elle contient toutes les métadonnées nécessaires ainsi que les liens vers d'autres nœuds.
 */
typedef struct FileNode {
    char name[MAX_NAME_LENGTH];     /**< Nom du fichier ou répertoire */
    FileType type;                  /**< Type (FILE_TYPE ou DIRECTORY_TYPE) */
    int permissions;                /**< Permissions (format octal, ex: 644) */
    int size;                       /**< Taille du fichier en octets */
    struct FileNode* parent;        /**< Pointeur vers le répertoire parent */
    struct FileNode* children[MAX_FILES]; /**< Tableau des enfants (pour les répertoires) */
    int child_count;                /**< Nombre d'enfants dans le répertoire */
    char* content;                  /**< Contenu du fichier */
    int is_open;                    /**< État du fichier (1=ouvert, 0=fermé) */
    int ref_count;                  /**< Nombre de références (pour les liens durs) */
    char* symlink_target;           /**< Cible du lien symbolique */
    int open_mode;                  /**< Mode d'ouverture actuel */
} FileNode;

/** @brief Pointeur vers le répertoire racine du système */
extern FileNode* root_directory;

/** @brief Pointeur vers le répertoire de travail actuel */
extern FileNode* current_directory;

/**
 * @brief Crée un nouveau fichier
 * @param path Chemin du fichier à créer
 * @param permissions Permissions du fichier (format octal)
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int create_file(const char* path, int permissions);

/**
 * @brief Crée un nouveau répertoire
 * @param path Chemin du répertoire à créer
 * @param permissions Permissions du répertoire (format octal)
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int create_directory(const char* path, int permissions);

/**
 * @brief Liste le contenu d'un répertoire
 * @param path Chemin du répertoire à lister
 */
void list_files(const char* path);

/**
 * @brief Copie un fichier
 * @param source Chemin du fichier source
 * @param destination Chemin de destination
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int copy_file(const char* source, const char* destination);

/**
 * @brief Déplace un fichier
 * @param source Chemin du fichier source
 * @param destination Chemin de destination
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int move_file(const char* source, const char* destination);

/**
 * @brief Supprime un fichier ou répertoire
 * @param path Chemin de l'élément à supprimer
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int delete_file(const char* path);

/**
 * @brief Modifie les permissions d'un fichier
 * @param path Chemin du fichier
 * @param permissions Nouvelles permissions (format octal)
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int set_permissions(const char* path, int permissions);

/**
 * @brief Change le répertoire courant
 * @param path Chemin du nouveau répertoire courant
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int change_directory(const char* path);

/**
 * @brief Recherche un fichier par son chemin
 * @param path Chemin du fichier
 * @return Pointeur vers le nœud trouvé, NULL si non trouvé
 */
FileNode* get_file_by_path(const char* path);

/**
 * @brief Ouvre un fichier
 * @param path Chemin du fichier
 * @param mode Mode d'ouverture ("r"=lecture, "w"=écriture, "rw"=les deux)
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int open_file(const char* path, const char* mode);

/**
 * @brief Ferme un fichier
 * @param path Chemin du fichier
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int close_file(const char* path);

/**
 * @brief Lit le contenu d'un fichier
 * @param path Chemin du fichier
 * @param buffer Buffer pour stocker le contenu lu
 * @param size Taille du buffer
 * @return Nombre d'octets lus, -1 en cas d'erreur
 */
int read_file(const char* path, char* buffer, int size);

/**
 * @brief Écrit dans un fichier
 * @param path Chemin du fichier
 * @param content Contenu à écrire
 * @return Nombre d'octets écrits, -1 en cas d'erreur
 */
int write_file(const char* path, const char* content);

/**
 * @brief Crée un lien dur
 * @param target Chemin de la cible
 * @param link_name Nom du lien
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int create_hard_link(const char* target, const char* link_name);

/**
 * @brief Crée un lien symbolique
 * @param target Chemin de la cible
 * @param link_name Nom du lien
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int create_symbolic_link(const char* target, const char* link_name);

/**
 * @brief Initialise le système de fichiers
 */
void init_file_system();

/**
 * @brief Sauvegarde l'état du système de fichiers
 */
void save_file_system();

/**
 * @brief Ferme proprement le système de fichiers
 */
void close_file_system();

/**
 * @brief Charge l'état du système de fichiers
 * @return 0 en cas de succès, -1 en cas d'échec
 */
int load_file_system();

/**
 * @brief Gère l'interface en ligne de commande
 */
void handle_command();

/**
 * @brief Obtient le chemin absolu du répertoire courant
 * @return Chaîne de caractères représentant le chemin
 */
char* get_current_path();

#endif // FILE_MANAGER_H
