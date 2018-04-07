#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

const unsigned int max_circuit_time = 10;
const unsigned short int min_iter = 10;
const unsigned short int max_iter = 50;
unsigned int curr_circuit_time = 0;
unsigned int circuit_size = 20;

typedef struct CircuitNode
{
  int state;
  char* id;
  char* func;
	struct CircuitNode* out;
	struct CircuitNode* inp_1;
  struct CircuitNode* inp_2;
	unsigned int states[max_circuit_time];
} CircuitNode;

typedef struct {
	struct CircuitNode** circuit_nodes;
  unsigned int size;
	unsigned int circuit_run_time;
	unsigned int curr_circuit_time;
} *Circuit_ptr;


CircuitNode* circuit[20]; //Abitary start size;

void processLine(char* str, unsigned int line_num);
unsigned int splitString(char str[], char* dl, char** res);
void trimString(char* s);
int is_empty(const char *s);
void append(CircuitNode* new_node);
CircuitNode* findNodebyId(char* id );
CircuitNode* makeNode(char* id, char* func);
void printCircuit( void );
void initCircuit( void );
void printNode( CircuitNode* node );
void printTruthTable( void );
void printCircuitInputs( void );
void decimalToBinaryArray(int decimal, int bin[], int numBits);
void initializeCircuitInputs(unsigned int i);
void simulateCircuit(unsigned int i);
void incrementCircuit( void );
unsigned int getGateOutput(CircuitNode* gate);
void printTimeseries( void );
unsigned int countInputs( void );
unsigned int countCircuitWires(Circuit_ptr circuit);
void freeCircuit(Circuit_ptr circuit);
Circuit_ptr newCircuit( void );
void finalizeBuild(Circuit_ptr circuit);

void processLine(char* str, unsigned int line_num) {
  //printf("processLine (%d) :  %s\n",line_num,str);
  char** str_arr = malloc(sizeof(char**));
  char* dl = " ";
  trimString(str);
  unsigned int length = splitString(str, dl, str_arr);
  CircuitNode* nodes[length];
  for (unsigned int k = 0; k < length; k++ ) nodes[k] = NULL;
  unsigned int word_num = 0;
  assert(length >= 0 && length <= 4);
  while (word_num < length) {
    char* word = strdup(str_arr[word_num]);
    char* gate_id = malloc(sizeof(char));
    CircuitNode* node;
    switch(word_num){
      case 3: //WIRE: ID = word, FUNC = "WIRE"
        node = findNodebyId(word);
        if (!node) {
          node = makeNode(word,"WIRE");
          append(node);
        }
        break;
      case 2: // WIRE
        node = findNodebyId(word);
        if (!node) {
          node = makeNode(word,"WIRE");
          append(node);
        }
        break;
      case 1: // GATE
        sprintf(gate_id,"%s%d","g",line_num);
        node = makeNode(gate_id,word);
          append(node);
        break;
      case 0: // WIRE
        node = findNodebyId(word);
        if (!node) {
          node = makeNode(word,"WIRE");
          append(node);
        }
        break;
    }
    nodes[word_num] = node;
    word_num += 1;
  }
  if (length > 3) {
    nodes[3]->out = nodes[1];
    nodes[1]->inp_2 = nodes[3];
  }
  if (length > 2) {
    nodes[2]->out = nodes[1];
    nodes[1]->inp_1 = nodes[2];
  }
  if (length > 1) {
    nodes[1]->out = nodes[0];
    nodes[0]->inp_1 = nodes[1];
  }
}

void simulateCircuit(unsigned int i) {
	// /* DBUG */ printf("Simulating internal state of circuit with input options (%d) for times up to (%d)\n",i,max_circuit_time);
	initializeCircuitInputs(i);
	for (unsigned int i = 0; i < max_circuit_time; i++) incrementCircuit();
}

void initializeCircuitInputs(unsigned int i) {
	curr_circuit_time = 0;
	unsigned int n_inp = countInputs();
	int inp_arr[n_inp];
	unsigned int curr_inp = 0;
	decimalToBinaryArray(i, inp_arr, n_inp); //inp_arr should be the length of n_inp
	for (unsigned int k = 0; k < circuit_size; k++){
		if (circuit[k]) {
		char* func = circuit[k]->func;
		char* id = circuit[k]->id;
		if (func && !strcmp(func,"IN")) {
			///* DBUG */ printf("found input");
			circuit[k]->states[curr_circuit_time] =/*circuit[k]->out->states[0] =*/inp_arr[curr_inp];
			curr_inp += 1;
		}
		else if (id && !strcmp(id,"one")) circuit[k]->states[curr_circuit_time] = 1;
		else if (id && !strcmp(id,"zero")) circuit[k]->states[curr_circuit_time] = 0;
		else circuit[k]->states[curr_circuit_time] = 0;
		}
	}
	curr_circuit_time = 1;
}

