#pragma once

/*cell configuration*/
const unsigned MAX_CELL_INPUTS_X2 = 16;
const unsigned RW_OP_OPERAND_LENGTH = 8;

/*redundant wire additon operation*/
#define RW_VALID_CHAR "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_"
#define RW_VALID_SPACE " \n\r\t"
#define RW_VALID_LOGIC "01"