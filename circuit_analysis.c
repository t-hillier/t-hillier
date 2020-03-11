/*
@author Candidate number: 10092
@date 27/04/2019
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


/** define a new type called cData that can have values for nodes, 
    resistances and values to determine whether a shunt or series 
	resistance is specified for an element. */
typedef struct circuitData
{
	int n1, n2, Gtrue, Rtrue;
	double R, G;
}cData;


/** define a new type called tData that can have values for source 
    voltage and resistance aswell as the load resistance. */
typedef struct termsData
{
	double VT, RS, RL;
	int INtrue;
}tData;


/** define a new type called oData that can have values for 
    all output variables including their respective dB values. */
typedef struct outputData
{
	double Vin, VindB, Vout, VoutdB, Iin, IindB, Iout, IoutdB,
		Pin, PindB, Zout, ZoutdB, Pout, PoutdB, Zin, ZindB,
		Av, AvdB, Ai, AidB;
}oData;


/** Routine to determine the number of circuit elements there are.
@param FILE *fin    pointer to the input file to open and read
@return int   The integer number of elements in the CIRCUIT block
*/
int find_no_of_cct_elements(FILE *fin)
{                                     
	char *line = (char*)malloc(sizeof(char)*150);   // 'line' buffer created to read each line of the file
	while (!feof(fin))
	{
		fgets(line, 150, fin);                      // Gets each line until the end of the file is reached
		if (strstr(line, "<CIRCUIT>\n"))            // Checks to see if the start circuit delimiter is on the line
		{
			int noOfElements = 0;
			while (!strstr(line, "</CIRCUIT>\n"))
			{
				fgets(line, 150, fin);              // Gets each new line in the CIRCUIT block
				if (strstr(line, "n1="))            // Incremenets the number of elements found if "n1=" is found
					noOfElements += 1;
			}
			free(line);                             // Gives back the memory used for retrieving lines to the operating system
			return noOfElements;
		}
	}
	printf("Error: Circuit delimiter not found. \n");   
	exit(-1);
}


/** Routine to check each element from the CIRCUIT block for errors. Returned values determine whether the program exits with value -1
@param cData *cPtr   pointer to the structure holding a given elements information 
@param int elementNo   The integer number of which element is being checked
@param int n1Check   returned int value from sscanf_s for the n1 value
@param int n2Check   returned int value from sscanf_s for the n2 value
@param int RCheck   returned int value from sscanf_s for the R value
@param int GCheck   returned int value from sscanf_s for the G value
@return int    0 if there is an error, 1 if there isn't
*/
int check_cct(cData *cPtr, int elementNo, int n1Check, int n2Check, int RCheck, int GCheck)
{
	// Solves for if the n2 value suggests its a shunt resistance though a series resistance value was given
	if (cPtr->n2 == 0 && cPtr->Rtrue == 1) {
		cPtr->G = 1 / cPtr->R;
		cPtr->R = 0;
		cPtr->Gtrue = 1;
		cPtr->Rtrue = 0; 
		printf_s("Warning: incompatible n2 and R values for element %d. Automatic corrections made.\n", elementNo);
	}
	// Solves for if the n2 value suggests its a series resistance though a shunt resistance value was given
	if (cPtr->n2 > 0 && cPtr->Gtrue == 1) {
		cPtr->R = 1 / cPtr->G;
		cPtr->G = 0;
		cPtr->Gtrue = 0;
		cPtr->Rtrue = 1;
		printf_s("Warning: incompatible n2 and G values for element %d. Automatic corrections made.\n", elementNo);
	}
	// Generates an error message and exits the program upon returning 0 to the call if the two node values of an element are equal
	if (cPtr->n1 == cPtr->n2) {
		printf_s("Error: Circuit element %d n1 and n2 connections are equal.\n", elementNo);
		return 0;
	}
	// Generates an error message and exits the program upon returning 0 to the call if the elements n1 value is negative
	if (cPtr->n1 < 0) {
		printf_s("Error: Circuit element %d has a negative n1 value.\n", elementNo);
		return 0;
	}
	// Generates an error message and exits the program upon returning 0 to the call if the elements n2 value is negative
	if (cPtr->n2 < 0) {
		printf_s("Error: Circuit element %d has a negative n2 value.\n", elementNo);
		return 0;
	}
	// Generates an error message and exits the program upon returning 0 to the call if the elements R value is negative
	if (cPtr->R < 0) {
		printf_s("Error: Circuit element %d has a negative R value.\n", elementNo);
		return 0;
	}
	// Generates an error message and exits the program upon returning 0 to the call if the elements G value is negative
	if (cPtr->G < 0) {
		printf_s("Error: Circuit element %d has a negative G value.\n", elementNo);
		return 0;
	}
	// Generates an error message and exits the program upon returning 0 to the call if the n1 data type was anything other than an integer
	if (n1Check == 0) {
		printf_s("Error: Circuit element %d has an invalid n1 data type.\n", elementNo);
		return 0;
	}
	// Generates an error message and exits the program upon returning 0 to the call if the n2 data type was anything other than an integer
	if (n2Check == 0) {
		printf_s("Error: Circuit element %d has an invalid n2 data type.\n", elementNo);
		return 0;
	}
	// Generates an error message and exits the program upon returning 0 to the call if the R data type was anything other than a double
	if (RCheck == 0) {
		printf_s("Error: Circuit element %d has an invalid R data type.\n", elementNo);
		return 0;
	}
	// Generates an error message and exits the program upon returning 0 to the call if the G data type was anything other than a double
	if (GCheck == 0) {
		printf_s("Error: Circuit element %d has an invalid G data type.\n", elementNo);
		return 0;
	}
	// No errors detected, returns 1 and continues the program
	return 1;
}


