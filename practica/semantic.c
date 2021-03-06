////////////////////////////////////////////////////////////////////////////////
//
// Pedro Gallego López
// Pedro Pablo Ruíz Huertas
// Álvaro Beltrán Camacho
// Alberto Luque Infante
//
// PL - Procesadores de Lenguajes - CCIA
//
// ETSIIT - UGR
//
// semantic.c
//
////////////////////////////////////////////////////////////////////////////////

#include "semantic.h"

inTS ts[MAX_IN];
int line = 1;
long int LIMIT = 0;
long int LIMIT_TF = 0;
int decVar = 0;
int decParam = 0;
int decFunction = 0;
int decSal=0;//CI
int decEnt=0;//CI
int subProg = 0;
tData globalType = NA;
int nParam = 0;
int currentFunction = -1;
int aux = 0;

// Devuelve si el atributo es array o no
int isArray(attrs e){

    return (e.nDim!=0);
}

// Devuelve si los dos posibles arrays que recibe tienen el mismo tamaño
int equalSize(attrs e1, attrs e2){

    return (e1.nDim == e2.nDim &&
        e1.tDim1 == e2.tDim1 &&
        e1.tDim2 == e2.tDim2);

}

int equalMulSize(attrs e1, attrs e2){

    return (e1.nDim == e2.nDim &&
        e1.tDim2 == e2.tDim1);

}

int compatibleSize(attrs e1, attrs e2){

    return (e1.nDim == e2.nDim &&
        e1.tDim2 == e2.tDim1);

}

// Guarda el type de la variable
int setType(attrs value){

    globalType = value.type;

}

///////////////////////////////////////////////////////////////////////////////
// Tabla de Símbolos
//

// Inserta una in en la tabla de símbolos
int tsAddIn(inTS in){

    // Si se tienen más entradas de las que puede alojar la tabla de símbolos
    // dará un error, si no, se inserta
	if(LIMIT < MAX_IN) {

		ts[LIMIT].in=in.in;
		ts[LIMIT].lex=in.lex;
		ts[LIMIT].type=in.type;
		ts[LIMIT].nParam=in.nParam;
		ts[LIMIT].nDim=in.nDim;
		ts[LIMIT].tDim1=in.tDim1;
		ts[LIMIT].tDim2=in.tDim2;

        // Se aumenta el contador de entradas
		LIMIT++;

        // Se muestra la tabla de símbolos por pantalla
		//printTS();

		return 1;

	} else {

		printf("Error Semantico(%d): Tope de la pila alcanzado.", line);

		return 0;

	}

}

// Elimina una in de la tabla de símbolos
int tsDelIn(){

    // Si la tabla de símbolos tiene alguna in puede eliminar la última
    if(LIMIT > 0){

		LIMIT--;
		return 1;

	}else{

		printf("Error Semantico(%d): Tabla de símbolos vacía", line);
		return 0;

	}

}

// Elimina las entradas de la tabla de símbolos hasta la mark de tope
void tsCleanIn(){

  while(ts[LIMIT-1].in != MARK && LIMIT > 0){//Quita todo hasta llegar a una marca
		LIMIT--;
	}
	if (ts[LIMIT-1].in == MARK) {//Quita la marca
		LIMIT--;
	}

}

// Busca una entrada según el id
int tsSearchId(attrs e){

  int i = LIMIT - 1;
	int found = 0;

	while (i > 0 && !found /*&& ts[i].in != MARK*/) {
		if (ts[i].in == VAR && strcmp(e.lex, ts[i].lex) == 0) {
			found = 1;
		} else{
			i--;
		}
	}


	if(!found) {
		//printf("Error semantico (%d): Identificador no declarado: %s\n", line, e.lex);//pregunta
		return -1;
	} else {
		return i;
	}

}

// Busca una función según el nombre
int tsSearchName(attrs e){

    int i = LIMIT - 1;
	int found = 0;


	while (i > 0 && !found /*&& ts[i].in != MARK*/) {
		if (ts[i].in == FUNCTION && strcmp(e.lex, ts[i].lex) == 0) {
			found = 1;
		} else{
			i--;
		}
	}

	if(!found) {
    	//printf("Error semantico (%d): Funcion no declarado: %s\n", line, e.lex);//pregunta
		return -1;
	} else {
		return i;
	}

}

// Añade un id
void tsAddId(attrs e){

    // Para añadir un id a la pila no se puede haber llegado al tope,
    // el id no puede existir y se deben estar declarando variables
	int j = LIMIT-1;
	int found = 0;

	if(j >= 0 && decVar == 1){
		// Se obtiene la posición de la mark del bloque
		while((ts[j].in != MARK) && (j >= 0) && !found){

			if(strcmp(ts[j].lex, e.lex) != 0){

				j--;

			} else{

				found = 1;
				printf("Error semantico (%d): El identificador ya existe: %s\n", line, e.lex);

	 		}

		}

		// Si no se ha encontrado significa que no existe, por lo que se añade
        // a la pila
		if(!found) {
			inTS newIn;
			newIn.in = VAR;
			newIn.lex = e.lex;
			newIn.type = globalType;
			newIn.nParam = 0;
			newIn.nDim=e.nDim;
			newIn.tDim1=e.tDim1;
			newIn.tDim2=e.tDim2;
			tsAddIn(newIn);

		}

	}
}

