#pragma once

/*display configuration*/
#define DISPLAY
#define RECORD

/*cell configuration*/
const unsigned MAX_CELL_INPUTS_X2 = 16;
const unsigned RW_OP_OPERAND_LENGTH = 8;

/*redundant wire additon operation*/
#define RW_VALID_CHAR "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_"
#define RW_VALID_SPACE " \n\r\t"
#define RW_VALID_LOGIC "01"

/*simulation configuration*/
const int MAX_RAND_NUM = 32767;
const int MAX_RAND_BIT = 15;
const int MAX_PARALLEL_NUM = 512;

/*signature configuration*/
const int SIGNATURE_SIZE = 512;
const int ODCMASK_SIZE = 3;