/** Routine to check the TERMS block values for errors. Returned values determine whether the program exits with value -1
@param tData *tPtr   pointer to the structure holding the TERMS block information
@param int VTcheck   returned int value from sscanf_s for the VT value, if specified
@param int INCheck   returned int value from sscanf_s for the IN value, if specified
@param int RSCheck   returned int value from sscanf_s for the RS value, if specified
@param int RLcheck   returned int value from sscanf_s for the RL value, if specified
@param int GScheck   returned int value from sscanf_s for the GS value, if specified
@param int GLcheck   returned int value from sscanf_s for the GL value, if specified
@return int   0 if there is an error, 1 if there isn't
*/
int check_terms(tData *tPtr, int VTcheck, int INcheck, int RScheck, int RLcheck, int GScheck, int GLcheck)
{
	// Generates an error and exits the program upon returning 0 to the call if the files specified current source data type is anything other than a double
	if (INcheck == 0) {
		printf_s("Error: Invalid data type for IN.\n");
		return 0;
	}
	// Generates an error and exits the program upon returning 0 to the call if the source voltage is <= 0
	if (tPtr->VT <= 0) {
		printf_s("Error: VT is required to be a positive value.\n");
		return 0;
	}
	// Generates an error and exits the program upon returning 0 to the call if the files specified source voltage data type is anything other than a double
	if (VTcheck == 0) {
		printf_s("Error: Invalid data type for VT.\n");
		return 0;
	}
	// Error generated if a negative load resistance is specified
	if (tPtr->RL < 0) {
		printf_s("Error: RL is required to be a positive value.\n");
		return 0;
	}
	// Generates an error and exits the program upon returning 0 to the call if the files specified load resistance data type is anything other than a double
	if (RLcheck == 0) {
		printf_s("Error: Invalid data type for RL.\n");
		return 0;
	}
	// Error generated if a negative source resistance is specified
	if (tPtr->RS < 0) {
		printf_s("Error: RS is required to be a positive value.\n");
		return 0;
	}
	// Generates an error and exits the program upon returning 0 to the call if the files specified source resistance data type is anything other than a double
	if (RScheck == 0) {
		printf_s("Error: Invalid data type for RS.\n");
		return 0;
	}
	// Generates an error and exits the program upon returning 0 to the call if the files specified source shunt resistance data type is anything other than a double
	if (GScheck == 0) {
		printf_s("Error: Invalid data type for GS.\n");
		return 0;
	}
	// Generates an error and exits the program upon returning 0 to the call if the files specified load shunt resistance data type is anything other than a double
	if (GLcheck == 0) {
		printf_s("Error: Invalid data type for GL.\n");
		return 0;
	}
	// No errors detected, returns 1 and continues the program
	return 1;
}


/** Routine to open the input file catching error messages if the file open fails. 
Exits the program with value -1 if the file open fails.
@return FILE*  The input file pointer
*/
FILE* open_input_file()
{ 
	FILE *fin;    
	printf("Opening <ABCD_input.dat> in <reading> mode\n\n");
	fflush(stdout); 

	fopen_s(&fin, "ABCD_input.dat", "r");
	if (fin == NULL)       // i.e.an error occurred
	{
		char *command = (char*)malloc(sizeof(char) * 50);
		strcpy_s(command, 4, "dir");
		system(command);
		perror("Error");   // prints description of the error 
		free(command);
		exit(-1);          // −1 indicates error 
	}
	return fin;
}


/** Routine to open the output file catching error messages if the file open fails. Exits the program with vsalue -1 if the file open fails.
@param char *filename   The name of the output file to open
@return FILE*  The input file pointer
*/
FILE* open_output_file(char *filename)
{
	FILE *fout;
	printf_s("\n");
	printf("Opening <%s> in <writing> mode\n\n", filename);
	fflush(stdout);

	fopen_s(&fout, filename, "w");
	if (fout == NULL)     // i.e.an error occurred
	{
		char *command = (char*)malloc(sizeof(char) * 50);
		strcpy_s(command, 4, "dir");
		system(command);
		perror("Error");  // prints description of the error 
		free(command);
		exit(-1);         // −1 indicates error 
	}
	return fout;
}


/** Routine to swap structures in order dependant on their n1 and n2 values. This routine is solely used by the function qsort.
@param const void *a   The first element 
@param const void *b   The second element
return int   -1 if *a comes before *b, 1 if *b comes before *a and 0 if they are the same
*/
int comparison(const void *a, const void *b)
{
	cData *p1 = (void*)a;
	cData *p2 = (void*)b;
	if ((p1->n1 - p2->n1) < 0)  // i.e. if the element pointed to by p1 has a n1 value that is less that p2's n1, swap. 
		return -1;
	else if ((p1->n1 - p2->n1) > 0) // If p1's n1 is a higher value than p2's n1 then they are in order, do not swap.
		return 1;
	else { // I.e. both elements n1 values are the same. 
		// Swaps if p1's n2 value is smaller than p2's or if p1's n2 is less than its n1.
		if ((p1->n2 - p2->n2) < 0 || (p1->n2 != 0 && p1->n2 < p1->n1)) 
			return -1;
		else if ((p1->n2 - p2->n2) > 0)  // If p1's n2 is a higher value than p2's n2 then they are in order, do not swap. 
			return 1;
		else 
			return 0;
	}
}