/* Function to convert a decinal number to binary number */
void decimalToBinaryArray(int decimal, int bin[], int numBits){
	for(unsigned int i = 0; i < numBits; i++){
			bin[i] = 1 & (decimal >> i);
	}
	//reverse array
	int i = numBits - 1;
 	int j = 0;
 	while(i > j) {
	 int temp = bin[i];
	 bin[i] = bin[j];
	 bin[j] = temp;
	 i--;
	 j++;
 	}
}

void printTimeseries( void ) {
		printf("t|");
		for (int i = 0; i < max_circuit_time; i++) {
			printf("%d",i);
			if (i < max_circuit_time - 1) printf(",");
		}
		printf("\n");
		simulateCircuit(0);
		for (unsigned int n = 0; n < circuit_size; n++) {
			if (circuit[n]) {
			char* func = circuit[n]->func;
			if (func && !strcmp(func,"WIRE")) {
				printf("%s|",circuit[n]->id);
				for (unsigned int t = 0; t < max_circuit_time; t++ ){
					printf("%d",circuit[n]->states[t]);
					if (t < max_circuit_time - 1) printf(",");
				}
				printf("\n");
			}
		}
	}
	printf("\n\n");
}

void printCircuitInputs( void ) {
	for (unsigned int j = 0; j < circuit_size; j++) {
		if (circuit[j]) {
			char* func = circuit[j]->func;
			if (func && !strcmp("IN",func)) printf("%s ",circuit[j]->out->id);
		}
	}
}

unsigned int countInputs( void ) {
	unsigned int n = 0;
	for (unsigned int j = 0; j < circuit_size; j++) {
		if (circuit[j]) {
			char* func = circuit[j]->func;
			if (func && !strcmp("IN",func)) n += 1;
		}
	}
	return n;
}

unsigned int countCircuitWires(Circuit_ptr circuit) {
  unsigned int n = 0;
  for (unsigned int j = 0; j < circuit_size; j++) {
    if (circuit->circuit_nodes[j]) {
      char* func = circuit->circuit_nodes[j]->func;
      if (func && !strcmp("WIRE",func)) n += 1;
    }
  }
  return n;
}

void printTruthTable( void ) {
	CircuitNode* out = findNodebyId("out");
	if (!out) {
		puts("No circuit output found, cannot write truth table");
		return;
	}
	unsigned int n_inp = countInputs();
	printCircuitInputs();
	unsigned int n_combo_inp = pow(2,n_inp);
	printf("out\n");
	for (unsigned int i = 0; i < n_combo_inp; i++ ) {
		simulateCircuit(i);
		unsigned int* states = out->states;
	  ///*DBG*/printf("%d == %d == %d ", states[max_circuit_time - 1], states[max_circuit_time - 2], states[max_circuit_time - 3]);
		int inp_arr[n_inp];
		decimalToBinaryArray(i, inp_arr, n_inp);
		for (unsigned int i = 0; i < n_inp; i++) printf("%d ",inp_arr[i]);
		if (states[max_circuit_time - 1] == states[max_circuit_time - 2] && states[max_circuit_time - 1] == states[max_circuit_time - 3])
			printf("%d",states[max_circuit_time - 1]);
		else printf("?");
    if (i < n_combo_inp - 1) printf("\n");
	}
}

CircuitNode* makeNode(char* id, char* func) {
  CircuitNode* new_node_p = malloc(sizeof(CircuitNode));
  CircuitNode new_node = {0, id, func, NULL, NULL, NULL};
  memcpy(new_node_p,&new_node,sizeof(CircuitNode));
  return new_node_p;
}

unsigned int splitString(char str[], char* dl, char** res){
    char *token;
    char *rest = str;
    unsigned int i = 0;
    while ((token = strtok_r(rest, dl, &rest))) {
        res[i] = malloc(20 * sizeof(char));
        res[i] = token;
        i++;
    }
    return i;
}

void trimString(char* s) {
    char * p = s;
    int l = strlen(p);
    while(isspace(p[l - 1])) p[--l] = 0;
    while(*p && isspace(* p)){
      ++p;
      --l;
    }
    memmove(s, p, l + 1);
}

int is_empty(const char *s) {
  while (*s != '\0') {
    if (!isspace((unsigned char)*s))
      return 0;
    s++;
  }
  return 1;
}

CircuitNode* findNodebyId(char* id) {
  ///*DBG*/printf("findNodebyId: %s\n",id);
    for (unsigned int k = 0; k < circuit_size; k++ ) {
      if (!circuit[k]) break;
      char* currId = circuit[k]->id;
      if (currId && !strcmp(id,currId)) {
          return circuit[k];
      }
    }
    //puts("Node not found, returning NULL");
    return NULL;
}

