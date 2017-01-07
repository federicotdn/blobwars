#include "blobBack.h"

int mostrarMenu();
int nuevoJuego(int modo1, int modo2);
void Jugar(tablero *tablero1, jugador jugadores[], int i);
int Turno(tablero *tablero1, jugador *player, jugador *enemigo);
int turnoHumano(tablero *tablero1, jugador *player, jugador *oponente);
int ingresaJugada(jugada *jug, char **filename);
int eleccionUsuario(jugada *jug, char **filename);
void imprimirErrorEntrada(int error);
void imprimirErrorJugada(int error);
void imprimirTablero(const tablero *tablero1);
void imprimirTitulo();
char *leerNomArchivo();

int main()
{
	imprimirTitulo();
	
	while (mostrarMenu());
	/* Se llama al menu hasta que el usuario decida salir.*/
	
	return 0;
}

int mostrarMenu()
{
	int seleccion;
	
	printf("\n....:::::BLOB WARS::::....\n\n");
	printf(" Elija una opcion:\n");
	printf(  "1) Nueva partida de a dos jugadores.\n");
	printf(  "2) Nueva partida contra el CPU.\n");
	printf(  "3) Cargar partida anterior.\n");
	printf(  "4) Salir.\n");
	do
		seleccion = getint("Ingrese su opcion:\n");
	while ( seleccion > 5 || seleccion < 1);
	
	switch (seleccion)
	{
		case 1:
			nuevoJuego(HUMANO, HUMANO);
			/* Opciones 1 y 2 llaman a nuevoJuego con los dos tipo de 
			 * jugadores. */
		break;
		
		case 2:
			nuevoJuego(HUMANO, CPU);
		break;
		
		case 3:
			resumirJuego();
			/* Opcion 3 llama a resumirJuego que se encarga de
			 * cargar los datos. */ 
		break;
		
		case 4:
			printf("\nCerrando Blob Wars.\n\n");
			return FALSE;
		break;
		
		case 5:
			nuevoJuego(CPU, CPU) /* Para pruebas. */;
		break;
	}
	
	return TRUE;
}

int nuevoJuego(int modo1, int modo2)
{
	int fils, cols;
	tablero tablero1;
	jugador jugadores[] = {{1, 0, 0, NULL, BLOB_JUG_1}, {2, 0, 0, NULL, BLOB_JUG_2}};
	jugadores[0].tipo = modo1;
	jugadores[1].tipo = modo2;
	
	/* Primero se crean dos jugadores, cada uno con un indice (1 y 2), 
	 * un modo que identifica si son humanos o CPU, la cantidad de blobs
	 * que tiene el jugador, un arreglo de puntos
	 * que corresponde a las coordenadas de cada blob y al final la etiqueta
	 * que representa al jugador (ej. A y Z).*/
	
	do
		fils = getint("Ingrese numero de filas:\n");
	while (fils < MIN_DIM || fils > MAX_DIM);
	
	do
		cols = getint("Ingrese numero de columnas:\n");
	while (cols < MIN_DIM || cols > MAX_DIM);
	
	inicializarPuntos(jugadores, fils, cols);
	/* Agrego dos puntos al arreglo de cada jugador en las posiciones
	 * adecuadas (esquinas). */
	
	tablero1.datos = crearTablero(&tablero1, fils, cols, TAB_JUEGO);
	/* Se inicializa el tablero: se le asigna el ancho y alto, y se llena
	 * adecuadamente. */
	
	
	if (tablero1.datos == NULL)
	{
		IMPRIMIR_ERROR_MEMORIA;
		return ERROR_MEMORIA;
	}
	
	printf("\nComienza el juego:\n\n");
	
	Jugar(&tablero1, jugadores, 0);
	
	/* Libero el tablero y los arreglos de blobs de los jugadores.*/
	liberarTablero(&tablero1);	
	free(jugadores[0].blobs);
	free(jugadores[1].blobs);
	/* El juego finalizo.*/
	
	return 1;
}

