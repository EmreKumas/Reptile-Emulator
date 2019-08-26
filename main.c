#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

//STRUCT DECLARATIONS
typedef struct variable_struct{

    char *name;
    int *value;
    int arraySize;
    struct variable_struct *next;

} Variable;

typedef struct label_struct{

    char *name;
    int address;
    struct label_struct *next;

} Label;

///////////////////////////////////////////////////////////////////////////////////////////////////

//FUNCTION DECLARATIONS
FILE *readFile();
void readDataSection(FILE *instructions);
void readCodeSection(FILE *instructions);
bool isInstruction(char *input);
void addToVariableList(char *variable_name, int variable_value, int array_size);
void addToLabelList(char *label_name, int label_address);
Variable *createVariable(char *variable_name, int variable_value, int array_size);
Label *createLabel(char *label_name, int label_address);
char *removeSpaces(char *string);
void run();
void ldiInstruction();
Variable *isVariable(char *name);
Label *isLabel(char *name);
int findJumpAddress(char *labelName);
void print();
void freeMemory();

///////////////////////////////////////////////////////////////////////////////////////////////////

//GLOBAL DECLARATIONS
Variable *variables = NULL;
Label *labels = NULL;

char program[1000][50];
int pc = 0;
long regs[4];

///////////////////////////////////////////////////////////////////////////////////////////////////

int main(){

    FILE *instructions = readFile();

    readDataSection(instructions);
    readCodeSection(instructions);
    run();
    print();

    freeMemory();

    return 0;
}

FILE *readFile(){

    FILE *fp;

    if(access("Instructions.txt", F_OK) != -1)
        fp = fopen("Instructions.txt", "r");
    else{
        printf("\"Instructions.txt\" does not exist. Please make sure it exists before starting this program.\n");
        exit(1);
    }

    return fp;
}

void readDataSection(FILE *instructions){

    char line[100];
    char *variable_name = NULL;
    char *variable_data = NULL;
    char *array_size = NULL;

    while(fgets(line, sizeof line, instructions) != NULL){

        variable_name = strtok(line, "\n\t\r ");

        if(strcmp(variable_name, ".data") == 0)
            continue;
        else if(strcmp(variable_name, ".code") == 0)
            break;

        //If this is a variable...
        if(variable_name[strlen(variable_name) - 1] == ':'){

            //Means this is a variable. We need to delete the ':' character.
            variable_name[strlen(variable_name) - 1] = '\0';
            variable_data = strtok(NULL, "\n\t\r ");

            //If this is an array...
            if(variable_data != NULL && strcmp(variable_data, ".space") == 0){

                array_size = strtok(NULL, "\n\t\r ");
                addToVariableList(variable_name, 0, (int) strtol(array_size, NULL, 10));
            }else{

                if(variable_data == NULL)
                    addToVariableList(variable_name, 0, 1);
                else if((variable_data[0] == '0') && (variable_data[1] == 'x'))
                    addToVariableList(variable_name, ((int) strtol(variable_data, NULL, 16)), 1);
                else if((variable_data[0] == '-' && (variable_data[1] >= 48 && variable_data[1] <= 57)) ||
                        (variable_data[0] >= 48 && variable_data[0] <= 57))
                    addToVariableList(variable_name, ((int) strtol(variable_data, NULL, 10)), 1);
            }
        }
    }
}