/** Routine to check whether a prefix has been specified with any series or shunt resistance value 
@param char *R   Buffer pointer to the character array holding the string with the series resistance information
@param char *G   Buffer pointer to the character array holding the string with the shunt resistance information
@param char *RS   Buffer pointer to the character array holding the string with the source series resistance information
@param char *GS   Buffer pointer to the character array holding the string with the source shunt resistance information
@param char *RL   Buffer pointer to the character array holding the string with the load series resistance information
@param char *GL   Buffer pointer to the character array holding the string with the load shunt resistance information
@return int   returns an int value used to determine what prefix, if any, has been specified
*/
int check_prefix(char *R, char *G, char *RS, char *GS, char *RL, char *GL) 
{
	char *p = NULL, *n = NULL, *u = NULL, 
		 *m = NULL, *k = NULL, *M = NULL, *giga = NULL; // pointers to find if a prefix is specified.
	if (R != NULL) {
		p = strstr(R + 1, "p");    //************************************
		n = strstr(R + 1, "n");    //
		u = strstr(R + 1, "u");    // Pointers to find whether a prefix 
		m = strstr(R + 1, "m");    // has been specified for any element
		k = strstr(R + 1, "k");    // or variable.
		M = strstr(R + 1, "M");    //
		giga = strstr(R + 1, "G"); //************************************
		if (p != NULL)
			return 1;
		else if (u != NULL)
			return 3;
		else if (m != NULL)
			return 4;
		else if (k != NULL)
			return 5;
		else if (M != NULL)
			return 6;
		else if (giga != NULL)
			return 7;
		else if (n != NULL) {
			if (*(n + 1) == '1' || *(n + 1) == '2') // i.e. A node has been found not the nano prefix
				return 0;
			return 2; // If its not a node that was found, return 2 which corresponds to nano
		}
		else
			return 0;
	}
	else if (G != NULL) {
		p = strstr(G + 1, "p");    //************************************
		n = strstr(G + 1, "n");    //
		u = strstr(G + 1, "u");    // Pointers to find whether a prefix 
		m = strstr(G + 1, "m");    // has been specified for any element
		k = strstr(G + 1, "k");    // or variable.
		M = strstr(G + 1, "M");    //
		giga = strstr(G + 1, "G"); //************************************
		if (p != NULL)
			return 1;
		else if (u != NULL)
			return 3;
		else if (m != NULL)
			return 4;
		else if (k != NULL)
			return 5;
		else if (M != NULL)
			return 6;
		else if (giga != NULL)
			return 7;
		else if (n != NULL) {
			if (*(n + 1) == '1' || *(n + 1) == '2') // i.e. A node has been found not the nano prefix
				return 0;
			return 2; // If its not a node that was found, return 2 which corresponds to nano
		}
		else
			return 0;
	}
	else if (RS != NULL) {
		p = strstr(RS + 2, "p");    //************************************
		n = strstr(RS + 2, "n");    //
		u = strstr(RS + 2, "u");    // Pointers to find whether a prefix 
		m = strstr(RS + 2, "m");    // has been specified for any element
		k = strstr(RS + 2, "k");    // or variable.
		M = strstr(RS + 2, "M");    //
		giga = strstr(RS + 2, "G"); //************************************
		if (p != NULL)
			return 1;
		else if (u != NULL)
			return 3;
		else if (m != NULL)
			return 4;
		else if (k != NULL)
			return 5;
		else if (M != NULL)
			return 6;
		else if (giga != NULL)
			return 7;
		else if (n != NULL) {
			if (*(n + 1) == '1' || *(n + 1) == '2') // i.e. A node has been found not the nano prefix
				return 0;
			return 2; // If its not a node that was found, return 2 which corresponds to nano
		}
		else
			return 0;
	}
	else if (GS != NULL) {
		p = strstr(GS + 2, "p");    //************************************
		n = strstr(GS + 2, "n");    //
		u = strstr(GS + 2, "u");    // Pointers to find whether a prefix 
		m = strstr(GS + 2, "m");    // has been specified for any element
		k = strstr(GS + 2, "k");    // or variable.
		M = strstr(GS + 2, "M");    //
		giga = strstr(GS + 2, "G"); //************************************
		if (p != NULL)
			return 1;
		else if (u != NULL)
			return 3;
		else if (m != NULL)
			return 4;
		else if (k != NULL)
			return 5;
		else if (M != NULL)
			return 6;
		else if (giga != NULL)
			return 7;
		else if (n != NULL) {
			if (*(n + 1) == '1' || *(n + 1) == '2') // i.e. A node has been found not the nano prefix
				return 0;
			return 2; // If its not a node that was found, return 2 which corresponds to nano
		}
		else
			return 0;
	}
	else if (RL != NULL) {
		p = strstr(RL + 2, "p");    //************************************
		n = strstr(RL + 2, "n");    //
		u = strstr(RL + 2, "u");    // Pointers to find whether a prefix 
		m = strstr(RL + 2, "m");    // has been specified for any element
		k = strstr(RL + 2, "k");    // or variable.
		M = strstr(RL + 2, "M");    //
		giga = strstr(RL + 2, "G"); //************************************
		if (p != NULL)
			return 1;
		else if (u != NULL)
			return 3;
		else if (m != NULL)
			return 4;
		else if (k != NULL)
			return 5;
		else if (M != NULL)
			return 6;
		else if (giga != NULL)
			return 7;
		else if (n != NULL) {
			if (*(n + 1) == '1' || *(n + 1) == '2') // i.e. A node has been found not the nano prefix
				return 0;
			return 2; // If its not a node that was found, return 2 which corresponds to nano
		}
		else
			return 0;
	}
	else if (GL != NULL) {
		p = strstr(GL + 2, "p");    //************************************
		n = strstr(GL + 2, "n");    //
		u = strstr(GL + 2, "u");    // Pointers to find whether a prefix 
		m = strstr(GL + 2, "m");    // has been specified for any element
		k = strstr(GL + 2, "k");    // or variable.
		M = strstr(GL + 2, "M");    //
		giga = strstr(GL + 2, "G"); //************************************
		if (p != NULL)
			return 1;
		else if (u != NULL)
			return 3;
		else if (m != NULL)
			return 4;
		else if (k != NULL)
			return 5;
		else if (M != NULL)
			return 6;
		else if (giga != NULL)
			return 7;
		else if (n != NULL) {
			if (*(n + 1) == '1' || *(n + 1) == '2') // i.e. A node has been found not the nano prefix
				return 0;
			return 2; // If its not a node that was found, return 2 which corresponds to nano
		}
		else
			return 0;
	}
	return 0;
}


