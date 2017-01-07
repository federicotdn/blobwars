#include "blobBack.h"

static int intentarMover(tablero *tab1, jugador *player, jugada *jugMueve, int dist);
static int puedeMoverse(tablero *tab1, punto origen, jugada *jugadaFinal, int dist);
static int intentarCapturar(tablero *tab1, jugador *player, jugador *oponente, jugada *jugCaptura, int *errFlag);
static jugada jugadaEnMapa(tablero *tab1, tablero *mapa, jugador *player, int max, int cantMax, int *dim, int *errFlag);
static jugada *calculaJugadas(tablero *tab1, jugador *player, jugada *totalJugadas, punto destino, int *dimTotalJugadas);
static int llenarMapa(tablero *mapa, tablero *tab1, jugador *oponente, int cantNum[]);
static void agregarPuntoMapa(tablero *mapa, tablero *tab1, punto lugar, int *max, int cantNum[]);
static int convertirEnemigos(tablero *tab1, jugador *player, jugador *oponente, punto destino);
static int sonVecinos(tablero *tab1, jugada jug, size_t radio);
static int sacarPunto(jugador *player, punto point);
static int cmppuntos(punto a, punto b);

blob **crearTablero(tablero *tab1, size_t fils, size_t cols, int modo)
{
	/* Recibe un puntero a un tablero, ancho, alto y un modo.  Reserva
	 * espacio para el campo "datos" del tablero (donde se almacenan 
	 * los blobs) y asigna el numero de filas y columnas.  Dependiendo 
	 * del modo (juego o vacio) llena el tablero con blobs o con ceros. */
	
	int i, j;
	blob **datos;
	
	tab1->fils = fils;
	tab1->cols = cols;
	
	datos = malloc(fils * sizeof(*datos));
	
	if(datos == NULL)
		return NULL;
		
	for (i = 0; i < fils; i++)
	{	
		datos[i] = malloc(cols * sizeof(**datos));
		
		if (datos[i] == NULL)
			return NULL;
			
		for (j = 0; j < cols; j++)
		{
			if (modo == TAB_JUEGO)
				datos[i][j] = VACIO;
			else if (modo == TAB_VACIO)
				datos[i][j] = 0;
		}
	}
	
	if (modo == TAB_JUEGO)
	{
		datos[0][0] = datos[fils - 1][0] = BLOB_JUG_1;
		datos[0][cols - 1] = datos[fils - 1][cols - 1] = BLOB_JUG_2;
		/* Coloco los blobs de las esquinas. */
	}
	
	return datos;
}

jugada turnoCPU(tablero *tab1, jugador *player, jugador *oponente, int *errFlag)
{	
	jugada jugadaFinal;

	/* Funcion del turno del CPU: devuelve una jugada (solo para 
	 * imprimir). Llama a moverBlobs una vez que se encontro una jugada
	 * satisfactoria.
	 * Primero intenta capturar blobs enemigos, si no puede, mueve un
	 * blob un espacio.  Si no puede, finalmente mueve un blob dos espacios. */

	if (intentarCapturar(tab1, player, oponente, &jugadaFinal, errFlag))
	{
		if (*errFlag)
			return jugadaFinal;
		
		if(moverBlobs(tab1, player, oponente, jugadaFinal) == ERROR_MEMORIA)
			*errFlag = TRUE;
		/* En cada caso moverBlobs se encarga de mover y convertir los
		 * blobs. */
		 
		return jugadaFinal;
	}
	
	if (*errFlag)
		return jugadaFinal;
	
	if (intentarMover(tab1, player, &jugadaFinal, 1))
	{
		moverBlobs(tab1, player, oponente, jugadaFinal);
		return jugadaFinal;
	}
	
	if (intentarMover(tab1, player, &jugadaFinal, 2))
	{
		moverBlobs(tab1, player, oponente, jugadaFinal);
		return jugadaFinal;
	}
	
	return jugadaFinal;
}

/*====================================================================*/
/*=====================Funciones CPU==================================*/
/*====================================================================*/