int resumirJuego()
{
	int i, j , bytes, modo, turno, filas, columnas;
	jugador jugadores[] = {{1, HUMANO, 0, NULL, BLOB_JUG_1}, {2, HUMANO, 0, NULL, BLOB_JUG_2}};
	tablero tablero1;
	punto puntoaux;
	char *filename = NULL;
	FILE *partida;
	
	/* Funcion similar a nuevoJuego, pero en vez de crear los elementos
	 * necesarios para una partida nueva, los carga de un archivo. Al 
	 * final llama a Jugar con estos datos. */
		
	system("ls");
	printf("\nIngresar el nombre del archivo a cargar:\n");
	
	filename = leerNomArchivo();
	
	if ((partida = fopen(filename, "rb")) == NULL)
	{
		printf("Hubo un error al intentar cargar el archivo, vuelva a intentarlo.\n\n");
		return NO_CARGO;
	}

	bytes = fread(&modo, sizeof(int), 1, partida) * sizeof(int);
	
	if (modo == 1)
		jugadores[1].tipo = CPU;
	
	bytes += fread(&turno, sizeof(int), 1, partida) * sizeof(int);
	bytes += fread(&filas, sizeof(int), 1, partida) * sizeof(int);
	bytes += fread(&columnas, sizeof(int), 1, partida) * sizeof(int);
	
	fseek(partida, sizeof(int) * 2, SEEK_CUR);
	bytes += sizeof(int) * 2;
		
	/* Cargo los datos en las variables de esta funcion.  La cantidad
	 * de blobs de cada jugador se saltea porque mas tarde, cuando se 
	 * agreguen los blobs al tablero, tambien se agregaran los blobs a
	 * los arreglos de blobs de cada jugador (lo cual aumenta la cantidad
	 * de blobs internamente). */

	tablero1.datos = crearTablero(&tablero1, filas, columnas, TAB_VACIO);
	
	if (tablero1.datos == NULL)
	{
		IMPRIMIR_ERROR_MEMORIA;
		return NO_CARGO;
	}
	
	for (i = 0 ; i < filas ; i++)
		for (j = 0 ; j < columnas ; j++)
		{
			bytes += fread(&tablero1.datos[i][j], sizeof(char), 1, partida);
			puntoaux.x = j;
			puntoaux.y = i;
			
			/* Se agrega el blob al tablero Y tambien se agrega al 
			 * arreglo de blobs del jugador adecuado. */
			
			if (tablero1.datos[i][j] == 'A')
				agregarPunto(&jugadores[0], puntoaux);
			else if (tablero1.datos[i][j] == 'Z')
				agregarPunto(&jugadores[1], puntoaux);
		}
	
	printf("cant bytes: %d\n", bytes);
	printf("Juego cargado con exito.\n");
	printf("-->Se cargaron %d bytes\n\n", bytes);
	printf("\nComienza el juego:\n\n");

	Jugar(&tablero1, jugadores, turno);
	
	liberarTablero(&tablero1);	
	free(jugadores[0].blobs);
	free(jugadores[1].blobs);
	
	return CARGO;
}

void Jugar(tablero *tablero1, jugador jugadores[], int i)
{
	int ganador;
	int estado_juego = NO_TERMINO;
	srand(time(NULL));
	
	if (i == 0) /* Si llega 0 entonces es un nuevo juego.*/
		i = rand()%2;
	else 
		i--;
	/* Si no, es un juego cargado.  Los turnos se manejan internamente
	 * con 0 y 1 en vez de 1 y 2. */
	
	imprimirTablero(tablero1);
	
	/* El estado de juego tiene 3 valores posibles: sigue, termino porque
	 * alguien gano, o termino porque el usuario salio o hubo un error
	 * de memoria. */
	
	while (estado_juego == NO_TERMINO)
	{
		printf("\nLe toca al jugador %d: (%d blobs)\n", i + 1, jugadores[i].cantBlobs);
		
		estado_juego = Turno(tablero1, &jugadores[i], &jugadores[inv(i)]);
		
		/* A turno le paso el tablero, la direccion del jugador adentro
		 * del arreglo, y la direccion del oponente adentro del arreglo.
		 * "jugador" seria el jugador al que le toca jugar en el momento.*/
		
		if (estado_juego == NO_TERMINO)
		{
			imprimirTablero(tablero1);
			i = inv(i); /* Si i era 1 lo paso a 0 y vice-versa. */
		}
	}
	
	if (estado_juego == TERMINO_GANO)
	{
		/* Solo se imprime si el juego termino porque alguien gano. */
		
		if ((ganador = calculaGanador(tablero1, jugadores)))
		{
			/* La variable "ganador" tiene un 0 si hay empate, o el numero
			 * de jugador que gano el juego. */
			printf("El ganador es: ");
			printf("Jugador %d con %d puntos\n\n", ganador, jugadores[ganador - 1].cantBlobs);
			printf("El resultado es:\n\n");
			imprimirTablero(tablero1);
		}
		else
			printf("No hay ganador: Empate\n\n");
	}
}