/** Routine to read the input file. It stores all relevant information from the circuit
   block whilst storing this information in its respective structure. Error checks are made upon reading values. 
@param cData *cPtr  Pointer to the array of structs used for storing CIRCUIT block information
@param FILE *fin  Pointer to the input file
*/
void read_cct(cData *cPtr, FILE *fin)
{
	char *line = (char*)malloc(sizeof(char) * 150); // To read in each line of the file
	while (!feof(fin))
	{
		fgets(line, 150, fin); // Gets each line of the file
		/*****************  CIRCUIT BLOCK  **********************/
		if (strstr(line, "<CIRCUIT>\n")) // Checks to see if the start circuit delimiter is on the line
		{
			char *n1 = NULL, *n2 = NULL, *R = NULL, *G = NULL; // Initialised pointers used to find if a variable is on a line.
			int elementNo = 0; // Used to record which element is being read for error checking and displaying if it occurs 
			int n1Check=0, n2Check=0, RCheck=0, GCheck=0; // Initialised check values used to store the return from scanning a variable
			while (!strstr(line, "</CIRCUIT>\n"))
			{
				if (line[0] != '#' && line[0] != '<') 
				{
					cPtr->n1 = 0; cPtr->n2 = 0; cPtr->R = 0; cPtr->G = 0; // Initialises each elements values 
					elementNo++; // Increments the number of elements found.
					n1 = strstr(line, "n1="); //************************************
					n2 = strstr(line, "n2="); // String pointers to where the node 
					R = strstr(line, "R=");   // or resistance values are specified
					G = strstr(line, "G=");   //************************************  
				}
				if (n1 != NULL) // I.e. if an "n1=" was found on the read line
				{
					n1Check = sscanf_s((n1 + 3), "%d%*[^\n]", &cPtr->n1); // Reads the n1 value but ignores the carriage return. stores in one index of an array of structs
					n1 = NULL; // Set to null for the next loop iteration 
				}
				if (n2 != NULL)
				{
					n2Check = sscanf_s((n2 + 3), "%d%*[^\n]", &cPtr->n2);
					n2 = NULL;
				}
				if (R != NULL)
				{
					cPtr->Gtrue = 0; // These are used for element checking purposes within the check_cct function.
					cPtr->Rtrue = 1; // I.e. determines whether a resistance or conductance value was specified 
					RCheck = sscanf_s((R + 2), "%lf%*[^\n]", &cPtr->R); // Reads the R value but ignores the carriage return if it preceeds the value
					switch (check_prefix(R, NULL, NULL, NULL, NULL, NULL)) { //************************************************************
						case 1: cPtr->R = cPtr->R * pow(10, -12); break;     //
						case 2: cPtr->R = cPtr->R * pow(10, -9);  break;     // Checks to see if a prefix is associated with the R value.
						case 3: cPtr->R = cPtr->R * pow(10, -6);  break;     // If it is, the value is multiplied accordingly for 
						case 4: cPtr->R = cPtr->R * pow(10, -3);  break;     // processing later in the program, i.e. calculating the
						case 5: cPtr->R = cPtr->R * pow(10, 3);   break;     // outputs.
						case 6: cPtr->R = cPtr->R * pow(10, 6);   break;     //
						case 7: cPtr->R = cPtr->R * pow(10, 9);   break;     //*************************************************************
						default: break;
					}
					if (check_cct(cPtr, elementNo, n1Check, n2Check, RCheck, 1) == 0)
						exit(-1); // I.e. an error was found with this elements values. Error description is printed via the check_cct function
					R = NULL;
					cPtr++; // Increments the structure array pointer for next element info storage
				}
				if (G != NULL) 
				{
					GCheck = sscanf_s((G + 2), "%lf%*[^\n]", &cPtr->G);
					switch (check_prefix(NULL, G, NULL, NULL, NULL, NULL)) {
						case 1: cPtr->G = cPtr->G * pow(10, -12); break;
						case 2: cPtr->G = cPtr->G * pow(10, -9);  break;
						case 3: cPtr->G = cPtr->G * pow(10, -6);  break;
						case 4: cPtr->G = cPtr->G * pow(10, -3);  break;
						case 5: cPtr->G = cPtr->G * pow(10, 3);   break;
						case 6: cPtr->G = cPtr->G * pow(10, 6);   break;
						case 7: cPtr->G = cPtr->G * pow(10, 9);   break;
						default: break;
					}
					cPtr->Gtrue = 1;
					cPtr->Rtrue = 0;
					if (check_cct(cPtr, elementNo, n1Check, n2Check, 1, GCheck) == 0)
						exit(-1);
					G = NULL;
					cPtr++;
				}
				fgets(line, 150, fin);    // Gets each new line in the CIRCUIT block
			}
		}
	}
	free(line); // Give the memory used to create line back to the operating system
}