static int intentarMover(tablero *tab1, jugador *player, jugada *jugMueve, int dist)
{
	int i;
	
	/* Recorro los blobs de "player" usando el arreglo de blobs en la
	 * estructura "jugador" (mas rapido que recorrer el tablero). Por
	 * cada blob me fijo si tiene un espacio vacio a cierta cantidad
	 * de distancia (dist), si se encuentra entonces se corta y se mueve
	 * el blob encontrado. */
	
	for (i = 0; i < player->cantBlobs; i++)
		if (puedeMoverse(tab1, player->blobs[i], jugMueve, dist))
			return TRUE;
			
	return FALSE;
}

static int puedeMoverse(tablero *tab1, punto origen, jugada *jugadaFinal, int dist)
{
	int i, j;
	punto aux;
	
	/* Dado un punto origen, busca un espacio libre en el tablero alrededor
	 * a un cierto radio (dist) y arma una jugada si es necesario, devoloviendo
	 * TRUE si encontro un espacio. */
	
	for (i = origen.y - dist; i <= origen.y + dist; i++)
		for (j = origen.x - dist; j <= origen.x + dist; j++)  
			if (i >= 0 && i < tab1->fils && j >= 0 && j < tab1->cols && tab1->datos[i][j] == VACIO)
			{
				aux.y = i;
				aux.x = j;
				
				/* Si el puntero recibido es NULL entonces no se necesita
				 * guardar nada. */
				if (jugadaFinal != NULL)
				{
					jugadaFinal->origen = origen;
					jugadaFinal->destino = aux;
				}
				
				return TRUE;
			}
			
	return FALSE;
}

static int intentarCapturar(tablero *tab1, jugador *player, jugador *oponente, jugada *jugCaptura, int *errFlag)
{
	tablero mapa;
	int max, cantNum[10] = {0}, dim = 0;
	
	/* Resumen de la funcion:  se crea un tablero mapa, que en cada celda
	 * tiene un numero representando cuantos blobs enemigos se capturarian
	 * si el jugador moviese un blob a ese lugar. Luego se empieza a recorrer
	 * el mapa empezando con los numeros mas grandes, buscando blobs del
	 * jugador en un radio de 2 lugares para poder crear una jugada. De 
	 * esta manera se prioriza la creacion de jugadas que capturan mas 
	 * blobs. */
	
	mapa.datos = crearTablero(&mapa, tab1->fils, tab1->cols, TAB_VACIO);
	/* El mapa es un tablero con las mismas dimensiones que "tab1", pero
	 * vacio (por ahora). */
	 
	if (mapa.datos == NULL)
	{
		*errFlag = TRUE;
		return ERROR_MEMORIA;
	}

	max = llenarMapa(&mapa, tab1, oponente, cantNum);
	/* Se llena el mapa, en max se almacena el maximo valor llegado (en
	 * total) y en el arreglo cantNum se almacena cuantas veces aparece
	 * cada numero en el mapa, ya que un numero puede aparecer mas de una
	 * vez (los valores posibles son de 0 a 8). */
	
	if (max > 0)	
		do {
			*jugCaptura = jugadaEnMapa(tab1, &mapa, player, max, cantNum[max], &dim, errFlag);
			
			/* Busco en el mapa celdas con valor "max", la funcion jugadaEnMapa
			 * se ocupa de buscar jugadas que capturen "max" cantidad de 
			 * blobs.  La variable "dim" almacena cuantas jugadas se 
			 * encontraron.  Si no se encuentra ninguna, "dim" queda 
			 * en 0, y el ciclo se repite pero con max - 1. */
			
			if (*errFlag)
				return ERROR_MEMORIA;
			
			max--;
		} while (max > 0 && dim == 0);
	
	liberarTablero(&mapa);
	
	/* Si max es 0 y dim es 0, no se encontro ninguna jugada que capture
	 * blobs enemigos. */
	
	if (max == 0 && dim == 0)
		return FALSE;
	else
		return TRUE;

}