int Turno(tablero *tablero1, jugador *player, jugador *oponente)
{
	int estado_juego = NO_TERMINO, errFlag = FALSE;
	jugada jugAux;
	
	/* La funcion simplemente llama a la funcion turnoHumano o turnoCPU
	 * dependiendo del tipo de jugador.  Al final del turno de "player",
	 * se fija si el oponente perdio y retorna el valor adecuado. */

	if (player->tipo == HUMANO)
		estado_juego = turnoHumano(tablero1, player, oponente);
		
	else if (player->tipo == CPU)
	{
		jugAux = turnoCPU(tablero1, player, oponente, &errFlag);
		/* Se imprime la jugada del CPU para mayor claridad. */
		
		if (!errFlag)
		{
			printf("\nDavid 2.0: Mi jugada es ");
			printf("[%d,%d]", jugAux.origen.y, jugAux.origen.x);
			printf("->[%d,%d].\n", jugAux.destino.y, jugAux.destino.x);
			printf("Presionar Enter para continuar.\n\n");
			
			BORRAR_BUFFER;
		}
	}
	
	if (errFlag)
	{
		/* errFlag se manda a turnoCPU para simplificar el checkeo de 
		 * errores. Se enciende si en algun momento hay algun error. */
		IMPRIMIR_ERROR_MEMORIA;
		return TERMINO_QUIT;
	}
	
	/* Si el usuario no salio del juego, entonces me puedo fijar si el
	 * oponente perdio. */
	if (estado_juego == NO_TERMINO)
		return finJuego(tablero1, oponente);
	else
		return estado_juego;
}

void imprimirTablero(const tablero *tablero1)
{
	/* Imprime el tablero mostrando numero de columna y fila, mostrando
	 * guiones bajos en vez de '0' como esta guardado en el tablero.*/
	
	int i, j;
	
	printf("##");
	
	for (i = 0; i < tablero1->cols; i++)
		printf("%2d ", i);
		
	printf("\n");
	
	for (i = 0; i < tablero1->fils; i++)
	{
		printf("%2d ", i);
		
		for (j = 0; j < tablero1->cols; j++)
		{
			printf("%c  ", (tablero1->datos[i][j]) == '0' ? '_' : tablero1->datos[i][j]);
		}
		printf("\n");
	}
}

