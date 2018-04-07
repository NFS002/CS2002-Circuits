/**********************************************
 ***************** Globals ********************
 ***********************************************/

/* This number (1000000) was chosen as the runtime upper limit because computing truth tables and timeseries for any longer times
  would signifiicantly reduce performance. */
const unsigned int max_circuit_time = 1000000;
/*If every node in the circuit was a wire, given the runtime limit, there could be a maximum of
 Log2(max_circuit_time) = Log10(max_circuit_time)/Log10(2) nodes in the circuit, so therfore this number (20) was chosen as the maximum circut size
 C99 does not have a log2 function, so we must convrert to log10. I have chosen to precompute
 this number, and round down to the closest integer, so as to improve performance of the program, rather than computing it every time it is used. */
const unsigned int circuit_size = 20;

/* The minimum run time for the circuit */
const unsigned int min_circuit_time = 6;

/* The current time at which the circuit is */
unsigned int curr_circuit_time = 0;

/* The run time for the circuit, determined by the size of the circuit,
* and set after the circuit has been fully formed */
unsigned int circuit_run_time;

/**
* \struct.
* \typedef.
* The CircuitNode struct, representing either a wire or a gate in the circuit.
*/
typedef struct CircuitNode
{
  int state; // This attribute is used in case of asynchronous simulation
  char* id; // The identifier (name), unique in the circuit.
  char* func; // The function of this circuitnode. See below for a list of all possible functions. Depending on the function, only some of the members of the struct are in use.
	struct CircuitNode* out; // The output of this CircuitNode
	struct CircuitNode* inp_1; // The first input of this CircuitNode
  struct CircuitNode* inp_2; // The second input of the Circuit Node, only used in case of binary operators.
	unsigned int states[max_circuit_time]; // This attribute is used in case of synchronous simulation
} CircuitNode;

/**
* List of all supported functions:
* AND
* NAND
* WIRE
* NOT
* OR
* NOR
* XOR
* IN
**/

/* The curcuit, an array of CurcuitNode elements */
CircuitNode* circuit[circuit_size];


/**********************************************
 ********** Function protoypes ****************
 ***********************************************/
void processLine(char* str, unsigned int line_num);
unsigned int splitString(char str[], char* dl, char** res);
void trimString(char* s);
int is_empty(const char *s);
void append(CircuitNode* new_node);
CircuitNode* findNodebyId(char* id);
CircuitNode* makeNode(char* id, char* func);
void printCircuit( void );
void initCircuit( void );
void printNode( CircuitNode* node );
void printTruthTable( bool reset );
void printCircuitInputs( void );
void decimalToBinaryArray(int decimal, int bin[], int numBits);
void initializeCircuitInputs(unsigned int i, bool reset, bool synchronous);
void simulateCircuit(unsigned int i, bool reset, bool synchronous);
void incrementCircuit(bool synchronous);
unsigned int getGateOutput(CircuitNode* gate, bool synchronous);
void printTimeseries(bool synchronous);
unsigned int countInputs( void );
void setCircuitRunTime( void );
void freeCircuit( void );
bool isCircuitComplete( void );