// Añade una mark de tope
void tsAddMark(){

    inTS inInitScope;

	inInitScope.in = MARK;
	inInitScope.lex = "{";
	inInitScope.type = NA;
	inInitScope.nParam = 0;
	inInitScope.nDim = 0;
	inInitScope.tDim1 = 0;
	inInitScope.tDim2 = 0;

	tsAddIn(inInitScope);

    // Se añaden a la tabla de símbolos los parámetros de la función como las
    // variables locales de ese bloque
	if(subProg == 1){

		int j = LIMIT - 2, mark = 0, funct = 0;

		while(j > 0 && ts[j].in == FORM){

			/*printf("\n\n");
			printIn(j);
			printf("\n\n");*/

			if(ts[j].in == FORM) {//Añade los parametros como variables locales

				inTS newIn;
				newIn.in = VAR;
				newIn.lex = ts[j].lex;
				newIn.type = ts[j].type;
				newIn.nParam = ts[j].nParam;
				newIn.nDim = ts[j].nDim;
				newIn.tDim1 = ts[j].tDim1;
				newIn.tDim2 = ts[j].tDim2;
				tsAddIn(newIn);

			}

			j--;

		}

	}

}

// Añade una in de subprograma
void tsAddSubprog(attrs e){

  inTS inSubProg;
	inSubProg.in = FUNCTION;
	inSubProg.lex = e.lex;
	inSubProg.nParam = 0;
	inSubProg.nDim = 0;
	inSubProg.tDim1 = 0;
	inSubProg.tDim2 = 0;
	inSubProg.type = e.type;

	currentFunction = LIMIT;//Guarda la posicion de la cabecera de la funcion actual
	tsAddIn(inSubProg);

}

// Añade una in de param formal
void tsAddParam(attrs e){//Añade un parametro si no esta ya añadido

    int j = LIMIT - 1, found = 0;

	while((j != currentFunction)  && (!found) ){

		if(strcmp(ts[j].lex, e.lex) != 0) {//Comprueba si el parametro ya existe

			j--;

		} else{

			found = 1;
			printf("Error Semantico(%d): Existe el parametro: %s\n", line, e.lex);

        }

	}

	if(!found) {

		inTS newIn;
		newIn.in = FORM;
		newIn.lex = e.lex;
		newIn.type = globalType;
		newIn.nParam = 0;
		newIn.nDim = e.nDim;
		newIn.tDim1 = e.tDim1;
		newIn.tDim2 = e.tDim2;
		tsAddIn(newIn);

	}

}

// Actualiza el número de parámetros de la función
void tsUpdateNparam(attrs e){

  ts[currentFunction].nParam = nParam;
	ts[currentFunction].nDim=e.nDim;
	ts[currentFunction].tDim1=e.tDim1;
	ts[currentFunction].tDim2=e.tDim2;

}

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Analizador Semántico
//

// Devuelve el índice de la entrada que sea la función en la que estamos
int tsGetNextFunction(){




  int i = LIMIT - 1 - decIF;

	int found = 0;

	while (i > 0 && !found) {

		if (ts[i].in == FUNCTION && ts[i+ts[i].nParam + 1].in == MARK && i+ts[i].nParam + 1 < LIMIT -1 - decIF) {
			found = 1;
		} else {
			i--;
		}

	}

	if(!found) {
		return -1;
	} else {
		return i;
	}

}

// Comprueba si el type de la expresión coincide con lo que devuelve la función //terminado
void tsCheckReturn(attrs expr, attrs* res){//expr es el return

    int index = tsGetNextFunction();


	if (index > -1) {

  		if (expr.type != ts[index].type) {
  			printf("Error Semantico(%d): La funcion devuelve un tipo que no se corresponde con su declaracion.\n", line);
  			return;
  		}

  		attrs tmp;
  		tmp.nDim = ts[index].nDim;
  		tmp.tDim1 = ts[index].tDim1;
  		tmp.tDim2 = ts[index].tDim2;

  		if (!equalSize(expr,tmp)) {
  			printf("Error Semantico(%d): La funcion devuelve una expresion que de un tamaño distinto al esperado.\n", line);
  			return;
  		}

  		res->type = expr.type;
  		res->nDim = expr.nDim;
  		res->tDim1 = expr.tDim1;
  		res->tDim2 = expr.tDim2;

	}else {

		printf("Error Semantico(%d): No se encuentra funcion declarada\n", line);
		return;

	}

}

// Devuelve el identificador
void tsGetId(attrs id, attrs* res){

    int index = tsSearchId(id);

	if(index==-1) {
        if(ts[index].in != FUNCTION)
		      printf("\nError Semántico(%d): Identificador %s no encontrado.\n", line, id.lex);
	} else {
		res->lex = strdup(ts[index].lex);
		res->type = ts[index].type;
		res->nDim = ts[index].nDim;
		res->tDim1 = ts[index].tDim1;
		res->tDim2 = ts[index].tDim2;

	}

}

// Realiza la comprobación de la operación !, &, ~ //terminado pregunta
void tsOpUnary(attrs op, attrs o, attrs* res){

  if (strcmp(op.lex,"&")==0 || strcmp(op.lex,"!")==0 || strcmp(op.lex,"~")==0){
      if (o.type != BOOLEANO || isArray(o)) {
    		printf("Error Semantico(%d): No es un atributo booleano", line);
    	}

    	res->type = BOOLEANO;
    	res->nDim = 0;
    	res->tDim1 = 0;
    	res->tDim2 = 0;
  }else{//Modificacion
      if ((o.type != FLOTANTE && o.type != ENTERO) || isArray(o)) {
        printf("Error Semantico(%d): No es un atributo numerico", line);
      }

      res->type = o.type;
      res->nDim = 0;
      res->tDim1 = 0;
      res->tDim2 = 0;
  }



}