int turnoHumano(tablero *tablero1, jugador *player, jugador *oponente)
{
	jugada jug;
	int eleccion, bytes, jugadaValida = FALSE;
	char *filename = NULL;
	
	/* do-while: si el usuario usa "save" o ingresa una jugada invalida,
	 * se le debe pedir que ingrese de nuevo un comando.*/
	
	do {
		eleccion = eleccionUsuario(&jug, &filename);
		/* La variable "eleccion" tiene ahora 5 posibles valores, que
		 * representan elecciones del usuario: jugar, guardar, salir,
		 * salir y guardar o error de memoria. */
		
		switch (eleccion)
		{
			case JUGADA:
			
				/* Primero compruebo que la jugada ingresada sea valida.*/
				jugadaValida = comprobarJugada(tablero1, *player, jug);	
			
				if (jugadaValida == VALIDA)
				{
					/* moverBlobs se encarga de mover y convertir blobs. */
					if (moverBlobs(tablero1, player, oponente, jug) == ERROR_MEMORIA)
					{
						IMPRIMIR_ERROR_MEMORIA;
						return TERMINO_QUIT;
					}
				}
				else
				{
					/* Si la jugada no es valida, se entra al ciclo do-while
					 * de nuevo. */
					eleccion = ERROR_JUGADA;
					imprimirErrorJugada(jugadaValida);
				}
			break;
			
			case GUARDAR:
				/* Guardo, y entro en el ciclo do-while de nuevo.*/
				bytes = save(filename, player, oponente, tablero1);
				if(bytes > 0)
				{
					printf("Archivo guardado con exito.\n");
					printf("-->Se han guardado %d bytes.\n", bytes);
				}
				else
					printf("Error inesperado, no se pudo guardar el archivo.\n\n");
					
				free(filename);
			break;
			
			case SALIR:
				printf("Finalizo el juego.\n");
				return TERMINO_QUIT;
			break;
			
			case SALIRGUARDAR:
				/* Igual que guardar y despues salir. */
				bytes = save(filename, player, oponente, tablero1);
				if(bytes > 0)
				{
					printf("Archivo guardado con exito.\n");
					printf("-->Se han guardado %d bytes.\n\n", bytes);
				}
				else
					printf("Error inesperado, no se pudo guardar el archivo.\n\n");
				free(filename);
				return TERMINO_QUIT;
			break;
			
			case ERROR_MEMORIA:
				IMPRIMIR_ERROR_MEMORIA;
				return TERMINO_QUIT;
			break;
		}
	} while (eleccion == GUARDAR || eleccion == ERROR_JUGADA);
	
	return NO_TERMINO;
}

int eleccionUsuario(jugada *jug, char **filename)
{
	/* Simplemente llama a ingresaJugada hasta que se haya ingresado
	 * un comando correctamente (simplifica la funcion).  Los parametros 
	 * se le pasan directamente. */
	
	int eleccion;
	printf("\n");
	do
		eleccion = ingresaJugada(jug, filename);
	while (eleccion == ERROR_LECTURA);
	
	return eleccion;
}

