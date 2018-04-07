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
#include "circuits.h"

void processLine(char* str, unsigned int line_num) {
  //printf("processLine (%d) :  %s\n",line_num,str);
  char** str_arr = malloc(4 * sizeof(char*));
  char* dl = " ";
  trimString(str);
  unsigned int length = splitString(str, dl, str_arr);
  CircuitNode* nodes[length];
  for (unsigned int k = 0; k < length; k++ ) nodes[k] = NULL;
  unsigned int word_num = 0;
  assert(length >= 0 && length <= 4);
  while (word_num < length) {
    char* word = strdup(str_arr[word_num]);
    char* gate_id = malloc(2 * sizeof(char));
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

void simulateCircuit(unsigned int i, bool reset, bool synchronous) {
	// /* DBUG */ printf("Simulating internal state of circuit with input options (%d) for times up to (%d)\n",i,circuit_run_time);
	initializeCircuitInputs(i, reset, synchronous);
	for (unsigned int i = 1; i < circuit_run_time; i++) incrementCircuit(synchronous);
}

void initializeCircuitInputs(unsigned int i, bool reset, bool synchronous) {
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
			if (synchronous) circuit[k]->states[curr_circuit_time] = inp_arr[curr_inp];
      else circuit[k]->state = inp_arr[curr_inp];
			curr_inp += 1;
		}
		else if (id && !strcmp(id,"one")) {
      if (synchronous) circuit[k]->states[curr_circuit_time] = 1;
      else circuit[k]->state = 1;
    }
		else if (id && !strcmp(id,"zero")) {
      if (synchronous) circuit[k]->states[curr_circuit_time] = 0;
      else circuit[k]->state = 0;
    }
		else {
      if (reset){
         if (synchronous) circuit[k]->states[curr_circuit_time] = 0;
         else circuit[k]->state = 0;
       }
      else {
        if (synchronous) circuit[k]->states[curr_circuit_time] = circuit[k]->states[circuit_run_time - 1];
        // else (do nothing, state remains the same)
      }
    }
	 }
	}
	curr_circuit_time = 1;
}