static jugada jugadaEnMapa(tablero *tab1, tablero *mapa, jugador *player, int max, int cantMax, int *dim, int *errFlag)
{
	int i, j, k;
	jugada *totalJugadas = NULL, jugadaRetorno;
	punto puntoAux;
	*dim = 0; /* dim es el tamaño de totalJugadas. */

	/* Funcion que busca jugadas que capturen "max" cantidad de blobs, 
	 * esto se logra buscando en el mapa celdas con valor igual a "max".  
	 * Cuando se encuentra una de estas celdas, se llama a calculaJugadas,
	 * que se encarga de buscar blobs del jugador que puedan moverse a 
	 * la celda encontrada.  Todas las jugadas encontradas se guardan en 
	 * el arreglo "totalJugadas".  Al final, dependiendo del tamaño de 
	 * "dim", se devuelve una jugada al azar o no. */
	
	for (i = 0; i < mapa->fils && cantMax > 0; i++)
		for (j = 0; j < mapa->cols && cantMax > 0; j++)
			if (mapa->datos[i][j] == max)
			{
				/* Recorro las filas y columnas del mapa buscando celdas
				 * con valor "max".  La busqueda termina cuando se encontraron
				 * cantMax veces el valor "max", o cuando se llega al final
				 * del mapa. */
				
				puntoAux.y = i;
				puntoAux.x = j;
				
				totalJugadas = calculaJugadas(tab1, player, totalJugadas, puntoAux, dim);
				/* Busco las jugadas posibles con DESTINO a esta celda 
				 * en el tablero, y las agrego a "totalJugadas". */
				
				if (*dim == -1)
				{
					*errFlag = TRUE;
					return jugadaRetorno;
				}
				cantMax--;
			}
	
	switch (*dim)
	{
		case 0:
			return jugadaRetorno;
		break;
		
		case 1:
			jugadaRetorno = totalJugadas[0]; 
			/* Devuelvo la primera y unica jugada encontrada. */
			free(totalJugadas);
			return jugadaRetorno;
		break;
		
		default:
			srand(time(NULL));
			k = rand() % *dim;
			/* Elijo una jugada al azar y la devuelvo. */
			jugadaRetorno = totalJugadas[k];
			free(totalJugadas);
			return jugadaRetorno;
		break;
		
	}
}

static jugada *calculaJugadas(tablero *tab1, jugador *player, jugada *totalJugadas, punto destino, int *dimTotalJugadas)
{
	int i, j;
	jugada jugadaAux, *aux;
	
	/* Arma una jugada con el parametro "destino" como destino.  Almacena
	 * la jugada en el arreglo "totalJugadas", agrandando su tamaño de 
	 * ser necesario. La jugada se arma buscando blobs del jugador en un
	 * radio de 2 bloques alrededor de la celda destino. */
	
	for (i = destino.y - 2; i <= destino.y + 2; i++)
		for (j = destino.x - 2; j <= destino.x + 2; j++)  
			if (i >= 0 && i < tab1->fils && j >= 0 && j < tab1->cols && tab1->datos[i][j] == player->etiqueta)
			{
				/* Se recorre un area de 25*25 alrededor del la posicion
				 * destino, siempre comprobando que la coordenada se 
				 * encuentre adentro del tablero de juego. */
				
				if (*dimTotalJugadas % BLOQUE == 0)
				{
					aux = realloc(totalJugadas, (*dimTotalJugadas + BLOQUE) * sizeof(jugada));
					
					if (aux == NULL)
					{
						(*dimTotalJugadas) = -1;
						free(totalJugadas);
						return NULL;
						/* Si hubo un problema devuelvo una dimension
						 * invalida. */
					}
					
					totalJugadas = aux;
					/* Modifico la copia del puntero, igualmente al final
					 * de la funcion se devuelve la copia. */
				}
				
				jugadaAux.destino = destino;
				jugadaAux.origen.y = i;
				jugadaAux.origen.x = j;
				
				totalJugadas[*dimTotalJugadas] = jugadaAux;
				(*dimTotalJugadas)++;
				/* Aumento la dimension por uno. */
			}
	
	return totalJugadas;
	/* Devuelvo el nuevo puntero al arreglo. */
}