/** Routine to read the input file. It stores all relevant information in the terms 
   block into its respective structure. Error checks are made upon reading a value 
@param tData *tPtr  Pointer to the structure for storing TERMS block information 
@param FILE *fin   Pointer to the input file
*/
void read_terms(tData *tPtr, FILE *fin)
{
	char *line = (char*)malloc(sizeof(char) * 150); // To read in each line of the file
	while (!feof(fin))
	{
		fgets(line, 150, fin); // Gets each line of the file	
		if (strstr(line, "<TERMS>\n")) // Checks to see if the start TERMS delimiter is on the line
		{
			/* Initialised values. VT is 1 as it is error checked whether it is 0 after each valid
			 line information that is read. Gets updated once "VT=" has been found on a line.
			 This block reads information similarly to how the CIRCUIT block operates.  */
			char *VT = NULL, *IN = NULL, *RS = NULL, *RL = NULL, *GS = NULL, *GL = NULL;
			int VTcheck = 1, INcheck = 1, RScheck = 1, RLcheck = 1, GScheck = 1, GLcheck = 1;
			tPtr->RL = 0; tPtr->RS = 0; tPtr->VT = 1; tPtr->INtrue = 0;
			while (!strstr(line, "</TERMS>\n"))
			{
				fgets(line, 150, fin);  // Gets each new line in the TERMS block
				if (line[0] != '#' && line[0] != '<')
				{
					VT = strstr(line, "VT="); /****************************************************/
					RS = strstr(line, "RS="); /* String pointers to the source and resistance values */
					RL = strstr(line, "RL="); /****************************************************/
					GS = strstr(line, "GS=");
					GL = strstr(line, "GL=");
					IN = strstr(line, "IN=");
				}
				if (IN != NULL)
				{
					INcheck = sscanf_s((IN + 3), "%lf%*[^\n]", &tPtr->VT); // Stores the IN value in VT, however, this is converted to VT after the while loop
					if (check_terms(tPtr, 1, INcheck, 1, 1, 1, 1) == 0) // Checks the IN value for errors. If there is, program exits with -1
						exit(-1);
					tPtr->INtrue = 1;
					IN = NULL;
				}
				if (VT != NULL)
				{
					VTcheck = sscanf_s((VT + 3), "%lf%*[^\n]", &tPtr->VT);
					if (check_terms(tPtr, VTcheck, 1, 1, 1, 1, 1) == 0)
						exit(-1);
					VT = NULL;
				}
				if (RS != NULL)
				{
					RScheck = sscanf_s((RS + 3), "%lf%*[^\n]", &tPtr->RS);
					switch (check_prefix(NULL, NULL, RS, NULL, NULL, NULL)) { // Checks to see if a prefix is associated with this value
					case 1: tPtr->RS = tPtr->RS * pow(10, -12); break;    // This method continues for GS, RL and GL
					case 2: tPtr->RS = tPtr->RS * pow(10, -9);  break;
					case 3: tPtr->RS = tPtr->RS * pow(10, -6);  break;
					case 4: tPtr->RS = tPtr->RS * pow(10, -3);  break;
					case 5: tPtr->RS = tPtr->RS * pow(10, 3);   break;
					case 6: tPtr->RS = tPtr->RS * pow(10, 6);   break;
					case 7: tPtr->RS = tPtr->RS * pow(10, 9);   break;
					default: break;
					}
					if (check_terms(tPtr, 1, 1, RScheck, 1, 1, 1) == 0) // Checks the value for errors
						exit(-1);
					RS = NULL;
				}
				if (GS != NULL)
				{
					GScheck = sscanf_s((GS + 3), "%lf%*[^\n]", &tPtr->RS);
					if (tPtr->RS != 0)
						tPtr->RS = 1 / tPtr->RS;
					switch (check_prefix(NULL, NULL, NULL, GS, NULL, NULL)) {
					case 1: tPtr->RS = tPtr->RS * pow(10, -12); break;
					case 2: tPtr->RS = tPtr->RS * pow(10, -9);  break;
					case 3: tPtr->RS = tPtr->RS * pow(10, -6);  break;
					case 4: tPtr->RS = tPtr->RS * pow(10, -3);  break;
					case 5: tPtr->RS = tPtr->RS * pow(10, 3);   break;
					case 6: tPtr->RS = tPtr->RS * pow(10, 6);   break;
					case 7: tPtr->RS = tPtr->RS * pow(10, 9);   break;
					default: break;
					}
					if (check_terms(tPtr, 1, 1, 1, 1, GScheck, 1) == 0)
						exit(-1);
					GS = NULL;
				}
				if (RL != NULL)
				{
					RLcheck = sscanf_s((RL + 3), "%lf%*[^\n]", &tPtr->RL);
					switch (check_prefix(NULL, NULL, NULL, NULL, RL, NULL)) {
					case 1: tPtr->RL = tPtr->RL * pow(10, -12); break;
					case 2: tPtr->RL = tPtr->RL * pow(10, -9);  break;
					case 3: tPtr->RL = tPtr->RL * pow(10, -6);  break;
					case 4: tPtr->RL = tPtr->RL * pow(10, -3);  break;
					case 5: tPtr->RL = tPtr->RL * pow(10, 3);   break;
					case 6: tPtr->RL = tPtr->RL * pow(10, 6);   break;
					case 7: tPtr->RL = tPtr->RL * pow(10, 9);   break;
					default: break;
					}
					if (check_terms(tPtr, 1, 1, 1, RLcheck, 1, 1) == 0)
						exit(-1);
					RL = NULL;
				}
				if (GL != NULL)
				{
					GLcheck = sscanf_s((GL + 3), "%lf%*[^\n]", &tPtr->RL);
					if (tPtr->RL != 0)
						tPtr->RL = 1 / tPtr->RL;
					switch (check_prefix(NULL, NULL, NULL, NULL, NULL, GL)) {
					case 1: tPtr->RL = tPtr->RL * pow(10, -12); break;
					case 2: tPtr->RL = tPtr->RL * pow(10, -9);  break;
					case 3: tPtr->RL = tPtr->RL * pow(10, -6);  break;
					case 4: tPtr->RL = tPtr->RL * pow(10, -3);  break;
					case 5: tPtr->RL = tPtr->RL * pow(10, 3);   break;
					case 6: tPtr->RL = tPtr->RL * pow(10, 6);   break;
					case 7: tPtr->RL = tPtr->RL * pow(10, 9);   break;
					default: break;
					}
					if (check_terms(tPtr, 1, 1, 1, 1, 1, GLcheck) == 0)
						exit(-1);
					GL = NULL;
				}
			}
			// If a Norton current source is specified, the current value stored in VT is multiplied with RS to create a voltage source.
			if (tPtr->INtrue == 1) {
				tPtr->VT = tPtr->VT * tPtr->RS;
			}
		}
	}
	free(line);
}


