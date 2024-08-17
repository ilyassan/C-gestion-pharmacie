#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h> 
#include <float.h> 
#include <ctype.h> 

#define TAUX_TVA 0.15
#define MAX_NOM 100


typedef struct {
    int code;
    char nom[MAX_NOM];
    int quantite;
    float prixHT;
} Produit;

typedef struct {
    int code;
    char nom[MAX_NOM];
    int quantite;
    float prixTTC;
    struct {
        int jour;
        int mois;
        int annee;
    } dateVente;
} Vente;

Produit *produits = NULL;
int nombreProduits = 0;
Vente *ventes = NULL;
int nombreVentes = 0;

const char *produitsFilename = "produits.dat";
const char *ventesFilename = "ventes.dat";

void afficherStatistiquesVentes();
void afficherListeProduitsOptions();

void listeProduits(Produit *trierProduits, int maxQuantite);
void acheterProduit(int code, const char *nom, int quantite, float prixHT);
void detailProduit(int code);
void venteProduit(int code, int quantite);
void supprimerProduit(int code);

void listeVentes();
void totalVentesAujourdHui();
void moyennePrixVentesAujourdHui();
void prixMaxMinVentesAujourdHui();
void produitLePlusVenduAujourdHui();

void ajouteProduitQuantite(int code, int quantite);
int chercheProduitParCode(int code);
int chercheVenteProduitParCode(int code);
void obtenirDateAujourdHui(int *jour, int *mois, int *annee);

void enregistrerProduits();
void enregistrerVentes();

Produit* copierProduitsArray();
Produit* trierProduitsParNom(int croissant);
Produit* trierProduitsParPrix(int croissant);

void nettoyageEtQuitter(int signal);
void chargerDonnees();
void endProgram();

void line();
void hashLine();

// ---------- Menu principal (des produits) ----------

int main(){
    chargerDonnees();
    signal(SIGINT, nettoyageEtQuitter);

    int running = 1;
    int choix;
    int index;
    int code, quantite;
    float prixHT;
    char nom[MAX_NOM];
    
    while (running != 0) {
        line();
        puts("Menu Principal:");
        puts("1. Liste des produits");
        puts("2. Ajouter un produit");
        puts("3. Détails d'un produit");
        puts("4. Vendre un produit");
        puts("5. Supprimer un produit");
        puts("6. Ventes statistics");
        puts("7. Quitter");
        line();
        printf("Entrez votre choix: ");
        scanf("%d", &choix);
        
        hashLine();

        switch (choix) {
        case 1:
            afficherListeProduitsOptions();
            continue;

        case 2:
            // Obtenir le code
            printf("Entrer le code du produit: ");
            if (scanf("%d", &code) != 1) {
                printf("Erreur: Code invalide.\n");
                while (getchar() != '\n');
                break;
            }

            index = chercheProduitParCode(code);
            if (index != -1) // Le produit existe déjà
            {
                // Obtenir la quantité à ajouter
                printf("Entrer la quantité à ajouter: ");
                if (scanf("%d", &quantite) != 1 || quantite <= 0) {
                    printf("Erreur: Quantité invalide.\n");
                    while (getchar() != '\n');
                    break;
                }
                ajouteProduitQuantite(index, quantite);
                break;
            }

            // Obtenir le nom
            printf("Entrer le nom du produit: ");
            getchar(); // Éviter le \n
            if (fgets(nom, sizeof(nom), stdin) == NULL) {
                printf("Erreur: Lecture du nom échouée.\n");
                break;
            }
            for (char *p = nom; *p; *p = toupper(*p), p++); // MAJUSCULE
            nom[strcspn(nom, "\n")] = '\0';

            // Obtenir la quantité
            printf("Entrer la quantité du produit: ");
            if (scanf("%d", &quantite) != 1 || quantite <= 0) {
                printf("Erreur: Quantité invalide.\n");
                while (getchar() != '\n');
                break;
            }

            // Obtenir le prix hors taxe
            printf("Entrer le prix (Hors Taxe) du produit: ");
            if (scanf("%f", &prixHT) != 1 || prixHT < 0) {
                printf("Erreur: Prix invalide.\n");
                while (getchar() != '\n');
                break;
            }
            
            acheterProduit(code, nom, quantite, prixHT);
            break;

        case 3:
            printf("Entrer le code du produit: ");
            if (scanf("%d", &code) != 1) {
                printf("Erreur: Code invalide.\n");
                while (getchar() != '\n');
                break;
            }

            detailProduit(code);
            break;

        case 4:
            printf("Entrer le code du produit: ");
            if (scanf("%d", &code) != 1) {
                printf("Erreur: Code invalide.\n");
                while (getchar() != '\n');
                break;
            }

            printf("Entrer la quantité du produit: ");
            if (scanf("%d", &quantite) != 1 || quantite <= 0) {
                printf("Erreur: Quantité invalide.\n");
                while (getchar() != '\n');
                break;
            }

            venteProduit(code, quantite);
            break;

        case 5:
            printf("Entrer le code du produit: ");
            if (scanf("%d", &code) != 1) {
                printf("Erreur: Code invalide.\n");
                while (getchar() != '\n');
                break;
            }

            supprimerProduit(code);
            break;

        case 6:
            afficherStatistiquesVentes();
            continue;

        case 7:
            running = 0;
            break;

        default:
            puts("Choix invalide.");
            continue;
        }

        // Retour au menu principal
        choix = 0;
        while (choix != 1 && running != 0) {
            hashLine();
            puts("1. Retour");
            printf("Entrez votre choix: ");
            scanf("%d", &choix);
        }
    }

    endProgram();

    puts("Au revoir!\n");

    return 0;
}