int ingresaJugada(jugada *jug, char **filename)
{
	char c, entrada[MAX_LARGO_ENTRADA], *save = "save ", *aux;
	int i = 0, saveLength = strlen(save);
	punto auxOrigen = {-1,-1}, auxDestino = {-1,-1};
	
	/* Funcion que lee la accion del usuario. La funcion devuelve un int
	 * representando el TIPO de eleccion de usuario (jugar, guardar, etc.)
	 * y guarda en los punteros recibidos la informacion adecuada.  Si el
	 * usuario ingresa una jugada, solo se graba en "*jug", y "filename"
	 * se ignora.  Si el usuario elije "save", el nombre se guarda en
	 * "*filename".  La funcion que invoca a esta sabe que datos utilizar
	 * por el valor que se devuelve al final. */
	
	printf("Accion: ");
	
	/* Primero leo los primeros X caracteres, y los primeros 4 los paso a 
	 * minuscula. */
	
	while ((c = getchar()) != '\n' && i < MAX_LARGO_ENTRADA - 1)
	{
		if (i < 4)
			entrada[i++] = tolower(c);
		else
			entrada[i++] = c;		
	}
	
	if (c != '\n')
	{
		printf("\nNOTA: Se leyeron solo %d caracteres.\n", MAX_LARGO_ENTRADA);
		BORRAR_BUFFER;
	}
	
	entrada[i] = 0;
	
	/* Comienzo analizando el primer caracter de "entrada". */
	
	switch (entrada[0])
	{
		case '[': /* Caso: Jugada */
			c = 0;
			sscanf(entrada, "[%d,%d][%d,%d%c", &auxOrigen.y, &auxOrigen.x, &auxDestino.y, &auxDestino.x, &c);	
				
			if (auxOrigen.y > -1 && auxOrigen.x > -1 && auxDestino.y > -1 && auxDestino.x > -1 && c == ']')
			{
				/* Si alguna de las coordenadas de "jug" siguen siendo
				 * negativas, entonces no se leyo correctamente alguno
				 * de los 4 numeros requeridos (o se ingreso un negativo)*/
				 
				jug->origen = auxOrigen;
				jug->destino = auxDestino;
				printf("\n");
				return JUGADA;
			}
			else
			{
				imprimirErrorEntrada(ERROR_COORDENADAS);
				return ERROR_LECTURA;
				/* Si devuelvo ERROR_LECTURA, esta funcion sera invocada
				 * una vez mas, inmediatamente. */
			}
		break;
		
		case 'q': /* Caso: Quit */
			if (strcmp(entrada, "quit") != 0)
			{
				imprimirErrorEntrada(ERROR_COMANDO);
				return ERROR_LECTURA;			
			}

			do {
				printf("\nDesea guardar la partida? (Y/N/y/n): ");
				c = getchar();
				if (c != '\n')
					BORRAR_BUFFER;
			} while (c != 'Y' && c != 'y' && c != 'N' && c != 'n');
			
			printf("\n");
			
			if (c == 'n' || c == 'N')
				return SALIR;
			else
			{
				printf("Ingresar nombre de archivo:\n\n");
				*filename = leerNomArchivo();
				/* Guardo en "filename" un nombre de archivo valido y salgo. */
				
				if (*filename == NULL)
					return ERROR_MEMORIA;
				return SALIRGUARDAR;
			}			
		break;
		
		case 's': /* Caso: Save */
		
			/* Primero me fijo que la primera parte de "entrada" concuerde
			 * con el comando para guardar, en este caso "save". */
			if (strncmp(entrada, save, saveLength) != 0)
			{
				imprimirErrorEntrada(ERROR_COMANDO);
				return ERROR_LECTURA;
			}
			
			if (strlen(entrada) == saveLength) /* El usuario no ingreso un nombre. */
			{
				imprimirErrorEntrada(ERROR_VACIO);
				return ERROR_LECTURA;
			}
			
			/* Checkeo que no haya caracteres invalidos. */
			for (i = saveLength; i < strlen(entrada); i++)
				if (!isalnum(entrada[i]) && entrada[i] != '_' && entrada[i] != '-' && entrada[i] != '.')
				{
					imprimirErrorEntrada(ERROR_NOMBRE);
					return ERROR_LECTURA;
				}
			
			aux = malloc(strlen(entrada) - saveLength + 1);
			
			if (aux == NULL)
				return ERROR_MEMORIA;
				
			/* Creo lugar para guardar el nombre, y empiezo a copiar lo que
			 * sigue de la palabra "save ". */
			strcpy(aux, entrada + saveLength);
			
			*filename = aux;
		
			return GUARDAR;						
		break;
		
		default: /* Si el primer caracter ya esta mal. */
			imprimirErrorEntrada(ERROR_COMANDO);
			return ERROR_LECTURA;
		break;
	}	
}

char *leerNomArchivo()
{
	char c, filename[MAX_LARGO_ENTRADA], *aux;
	int i, nombreValido;
	
	/* Lee una cierta cantidad de caracteres validos para usar como 
	 * nombre de archivo. Devuelve un puntero al nombre, o NULL si 
	 * hubo un error de memoria. */
	
	do {
		nombreValido = TRUE;
		i = 0;
		filename[0] = 0;
		c = 0;
		
		while ((c = getchar()) != '\n' && i < MAX_LARGO_ENTRADA - 1)
			filename[i++] = tolower(c);

		if (c != '\n' && i > 0)
		{
			printf("\n(NOTA: se leyeron solo %d caracteres) ", MAX_LARGO_ENTRADA);
			BORRAR_BUFFER;
		}
		
		if (i == 0)
		{
			imprimirErrorEntrada(ERROR_VACIO);
			nombreValido = FALSE;
			continue;
			/* Si el usuario no ingreso ningun caracter entonces el 
			 * nombre es invalido y puedo saltar directamente a la 
			 * siguiente iteracion del ciclo. */
		}
		
		filename[i] = 0;
		
		for (i = 0; filename[i] && nombreValido; i++)
			if (!isalnum(filename[i]) && filename[i] != '_' && filename[i] != '-' && filename[i] != '.')
			{
				nombreValido = FALSE;
				imprimirErrorEntrada(ERROR_NOMBRE);
			}
		
	} while (!nombreValido);
	
	aux = malloc(strlen(filename));
	if (aux == NULL)
	{
		IMPRIMIR_ERROR_MEMORIA;
		return NULL;
	}
	
	strcpy(aux, filename);
	
	return aux;
}