/*====================================================================*/
/*=====================Funciones Mapa=================================*/
/*====================================================================*/

static int llenarMapa(tablero *mapa, tablero *tab1, jugador *oponente, int cantNum[])
{
	int i, max = 0;
	
	for (i = 0; i < oponente->cantBlobs; i++)
		agregarPuntoMapa(mapa, tab1, oponente->blobs[i], &max, cantNum);
		/* Recorro los blobs del arreglo de blobs del oponente, y sumo
		 * 1 a el valor de las celdas adyacentes en el mapa por cada blob. */

	return max;
}

static void agregarPuntoMapa(tablero *mapa, tablero *tab1, punto lugar, int *max, int cantNum[])
{
	int i, j;
	
	for (i = lugar.y - 1; i <= lugar.y + 1; i++)
		for (j = lugar.x - 1; j <= lugar.x + 1; j++)
			if (i >= 0 && i < mapa->fils && j >= 0 && j < mapa->cols && tab1->datos[i][j] == VACIO)
			{
				(mapa->datos[i][j])++;
				/* Aumento por 1 la celda adyacente al punto donde se 
				 * encuentra el blob enemigo. */
				
				if (mapa->datos[i][j] > *max) 
					*max = mapa->datos[i][j];
				/* Si el valor supera a "max", actualizo el maximo. */
					
				if (mapa->datos[i][j] > 1)
					(cantNum[mapa->datos[i][j] - 1])--;
				(cantNum[mapa->datos[i][j]])++;
				/* Si el nuevo valor es N, aumento por 1 a la cantidad de 
				 * veces que ocurre el valor N en el mapa.  Resto 1 a la
				 * cantidad de veces que ocurre el numero N-1 en el mapa. 
				 * Esto sirve para mas tarde agilizar la busqueda del numero
				 * N en el mapa. */
			}
}
	
/*====================================================================*/
/*=====================Fin de Funciones Mapa==========================*/
/*====================================================================*/

int finJuego(tablero *tab1, jugador *player)
{
	int i;
	
	for (i = 0; i < player->cantBlobs; i++)	
	{
		if(puedeMoverse(tab1, player->blobs[i], NULL, 2))
			return NO_TERMINO;
	}
	
	return TERMINO_GANO;
	
	/* La funcion recorre las fichas del jugador fijandose si cada una tiene  
	 * por lo menos un espacio disponible para moverse.  Apenas encuentra una,
	 * la funcion devuelve que la partida todavia no termino. Si no, devuelve 
	 * que ya termino. Si el jugador no tiene blobs el ciclo se saltea, y 
	 * termina el juego.*/
}

int calculaGanador(tablero *tab1, jugador jugadores[])
{
	int indice, i, j;
	
	/* Devuelve el numero de jugador que gano, y 0 si hay empate. */
	
	if (jugadores[0].cantBlobs > jugadores[1].cantBlobs)
		indice = 0;
	else if (jugadores[0].cantBlobs < jugadores[1].cantBlobs)
		indice =  1;
	else
		return 0;
		
	/* Si los jugadores tienen la misma cantidad de blobs, hay empate. */
	
	for (i = 0; i < tab1->fils; i++)
		for (j = 0; j < tab1->cols; j++)		
			if (tab1->datos[i][j] == VACIO)
			{
				tab1->datos[i][j] = jugadores[indice].etiqueta;
				(jugadores[indice].cantBlobs)++;
				/* Se agregan los espacio*/
			}
			
	return indice + 1;
}