void readCodeSection(FILE *instructions){

    char line[100];
    char preserve_line[100];
    char previous_token[100];
    char *token = NULL;

    while(fgets(line, sizeof line, instructions) != NULL){

        strcpy(preserve_line, line);
        token = strtok(line, "\n\t\r ");

        while(token != NULL){

            if(isInstruction(token)){

                strcpy(program[pc], removeSpaces(preserve_line));
                pc++;
            }
            else{ //LABEL

                strcpy(previous_token, token);
                //Lets check if the next word is a valid instruction.
                token = strtok(NULL, "\n\t\r ");

                if(token == NULL && fgets(line, sizeof line, instructions) != NULL){
                    strcpy(preserve_line, line);
                    token = strtok(line, "\n\t\r ");

                    if(isInstruction(token)){
                        addToLabelList(previous_token, pc);
                        continue;
                    }
                    else{

                        printf("An invalid instruction has been entered!\n");
                        exit(-1);
                    }
                }else{

                    if(isInstruction(token)){
                        addToLabelList(previous_token, pc);
                        char *ret = strchr(removeSpaces(preserve_line), ' ');
                        strcpy(preserve_line, (ret + 1));
                        continue;
                    }
                    else{

                        printf("An invalid instruction has been entered!\n");
                        exit(-1);
                    }
                }
            }

            token = strtok(NULL, "\n\t\r ");
        }
    }

    //We can close the file pointer.
    fclose(instructions);
}

bool isInstruction(char *input){

    return (strcmp(input, "ldi") == 0) || (strcmp(input, "LDI") == 0) || (strcmp(input, "ld") == 0) || (strcmp(input, "LD") == 0) ||
           (strcmp(input, "st") == 0) || (strcmp(input, "ST") == 0) || (strcmp(input, "jz") == 0) || (strcmp(input, "JZ") == 0) ||
           (strcmp(input, "jmp") == 0) || (strcmp(input, "JMP") == 0) || (strcmp(input, "add") == 0) || (strcmp(input, "ADD") == 0) ||
           (strcmp(input, "sub") == 0) || (strcmp(input, "SUB") == 0) || (strcmp(input, "and") == 0) || (strcmp(input, "AND") == 0) ||
           (strcmp(input, "or") == 0) || (strcmp(input, "OR") == 0) || (strcmp(input, "xor") == 0) || (strcmp(input, "XOR") == 0) ||
           (strcmp(input, "not") == 0) || (strcmp(input, "NOT") == 0) || (strcmp(input, "mov") == 0) || (strcmp(input, "MOV") == 0) ||
           (strcmp(input, "inc") == 0) || (strcmp(input, "INC") == 0) || (strcmp(input, "dec") == 0) || (strcmp(input, "DEC") == 0);
}

void addToVariableList(char *variable_name, int variable_value, int array_size){

    if(variables == NULL)
        variables = createVariable(variable_name, variable_value, array_size);
    else{

        Variable *current_variable = variables;
        Variable *previous_variable = variables;

        while(current_variable != NULL){
            previous_variable = current_variable;
            current_variable = current_variable->next;
        }

        current_variable = createVariable(variable_name, variable_value, array_size);
        previous_variable->next = current_variable;
    }
}

void addToLabelList(char *label_name, int label_address){

    if(labels == NULL)
        labels = createLabel(label_name, label_address);
    else{

        Label *current_label = labels;
        Label *previous_label = labels;

        while(current_label != NULL){
            previous_label = current_label;
            current_label = current_label->next;
        }

        current_label = createLabel(label_name, label_address);
        previous_label->next = current_label;
    }
}

Variable *createVariable(char *variable_name, int variable_value, int array_size){

    Variable *variable = malloc(sizeof variable);

    char *name = malloc(sizeof variable_name);
    strcpy(name, variable_name);
    variable->name = name;

    if(array_size == 1){

        int *address = malloc(sizeof(int));
        variable->value = address;
        *(variable->value) = variable_value;
        variable->arraySize = 1;
    }else{

        int *address = malloc(array_size * sizeof(int));
        variable->value = address;
        variable->arraySize = array_size;

        for(int i = 0; i < array_size; i++) variable->value[i] = 0;
    }

    variable->next = NULL;

    return variable;
}

Label *createLabel(char *label_name, int label_address){

    Label *label = malloc(sizeof label);

    char *name = malloc(sizeof label_name);
    strcpy(name, label_name);
    label->name = name;

    label->address = label_address;

    label->next = NULL;

    return label;
}

char *removeSpaces(char *string){

    char *newString = strtok(string, "\n\r");

    while(isspace(*newString)) // NOLINT
        newString++;

    return newString;
}