void acheterProduit(int code, const char *nom, int quantite, float prixHT) {
    if (code < 0 || strlen(nom) < 1 || quantite < 1 || prixHT < 1) {
        puts("Données invalides, impossible de créer le produit");
        return;
    }
    // Utilisation d'une variable temporaire pour éviter la perte de données
    Produit *temp = (Produit*) realloc(produits, (nombreProduits + 1) * sizeof(Produit));

    if (temp == NULL) {
        puts("Problème lors de l'ajout du produit");
        return;
    }
    produits = temp;
    
    produits[nombreProduits].code = code;
    strcpy(produits[nombreProduits].nom, nom);
    produits[nombreProduits].quantite = quantite;
    produits[nombreProduits].prixHT = prixHT;

    nombreProduits++;

    puts("Produit ajouté avec succès.");
}

void ajouteProduitQuantite(int index, int quantite) {
    produits[index].quantite += quantite;
    puts("Quantité ajouté avec succès.");
}

void detailProduit(int code){
    if (code < 1)
    {
        puts("\nCode invalides");
        return;
    }

    int index = chercheProduitParCode(code);
    
    if (index == -1) // Non trouvé
    {
        printf("\nNo trouvé un produit avec code %d\n", code);
        return;
    }

    puts("\nDetails de produit:");
    printf("\tCode: %d\n", code);
    printf("\tNom: %s\n", produits[index].nom);
    printf("\tQuantité: %d\n", produits[index].quantite);
    printf("\tPrix (Hors Taxe): %.2f\n", produits[index].prixHT);
    printf("\tPrix Total (HT): %.2f\n", produits[index].quantite * produits[index].prixHT);
    printf("\tPrix (TTC): %.2f\n", produits[index].prixHT * (1 + TAUX_TVA));
    printf("\tPrix Total (TTC): %.2f\n", produits[index].quantite * produits[index].prixHT * (1 + TAUX_TVA));
}

void venteProduit(int code, int quantite) {
    if (code < 1) {
        puts("\nCode invalide");
        return;
    }
    if (quantite <= 0) {
        puts("\nQuantité invalide");
        return;
    }

    int index = chercheProduitParCode(code);

    if (index == -1) {
        puts("\nProduit non trouvé");
        return;
    }

    if (produits[index].quantite < quantite) {
        puts("\nStock insuffisant");
        return;
    }
    
    produits[index].quantite -= quantite;

    if (produits[index].quantite == 0)
    {
        supprimerProduit(code);
    }
    
    // Utilisation d'une variable temporaire pour éviter la perte de données
    Vente *temp = (Vente*) realloc(ventes, (nombreVentes + 1) * sizeof(Vente));

    if (temp == NULL) {
        puts("Problème lors de vente du produit");
        return;
    }
    ventes = temp;

    ventes[nombreVentes].code = code;
    strcpy(ventes[nombreVentes].nom, produits[index].nom);
    ventes[nombreVentes].quantite = quantite;
    ventes[nombreVentes].prixTTC = produits[index].prixHT * (1 + TAUX_TVA);

    int jour, mois, annee;
    obtenirDateAujourdHui(&jour, &mois, &annee);

    ventes[nombreVentes].dateVente.jour = jour;
    ventes[nombreVentes].dateVente.mois = mois;
    ventes[nombreVentes].dateVente.annee = annee;

    nombreVentes++;

    printf("%d produit(s) vendu(s) avec succès.\n", quantite);
}