int comprobarJugada(tablero *tab1, jugador player, jugada jug)
{
	/* Comprueba que una jugada sea valida, y devuelve un codigo que 
	 * indica cual es el error.  Si la jugada es valida, devuelve el 
	 * codigo VALIDA. */
	
	if (jug.origen.x < 0 || jug.origen.y < 0 || jug.origen.x > tab1->cols - 1 || jug.origen.y > tab1->fils - 1)
		return ERROR_ORIGEN_AFUERA;
		
	if (jug.destino.x < 0 || jug.destino.y < 0 || jug.destino.x > tab1->cols - 1 || jug.destino.y > tab1->fils - 1)
		return ERROR_DESTINO_AFUERA;
		
	if (tab1->datos[jug.origen.y][jug.origen.x] == VACIO)
		return ERROR_ORIGEN_VACIO;
	
	if ((player.indice == 1 && tab1->datos[jug.origen.y][jug.origen.x] != BLOB_JUG_1) || (player.indice == 2 && tab1->datos[jug.origen.y][jug.origen.x] != BLOB_JUG_2))
		return ERROR_ORIGEN_INVALIDO;
	
	if (tab1->datos[jug.destino.y][jug.destino.x] != VACIO)
		return ERROR_DESTINO_OCUPADO;
		
	if (sonVecinos(tab1, jug, 2))
		return VALIDA;
	else
		return ERROR_DESTINO_LEJOS;
}

static int convertirEnemigos(tablero *tab1, jugador *player, jugador *oponente, punto destino)
{
	signed int i, j;
	punto aux;
	
	/* Se recorre un area de 9*9 alrededor de la posicion destino, 
	 * convirtiendo todos los enemigos que se encuentren. */
	
	for (i = destino.y - 1; i <= destino.y + 1; i++)
		for (j = destino.x - 1; j <= destino.x + 1; j++)
			if (i >= 0 && i < tab1->fils && j >= 0 && j < tab1->cols)
				if (tab1->datos[i][j] == oponente->etiqueta)
				{
					/* El ciclo comprueba que la posicion analizada este
					 * adentro del tablero, y que en esa posicion haya un
					 * enemigo. */
					 
					tab1->datos[i][j] = player->etiqueta;
					
					aux.y = i;
					aux.x = j;
					sacarPunto(oponente, aux);
					agregarPunto(player, aux);
					
					/* Una vez que encontre un enemigo, le saco el blob 
					 * al enemigo y se lo agrego al jugador. */
					
					if (player->blobs == NULL)
						return ERROR_MEMORIA;
					/* Si hay un error de memoria devuelvo un codigo
					 * adecuado. */
				}
	return TRUE;
}

int moverBlobs(tablero *tab1, jugador *player, jugador *oponente, jugada jug)
{
	if (!sonVecinos(tab1, jug, 1))
	{
		/* Si el origen y el destino estan a distancia 2, se borra el 
		 * blob de origen, y se crea un blob en el destino. */
		tab1->datos[jug.origen.y][jug.origen.x] = VACIO;
		sacarPunto(player, jug.origen);
	}

	tab1->datos[jug.destino.y][jug.destino.x] = player->etiqueta;
	agregarPunto(player, jug.destino);
	
	/* Se agrega un blob del jugador en la posicion destino, en el tablero
	 * y en el arreglo de blobs del jugador. */
	
	if (player->blobs == NULL)
		return ERROR_MEMORIA;

	return convertirEnemigos(tab1, player, oponente, jug.destino);
}

static int sonVecinos(tablero *tab1, jugada jug, size_t radio)
{
	signed int r = (signed int)radio;
			if (jug.destino.y >= 0 && jug.destino.y < tab1->fils && jug.destino.x >= 0 && jug.destino.y < tab1->cols)
				if (jug.destino.y-jug.origen.y>=-r && jug.destino.y-jug.origen.y<=r && jug.destino.x-jug.origen.x<=r && jug.destino.x-jug.origen.x>=-r)
					return TRUE;
	
	return FALSE;
	
	/* La funcion comprueba que dos puntos esten a cierta distancia
	 * "radio". */
}

void liberarTablero(tablero *tablero1)
{
	int i;
	
	/* Recibe un tablero y libera todos los vectores internos, y luego
	 * libera el puntero al tablero. */
	
	for (i = 0; i < tablero1->fils; i++)
		free(tablero1->datos[i]);
	
	free(tablero1->datos);
}

