#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "getnum.h"

/*====================================================================*/
/*========================CONSTANTES==================================*/
/*====================================================================*/

#define TRUE 1
#define FALSE !TRUE

#define VACIO '0'
#define BLOB_JUG_1 'A'
#define BLOB_JUG_2 'Z'

#define HUMANO 0
#define CPU 1

#define TAB_VACIO 1
#define TAB_JUEGO 2

/* Codigos para estado de juego. */
#define TERMINO_QUIT 1
#define TERMINO_GANO 2
#define NO_TERMINO 3

/* Codigos para ingresaJugada. */
#define JUGADA 1
#define GUARDAR 2
#define SALIR 3
#define SALIRGUARDAR 4
#define ERROR_LECTURA 5
#define ERROR_JUGADA 6

#define ERROR_COORDENADAS 10
#define ERROR_NOMBRE 11
#define ERROR_COMANDO 12
#define ERROR_VACIO 13

#define BLOQUE 5
#define BLOQUE2 10

#define MAX_DIM 100
#define MIN_DIM 5
#define MAX_LARGO_ENTRADA 30

#define ERROR_MEMORIA 0

/* Codigos de error para jugadas invalidas. */
#define ERROR_ORIGEN_AFUERA 1
#define ERROR_DESTINO_AFUERA 2
#define VALIDA 3
#define ERROR_ORIGEN_INVALIDO 4
#define ERROR_ORIGEN_VACIO 5
#define ERROR_DESTINO_OCUPADO 6
#define ERROR_DESTINO_LEJOS 7

#define NO_CARGO 0
#define CARGO 1

#define BORRAR_BUFFER while (getchar() != '\n')

#define IMPRIMIR_ERROR_MEMORIA fprintf(stderr, "\n\n::ERROR FATAL: NO HAY MEMORIA::\n\n")

#define inv(x) ((x + 1) % 2)

/*====================================================================*/
/*=====================ESTRUCTURAS/TIPOS==============================*/
/*====================================================================*/

typedef unsigned char blob;

typedef struct {
	int x;
	int y;
} punto;

typedef struct {
	int indice;
	int tipo;
	int cantBlobs;
	punto *blobs;
	char etiqueta;
} jugador;

typedef struct {
	blob **datos;
	int fils;
	int cols;
} tablero;

typedef struct {
	punto origen;
	punto destino;
} jugada;

/*====================================================================*/
/*=====================PROTOTIPOS DE BACKEND==========================*/
/*====================================================================*/

blob **crearTablero(tablero *tab1, size_t fils, size_t cols, int modo);
int finJuego(tablero *tablero1, jugador *player);
int calculaGanador(tablero *tab1, jugador jugadores[]);
int comprobarJugada(tablero *tab1, jugador player, jugada jug);
int moverBlobs(tablero *tab1, jugador *player, jugador *oponente, jugada jug);
jugada turnoCPU(tablero *tab1, jugador *player, jugador *oponente, int *errFlg);
void liberarTablero(tablero *tablero1);
void inicializarPuntos(jugador jugadores[], int fils, int cols);
int save(char *filename, jugador *player, jugador *oponente, tablero *tablero1);
int resumirJuego();
int agregarPunto(jugador *player, punto point);