void supprimerProduit(int code){
    if (code < 1)
    {
        puts("\nCode invalides");
        return;
    }

    int index = chercheProduitParCode(code);
    
    if (index == -1) // Non trouvé
    {
        printf("\nNo trouvé un produit avec code %d\n", code);
        return;
    }

    // Déplace les autres produits à la place du produit supprimé
    for (int i = index; i < nombreProduits - 1; i++)
    {
        produits[i] = produits[i + 1];
    }

    // Réduire le nombre de produits
    nombreProduits--;

    // Utilisation d'une variable temporaire pour éviter la perte de données
    Produit *temp = (Produit*) realloc(produits, nombreProduits * sizeof(Produit));

    if (temp == NULL && nombreProduits > 0) {
        puts("Problème lors de supprimer le produit");
        return;
    }
    produits = temp;
    
    puts("Produit supprimé avec succès.");
}

// ---------- Menu secondaire (liste des produits) ----------

void afficherListeProduitsOptions() {
    int choix, maxQuantite;

    while (1) {
        line();
        puts("Lister les produits:");
        puts("1. Selon l'ordre de creation");
        puts("2. Selon l'ordre alphabetique (croissante)");
        puts("3. Selon l'ordre alphabetique (décroissante)");
        puts("4. Selon l'ordre du prix (croissante)");
        puts("5. Selon l'ordre du prix (décroissante)");
        puts("6. Selon la quantite inferieur à (x)");
        puts("7. Retour au menu principal");
        line();
        printf("Entrez votre choix: ");
        scanf("%d", &choix);

        hashLine();

        switch (choix) {
            case 1:
                listeProduits(produits, -1);
                break;
            case 2:
                listeProduits(trierProduitsParNom(1), -1); // croissant
                break;
            case 3:
                listeProduits(trierProduitsParNom(0), -1); // décroissant
                break;
            case 4:
                listeProduits(trierProduitsParPrix(1), -1); // croissant
                break;
            case 5:
                listeProduits(trierProduitsParPrix(0), -1); // décroissant
                break;
            case 6:
                printf("Entrer la max quantite pour filtrer les produits: ");
                if (scanf("%d", &maxQuantite) != 1 || maxQuantite < 0) {
                    printf("Erreur: Quantité invalide.\n");
                    while (getchar() != '\n');
                    break;
                }
                listeProduits(produits, maxQuantite); // décroissant
                break;
            case 7:
                return; // Retourne au menu principal
            default:
                puts("Choix invalide.");
                continue;
        }

        // Retour au menu principal
        choix = 0;
        while (choix != 1) {
            hashLine();
            puts("1. Retour");
            printf("Entrez votre choix: ");
            scanf("%d", &choix);
        }
    }
}

void listeProduits(Produit *listeProduits, int maxQuantite) {
    puts("Liste des produits:\n");

    if (nombreProduits == 0 || maxQuantite == 0) {
        puts("\tAucun produit à afficher.");
        return;
    }

    // si le tableau est trié par nom ou par prix
    if (listeProduits != produits)
    {
        for (int i = 0; i < nombreProduits; i++) {
            printf("\t%-4d => code: %-6d / nom: %-12s / quantité: %-2d / prix TTC: %6.2f\n",
                i + 1, listeProduits[i].code, listeProduits[i].nom, listeProduits[i].quantite, listeProduits[i].prixHT * (1 + TAUX_TVA)
            );
        }
        free(listeProduits);
        return;
    }

    if (maxQuantite > 0)
    {
        int nombreProduitsAffichier = 0;
       for (int i = 0; i < nombreProduits; i++) {
            if (listeProduits[i].quantite <= maxQuantite) {
                nombreProduitsAffichier++;
                printf("\t%-4d => code: %-6d / nom: %-12s / quantité: %-2d / prix TTC: %6.2f\n",
                    nombreProduitsAffichier, listeProduits[i].code, listeProduits[i].nom, listeProduits[i].quantite, listeProduits[i].prixHT * (1 + TAUX_TVA)
                );
            }
        }
        
        if (nombreProduitsAffichier == 0) {
            printf("\tAucun produit avec une quantité inférieure ou égale à %d.\n", maxQuantite);
        }
        return;
    }
    

    // le tableau est déjà trié par date de création 
    for (int i = nombreProduits - 1; i >= 0; i--) {
        printf("\t%-4d => code: %-6d / nom: %-12s / quantité: %-2d / prix TTC: %6.2f\n",
            nombreProduits - i, listeProduits[i].code, listeProduits[i].nom, listeProduits[i].quantite, listeProduits[i].prixHT * (1 + TAUX_TVA)
        );
    }
}