// Realiza la comprobación de la operación +, - //pregunta op
void tsOpSign(attrs op, attrs o, attrs* res){

  if ((o.type != FLOTANTE && o.type != ENTERO) || isArray(o)) {
		printf("Error Semantico(%d): No se puede aplicar el operador signo a la expresion", line);
	}

	res->type = o.type;
	res->nDim = 0;
	res->tDim1 = 0;
	res->tDim2 = 0;

}

// Realiza la comprobación de la operación +, - binaria//terminado array
void tsOpSignBin(attrs o1, attrs op, attrs o2, attrs* res){

  if (o1.type != o2.type) {
	    printf("Error Semantico(%d): Las expresiones deben ser del mismo tipo", line);
  		return;
  }

	if (o1.type != ENTERO && o1.type != FLOTANTE) {
		printf("Error Semantico(%d): Las expresiones deben ser enteros o flotantes", line);
		return;
	}




	if(!isArray(o1) && !isArray(o2)){
		res->type = o1.type;
		res->nDim = o1.nDim;
		res->tDim1 = o1.tDim1;
		res->tDim2 = o1.tDim2;
	}

	if (isArray(o1) && isArray(o2)){

		if(equalSize(o1,o2)){

			res->type = o1.type;
			res->nDim = o1.nDim;
			res->tDim1 = o1.tDim1;
			res->tDim2 = o1.tDim2;

		} else {

            printf("Error Semantico(%d): Los arrays deben ser del mismo tamaño", line);
			return;

		}

	} else {
		if (isArray(o1) && !isArray(o2)) {//terminado
			res->type = o1.type;
			res->nDim = o1.nDim;
			res->tDim1 = o1.tDim1;
			res->tDim2 = o1.tDim2;

		}

		if (!isArray(o1) && isArray(o2)){

			if (strcmp(op.lex,"-")==0){

				printf("Error Semantico(%d): operación no permitida.", line);
				return;

			} else {

				res->type = o2.type;
				res->nDim = o2.nDim;
				res->tDim1 = o2.tDim1;
				res->tDim2 = o2.tDim2;

			}

		}

	}


}

// Realiza la comprobación de la operación *, /
void tsOpMul(attrs o1, attrs op, attrs o2, attrs* res){

    if (o1.type != o2.type) {
		printf("Error Semantico(%d): Las expresiones deben ser del mismo tipo.", line);
		return;
	}

	if (o1.type != ENTERO && o1.type != FLOTANTE) {
		printf("Error Semantico(%d): Tipo no válido. Debe ser entero o flotante.", line);
		return;
	}

	if (isArray(o1) && isArray(o2)){

		if(equalMulSize(o1,o2)){
			if (strcmp(op.lex,"/")==0){

				printf("Error Semantico(%d): Operación no permitida.", line);
				return;

			}else{

				res->type = o1.type;
				res->nDim = o1.nDim;
				res->tDim1 = o1.tDim1;
				res->tDim2 = o2.tDim2;
			}

		} else {

            printf("Error Semantico(%d): Los arrays deben ser compatibles", line);
			return;

		}

	} else {

		if (isArray(o1) && !isArray(o2)) {

			res->type = o1.type;
			res->nDim = o1.nDim;
			res->tDim1 = o1.tDim1;
			res->tDim2 = o1.tDim2;

		}

		if (!isArray(o1) && isArray(o2)){

			if (strcmp(op.lex,"/")==0){

				printf("Error Semantico(%d): Operación no permitida.", line);
				return;

			} else {

				res->type = o2.type;
				res->nDim = o2.nDim;
				res->tDim1 = o2.tDim1;
				res->tDim2 = o2.tDim2;

			}

		}

	}

}

// Realiza la comprobación de la operación &&
void tsOpAnd(attrs o1, attrs op, attrs o2, attrs* res){

    if (o1.type != o2.type) {
		printf("Error Semantico (%d): Las expresiones deben ser del mismo tipo.", line);
		return;
	}
	if (o1.type != BOOLEANO || isArray(o1) || isArray(o2)) {
		printf("Error Semantico(%d):Tipo no válido. Se esperaba BOOLEANO", line);
		return;
	}

	res->type = BOOLEANO;
	res->nDim = 0;
	res->tDim1 = 0;
	res->tDim2 = 0;

}

void tsOpXor(attrs o1, attrs op, attrs o2, attrs* res){

    if (o1.type != o2.type) {
		printf("Error Semantico (%d): Las expresiones deben ser del mismo tipo.", line);
		return;
	}
	if (o1.type != BOOLEANO || isArray(o1) || isArray(o2)) {
		printf("Error Semantico(%d):Tipo no válido. Se esperaba BOOLEANO", line);
		return;
	}

	res->type = BOOLEANO;
	res->nDim = 0;
	res->tDim1 = 0;
	res->tDim2 = 0;

}


// Realiza la comprobación de la operación ||
void tsOpOr(attrs o1, attrs op, attrs o2, attrs* res){

    if (o1.type != o2.type) {
		printf("Error Semantico (%d): Las expresiones deben ser del mismo tipo.", line);
		return;
	}
	if (o1.type != BOOLEANO || isArray(o1) || isArray(o2)) {
		printf("Error Semantico(%d):Tipo no válido. Se esperaba BOOLEANO", line);
		return;
	}

	res->type = BOOLEANO;
	res->nDim = 0;
	res->tDim1 = 0;
	res->tDim2 = 0;

}