/* Routine to print the required output labels, its units and its values to the output file.
@param double *vals   An array of the values of the required output variables
@param char *units   An array of the units required 
@param char *label   The output variable labels 
@param char *filename   The found output filename to be written to
@Param int *valsSize   The size of the vals array
*/
void print_outputs(double *vals, char *units, char *label, char *filename, int *valsSize) {
	FILE *fout = open_output_file(filename); // Opens the output file determined by "outFile" 
	int size = strlen(label) - 2; // Each variable is concatenated with ', ' after it. This ensures it will not be printed on the last variable
	for (int i = 0; i < size; i++) {
		fputc(label[i], fout);  // Prints the contents of the label array to the output file and carriage returns once finished
		if (i == size - 1)
			fprintf_s(fout, "\n");
	}

	size = strlen(units) - 2;
	for (int i = 0; i < size; i++) {
		fputc(units[i], fout); // Prints the contents of the units array to the output file and carriage returns once finished
		if (i == size - 1)
			fprintf_s(fout, "\n");
	}

	for (int i = 0; i < *valsSize; i++) {
		if (i == *valsSize - 1) { // i.e. at the end of the array, therefore no comma needed to be printed along with the value
			if (vals[i] > 0) // Positive value requires 3 spaces before a value 
				fprintf_s(fout, "   %.4e\n\n", vals[i]);
			else // Negative values require 2 spaces before the value according to test 'perfect' files
				fprintf_s(fout, "  %.4e\n\n", vals[i]);
		}
		else if (i == 0) { // i.e. at the beginning of the array
			fprintf_s(fout, "  %.4e,", vals[i]);
		}
		else if (vals[i] > 0) // i.e. values between the first and last value  
			fprintf_s(fout, "   %.4e,", vals[i]);
		else 
			fprintf_s(fout, "  %.4e,", vals[i]);
	}
	fclose(fout); // Close the output file
}