// ---------- Menu secondaire (des ventes) ----------

void afficherStatistiquesVentes() {
    int choixStatistiques;

    while (1) {
        line();
        puts("Menu Statistiques de Vente:");
        puts("1. Liste des ventes");
        puts("2. Total des ventes du jour");
        puts("3. Moyenne des prix de ventes du jour");
        puts("4. Prix max et min des ventes du jour");
        puts("5. Produit le plus vendu aujourd'hui");
        puts("6. Retour au menu principal");
        line();
        printf("Entrez votre choix: ");
        scanf("%d", &choixStatistiques);

        hashLine();

        switch (choixStatistiques) {
            case 1:
                listeVentes();
                break;
            case 2:
                totalVentesAujourdHui();
                break;
            case 3:
                moyennePrixVentesAujourdHui();
                break;
            case 4:
                prixMaxMinVentesAujourdHui();
                break;
            case 5:
                produitLePlusVenduAujourdHui();
                break;
            case 6:
                return; // Retourne au menu principal
            default:
                puts("Choix invalide.");
                continue;
        }

        // Retour au menu principal
        choixStatistiques = 0;
        while (choixStatistiques != 1) {
            hashLine();
            puts("1. Retour");
            printf("Entrez votre choix: ");
            scanf("%d", &choixStatistiques);
        }
    }
}

void listeVentes(){
    puts("Liste des ventes:\n");

    if (nombreProduits == 0) {
        puts("\tAucune vente à afficher.");
        return;
    }
    for (int i = 0; i < nombreVentes; i++) {
        printf(
            "\t%d => code: %d / nom: %s / quantité: %d / prix TTC: %.2f / date de vente: %d/%d/%d \n",
            i + 1, ventes[i].code, ventes[i].nom, ventes[i].quantite, ventes[i].prixTTC,
            ventes[i].dateVente.mois, ventes[i].dateVente.jour, ventes[i].dateVente.annee
        );
    }
}

void totalVentesAujourdHui(){
    int aujourdHuiJour, aujourdHuiMois, aujourdHuiAnnee;
    obtenirDateAujourdHui(&aujourdHuiJour, &aujourdHuiMois, &aujourdHuiAnnee);

    int aujourdHuiVentes = 0;

    for (int i = 0; i < nombreVentes; i++) {
        if (ventes[i].dateVente.jour == aujourdHuiJour &&
            ventes[i].dateVente.mois == aujourdHuiMois &&
            ventes[i].dateVente.annee == aujourdHuiAnnee) {
                aujourdHuiVentes++;
        }
    }

    int codesProduits[aujourdHuiVentes];

    int nombreProduitsDistincts = 0;
    int quantiteSomme = 0;
    float prixVenteTotalTTC = 0;

    for (int i = 0; i < nombreVentes; i++) {
        if (ventes[i].dateVente.jour == aujourdHuiJour &&
            ventes[i].dateVente.mois == aujourdHuiMois &&
            ventes[i].dateVente.annee == aujourdHuiAnnee) {

            // ### Concerne le nombre des produits distincts vendu aujourd'hui ###

            int trouve = 0;
            // Vérifier si le produit a déjà été compté
            for (int j = 0; j < nombreProduitsDistincts; j++) {
                if (codesProduits[j] == ventes[i].code) {
                    trouve = 1;
                    break;
                }
            }
            if (!trouve) {
                // Ajouter le code produit à la liste des produits distincts
                codesProduits[nombreProduitsDistincts] = ventes[i].code;
                nombreProduitsDistincts++;
            }

            // ### Concerne la somme de quantité des produits vendu aujourd'hui ###
            quantiteSomme += ventes[i].quantite;

            // ### Concerne la somme de prix de vente total aujourd'hui ###
            prixVenteTotalTTC += (ventes[i].quantite * ventes[i].prixTTC);
        }
    }


    printf("\n\tNombre de type des produits vendus aujourd'hui : %d\n", nombreProduitsDistincts);
    printf("\tNombre des produits vendus aujourd'hui : %d\n", quantiteSomme);
    printf("\tLe prix de vente total TTC des produits aujourd'hui : %.2f\n", prixVenteTotalTTC);
}