/* Function to convert a decinal number to binary number */
void decimalToBinaryArray(int decimal, int bin[], int numBits){
	for(int i = 0; i < numBits; i++){
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

void printTimeseries( bool synchronous ) {
		printf("t|");
		for (unsigned int i = 0; i < circuit_run_time; i++) {
			if (i < circuit_run_time - 1) {
        if (synchronous) printf("%d,",i);
        else printf(". ");
      }
      else printf("%d\n",i);
		}
		simulateCircuit(0, false, synchronous);
		for (unsigned int n = 0; n < circuit_size; n++) {
			if (circuit[n]) {
			char* func = circuit[n]->func;
			if (func && !strcmp(func,"WIRE")) {
				printf("%s|",circuit[n]->id);
				for (unsigned int t = 0; t < circuit_run_time; t++ ){
					if (synchronous) printf("%d",circuit[n]->states[t]);
          else printf("%d",circuit[n]->state);
					if (t < circuit_run_time - 1) printf(",");
				}
				printf("\n");
			}
		}
	}
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

/* Function which prints a truth table for the circuit to stdout. Every possible combination of
inputs to the circut are looped through, and with each combination, the state of each wire in the
circuit is calculated until the circuit stabilises or the run_time_limit for the circuit is reached.
If the circuit does not contain an ouput, the table cannot be printed. The booleamn parameter 'reset'
is used to indicate whether the value of each non-input in the circuit should be reset to 0 at the start of each simulation
or whether they should be left in the same state as they ended on, in the previous simulation.   */
void printTruthTable( bool reset ) {
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
		simulateCircuit(i, reset, true);
		unsigned int* states = out->states;
	  ///*DBG*/printf("%d == %d == %d ", states[circuit_run_time - 1], states[circuit_run_time - 2], states[circuit_run_time - 3]);
		int inp_arr[n_inp];
		decimalToBinaryArray(i, inp_arr, n_inp);
		for (unsigned int i = 0; i < n_inp; i++) printf("%d ",inp_arr[i]);
		if (states[circuit_run_time - 1] == states[circuit_run_time - 2] && states[circuit_run_time - 1] == states[circuit_run_time - 3])
			printf("%d\n",states[circuit_run_time - 1]);
		else printf("?\n");
	}
}

CircuitNode* makeNode(char* id, char* func) {
  CircuitNode* new_node_p = malloc(sizeof(CircuitNode));
  CircuitNode new_node = {0, id, func, NULL, NULL, NULL, {0}};
  memcpy(new_node_p,&new_node,sizeof(CircuitNode));
  return new_node_p;
}

void freeCircuit( void ) {
  for(unsigned int i = 0; i < circuit_size; i++) {
    if (circuit[i]) free(circuit[i]);
  }
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

void incrementCircuit(bool synchronous){
	///*DBUG*/printf("\nIncrementing internal state of circuit\n");
	for (unsigned int j = 0; j < circuit_size; j++) {
		if (circuit[j]) {
			char* func = circuit[j]->func;
			char* id = circuit[j] -> id;
			if (!strcmp("zero", id)) {
          if (synchronous) circuit[j]->states[curr_circuit_time] = 0;
          else circuit[j]->state = 0;
      }
			else if (!strcmp("one", id))  {
        if (synchronous) circuit[j]->states[curr_circuit_time] = 1;
        else circuit[j]->state = 1;
      }
			else if (!strcmp("IN",func)) {
        if (synchronous) circuit[j]->states[curr_circuit_time] = circuit[j]->states[curr_circuit_time - 1];
        //else: do nothing, state remains the same
      }
			else if (!strcmp("WIRE",func)) {
        if (synchronous) circuit[j]->states[curr_circuit_time] = getGateOutput(circuit[j]->inp_1, synchronous);
        else circuit[j]->state = getGateOutput(circuit[j]->inp_1, synchronous);
      }
		}
	}
	curr_circuit_time += 1;
}

unsigned int getGateOutput(CircuitNode* gate, bool synchronous) {
	if (!strcmp("AND",gate->func)) {
    if (synchronous) return gate->inp_1->states[curr_circuit_time - 1] & gate->inp_2->states[curr_circuit_time - 1];
    else return gate->inp_1->state & gate->inp_2->state;
  }
	else if (!strcmp("NAND",gate->func)) {
    if (synchronous) return !(gate->inp_1->states[curr_circuit_time - 1] & gate->inp_2->states[curr_circuit_time - 1]);
    else return !(gate->inp_1->state & gate->inp_2->state);
  }
	else if (!strcmp("OR",gate->func)) {
    if (synchronous) return gate->inp_1->states[curr_circuit_time - 1] | gate->inp_2->states[curr_circuit_time - 1];
    else return gate->inp_1->state | gate->inp_2->state;
  }
	else if (!strcmp("NOR",gate->func)) {
    if (synchronous) return !(gate->inp_1->states[curr_circuit_time - 1] | gate->inp_2->states[curr_circuit_time - 1]);
    else return !(gate->inp_1->state | gate->inp_2->state);
  }
	else if (!strcmp("XOR",gate->func)) {
    if (synchronous) return gate->inp_1->states[curr_circuit_time - 1] ^ gate->inp_2->states[curr_circuit_time - 1];
    else return gate->inp_1->state ^ gate->inp_2->state;
  }
	else if (!strcmp("IN",gate->func)) {
    if (synchronous) return gate->states[curr_circuit_time - 1];
    else return gate->state;
  }
	else if (!strcmp("NOT",gate->func)) {
    if (synchronous) return !(gate->inp_1->states[curr_circuit_time - 1]);
    else return !(gate->inp_1->state);
  }
	else {
		printf("Unknown gate function: %s. Assuming buffer\n",gate->func);
		if (synchronous) return gate->inp_1->states[curr_circuit_time - 1];
    else return gate->inp_1->state;
	}
}

void setCircuitRunTime( void ) {
  unsigned int n = 0;
  for (unsigned int j = 0; j < circuit_size; j++) {
    if (circuit[j]) {
      char* func = circuit[j]->func;
      if (func && !strcmp("WIRE",func)) n += 1;
    }
  }
  circuit_run_time = pow(2,n) + 1;
  if (circuit_run_time > max_circuit_time) circuit_run_time = max_circuit_time;
  else if (circuit_run_time < min_circuit_time) circuit_run_time = min_circuit_time;
}

bool isCircuitComplete( void ) {
  unsigned int j = 0, complete = true;
  for ( ; j < circuit_size; j++) {
    if (circuit[j]) {
      if (!strcmp("one", circuit[j]->id) || !strcmp("zero", circuit[j]->id)) complete = complete && (!circuit[j]->inp_1 && !circuit[j]->inp_2 && circuit[j]->out);
      else if (!strcmp("out", circuit[j]->id)) complete = complete && circuit[j]->inp_1;
      else if (!strcmp("WIRE",circuit[j]->func)) complete = complete && (circuit[j]->inp_1 && circuit[j]->out);
      else if (!strcmp("IN",circuit[j]->func)) complete = complete && circuit[j]->out;
      else if (!strcmp("NOT",circuit[j]->func)) complete = complete && circuit[j]->inp_1 && circuit[j]->out;
      else complete = complete && (circuit[j]->inp_1 && circuit[j]->inp_2 && circuit[j]->out);
    }
    else break;
  }
  return (j > 0 && complete);
}

int main() {

  /* Initialize all circuit elements to null */
  initCircuit();

  /* The line read from stdin */
  char* line = NULL;

  /* The size of the buffer to read from stdin */
  size_t len = 0;

  /* Line number, used to identify particular gates in the circuit */
  unsigned int line_num = 0;

  /*Read from stdin line by line, and build the circuit with each line */
  while (getline(&line, &len, stdin) != -1){
    if (is_empty(line)) continue;
    processLine(line, line_num++);
    if (isCircuitComplete()) break;
  }
  free(line);
  setCircuitRunTime();
  //printCircuit();
  printTruthTable(true);
  //printTimeseries(true);
  freeCircuit();
	return 0;
}