void run(){

    char full_instruction[100], *instruction, *op1, *op2, *op3;
    int regNumber, zeroFlag = 0;
    long op2Location, op2Value, op3Value, *address;

    for(int current_pc = 0; current_pc < pc; current_pc++){

        strcpy(full_instruction, program[current_pc]);
        instruction = strtok(full_instruction, " ");

        if(strcmp(instruction, "ldi") == 0 || strcmp(instruction, "LDI") == 0){

            ldiInstruction();
        }
        else if(strcmp(instruction, "ld") == 0 || strcmp(instruction, "LD") == 0){

            op1 = strtok(NULL, " ");
            op2 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            op2Location = regs[(int) strtol(op2, NULL, 10)];
            address = (long *) op2Location;

            regs[regNumber] = *address;

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
        else if(strcmp(instruction, "st") == 0 || strcmp(instruction, "ST") == 0){

            op1 = strtok(NULL, " ");
            op2 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            op2Location = regs[(int) strtol(op2, NULL, 10)];
            address = (long *) op2Location;

            *address = regs[regNumber];
        }
        else if(strcmp(instruction, "jz") == 0 || strcmp(instruction, "JZ") == 0){

            op1 = strtok(NULL, " ");

            //Now, we need to find the correct label to jump.
            int jumpAddress = findJumpAddress(op1);

            //Since this is jump_zero we need to make sure that previous instructions result is zero.
            if(zeroFlag)
                current_pc = jumpAddress - 1;
        }
        else if(strcmp(instruction, "jmp") == 0 || strcmp(instruction, "JMP") == 0){

            op1 = strtok(NULL, " ");

            //Now, we need to find the correct label to jump.
            int jumpAddress = findJumpAddress(op1);

            current_pc = jumpAddress - 1;
        }
        else if(strcmp(instruction, "add") == 0 || strcmp(instruction, "ADD") == 0){

            op1 = strtok(NULL, " ");
            op2 = strtok(NULL, " ");
            op3 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            op2Value = regs[(int) strtol(op2, NULL, 10)];
            op3Value = regs[(int) strtol(op3, NULL, 10)];
            regs[regNumber] = op2Value + op3Value;

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
        else if(strcmp(instruction, "sub") == 0 || strcmp(instruction, "SUB") == 0){

            op1 = strtok(NULL, " ");
            op2 = strtok(NULL, " ");
            op3 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            op2Value = regs[(int) strtol(op2, NULL, 10)];
            op3Value = regs[(int) strtol(op3, NULL, 10)];
            regs[regNumber] = op2Value - op3Value;

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
        else if(strcmp(instruction, "and") == 0 || strcmp(instruction, "AND") == 0){

            op1 = strtok(NULL, " ");
            op2 = strtok(NULL, " ");
            op3 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            op2Value = regs[(int) strtol(op2, NULL, 10)];
            op3Value = regs[(int) strtol(op3, NULL, 10)];
            regs[regNumber] = op2Value & op3Value; // NOLINT

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
        else if(strcmp(instruction, "or") == 0 || strcmp(instruction, "OR") == 0){

            op1 = strtok(NULL, " ");
            op2 = strtok(NULL, " ");
            op3 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            op2Value = regs[(int) strtol(op2, NULL, 10)];
            op3Value = regs[(int) strtol(op3, NULL, 10)];
            regs[regNumber] = op2Value | op3Value; // NOLINT

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
        else if(strcmp(instruction, "xor") == 0 || strcmp(instruction, "XOR") == 0){

            op1 = strtok(NULL, " ");
            op2 = strtok(NULL, " ");
            op3 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            op2Value = regs[(int) strtol(op2, NULL, 10)];
            op3Value = regs[(int) strtol(op3, NULL, 10)];
            regs[regNumber] = op2Value ^ op3Value; // NOLINT

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
        else if(strcmp(instruction, "not") == 0 || strcmp(instruction, "NOT") == 0){

            op1 = strtok(NULL, " ");
            op2 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            op2Value = regs[(int) strtol(op2, NULL, 10)];
            regs[regNumber] = ~op2Value; // NOLINT

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
        else if(strcmp(instruction, "mov") == 0 || strcmp(instruction, "MOV") == 0){

            op1 = strtok(NULL, " ");
            op2 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            op2Value = regs[(int) strtol(op2, NULL, 10)];
            regs[regNumber] = op2Value;

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
        else if(strcmp(instruction, "inc") == 0 || strcmp(instruction, "INC") == 0){

            op1 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            regs[regNumber] = regs[regNumber] + 1;

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
        else if(strcmp(instruction, "dec") == 0 || strcmp(instruction, "DEC") == 0){

            op1 = strtok(NULL, " ");

            regNumber = (int) strtol(op1, NULL, 10);
            regs[regNumber] = regs[regNumber] - 1;

            if(regs[regNumber] == 0) zeroFlag = 1;
            else zeroFlag = 0;
        }
    }
}

void ldiInstruction(){

    char *op1, *op2;
    int regNumber;

    op1 = strtok(NULL, " ");
    op2 = strtok(NULL, " ");

    regNumber = (int) strtol(op1, NULL, 10);

    if(op2[0] == '0' && op2[1] == 'x')
        regs[regNumber] = strtol(op2 + 2, NULL, 16);
    else if((op2[0] == '-' && (op2[1] >= 48 && op2[1] <= 57)) || (op2[0] >= 48 && op2[0] <= 57))
        regs[regNumber] = strtol(op2, NULL, 10);
    else{

        //Means this is a variable or label.
        Variable *variable = isVariable(op2);

        if(variable != NULL){

            //Means this is a variable.
            regs[regNumber] = (long) variable->value;
        }
        else{

            //Then it must be a label.
            Label *label = isLabel(op2);

            if(label != NULL){

                //Means this is a label.
                regs[regNumber] = label->address;
            }else{

                printf("Instructions contain invalid label or variable!!!");
                exit(-1);
            }
        }
    }
}

Variable *isVariable(char *name){

    Variable *currentVariable = variables;
    while(currentVariable != NULL){

        if(strcmp(name, currentVariable->name) == 0)
            break;

        currentVariable = currentVariable->next;
    }

    return currentVariable;
}

Label *isLabel(char *name){

    Label *currentLabel = labels;
    while(currentLabel != NULL){

        if(strcmp(name, currentLabel->name) == 0)
            break;

        currentLabel = currentLabel->next;
    }

    return currentLabel;
}

int findJumpAddress(char *labelName){

    Label *currentLabel = labels;

    while(currentLabel != NULL){

        if(strcmp(labelName, currentLabel->name) == 0)
            break;

        currentLabel = currentLabel->next;
    }

    return currentLabel != NULL ? currentLabel->address : -1;
}

void print(){

    int i;

    printf("\nREGISTER CONTENTS:\n");
    for(i = 0; i < 4; i++)
        printf("Register %d : 0x%04hx\n", i, (short) regs[i]);

    printf("\nDATA MEMORY CONTENTS:\n");
    Variable *currentVariable = variables;
    while(currentVariable != NULL){

        if(currentVariable->arraySize > 1)
            for(i = 0; i < currentVariable->arraySize; i++)
                printf("%s[%d] : 0x%04hx\n", currentVariable->name, i, (short) *((currentVariable->value) + i));
        else
            printf("%s : 0x%04hx\n", currentVariable->name, (short) *(currentVariable->value));

        currentVariable = currentVariable->next;
    }
}

void freeMemory(){

    Variable *currentVariable = variables;
    Variable *previousVariable;
    while(currentVariable != NULL){
        previousVariable = currentVariable;
        currentVariable = currentVariable -> next;
        free(previousVariable);
    }

    Label *currentLabel = labels;
    Label *previousLabel;
    while(currentLabel != NULL){
        previousLabel = currentLabel;
        currentLabel = currentLabel -> next;
        free(previousLabel);
    }
}