void moyennePrixVentesAujourdHui(){
    int aujourdHuiJour, aujourdHuiMois, aujourdHuiAnnee;
    obtenirDateAujourdHui(&aujourdHuiJour, &aujourdHuiMois, &aujourdHuiAnnee);

    float prixVenteTotalTTC = 0;
    int quantiteSomme = 0;
    for (int i = 0; i < nombreVentes; i++)
    {
        if (ventes[i].dateVente.jour == aujourdHuiJour &&
            ventes[i].dateVente.mois == aujourdHuiMois &&
            ventes[i].dateVente.annee == aujourdHuiAnnee) {
                prixVenteTotalTTC += (ventes[i].prixTTC * ventes[i].quantite);
                quantiteSomme += ventes[i].quantite;
        }   
    }

   if (quantiteSomme > 0) {
        float moyenne = prixVenteTotalTTC / quantiteSomme;
        printf("\n\tLe prix moyen des ventes aujourd'hui : %.2f\n", moyenne);
    } else {
        printf("\tAucune vente aujourd'hui pour calculer la moyenne.\n");
    }
}

void prixMaxMinVentesAujourdHui(){
    int aujourdHuiJour, aujourdHuiMois, aujourdHuiAnnee;
    obtenirDateAujourdHui(&aujourdHuiJour, &aujourdHuiMois, &aujourdHuiAnnee);

    float maxPrix = 0;
    float minPrix = FLT_MAX;
    float maxPrixQuantite = 0;
    float minPrixQuantite = FLT_MAX;

    // Iterate through the sales records
    for (int i = 0; i < nombreVentes; i++) {
        if (ventes[i].dateVente.jour == aujourdHuiJour &&
            ventes[i].dateVente.mois == aujourdHuiMois &&
            ventes[i].dateVente.annee == aujourdHuiAnnee) {

            if (ventes[i].prixTTC > maxPrix) {
                maxPrix = ventes[i].prixTTC;
            }

            if (ventes[i].prixTTC < minPrix) {
                minPrix = ventes[i].prixTTC;
            }

            if (ventes[i].prixTTC * ventes[i].quantite > maxPrixQuantite) {
                maxPrixQuantite = ventes[i].prixTTC * ventes[i].quantite;
            }

            if (ventes[i].prixTTC * ventes[i].quantite < minPrixQuantite) {
                minPrixQuantite = ventes[i].prixTTC * ventes[i].quantite;
            }
        }
    }

    if (maxPrix > 0 || minPrix < __FLT_MAX__) {
        printf("\n\tLe prix maximum des produits vendus aujourd'hui : %.2f\n", maxPrix);
        printf("\tLe prix minimum des produits vendus aujourd'hui : %.2f\n", minPrix);
        printf("\tLe prix maximum en fonction de la quantité vendue aujourd'hui : %.2f\n", maxPrixQuantite);
        printf("\tLe prix minimum en fonction de la quantité vendue aujourd'hui : %.2f\n", minPrixQuantite);
    } else {
        printf("\n\tAucune vente aujourd'hui pour déterminer les prix.\n");
    }

}