// Realiza la comprobación de la operación ==, !=
void tsOpEqual(attrs o1, attrs op, attrs o2, attrs* res){

    if (o1.type != o2.type) {
		printf("Error Semantico (%d): Las expresiones deben ser del mismo tipo.", line);
		return;
	}
	if (isArray(o1) || isArray(o2)) {
		printf("Error Semantico(%d):Tipo no válido.  Se esperaba ENTERO o FLOTANTE.", line);
		return;
	}

	res->type = BOOLEANO;
	res->nDim = 0;
	res->tDim1 = 0;
	res->tDim2 = 0;

}

// Realiza la comprobación de la operación <, >, <=, >=, <>
void tsOpRel(attrs o1, attrs op, attrs o2, attrs* res){

    if (o1.type != o2.type) {

		printf("Error Semantico (%d): Las expresiones deben ser del mismo tipo.", line);
		return;
	}
	if ((o1.type != ENTERO && o1.type != FLOTANTE) || isArray(o1) || isArray(o2)) {
		printf("Error Semantico(%d):Tipo no válido.  Se esperaba ENTERO o FLOTANTE.", line);
		return;
	}

	res->type = BOOLEANO;
	res->nDim = 0;
	res->tDim1 = 0;
	res->tDim2 = 0;

}






// Realiza la comprobación de la llamada a una función
void tsFunctionCall(attrs id, attrs* res){

    int index = tsSearchName(id);

	if(index==-1) {

		currentFunction = -1;

		printf("\nError Semántico(%d)): Funcion: Identificador %s no encontrado.\n", line, id.lex);

  } else {

  		if (nParam != ts[index].nParam) {
  			printf("Error Semantico(%d): Número de parámetros no válido.\n", line);
  		} else {

  			currentFunction = index;
  			res->lex = strdup(ts[index].lex);
  			res->type = ts[index].type;
  			res->nDim = ts[index].nDim;
  			res->tDim1 = ts[index].tDim1;
  			res->tDim2 = ts[index].tDim2;

  		}

	}

}

// Realiza la comprobación de cada parámetro de una función
void tsCheckParam(attrs param, int checkParam){

  int posParam = (currentFunction ) + (checkParam);//posicion de la pila donde esta el parametro de la posicion checkParam

	int error =  checkParam;

	if (param.type != ts[posParam].type) {
		printf("Error Semantico(%d): Tipo del parámetro (%d) no válido.\n", line, error);
		return;
	}

	if (param.nDim != ts[posParam].nDim || param.tDim1 != ts[posParam].tDim1  || param.tDim2 != ts[posParam].tDim2) {
		printf("Error Semantico(%d): Tamanio del parámetro (%d) no válido.\n", line, error);
		return;
	}

}

//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Visualización
//

// Muestra una in de la tabla de símbolos
void printIn(int row){

    inTS e = ts[row];
	printf("\n\nTipo Entrada: %d\nLexema: %s\nTipo Dato: %d\nNum Parametros: %d\nDimensiones[i][j]: %d[%d][%d]\n",
		e.in, e.lex, e.type, e.nParam, e.nDim, e.tDim1, e.tDim2);

}

// Muestra el type de la in
void printInType(tIn type){



}

// Muestra el type del dato recibido
void printDataType(tData type){



}



// Muestra la tabla de símbolos
void printTS(){

    int j = 0;
	char *t, *e;

	printf("--------------------------------\n");
	while(j <= LIMIT-1) {
		if(ts[j].in == 0) { e = "MARK"; }
		if(ts[j].in == 1) { e = "FUNCTION"; }
		if(ts[j].in == 2) { e = "VAR"; }
		if(ts[j].in == 3) { e = "FORM"; }

		if(ts[j].type == 0) { t = "NO_ASIG"; }
		if(ts[j].type == 1) { t = "ENTERO"; }
		if(ts[j].type == 2) { t = "FLOTANTE"; }
		if(ts[j].type == 3) { t = "CARACTER"; }
		if(ts[j].type == 4) { t = "BOOLEANO"; }
		if(ts[j].type == 5) { t = "STRING"; }
		if(ts[j].type == 6) { t = "MATRIZ"; }
		if(ts[j].type == 7) { t = "NA"; }
		printf("----ELEMENTO %d-----------------\n", j);
		printf("-Entrada: %-12s", e);
		printf("-Lexema: %-12s", ts[j].lex);
		printf("-type: %-10s", t);
		printf("-nParam: %-4d", ts[j].nParam);
		printf("-nDim: %-4d", ts[j].nDim);
		printf("-tDim1: %-4d", ts[j].tDim1);
		printf("-tDim2: %-4d\n", ts[j].tDim2);
		j++;
	}
	printf("--------------------------------\n");

}

// Muestra un atributo recibido
void printAttr(attrs e, char *msg){

    char *t;

	if(e.type == 0) { t = "NO_ASIG"; }
	if(e.type == 1) { t = "ENTERO"; }
	if(e.type == 2) { t = "FLOTANTE"; }
	if(e.type == 3) { t = "CARACTER"; }
	if(e.type == 4) { t = "BOOLEANO"; }
	if(e.type == 5) { t = "STRING"; }
	if(e.type == 6) { t = "MATRIZ"; }
	if(e.type == 7) { t = "NA"; }
	printf("------%s-------------------------\n", msg);
	printf("-Atributos: %-4d", e.attr);
	printf("-Lexema: %-12s", e.lex);
	printf("-type: %-10s", t);
	printf("-nDim: %-4d", e.nDim);
	printf("-tDim1: %-4d", e.tDim1);
	printf("-tDim2: %-4d\n", e.tDim2);
	printf("-------------------------------\n");

}