int agregarPunto(jugador *player, punto point)
{
	punto *aux;
	/* Recibe un jugador y una coordenada, y agrega la coordenada al arreglo
	 * de blobs del jugador.  La coordenada representa la posicion de 
	 * un blob en el tablero. */
	 
	if (player->cantBlobs % BLOQUE2 == 0)
	{
		aux = realloc(player->blobs, (player->cantBlobs + BLOQUE2) * sizeof(punto));
		if (aux == NULL)
		{
			free(player->blobs);
			return FALSE;
		}
		player->blobs = aux;
	}
	
	player->blobs[(player->cantBlobs)++] = point;
	/* Aumento la cantidad de blobs del jugador por 1. */
	
	return TRUE;
}

static int sacarPunto(jugador *player, punto point)
{
	int i;
	/* Recibe un jugador y una coordenada, y quita la coordenada del
	 * arreglo de blobs del jugador.  La coordenada representa un 
	 * blob en el tablero. */
	
	for (i = 0; i < player->cantBlobs; i++)
		if (cmppuntos(point, player->blobs[i]))
		{
			(player->cantBlobs)--;
			player->blobs[i] = player->blobs[player->cantBlobs];
			/* Si encuentro el elemento, copio el ultimo elemento a la 
			 * posicion del elemento encontrado, y reduzco la dimension
			 * por uno.  No es necesario "realocar" el puntero porque
			 * normalmente la cantidad de blobs que se agregan es mayor 
			 * a la que se quita. */
			return TRUE;
		}
	return FALSE;
}

static int cmppuntos(punto a, punto b)
{
	return (a.x == b.x && a.y == b.y);
}

void inicializarPuntos(jugador jugadores[], int fils, int cols)
{
	/* La funcion agrega los primeros 2 puntos iniciales a el arreglo
	 * de blobs de cada jugador. */
	
	punto aux = {0, 0};
	agregarPunto(jugadores, aux);
	aux.y = fils - 1;
	agregarPunto(jugadores, aux);
	aux.x = cols - 1;
	agregarPunto(jugadores + 1, aux);
	aux.y = 0;
	agregarPunto(jugadores + 1, aux);
}

int save(char *filename, jugador *player, jugador *oponente, tablero *tablero1)
{
	FILE *partida;
	int i, j, modo, turno, filas, columnas, blobs1, blobs2, bytes = 0;
	
	/* Guarda la partida en un archivo, usando los datos proporcionados
	 * por turnoHumano. */
	
	modo = player->tipo + oponente->tipo;
	turno = player->indice;
	filas = tablero1->fils;
	columnas = tablero1->cols;
	
	if ((partida = fopen(filename, "w+b")) == NULL)
		return 0;

	bytes = fwrite(&modo, sizeof(int), 1, partida) * sizeof(int);
	bytes += fwrite(&turno, sizeof(int), 1, partida) * sizeof(int);
	bytes += fwrite(&filas, sizeof(int), 1, partida) * sizeof(int);
	bytes += fwrite(&columnas, sizeof(int), 1, partida) * sizeof(int);
	
	if (turno == 1)
	{
		blobs1 = player->cantBlobs;
		blobs2 = oponente->cantBlobs;
	}
	else if (turno == 2)
	{
		blobs1 = oponente->cantBlobs;
		blobs2 = player->cantBlobs;
	}
	
	bytes += fwrite(&blobs1, sizeof(int), 1, partida) * sizeof(int);
	bytes += fwrite(&blobs2, sizeof(int), 1, partida) * sizeof(int);
	
	for (i = 0 ; i < filas ; i++)
		for (j = 0 ; j < columnas ; j++)
			bytes += fwrite(&tablero1->datos[i][j], sizeof(char), 1, partida);
	
	fclose(partida);
	
	/* Devuelve la cantidad de bytes guardados, o 0 si hubo un problema. */
	
	return bytes;
}