void produitLePlusVenduAujourdHui() {
    int aujourdHuiJour, aujourdHuiMois, aujourdHuiAnnee;
    obtenirDateAujourdHui(&aujourdHuiJour, &aujourdHuiMois, &aujourdHuiAnnee);

    int aujourdHuiVentes = 0;

    for (int i = 0; i < nombreVentes; i++) {
        if (ventes[i].dateVente.jour == aujourdHuiJour &&
            ventes[i].dateVente.mois == aujourdHuiMois &&
            ventes[i].dateVente.annee == aujourdHuiAnnee) {
                aujourdHuiVentes++;
        }
    }

    int quantitesProduits[aujourdHuiVentes];
    memset(quantitesProduits, 0, sizeof(quantitesProduits)); // Initialisation à zéro
    int codesProduits[aujourdHuiVentes];
    int nombreProduitsDistincts = 0;


    for (int i = 0; i < nombreVentes; i++) {
        if (ventes[i].dateVente.jour == aujourdHuiJour &&
            ventes[i].dateVente.mois == aujourdHuiMois &&
            ventes[i].dateVente.annee == aujourdHuiAnnee) {

            int produitTrouve = 0;
            
            for (int j = 0; j < nombreProduitsDistincts; j++) {
                if (codesProduits[j] == ventes[i].code) {
                    quantitesProduits[j] += ventes[i].quantite;
                    produitTrouve = 1;
                    break;
                }
            }

            if (!produitTrouve) {
                codesProduits[nombreProduitsDistincts] = ventes[i].code;
                quantitesProduits[nombreProduitsDistincts] = ventes[i].quantite;
                nombreProduitsDistincts++;
            }
        }
    }

    int produitLePlusVendu = -1;
    int quantiteMax = 0;

    for (int i = 0; i < nombreProduitsDistincts; i++) {
        if (quantitesProduits[i] > quantiteMax) {
            quantiteMax = quantitesProduits[i];
            produitLePlusVendu = codesProduits[i];
        }
    }


    if (produitLePlusVendu != -1) {
        int index = chercheVenteProduitParCode(produitLePlusVendu);

        puts("Le produit le plus vendu aujourd'hui:");
        printf("\tCode: %d\n", ventes[index].code);
        printf("\tNom: %s\n", ventes[index].nom);
        printf("\tTotal Quantité Vendu: %d\n", quantiteMax);
        printf("\tTotal Prix Vendu: %.2f\n", quantiteMax * ventes[index].prixTTC);
    } else {
        printf("\tAucune vente aujourd'hui pour déterminer le produit le plus vendu.\n");
    }
}


// ---------- Charger data depuis les fichiers ----------

void chargerProduits() {
    FILE *file = fopen(produitsFilename, "r");
    if (file == NULL) {
        return;
    }

    fscanf(file, "%d", &nombreProduits);

    produits = (Produit*)malloc(nombreProduits * sizeof(Produit));
    if (produits == NULL) {
        fclose(file);
        return;
    }

    for (int i = 0; i < nombreProduits; i++) {
        int code, quantite;
        float prixHT;
        char nom[MAX_NOM];

        if (fscanf(file, "%d", &code) != 1) break;
        fgetc(file);
        
        if (fgets(nom, sizeof(nom), file) == NULL) break;
        nom[strcspn(nom, "\n")] = '\0';
        

        if (fscanf(file, "%d", &quantite) != 1) break;
        fgetc(file);
        
        if (fscanf(file, "%f", &prixHT) != 1) break;
        fgetc(file);
        

        produits[i].code = code;
        strcpy(produits[i].nom, nom);
        produits[i].quantite = quantite;
        produits[i].prixHT = prixHT;
    }
    
    fclose(file);
    puts("Produits chargés avec succès.");
}

void chargerVentes() {
    FILE *file = fopen(ventesFilename, "r");
    if (file == NULL) {
        return;
    }

    fscanf(file, "%d", &nombreVentes);

    ventes = (Vente*)malloc(nombreVentes * sizeof(Vente));
    if (ventes == NULL) {
        fclose(file);
        return;
    }

    for (int i = 0; i < nombreVentes; i++) {
        int code, quantite, jour, mois, annee;
        float prixTTC;
        char nom[MAX_NOM];

        if (fscanf(file, "%d", &code) != 1) break;
        fgetc(file);
        
        if (fgets(nom, sizeof(nom), file) == NULL) break;
        nom[strcspn(nom, "\n")] = '\0';
        

        if (fscanf(file, "%d", &quantite) != 1) break;
        fgetc(file);
        
        if (fscanf(file, "%f", &prixTTC) != 1) break;
        fgetc(file);
        
        if (fscanf(file, "%d", &jour) != 1) break;
        fgetc(file);

        if (fscanf(file, "%d", &mois) != 1) break;
        fgetc(file);

        if (fscanf(file, "%d", &annee) != 1) break;
        fgetc(file);


        ventes[i].code = code;
        strcpy(ventes[i].nom, nom);
        ventes[i].quantite = quantite;
        ventes[i].prixTTC = prixTTC;
        ventes[i].dateVente.jour = jour;
        ventes[i].dateVente.mois = mois;
        ventes[i].dateVente.annee = annee;
    }
    
    fclose(file);
    puts("Ventes chargés avec succès.");
}