/** Routine to find the required outputs from the output file and their order.
@param oData *oPtr   Pointer to the structure holding all output values
@param FILE *fin   Pointer to the input file
*/
void find_outputs(oData *oPtr, FILE *fin)  
{
	// Arrays to store what variables are required, their values and units, aswell as the output file name.
	double *values = (double*)malloc(sizeof(double) * 150);
	char *units = (char*)malloc(sizeof(char) * 150);
	char *label = (char*)malloc(sizeof(char) * 150);
	char *outFile = (char*)malloc(sizeof(char) * 150);
	// Initialised with null termination as required for concatenating the units strings to the char array
	units[0] = '\0'; label[0] = '\0';
	// Used to get each line of the OUTPUT block
	char *line = (char*)malloc(sizeof(char) * 150);
	// Incrementer for the values array
	int n = 0;
	while (!feof(fin))
	{
		fgets(line, 150, fin); // Gets first line of the file
		/****************   OUTPUT BLOCK  *********************/
		if (strstr(line, "<OUTPUT>\n"))
		{
			// Pointers for finding their respective variable and the output file name
			char *filename = NULL, *Vin = NULL, *Vout = NULL, *Iin = NULL, *Iout = NULL, *Pin = NULL,
				 *Pout = NULL, *Zin = NULL, *Zout = NULL, *Av = NULL, *Ai = NULL, *dB = NULL; 
			while (!strstr(line, "</OUTPUT>\n"))
			{
				fgets(line, 150, fin); // Retrieves each line of the OUTPUT block on every loop iteration 
				if (line[0] != '#' && line[0] != '<')
				{
					Vin = strstr(line, "Vin"); Vout = strstr(line, "Vout");         // Finds whether a variable is specified 
					Iin = strstr(line, "Iin"); Iout = strstr(line, "Iout");         // on a line aswell as the output file name.
					Pin = strstr(line, "Pin"); Pout = strstr(line, "Pout");        
					Zin = strstr(line, "Zin"); Zout = strstr(line, "Zout");
					Av = strstr(line, "Av");  Ai = strstr(line, "Ai");
					dB = strstr(line, "dB"); filename = strstr(line, "File_name=");
				}
				if (filename != NULL) { // Reads the required output filename and stores it ready for writing to it
					int fileCheck = sscanf_s((filename + 10), "%s%*[^\n]", outFile, sizeof(char) * 150);
					if (fileCheck == 0) { // I.e. the file name was not a string or couldn't find it
						printf_s("Error: No output file name specified.\n");
						exit(-1);
					}
				}
				if (Vin != NULL) {
					if (dB != NULL) { // I.e. A dB input voltage has been specified
						values[n] = oPtr->VindB; // The value is pulled from the output structure and stored in the values array
						strcat_s(units, sizeof(char) * 150, "dBV, \0"); 
						strcat_s(label, sizeof(char) * 150, "Vin, \0"); // The label of this variable (Vin) is stored for printing to the output file
						n++; // Increments the integer used for accessing indices of the values array                     
						Vin = NULL; // These pointers are reset to null for the next loop iteration        
						dB = NULL;               
					}
					else {
						values[n] = oPtr->Vin;
						strcat_s(units, sizeof(char) * 150, "V, \0"); 
						strcat_s(label, sizeof(char) * 150, "Vin, \0"); // The label of this variable (Vin) is stored for printing to the output file
						n++; // Increments the integer used for accessing indices of the values array
						Vin = NULL; // Resets the pointer to null for the next loop iteration
						//***************************************************************************
						// This procedure continues for each output seen from the else if statements
						//***************************************************************************
					}
				}
				else if (Vout != NULL) {
					if (dB != NULL) {
						values[n] = oPtr->VoutdB;
						strcat_s(units, sizeof(char) * 150, "dBV, \0"); 
						strcat_s(label, sizeof(char) * 150, "Vout, \0");
						n++;
						Vout = NULL;
						dB = NULL;
					}
					else {
						values[n] = oPtr->Vout;
						strcat_s(units, sizeof(char) * 150, "V, \0"); 
						strcat_s(label, sizeof(char) * 150, "Vout, \0");
						n++;
						Vout = NULL;
					}
				}
				else if (Iin != NULL) {
					if (dB != NULL) {
						values[n] = oPtr->IindB;
						strcat_s(units, sizeof(char) * 150, "dBI, \0"); 
						strcat_s(label, sizeof(char) * 150, "Iin, \0");
						n++;
						Iin = NULL;
						dB = NULL; 
					}
					else {
						values[n] = oPtr->Iin;
						strcat_s(units, sizeof(char) * 150, "I, \0"); 
						strcat_s(label, sizeof(char) * 150, "Iin, \0");
						n++;
						Iin = NULL;
					}
				}
				else if (Iout != NULL) {
					if (dB != NULL) {
						values[n] = oPtr->IoutdB;
						strcat_s(units, sizeof(char) * 150, "dBI, \0"); 
						strcat_s(label, sizeof(char) * 150, "Iout, \0");
						n++;
						Iout = NULL;
						dB = NULL;
					}
					else {
						values[n] = oPtr->Iout;
						strcat_s(units, sizeof(char) * 150, "I, \0"); 
						strcat_s(label, sizeof(char) * 150, "Iout, \0");
						n++;
						Iout = NULL;
					}
				}
				else if (Pin != NULL) {
					if (dB != NULL) {
						values[n] = oPtr->PindB;
						strcat_s(units, sizeof(char) * 150, "dBW, \0"); 
						strcat_s(label, sizeof(char) * 150, "Pin, \0");
						n++;
						Pin = NULL;
						dB = NULL;
					}
					else {
						values[n] = oPtr->Pin;
						strcat_s(units, sizeof(char) * 150, "W, \0"); 
						strcat_s(label, sizeof(char) * 150, "Pin, \0");
						n++;
						Pin = NULL;
					}
				}
				else if (Pout != NULL) {
					if (dB != NULL) {
						values[n] = oPtr->PoutdB;
						strcat_s(units, sizeof(char) * 150, "dBW, \0"); 
						strcat_s(label, sizeof(char) * 150, "Pout, \0");
						n++;
						Pout = NULL;
						dB = NULL;
					}
					else {
						values[n] = oPtr->Pout;
						strcat_s(units, sizeof(char) * 150, "W, \0"); 
						strcat_s(label, sizeof(char) * 150, "Pout, \0");
						n++;
						Pout = NULL;
					}
				}
				else if (Zin != NULL) {
					if (dB != NULL) {
						values[n] = oPtr->ZindB;
						strcat_s(units, sizeof(char) * 150, "dBOhm, \0"); 
						strcat_s(label, sizeof(char) * 150, "Zin, \0");
						n++;
						Zin = NULL;
						dB = NULL;
					}
					else {
						values[n] = oPtr->Zin;
						strcat_s(units, sizeof(char) * 150, "Ohm, \0");
						strcat_s(label, sizeof(char) * 150, "Zin, \0");
						n++;
						Zin = NULL;
					}
				}
				else if (Zout != NULL) {
					if (dB != NULL) {
						values[n] = oPtr->ZoutdB;
						strcat_s(units, sizeof(char) * 150, "dBOhm, \0"); 
						strcat_s(label, sizeof(char) * 150, "Zout, \0");
						n++;
						Zout = NULL;
						dB = NULL;
					}
					else {
						values[n] = oPtr->Zout;
						strcat_s(units, sizeof(char) * 150, "Ohm, \0");
						strcat_s(label, sizeof(char) * 150, "Zout, \0");
						n++;
						Zout = NULL;
					}
				}
				else if (Av != NULL) {
					if (dB != NULL) {
						values[n] = oPtr->AvdB;
						strcat_s(units, sizeof(char) * 150, "dB, \0"); 
						strcat_s(label, sizeof(char) * 150, "Av, \0");
						n++;
						Av = NULL;
						dB = NULL;
					}
					else {
						values[n] = oPtr->Av;
						strcat_s(units, sizeof(char) * 150, "L, \0"); 
						strcat_s(label, sizeof(char) * 150, "Av, \0");
						n++;
						Av = NULL;
					}
				}
				else if (Ai != NULL) {
					if (dB != NULL) {
						values[n] = oPtr->AidB;
						strcat_s(units, sizeof(char) * 150, "dB, \0"); 
						strcat_s(label, sizeof(char) * 150, "Ai, \0");
						n++;
						Ai = NULL;
						dB = NULL;
					}
					else {
						values[n] = oPtr->Ai;
						strcat_s(units, sizeof(char) * 150, "L, \0");
						strcat_s(label, sizeof(char) * 150, "Ai, \0");
						n++;
						Ai = NULL;
					}
				}
			}
		}
	}
	print_outputs(values, units, label, outFile, &n); // Prints the required info to the output file
	free(values);
	free(units);
	free(label);
	free(line);
	free(outFile); 	// Gives back the memory used for creating these arrays to the operating system and closes the output file.
}