void imprimirErrorEntrada(int error) 
{
	/* Recibe un codigo de error e imprime el mensaje correspondiente. */
	switch (error)
	{
		case ERROR_COORDENADAS:
			printf("\nLas coordenadas no son validas, intentar nuevamente.\n\n");
		break;	
		case ERROR_NOMBRE:
			printf("\nEl nombre del archivo solo puede tener letras, numeros y guiones bajos.\n");
			printf("Intentar de nuevo.\n\n");
		break;		
		case ERROR_COMANDO:
			printf("\nError en el comando ingresado, ");
			printf("los comandos posibles son:\n");
			printf("- save [nombre archivo]\n");
			printf("- quit\n");
			printf("- o su jugada ([X,Y][X,Y])\n\n");
		break;
		case ERROR_VACIO:
			printf("\nEl nombre de archivo no puede ser vacio.\n");
			printf("Intentar de nuevo\n\n");
		break;
	}
}

void imprimirErrorJugada(int error)
{
	switch (error)
	{
		case ERROR_DESTINO_AFUERA:
			printf("Jugada invalida: la coordenada de destino esta afuera del tablero.\n");
		break;
		case ERROR_ORIGEN_AFUERA:
			printf("Jugada invalida: la coordenada de origen esta afuera del tablero.\n");
		break;
		case ERROR_ORIGEN_INVALIDO:
			printf("Jugada invalida: la coordenada de origen apunta a un blob enemigo.\n");
		break;
		case ERROR_ORIGEN_VACIO:
			printf("Jugada invalida: la coordenada de origen no apunta a un blob.\n");
		break;
		case ERROR_DESTINO_LEJOS:
			printf("Jugada invalida: se puede desplazar un blob como maximo 2 celdas.\n");
		break;
		case ERROR_DESTINO_OCUPADO:
			printf("Jugada invalida: la coordenada de destino ya esta ocupada.\n");
		break;
	}
}

void imprimirTitulo()
{
	printf(" _______   __        ______   _______         __       __   ______   _______    ______  \n");
	printf("|       \\ |  \\      /      \\ |       \\       |  \\  _  |  \\ /      \\ |       \\  /      \\ \n");
	printf("| OOOOOOO\\| OO     |  OOOOOO\\| OOOOOOO\\      | OO / \\ | OO|  OOOOOO\\| OOOOOOO\\|  OOOOOO\\\n");
	printf("| OO__/ OO| OO     | OO  | OO| OO__/ OO      | OO/  O\\| OO| OO__| OO| OO__| OO| OO___\\OO\n");
	printf("| OO    OO| OO     | OO  | OO| OO    OO      | OO  OOO\\ OO| OO    OO| OO    OO \\OO    \\ \n");
	printf("| OOOOOOO\\| OO     | OO  | OO| OOOOOOO\\      | OO OO\\OO\\OO| OOOOOOOO| OOOOOOO\\ _\\OOOOOO\\\n");
	printf("| OO__/ OO| OO_____| OO__/ OO| OO__/ OO      | OOOO  \\OOOO| OO  | OO| OO  | OO|  \\__| OO\n");
	printf("| OO    OO| OO     \\\\OO    OO| OO    OO      | OOO    \\OOO| OO  | OO| OO  | OO \\OO    OO\n");
	printf(" \\OOOOOOO  \\OOOOOOOO \\OOOOOO  \\OOOOOOO        \\OO      \\OO \\OO   \\OO \\OO   \\OO  \\OOOOOO\n\n\n");
}