// ---------- Enregistrées data dans les fichiers ----------

void enregistrerProduits() {
    FILE *file = fopen(produitsFilename, "w");
    if (file == NULL) {
        puts("Échec de l'enregistrement des produits !!");
        return;
    }

    fprintf(file, "%d\n", nombreProduits);

    for (int i = 0; i < nombreProduits; i++) {
        fprintf(file, "%d\n", produits[i].code);
        fprintf(file, "%s\n", produits[i].nom);
        fprintf(file, "%d\n", produits[i].quantite);
        fprintf(file, "%.2f\n", produits[i].prixHT);
    }

    fclose(file);
}

void enregistrerVentes(){
    FILE *file = fopen(ventesFilename, "w");
    if (file == NULL) {
        puts("Échec de l'enregistrement des ventes !!");
        return;
    }

    fprintf(file, "%d\n", nombreVentes);

    for (int i = 0; i < nombreVentes; i++) {
        fprintf(file, "%d\n", ventes[i].code);
        fprintf(file, "%s\n", ventes[i].nom);
        fprintf(file, "%d\n", ventes[i].quantite);
        fprintf(file, "%.2f\n", ventes[i].prixTTC);
        fprintf(file, "%d\n", ventes[i].dateVente.jour);
        fprintf(file, "%d\n", ventes[i].dateVente.mois);
        fprintf(file, "%d\n", ventes[i].dateVente.annee);
    }

    fclose(file);
}

// ---------- Trier les produits ----------

Produit* trierProduitsParNom(int croissant){

    Produit *trierProduits = copierProduitsArray();

    for (int i = 0; i < nombreProduits; i++)
    {
        for (int j = i + 1; j < nombreProduits; j++)
        {
            int cmp = strcmp(trierProduits[i].nom, trierProduits[j].nom);
            if ( /* croissant */ (croissant == 1 && cmp > 0) ||  /* déroissant */ (croissant == 0 && cmp < 0)) {
                Produit temp = trierProduits[i];
                trierProduits[i] = trierProduits[j];
                trierProduits[j] = temp;
            }
        }
    }
    
    return trierProduits;
}

Produit* trierProduitsParPrix(int croissant){

    Produit *trierProduits = copierProduitsArray();

    for (int i = 0; i < nombreProduits; i++)
    {
        for (int j = i + 1; j < nombreProduits; j++)
        {
            if ( /* croissant */ (croissant == 1 && trierProduits[i].prixHT > trierProduits[j].prixHT) ||
                 /* déroissant */ (croissant == 0 && trierProduits[i].prixHT < trierProduits[j].prixHT)) {
                Produit temp = trierProduits[i];
                trierProduits[i] = trierProduits[j];
                trierProduits[j] = temp;
            }
        }
    }
    
    return trierProduits;
}

// ---------- Auxiliaires ----------

int chercheProduitParCode(int code) {
    if (nombreProduits == 0) return -1;

    for (int i = 0; i < nombreProduits; i++) {
        if (produits[i].code == code) {
            return i; // Trouvé
        }
    }
    
    return -1; // Non trouvé
}

int chercheVenteProduitParCode(int code) {
    if (nombreVentes == 0) return -1;

    for (int i = 0; i < nombreVentes; i++) {
        if (ventes[i].code == code) {
            return i; // Trouvé
        }
    }
    
    return -1; // Non trouvé
}

void obtenirDateAujourdHui(int *jour, int *mois, int *annee) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    *jour = tm_info->tm_mday;
    *mois = tm_info->tm_mon + 1;
    *annee = tm_info->tm_year + 1900;
}

Produit* copierProduitsArray() {
    Produit *copy = (Produit *)malloc(nombreProduits * sizeof(Produit));
    if (copy == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    memcpy(copy, produits, nombreProduits * sizeof(Produit));
    return copy;
}

void nettoyageEtQuitter(int signal) {
    // Effectuer les opérations de nettoyage
    endProgram();

    puts("\nProgramme interrompu. Tâches enregistrées.");
    exit(0);
}

void chargerDonnees(){
    chargerProduits();
    chargerVentes();
}

void endProgram(){
    enregistrerProduits();
    enregistrerVentes();
    free(produits);
    free(ventes);
}

void line() {
    printf("\n-----------------------\n");
}
void hashLine(){
    printf("\n#######################\n");
}