/** Routine to multiply each elements matrix in order and returns the circuits equivalent impedance matrix 
@param cData *element   Pointer to the array of structs used for each CIRCUIT elements information
@param int size   The number of elements within the circuit
@return double**   Pointer to a 2D array which is the circuits equivalent impedance matrix (ABCD matrix)
*/
double** create_ABCD(cData *element, int size)
{
	// Creates two empty ABCD matrices which will be used to multiply the elements ABCD matrices together in order.
	double **mat2 = (double **)malloc(2 * sizeof(double *));
	double **result = (double **)malloc(2 * sizeof(double *));
	for (int i = 0; i < 2; i++)
	{
		result[i] = (double *)malloc(2 * sizeof(double));
		mat2[i] = (double *)malloc(2 * sizeof(double));
	}

	result[0][0] = 1; result[0][1] = 0; result[1][0] = 0; result[1][1] = 1;
	// Assigns first elements matrix
	if (element->Rtrue == 1)
		result[0][1] = element->R;
	else
		result[1][0] = element->G;

	// Assigns successive elements matrices and multiplies them in order
	for (int i = 1; i < size; i++) 
	{
		mat2[0][0] = 1; mat2[0][1] = 0; mat2[1][0] = 0; mat2[1][1] = 1;
		// Assigns second elements matrix
		if ((element+1)->Rtrue == 1)
			mat2[0][1] = (element+1)->R;
		else
			mat2[1][0] = (element+1)->G;

		// Multiplies the cascade of ordered elements matrices
		result[0][0] = result[0][0] + (result[0][1] * mat2[1][0]); // A + BC
		result[0][1] = result[0][1] + (result[0][0] * mat2[0][1]); // B + AB
		result[1][0] = result[1][0] + (result[1][1] * mat2[1][0]); // C + DC
		result[1][1] = result[1][1] + (result[1][0] * mat2[0][1]); // D + CB
		
		element++; // increments the circuits array of structures pointer
	}
	free(mat2); 
	return result;
}


/** Routine to calculate all possible outputs and stores that information within the 'oData' structure
@param oData *out   Pointer to the structure holding the output information
@param tData *terms   Pointer to the structure holding the terms information
@param double **ABCD   Pointer to an array of pointers which holds the equivalent impedance matrix information
*/
void calc_outputs(oData *out, tData *terms, double **ABCD)
{
	// Creates the inverse equivalent impedance matrix
	double **invABCD = (double **)malloc(2 * sizeof(double *));
	for (int i = 0; i < 2; i++)
	{
		invABCD[i] = (double *)malloc(2 * sizeof(double));
	} 
	double K = 1 / ((ABCD[0][0] * ABCD[1][1]) - (ABCD[0][1] * ABCD[1][0])); 
	invABCD[0][0] = ABCD[1][1] * K; 
	invABCD[0][1] = -ABCD[0][1] * K;
	invABCD[1][0] = -ABCD[1][0] * K;
	invABCD[1][1] = ABCD[0][0] * K;

	// Calculates all possible outputs
	out->Zin = (((ABCD[0][0] * terms->RL) + ABCD[0][1]) / ((ABCD[1][0] * terms->RL) + ABCD[1][1]));
	out->ZindB = 20 * log10(out->Zin);
	out->Vin = terms->VT * (out->Zin / (terms->RS + out->Zin));
	out->VindB = 20 * log10(out->Vin);
	out->Iin = out->Vin / out->Zin;
	out->IindB = 20 * log10(out->Iin);
	out->Pin = out->Vin * out->Iin;
	out->PindB = 10 * log10(out->Pin);
	out->Zout = ((ABCD[1][1] * terms->RS) + ABCD[0][1]) / ((ABCD[1][0] * terms->RS) + ABCD[0][0]);
	out->ZoutdB = 20 * log10(out->Zout); 
	out->Vout = (invABCD[0][0] * out->Vin) + (invABCD[0][1] * out->Iin); 
	out->VoutdB = 20 * log10(out->Vout);
	out->Iout = (invABCD[1][0] * out->Vin) + (invABCD[1][1] * out->Iin); 
	out->IoutdB = 20 * log10(out->Iout);
	out->Pout = out->Vout * out->Iout;
	out->PoutdB = 10 * log10(out->Pout);
	out->Av = out->Vout / out->Vin;
	out->AvdB = 20 * log10(out->Av);
	out->Ai = out->Iout / out->Iin;
	out->AidB = 20 * log10(out->Ai);

	free(invABCD); // Gives back the memory from creating the inverse of the equivalent impedance matrix
}


/** main function
* @return int, 0 on successful exit
*/
int main() {
	FILE *fin = open_input_file();                           // Input file pointer.
	int n = find_no_of_cct_elements(fin);                    // Finds how many circuit elements there are for CIRCUIT array of structures allocation 
	rewind(fin);                                             // Rewinds the input file pointer to the start of the file
	cData *cctData = (cData*)malloc(sizeof(cData) * (n+1));  // Creates the CIRCUIT block array of structures 
	read_cct(cctData, fin);                                  // Reads the CIRCUIT block and stores element information in its respective structure
	rewind(fin);                                             // Rewinds the input file pointer to the start of the file
	tData termData;                                          // Creates the TERMS block structure
	read_terms(&termData, fin);                              // Reads the TERMS block and stores source and load information in its respective structure
	qsort(cctData, n, sizeof(cctData[0]), comparison);       // Sorts the array of structures for the circuit block in ascending order
	double **ABCD = (double **)malloc(2 * sizeof(double *)); // Memory for the equivalent impedance matrix
	for (int i = 0; i < 2; i++)
	{
		ABCD[i] = (double *)malloc(2 * sizeof(double));      
	}
	ABCD = create_ABCD(cctData, n);                          // Creates and stores the equivalent impedance matrix in 'ABCD'
	oData outData;                                           // Creates the OUTPUTS block structure
	rewind(fin);                                             // Rewinds the input file pointer to the start of the file
	calc_outputs(&outData, &termData, ABCD);                 // Finds all possible outputs and stores them within the OUTPUTS structure
	find_outputs(&outData, fin);                             // Finds and writes the required outputs in order to the output file
	printf_s("Output file written succesfully.\n");
	fclose(fin);                                             // Closes the input file
	free(cctData);                                           // Gives back the CIRCUIT array of structs memory to the operating system
	free(ABCD);                                              // Gives back the equivalent impedance matrix memory to the operating system
	return 0;
}