/* Initialize all ciruit elements to null */
void initCircuit( void ) {
  for (unsigned int k = 0; k < circuit_size; k++ ) circuit[k] = NULL;
}

Circuit_ptr newCircuit( void ) {
  Circuit_ptr circuit = ((Circuit_ptr) malloc(sizeof(Circuit_ptr)));
  circuit->size = 0;
  circuit->curr_circuit_time = 0;
  circuit->circuit_run_time = 0;
  circuit->circuit_nodes = NULL;
  return circuit;
}

void finalizeBuild(Circuit_ptr circuit) {
  circuit->circuit_run_time = pow(2,countCircuitWires(circuit));
}

void freeCircuit(Circuit_ptr circuit) {
  for(int i = 0; i < circuit->size; i ++ ) free(circuit->circuit_nodes[i]);
  free(circuit);
}

void append(CircuitNode* new_node){
  //printf("Appending new node to circuit:  %s\n",new_node->id);
    for (unsigned int k = 0; k < circuit_size; k++ ) {
      if (!circuit[k]) {
        circuit[k] = new_node;
        //printf("Sucessfully appended\n");
        return;
      }
    }
    puts("Circuit full, append failed\n");
}

void printCircuit( void ){
  printf("\n---Print circuit---\n");
    for (unsigned int k = 0; k < circuit_size; k++ ) {
      if (!circuit[k]) break;
      printf("Circuit node (%d):",k);
      printNode(circuit[k]);
      printf("---Input 1----");
      if (circuit[k]->inp_1) printNode(circuit[k]->inp_1);
      else printf("\nNo input  found\n");
      puts("--------------");
      printf("---Input 2----");
      if (circuit[k]->inp_2)printNode(circuit[k]->inp_2);
      else printf("\nNo second input found\n");
      puts("--------------");
      printf("---Output---");
      if (circuit[k]->out) printNode(circuit[k]->out);
      else printf("\nNo output found\n");
      puts("--------------");
    }
    puts("--------------");
}

void printNode(CircuitNode* node ) {
  printf("\nAddress: (%p)\nId:  (%s)\nFunction:  (%s)\n",
    node,node->id,node->func);
}

void incrementCircuit( void ){
	///*DBUG*/printf("\nIncrementing internal state of circuit\n");
	for (unsigned int j = 0; j < circuit_size; j++) {
		if (circuit[j]) {
			char* func = circuit[j]->func;
			char* id = circuit[j] -> id;
			if (!strcmp("zero", id))  circuit[j]->states[curr_circuit_time] = 0;
			else if (!strcmp("one", id))  circuit[j]->states[curr_circuit_time] = 1;
			else if (!strcmp("IN",func)) circuit[j]->states[curr_circuit_time] = circuit[j]->states[curr_circuit_time - 1];
			else if (!strcmp("WIRE",func)) circuit[j]->states[curr_circuit_time] = getGateOutput(circuit[j]->inp_1);
		}
	}
	curr_circuit_time += 1;
}

unsigned int getGateOutput(CircuitNode* gate) {
	if (!strcmp("AND",gate->func)) return gate->inp_1->states[curr_circuit_time - 1] & gate->inp_2->states[curr_circuit_time - 1];
	else if (!strcmp("NAND",gate->func)) return !(gate->inp_1->states[curr_circuit_time - 1] & gate->inp_2->states[curr_circuit_time - 1]);
	else if (!strcmp("OR",gate->func)) return gate->inp_1->states[curr_circuit_time - 1] | gate->inp_2->states[curr_circuit_time - 1];
	else if (!strcmp("NOR",gate->func)) return !(gate->inp_1->states[curr_circuit_time - 1] | gate->inp_2->states[curr_circuit_time - 1]);
	else if (!strcmp("XOR",gate->func)) return gate->inp_1->states[curr_circuit_time - 1] ^ gate->inp_2->states[curr_circuit_time - 1];
	else if (!strcmp("IN",gate->func))  return gate->states[curr_circuit_time - 1];
	else if (!strcmp("NOT",gate->func)) return !(gate->inp_1->states[curr_circuit_time - 1]);
	else {
		printf("Unknown gate function: %s. Assuming buffer\n",gate->func);
		return gate->inp_1->states[curr_circuit_time - 1];
	}
}

int main() {

  initCircuit();

  /* The line read from stdin */
  char* line = NULL;

  /* The size of the buffer to read from stdin */
  size_t len = 0;

  /* Line number */
  unsigned int line_num = 0;

  /*Read from stdin line by line, and build the circuit with each line */
  while (getline(&line, &len, stdin) != -1){
    if (is_empty(line)) break;
    else {
      processLine(line, line_num);
      line_num++;
    }
  }
    free(line);
  printTruthTable();
  //printCircuit();
  //printTimeseries();
	return 0;
}
