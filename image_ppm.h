#ifndef IMAGE_PPM_H
#define IMAGE_PPM_H

// ----------------------------------------------------------------------------
// Filename        : image_ppm.c
// Description     :
// Created On      : Tue Mar 31 13:26:36 2005
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define allocation_tableau(nom, type, nombre) \
if( (nom = (type*) calloc (nombre, sizeof(type) ) ) == NULL ) \
{\
 printf("\n Allocation dynamique impossible pour un pointeur-tableau \n");\
 exit(1);\
}

typedef unsigned char OCTET;

/*===========================================================================*/
void ignorer_commentaires(FILE * f);
/*===========================================================================*/


/*===========================================================================*/
void ecrire_image_ppm(const char  nom_image[], OCTET *pt_image, int nb_lignes, int nb_colonnes);
/*===========================================================================*/

/*===========================================================================*/		
void lire_nb_lignes_colonnes_image_ppm(const char nom_image[], int *nb_lignes, int *nb_colonnes);
/*===========================================================================*/
/*===========================================================================*/
void lire_image_ppm(const char  nom_image[], OCTET *pt_image, int taille_image);

/*===========================================================================*/
/*===========================================================================*/

void planR(OCTET *pt_image, OCTET *src, int taille_image);
/*===========================================================================*/
/*===========================================================================*/

void planV(OCTET *pt_image, OCTET *src, int taille_image);

/*===========================================================================*/
/*===========================================================================*/

void planB(OCTET *pt_image, OCTET *src, int taille_image);
   
/*===========================================================================*/   
/*===========================================================================*/

void ecrire_image_pgm(const char  nom_image[], const OCTET *pt_image, int nb_lignes, int nb_colonnes);
/*===========================================================================*/

void lire_nb_lignes_colonnes_image_pgm(const char nom_image[], int *nb_lignes, int *nb_colonnes);
/*===========================================================================*/
/*===========================================================================*/
void lire_image_pgm(const char  nom_image[], OCTET *pt_image, int taille_image);
/*===========================================================================*/
#endif /* IMAGE_PPM_H */