/********************************************
**************generacion codigo**************
*********************************************/

FILE * file;
FILE * fileMain;
FILE * fileSubProg;

tData tipoTMP = 0;
tData tipoArray = 0;

inTS TF[MAX_IN];

int isMain = 1;
int isAsig = 0;
int temp = 0;
int tempUsado = 0;
int etiq = 0;
int varPrinc=0;
int decIF = 0,decElse=0;
int hayError = 0;

int numSubPro=0;
primeraExpresion = 0;

char * variables;
char * argumentos;
char * nombreFuncion;

char * temporal(){
	char * cadena;
	cadena = (char *) malloc(20);
	sprintf(cadena, "temp%d",temp);
	temp++;
	return cadena;
}
char * etiqueta(){
	char * cadena;
	cadena = (char *) malloc(20);
	sprintf(cadena, "etiqueta_%d",etiq);
	etiq++;
	return cadena;
}

int isOpRel(attrs op){
	int loes = 0;

	//fputs(sent,file);
	if(!strcmp(op.lex, ">"))
		loes=1;
	if(!strcmp(op.lex, "<"))
		loes=1;
	if(!strcmp(op.lex, ">="))
		loes=1;
	if(!strcmp(op.lex, "<="))
		loes=1;
	if(!strcmp(op.lex, "||"))
		loes=1;
	if(!strcmp(op.lex, "&&"))
		loes=1;
	if(!strcmp(op.lex, "^"))
		loes=1;
	if(!strcmp(op.lex, "!="))
		loes=1;

	return loes;
}

// Muestra la tabla de F
void printTF(){

    int j = 1;
	char *t, *e;

	printf("--------------------------------\n");
	while(j <= LIMIT_TF) {
    if(TF[j].in == descriptor) { e = "descriptor"; }

		t = ( TF[j].descriptor.EtiquetaEntrada);

		printf("----ELEMENTO %d-----------------\n", j);
		printf("-Entrada: %-12s", e);
		printf("-etiqueta: %s", t);
		j++;
	}
	printf("--------------------------------\n");

}

void generaExpresionSigno(attrs signo, attrs b, attrs* res){

   char * sent;
   char * temp2=temporal();
   sent = (char *) malloc(1000);

   if (isAsig == 1){
    sprintf(sent,"{ //Comienzo de traducción de la asignación\n");
    isAsig=2;
   }

   if(b.type == ENTERO){
     if(b.nDim==0)  sprintf(sent,"%sint %s;\n",sent,temp2);
   }
   else if(b.type == FLOTANTE){
     if(b.nDim==0)    sprintf(sent,"%sdouble %s;\n",sent,temp2);
   }

    if (b.nDim == 0) sprintf(sent,"%s%s = %s%s;\n",sent,temp2,signo.lex,b.lex);

    res->lex=temp2;
    if (primeraExpresion==1)
      sprintf(variables, "%s%s",variables,sent );
    else
     fputs(sent,file);
   free(sent);
}

void generaExpresion(attrs a, attrs op, attrs b, attrs* res){

  	char * sent;
   	char * temp2=temporal();
  	sent = (char *) malloc(1000);

	if (isAsig == 1){
		sprintf(sent,"{ //Comienzo de traducción de la asignación\n");
		isAsig=2;
	}
	if(isOpRel(op)){ //La expresión es booleana
    		sprintf(sent,"%sint %s;\n",sent,temp2);
	}
	else{
  		if(a.type == ENTERO){
			if(b.nDim==0)
				sprintf(sent,"%sint %s;\n",sent,temp2);
			else if (b.nDim==1){
					if(a.nDim==0){
						sprintf(sent,"%sint %s[%i];\n",sent,temp2,b.tDim1);
					}else{
						sprintf(sent,"%sint %s[%i];\n",sent,temp2,a.tDim1);
					}
				}
				else if (b.nDim==2){
					if(a.nDim==0){
						sprintf(sent,"%sint %s[%i][%i];\n",sent,temp2,b.tDim1,b.tDim2);
					}else{
						sprintf(sent,"%sint %s[%i][%i];\n",sent,temp2,a.tDim1,b.tDim2);
					}
				}
  		}
  		else if(a.type == FLOTANTE){
			if(b.nDim==0)
				sprintf(sent,"%sdouble %s;\n",sent,temp2);
						else if (b.nDim==1){
					if(a.nDim==0){
						sprintf(sent,"%sdouble %s[%i];\n",sent,temp2,b.tDim1);
					}else{
						sprintf(sent,"%sdouble %s[%i];\n",sent,temp2,a.tDim1);
					}
				}
				else if (b.nDim==2){
					if(a.nDim==0){
						sprintf(sent,"%sdouble %s[%i][%i];\n",sent,temp2,b.tDim1,b.tDim2);
					}else{
						sprintf(sent,"%sdouble %s[%i][%i];\n",sent,temp2,a.tDim1,b.tDim2);
					}
				}
  		}
  		else if(a.type == CARACTER){
  			sprintf(sent,"%schar %s;\n",sent,temp2);
  		}
  		else if(a.type == BOOLEANO){
  			/*LIMIT++;
  			ts[LIMIT].in = descriptor;
  			ts[LIMIT].descriptor.EtiquetaSalida = etiqueta();*/
    			sprintf(sent,"%sint %s;\n",sent,temp2);
  		}
	}
  	/*if(a.nDim == 1){
  		sprintf(sent,"%s[%d]",sent, a.tDim1);
  	}
  	if(a.nDim == 2){
  		sprintf(sent,"%s[%d][%d]",sent, a.tDim1, a.tDim2);
  	}*/

    if (a.nDim == 0){
      if (b.nDim == 0)
  	   sprintf(sent,"%s%s = %s%s%s;\n",sent,temp2,a.lex,op.lex,b.lex);
      else if (b.nDim==1 && (strcmp(op.lex, "*") == 0))
       sprintf(sent,"%smultplicacionEscalarVectorEntero(%s,%s,%s, %i);\n",sent,a.lex,b.lex,temp2,b.tDim1);
    }else if(a.nDim == 1){
     if(a.type == ENTERO){
      if(strcmp(op.lex, "+") == 0)
       sprintf(sent,"%ssumaVectoresEnteros (%s, %s, %s, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1);
      else if(strcmp(op.lex, "-") == 0)
        sprintf(sent,"%srestaVectoresEnteros (%s, %s, %s, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1);
      else if(strcmp(op.lex, "*") == 0)
        sprintf(sent,"%smultiplicacionVectoresEnteros (%s, %s, %s, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1);
      else if(strcmp(op.lex, "/") == 0)
        sprintf(sent,"%sdivisionVectoresEnteros (%s, %s, %s, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1);
    }else if(a.type == FLOTANTE)
     if(strcmp(op.lex, "+") == 0)
      sprintf(sent,"%ssumaVectoresReales (%s, %s, %s, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1);
     else if(strcmp(op.lex, "-") == 0)
       sprintf(sent,"%srestaVectoresReales (%s, %s, %s, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1);
     else if(strcmp(op.lex, "*") == 0)
       sprintf(sent,"%smultiplicacionVectoresReales (%s, %s, %s, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1);
     else if(strcmp(op.lex, "/") == 0)
       sprintf(sent,"%sdivisionVectoresReales (%s, %s, %s, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1);
    } else
      if(a.type == ENTERO){
       if(strcmp(op.lex, "+") == 0)
        sprintf(sent,"%ssumaMatricesEnteros (%s, %s, %s, %i, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1,a.tDim2);
       else if(strcmp(op.lex, "-") == 0)
         sprintf(sent,"%srestaMatricesEnteros (%s, %s, %s, %i, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1,a.tDim2);
       else if(strcmp(op.lex, "*") == 0)
         sprintf(sent,"%smultiplicacionMatricesEnteros (%s, %s, %s, %i, %i, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1,a.tDim2,b.tDim2);
     }else if(a.type == FLOTANTE)
      if(strcmp(op.lex, "+") == 0)
       sprintf(sent,"%ssumaMatricesReales (%s, %s, %s, %i, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1,a.tDim2);
      else if(strcmp(op.lex, "-") == 0)
        sprintf(sent,"%srestaMatricesReales (%s, %s, %s, %i, %i);\n",sent,a.lex,b.lex,temp2,a.tDim1,a.tDim2);
      else if(strcmp(op.lex, "*") == 0)
        sprintf(sent,"%smultiplicacionMatricesReales (%s, %s, %s, %i, %i %i);\n",sent,a.lex,b.lex,temp2,a.tDim1,a.tDim2,b.tDim2);


    res->lex=temp2;
    if (primeraExpresion==1)
      sprintf(variables, "%s%s",variables,sent );
    else
  	  fputs(sent,file);
  	free(sent);
}

void generaAsignacion(attrs a, attrs op, attrs b){

  	char * sent;
  	sent = (char *) malloc(1000);
	if(isAsig == 2){
        if (a.nDim==0)
            sprintf(sent,"%s = %s; \n} //Fin de traducción de la asignación\n",a.lex,b.lex);
        else if (a.nDim==1){
            if (ENTERO == a.type)
              sprintf(sent,"asignacionVectorEnteros(%s, %s, %i); \n} //Fin de traducción de la asignación\n",a.lex,b.lex,a.tDim1);
            else if (FLOTANTE == a.type)
              sprintf(sent,"asignacionVectorReales(%s, %s, %i); \n} //Fin de traducción de la asignación\n",a.lex,b.lex,a.tDim1);
        }else if (a.nDim == 2)
            if (ENTERO == a.type)
              sprintf(sent,"asignacionMatrizEnteros(%s, %s, %i, %i); \n} //Fin de traducción de la asignación\n",a.lex,b.lex,a.tDim1,a.tDim2);
            else if (FLOTANTE == a.type)
              sprintf(sent,"asignacionMatrizReales(%s, %s, %i, %i); \n} //Fin de traducción de la asignación\n",a.lex,b.lex,a.tDim1,a.tDim2);
		isAsig = 0;
	}
	else if (a.nDim==0)
      sprintf(sent,"%s = %s;\n",a.lex,b.lex);
  else if (a.nDim==1){
      if (ENTERO == a.type)
        sprintf(sent,"asignacionVectorEnteros(%s, %s, %i);\n",a.lex,b.lex,a.tDim1);
      else if (FLOTANTE == a.type)
        sprintf(sent,"asignacionVectorReales(%s, %s, %i);\n",a.lex,b.lex,a.tDim1);
  }else if (a.nDim == 2)
      if (ENTERO == a.type)
        sprintf(sent,"asignacionMatrizEnteros(%s, %s, %i, %i);\n",a.lex,b.lex,a.tDim1,a.tDim2);
      else if (FLOTANTE == a.type)
        sprintf(sent,"asignacionMatrizReales(%s, %s, %i, %i);\n",a.lex,b.lex,a.tDim1,a.tDim2);

  	fputs(sent,file);
  	free(sent);
}

void generaIf(attrs a){
	int topeTMP = LIMIT_TF;
	while(TF[topeTMP].in != descriptor){
		topeTMP--;
	}
	char * sent;
  	sent = (char *) malloc(1000);
    	sprintf(sent,"if (!%s) goto %s;\n",a.lex,TF[topeTMP].descriptor.EtiquetaElse);
  	fputs(sent,file);
  	free(sent);
}

void generaDoWhile(attrs a){
	int topeTMP = LIMIT_TF;
	while(TF[topeTMP].in != descriptor){
		topeTMP--;
	}
	char * sent;
  	sent = (char *) malloc(1000);
    	sprintf(sent,"if (%s) goto %s;\n",a.lex,TF[topeTMP].descriptor.EtiquetaEntrada);
  	fputs(sent,file);
  	free(sent);
}

void generaWhile(attrs a){
	int topeTMP = LIMIT_TF;
	while(TF[topeTMP].in != descriptor){
		topeTMP--;
	}
	char * sent;
  	sent = (char *) malloc(1000);
    	sprintf(sent,"if (!%s) goto %s;\n",a.lex,TF[topeTMP].descriptor.EtiquetaSalida);
  	fputs(sent,file);
  	free(sent);
}


// Abre un fichero para crear el código intermedio
void generaFich(){

    file = fopen("generated.c","w");
    fileMain = file;
    fileSubProg = fopen("dec_fun.c","w");

	fputs("#include <stdio.h>\n#include \"dec_dat.c\"\n",file);
	//fputs("\nint main(int argc, char *argv[] )",file);

}

// Cerrar fichero
void closeInter(){
    fclose(fileSubProg);
    fclose(fileMain);
    if(hayError)
	remove("generated.c");

}


void generaDecVar(attrs a){
	char * sent;
	sent = (char *) malloc(1000);
	if(tipoTMP == ENTERO){
		sprintf(sent,"int %s",a.lex);
	}
	else if(tipoTMP == FLOTANTE){
		sprintf(sent,"float %s",a.lex);
	}
	else if(tipoTMP == CARACTER){
		sprintf(sent,"char %s",a.lex);
	}
	else if(tipoTMP == BOOLEANO){
		/*LIMIT++;
		ts[LIMIT].in = descriptor;
		ts[LIMIT].descriptor.EtiquetaSalida = etiqueta();*/
		sprintf(sent,"int %s",a.lex);
	}
	if(a.nDim == 1){
		sprintf(sent,"%s[%d]",sent, a.tDim1);
	}
	if(a.nDim == 2){
		sprintf(sent,"%s[%d][%d]",sent, a.tDim1, a.tDim2);
	}
	sprintf(sent,"%s;\n",sent);
	fputs(sent,file);
	free(sent);
}
void genera(int type,attrs dest,attrs a, attrs op, attrs b){
	char * sent;
	sent = (char *) malloc(200);
	if(type == 1){

		sprintf(sent,"int %s;\n%s = %s %s %s;\n",dest.lex,dest.lex,a.lex,op.lex,b.lex);
		fputs(sent,file);
	}
	else if(type == 4 ){
		sprintf(sent,"%s %s %s %s\n",dest.lex,a.lex,op.lex,b.lex);
		fputs(sent,file);
	}
	free(sent);
}
/*	1. else y salida
	2. entrada y salida
*/
void insertaDesc(int type){
	LIMIT_TF++;
	TF[LIMIT_TF].in = descriptor;

	if(type == 1){
		TF[LIMIT_TF].descriptor.EtiquetaElse = etiqueta();
		TF[LIMIT_TF].descriptor.EtiquetaSalida = etiqueta();

	}else if(type == 2){
		TF[LIMIT_TF].descriptor.EtiquetaEntrada = etiqueta();
		TF[LIMIT_TF].descriptor.EtiquetaSalida = etiqueta();
	}

}
void eliminaDesc(){
	LIMIT_TF--;
}
/*	1.if con else
	2.while
	3.if sin else
*/
void insertaCond(int type){

	char * cadena, *sent;
	int topeTMP = LIMIT_TF;
	cadena = (char *) malloc(20);
	sent = (char *) malloc(150);


	while(TF[topeTMP].in != descriptor){
		topeTMP--;
	}
	if(type == 1){
		sprintf(cadena, "temp%d",temp-1);
		TF[topeTMP].lex = (char *) malloc(50);
		strcpy(TF[topeTMP].lex,cadena);
		sprintf(sent,"if(!%s) goto %s;\n",cadena,TF[topeTMP].descriptor.EtiquetaElse);
	}
	else if(type == 2){
				sprintf(cadena, "temp%d",temp-1);
				sprintf(sent,"if(!%s) goto %s;\n",cadena,TF[topeTMP].descriptor.EtiquetaSalida);
			}

	fputs(sent,file);
	free(sent);
	free(cadena);
}
void insertaEtiqElse(){
	int topeTMP = LIMIT_TF;
	char * sent;
	sent = (char *) malloc(200);

	while(TF[topeTMP].in != descriptor && topeTMP>0){
		topeTMP--;
	}
	//if(decElse == 1){
		sprintf(sent,"goto %s;\n%s:\n",TF[topeTMP].descriptor.EtiquetaSalida,TF[topeTMP].descriptor.EtiquetaElse);
	/*}
	else{
		sprintf(sent,"%s:",TF[topeTMP].descriptor.EtiquetaElse);
		}*/
	fputs(sent,file);
	free(sent);
  //printf("FUERA_ELSE\n" );
}

void insertaEtiqSalida(){
	int topeTMP = LIMIT_TF;
	char * sent;
	sent = (char *) malloc(200);
  //printf("\nAntes\n");
	while(TF[topeTMP].in != descriptor && topeTMP>0){
    //printf("\nDentro while %d\n", topeTMP);
		topeTMP--;
	}

	sprintf(sent,"%s:\n{}\n",TF[topeTMP].descriptor.EtiquetaSalida);

	fputs(sent,file);
	free(sent);
  //printf("FUERA\n" );
}
void insertaEtiqEntrada(){
	int topeTMP = LIMIT_TF;
	char * sent;
	sent = (char *) malloc(200);
	while(TF[topeTMP].in != descriptor){
		topeTMP--;
	}

	sprintf(sent,"%s:\n",TF[topeTMP].descriptor.EtiquetaEntrada);
	fputs(sent,file);
  free(sent);
}
void insertaGotoEntrada(){
	int topeTMP = LIMIT_TF;
	char * sent;
	sent = (char *) malloc(200);
	while(TF[topeTMP].in != descriptor){
		topeTMP--;
	}

	sprintf(sent,"goto %s;\n",TF[topeTMP].descriptor.EtiquetaEntrada);
	fputs(sent,file);
  free(sent);
}
void generaEntSal(int type,attrs a){

	if(type == 1){
		fputs("scanf(\"%",file);
		if(a.type == ENTERO) fputs("d",file);
		if(a.type == FLOTANTE) fputs("f",file);
		if(a.type == CARACTER) fputs("c",file);
		if(a.type == BOOLEANO) fputs("d",file);
		fputs("\",&",file);
		fputs(a.lex,file);
		fputs(");",file);
		fputs("\n",file);
	}
	if(type == 2){
		if(a.type != NA){
      char * sent;
      sent = (char *) malloc(200);
      if (a.nDim==0){
  			fputs("printf(\"%",file);
  			if(a.type == ENTERO) fputs("d",file);
  			else if(a.type == FLOTANTE) fputs("f",file);
  			else if(a.type == CARACTER) fputs("c",file);
  			else if(a.type == BOOLEANO) fputs("d",file);
  			fputs("\",",file);
  			fputs(a.lex,file);
  			fputs(");",file);
      }else if(a.nDim == 1){
        if (a.type == ENTERO)
          sprintf(sent,"mostrarVectorEnteros(%s,%i);",a.lex,a.tDim1);
        else if (a.type == FLOTANTE)
          sprintf(sent,"mostrarVectorReales(%s,%i);",a.lex,a.tDim1);
        fputs(sent,file);
        free(sent);
      }else if (a.nDim == 2){
        if (a.type == ENTERO)
          sprintf(sent,"mostrarMatrizEnteros(%s,%i,%i);",a.lex,a.tDim1,a.tDim2);
        else if (a.type == FLOTANTE)
          sprintf(sent,"mostrarMatrizReales(%s,%i,%i);",a.lex,a.tDim1,a.tDim2);
        fputs(sent,file);
        free(sent);
      }
		}else {
			fputs("printf(",file);
			fputs(a.lex,file);
			fputs(");",file);
		}
		fputs("\n",file);
	}
	if(type == 3){
		fputs("printf(",file);
		fputs(a.lex,file);
		fputs(");\n",file);
	}

}

void generaCabeceraFuncion( attrs id){
	char * sent;
  sent = (char *) malloc(200);
  if(id.type == ENTERO) fputs("int",file);
  if(id.type == FLOTANTE) fputs("double",file);
  if(id.type == CARACTER) fputs("char",file);
  if(id.type == BOOLEANO) fputs("int",file);
  sprintf(sent, " %s (",id.lex);
  fputs(sent,file);
  free(sent);
}

void generarListaParametros(attrs type,attrs id){
  char * sent;
  sent = (char *) malloc(200);
  if(type.type == ENTERO) fputs(", int",file);
  if(type.type == FLOTANTE) fputs(", double",file);
  if(type.type == CARACTER) fputs(", char",file);
  if(type.type == BOOLEANO) fputs(", int",file);
  sprintf(sent, " %s ",id.lex);
  fputs(sent,file);
  free(sent);
}

void generarPrimerParametro(attrs type,attrs id){
  char * sent;
  sent = (char *) malloc(200);
  if(type.type == ENTERO) fputs("int",file);
  if(type.type == FLOTANTE) fputs("double",file);
  if(type.type == CARACTER) fputs("char",file);
  if(type.type == BOOLEANO) fputs("int",file);
  sprintf(sent, " %s ",id.lex);
  fputs(sent,file);
  free(sent);
}

void generarNombreFuncion(attrs id, attrs* res){
  if (primeraExpresion==1){
	nombreFuncion = (char *) malloc(2000);
    sprintf(nombreFuncion, "%s( %s )",id.lex,argumentos);
	res->lex=nombreFuncion;
    free(argumentos);
    primeraExpresion =0;
  }else{
    char * sent;
    sent = (char *) malloc(200);
    sprintf(sent, "%s();\n",id.lex);
    fputs(sent,file);
    free(sent);
  }
}

void generarListaExpresiones(attrs a){
  if (primeraExpresion==0){
    variables = (char *) malloc(1000);
    argumentos = (char *) malloc(1000);
    primeraExpresion =1;
  }
  sprintf(argumentos, "%s , %s ",argumentos,a.lex);
}

void generarPrimeraExpresion(attrs a){
	if(primeraExpresion==0){
		variables = (char *) malloc(1000);
    	argumentos = (char *) malloc(1000);
		primeraExpresion=1;
	}
	sprintf(argumentos, "%s",a.lex);
}
