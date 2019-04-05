#include "bytecode/Constants.h"

const char *Constants::CpInfoMnemonic[] = {
    "",
    "UTF8",                // 1
    "",
    "Integer",             // 3
    "Float",
    "Long",
    "Double",
    "Class",
    "String",
    "Fieldref",
    "Methodref",
    "InterfaceMethodref",
    "NameAndType",         // 12
    "",
    "",
    "MethodHandle",        // 15
    "MethodType",          // 16
    "",
    "InvokeDynamic"        // 18
};

const char *Constants::ReferenceKindMnemonic[] = {
    "",
    "getField",
    "getStatic",
    "putField",
    "putStatic",
    "invokeVirtual",
    "invokeStatic",
    "invokeSpecial",
    "newInvokeSpecial"
};

const char *Constants::OpcodeMnemonic[] = {
    "nop",          /* 0*/
    "aconst_null",  /* 1*/
    "iconst_m1",    /* 2*/
    "iconst_0",     /* 3*/
    "iconst_1",     /* 4*/
    "iconst_2",     /* 5*/
    "iconst_3",     /* 6*/
    "iconst_4",     /* 7*/
    "iconst_5",     /* 8*/
    "lconst_0",     /* 9*/
    "lconst_1",     /* 10*/
    "fconst_0",     /* 11*/
    "fconst_1",     /* 12*/
    "fconst_2",     /* 13*/
    "dconst_0",     /* 14*/
    "dconst_1",     /* 15*/
    "bipush",       /* 16*/
    "sipush",       /* 17*/
    "ldc",          /* 18*/
    "ldc_w",        /* 19*/
    "ldc2_w",       /* 20*/
    "iload",        /* 21*/
    "lload",        /* 22*/
    "fload",        /* 23*/
    "dload",        /* 24*/
    "aload",        /* 25*/
    "iload_0",      /* 26*/
    "iload_1",      /* 27*/
    "iload_2",      /* 28*/
    "iload_3",      /* 29*/
    "lload_0",      /* 30*/
    "lload_1",      /* 31*/
    "lload_2",      /* 32*/
    "lload_3",      /* 33*/
    "fload_0",      /* 34*/
    "fload_1",      /* 35*/
    "fload_2",      /* 36*/
    "fload_3",      /* 37*/
    "dload_0",      /* 38*/
    "dload_1",      /* 39*/
    "dload_2",      /* 40*/
    "dload_3",      /* 41*/
    "aload_0",      /* 42*/
    "aload_1",      /* 43*/
    "aload_2",      /* 44*/
    "aload_3",      /* 45*/
    "iaload",       /* 46*/
    "laload",       /* 47*/
    "faload",       /* 48*/
    "daload",       /* 49*/
    "aaload",       /* 50*/
    "baload",       /* 51*/
    "caload",       /* 52*/
    "saload",       /* 53*/
    "istore",       /* 54*/
    "lstore",       /* 55*/
    "fstore",       /* 56*/
    "dstore",       /* 57*/
    "astore",       /* 58*/
    "istore_0",     /* 59*/
    "istore_1",     /* 60*/
    "istore_2",     /* 61*/
    "istore_3",     /* 62*/
    "lstore_0",     /* 63*/
    "lstore_1",     /* 64*/
    "lstore_2",     /* 65*/
    "lstore_3",     /* 66*/
    "fstore_0",     /* 67*/
    "fstore_1",     /* 68*/
    "fstore_2",     /* 69*/
    "fstore_3",     /* 70*/
    "dstore_0",     /* 71*/
    "dstore_1",     /* 72*/
    "dstore_2",     /* 73*/
    "dstore_3",     /* 74*/
    "astore_0",     /* 75*/
    "astore_1",     /* 76*/
    "astore_2",     /* 77*/
    "astore_3",     /* 78*/
    "iastore",      /* 79*/
    "lastore",      /* 80*/
    "fastore",      /* 81*/
    "dastore",      /* 82*/
    "aastore",      /* 83*/
    "bastore",      /* 84*/
    "castore",      /* 85*/
    "sastore",      /* 86*/
    "pop",          /* 87*/
    "pop2",         /* 88*/
    "dup",          /* 89*/
    "dup_x1",       /* 90*/
    "dup_x2",       /* 91*/
    "dup2",         /* 92*/
    "dup2_x1",      /* 93*/
    "dup2_x2",      /* 94*/
    "swap",         /* 95*/
    "iadd",         /* 96*/
    "ladd",         /* 97*/
    "fadd",         /* 98*/
    "dadd",         /* 99*/
    "isub",         /* 100*/
    "lsub",         /* 101*/
    "fsub",         /* 102*/
    "dsub",         /* 103*/
    "imul",         /* 104*/
    "lmul",         /* 105*/
    "fmul",         /* 106*/
    "dmul",         /* 107*/
    "idiv",         /* 108*/
    "ldiv",         /* 109*/
    "fdiv",         /* 110*/
    "ddiv",         /* 111*/
    "irem",         /* 112*/
    "lrem",         /* 113*/
    "frem",         /* 114*/
    "drem",         /* 115*/
    "ineg",         /* 116*/
    "lneg",         /* 117*/
    "fneg",         /* 118*/
    "dneg",         /* 119*/
    "ishl",         /* 120*/
    "lshl",         /* 121*/
    "ishr",         /* 122*/
    "lshr",         /* 123*/
    "iushr",        /* 124*/
    "lushr",        /* 125*/
    "iand",         /* 126*/
    "land",         /* 127*/
    "ior",          /* 128*/
    "lor",          /* 129*/
    "ixor",         /* 130*/
    "lxor",         /* 131*/
    "iinc",         /* 132*/
    "i2l",          /* 133*/
    "i2f",          /* 134*/
    "i2d",          /* 135*/
    "l2i",          /* 136*/
    "l2f",          /* 137*/
    "l2d",          /* 138*/
    "f2i",          /* 139*/
    "f2l",          /* 140*/
    "f2d",          /* 141*/
    "d2i",          /* 142*/
    "d2l",          /* 143*/
    "d2f",          /* 144*/
    "i2b",          /* 145*/
    "i2c",          /* 146*/
    "i2s",          /* 147*/
    "lcmp",         /* 148*/
    "fcmpl",        /* 149*/
    "fcmpg",        /* 150*/
    "dcmpl",        /* 151*/
    "dcmpg",        /* 152*/
    "ifeq",         /* 153*/
    "ifne",         /* 154*/
    "iflt",         /* 155*/
    "ifge",         /* 156*/
    "ifgt",         /* 157*/
    "ifle",         /* 158*/
    "if_icmpeq",    /* 159*/
    "if_icmpne",    /* 160*/
    "if_icmplt",    /* 161*/
    "if_icmpge",    /* 162*/
    "if_icmpgt",    /* 163*/
    "if_icmple",    /* 164*/
    "if_acmpeq",    /* 165*/
    "if_acmpne",    /* 166*/
    "goto",         /* 167*/
    "jsr",          /* 168*/
    "ret",          /* 169*/
    "tableswitch",  /* 170*/
    "lookupswitch", /* 171*/
    "ireturn",      /* 172*/
    "lreturn",      /* 173*/
    "freturn",      /* 174*/
    "dreturn",      /* 175*/
    "areturn",      /* 176*/
    "return",       /* 177*/
    "getstatic",    /* 178*/
    "putstatic",    /* 179*/
    "getfield",     /* 180*/
    "putfield",     /* 181*/
    "invokevirtual",  /* 182*/
    "invokespecial",  /* 183*/
    "invokestatic", /* 184*/
    "invokeinterface",  /* 185*/
    "invokedynamic",  /* 186 */
    "new",          /* 187*/
    "newarray",     /* 188*/
    "anewarray",    /* 189*/
    "arraylength",  /* 190*/
    "athrow",       /* 191*/
    "checkcast",    /* 192*/
    "instanceof",   /* 193*/
    "monitorenter", /* 194*/
    "monitorexit",  /* 195*/
    "wide",         /* 196*/
    "multianewarray", /* 197*/
    "ifnull",       /* 198*/
    "ifnonnull",    /* 199*/
    "goto_w",       /* 200*/
    "jsr_w"         /* 201*/
};

const uint8_t Constants::InstructionLength[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 2, 3, //20
    3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //40
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1, //60
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //80
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //100
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //120
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, //140
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, //160
    3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 1, 1, 1, 1, 1, 1, 3, 3, //180
    3, 3, 3, 3, 3, 5, 5, 3, 2, 3, 1, 1, 3, 3, 1, 1, 0, 4, 3, 3, //200
    5, 5
};

const int Constants::OpCodeStackDelta[] = {
    0, /* 000 NOP */
    1, /* 001 ACONST_NULL */
    1, /* 002 ICONST_M1 */
    1, /* 003 ICONST_0 */
    1, /* 004 ICONST_1 */
    1, /* 005 ICONST_2 */
    1, /* 006 ICONST_3 */
    1, /* 007 ICONST_4 */
    1, /* 008 ICONST_5 */
    2, /* 009 LCONST_0 */
    2, /* 010 LCONST_1 */
    1, /* 011 FCONST_0 */
    1, /* 012 FCONST_1 */
    1, /* 013 FCONST_2 */
    2, /* 014 DCONST_0 */
    2, /* 015 DCONST_1 */
    1, /* 016 BIPUSH */
    1, /* 017 SIPUSH */
    1, /* 018 LDC */
    1, /* 019 LDC_W */
    2, /* 020 LDC2_W */
    1, /* 021 ILOAD */
    2, /* 022 LLOAD */
    1, /* 023 FLOAD */
    2, /* 024 DLOAD */
    1, /* 025 ALOAD */
    1, /* 026 ILOAD_0 */
    1, /* 027 ILOAD_1 */
    1, /* 028 ILOAD_2 */
    1, /* 029 ILOAD_3 */
    2, /* 030 LLOAD_0 */
    2, /* 031 LLOAD_1 */
    2, /* 032 LLOAD_2 */
    2, /* 033 LLOAD_3 */
    1, /* 034 FLOAD_0 */
    1, /* 035 FLOAD_1 */
    1, /* 036 FLOAD_2 */
    1, /* 037 FLOAD_3 */
    2, /* 038 DLOAD_0 */
    2, /* 039 DLOAD_1 */
    2, /* 040 DLOAD_2 */
    2, /* 041 DLOAD_3 */
    1, /* 042 ALOAD_0 */
    1, /* 043 ALOAD_1 */
    1, /* 044 ALOAD_2 */
    1, /* 045 ALOAD_3 */
    -1, /* 046 IALOAD */
    0, /* 047 LALOAD */
    -1, /* 048 FALOAD */
    0, /* 049 DALOAD */
    -1, /* 050 AALOAD */
    -1, /* 051 BALOAD */
    -1, /* 052 CALOAD */
    -1, /* 053 SALOAD */
    -1, /* 054 ISTORE */
    -2, /* 055 LSTORE */
    -1, /* 056 FSTORE */
    -2, /* 057 DSTORE */
    -1, /* 058 ASTORE */
    -1, /* 059 ISTORE_0 */
    -1, /* 060 ISTORE_1 */
    -1, /* 061 ISTORE_2 */
    -1, /* 062 ISTORE_3 */
    -2, /* 063 LSTORE_0 */
    -2, /* 064 LSTORE_1 */
    -2, /* 065 LSTORE_2 */
    -2, /* 066 LSTORE_3 */
    -1, /* 067 FSTORE_0 */
    -1, /* 068 FSTORE_1 */
    -1, /* 069 FSTORE_2 */
    -1, /* 070 FSTORE_3 */
    -2, /* 071 DSTORE_0 */
    -2, /* 072 DSTORE_1 */
    -2, /* 073 DSTORE_2 */
    -2, /* 074 DSTORE_3 */
    -1, /* 075 ASTORE_0 */
    -1, /* 076 ASTORE_1 */
    -1, /* 077 ASTORE_2 */
    -1, /* 078 ASTORE_3 */
    -3, /* 079 IASTORE */
    -4, /* 080 LASTORE */
    -3, /* 081 FASTORE */
    -4, /* 082 DASTORE */
    -3, /* 083 AASTORE */
    -3, /* 084 BASTORE */
    -3, /* 085 CASTORE */
    -3, /* 086 SASTORE */
    -1, /* 087 POP */
    -2, /* 088 POP2 */
    1, /* 089 DUP */
    127, /* 090 DUP_X1 */
    127, /* 091 DUP_X2 */
    127, /* 092 DUP2 */
    127, /* 093 DUP2_X1 */
    127, /* 094 DUP2_X2 */
    127, /* 095 SWAP */
    -1, /* 096 IADD */
    -2, /* 097 LADD */
    -1, /* 098 FADD */
    -2, /* 099 DADD */
    -1, /* 100 ISUB */
    -2, /* 101 LSUB */
    -1, /* 102 FSUB */
    -2, /* 103 DSUB */
    -1, /* 104 IMUL */
    -2, /* 105 LMUL */
    -1, /* 106 FMUL */
    -2, /* 107 DMUL */
    -1, /* 108 IDIV */
    -2, /* 109 LDIV */
    -1, /* 110 FDIV */
    -2, /* 111 DDIV */
    -1, /* 112 IREM */
    -2, /* 113 LREM */
    -1, /* 114 FREM */
    -2, /* 115 DREM */
    0, /* 116 INEG */
    0, /* 117 LNEG */
    0, /* 118 FNEG */
    0, /* 119 DNEG */
    -1, /* 120 ISHL */
    -1, /* 121 LSHL */
    -1, /* 122 ISHR */
    -1, /* 123 LSHR */
    -1, /* 124 IUSHR */
    -1, /* 125 LUSHR */
    -1, /* 126 IAND */
    -2, /* 127 LAND */
    -1, /* 128 IOR */
    -2, /* 129 LOR */
    -1, /* 130 IXOR */
    -2, /* 131 LXOR */
    0, /* 132 IINC */
    1, /* 133 I2L */
    0, /* 134 I2F */
    1, /* 135 I2D */
    -1, /* 136 L2I */
    -1, /* 137 L2F */
    0, /* 138 L2D */
    0, /* 139 F2I */
    1, /* 140 F2L */
    1, /* 141 F2D */
    -1, /* 142 D2I */
    0, /* 143 D2L */
    -1, /* 144 D2F */
    0, /* 145 I2B */
    0, /* 146 I2C */
    0, /* 147 I2S */
    -3,/* 148 LCMP */
    -1, /* 149 FCMPL */
    -1, /* 150 FCMPG */
    -3, /* 151 DCMPL */
    -3, /* 152 DCMPG */
    -1, /* 153 IFEQ */
    -1, /* 154 IFNE */
    -1, /* 155 IFLT */
    -1, /* 156 IFGE */
    -1, /* 157 IFGT */
    -1, /* 158 IFLE */
    -2, /* 159 IF_ICMPEQ */
    -2, /* 160 IF_ICMPNE */
    -2, /* 161 IF_ICMPLT */
    -2, /* 162 IF_ICMPGE */
    -2, /* 163 IF_ICMPGT */
    -2, /* 164 IF_ICMPLE */
    -2, /* 165 IF_ACMPEQ */
    -2, /* 166 IF_ACMPNE */
    0, /* 167 GOTO */
    -127, /* 168 JSR */
    -127, /* 169 RET */
    -1, /* 170 TABLESWITCH */
    -1, /* 171 LOOKUPSWITCH */
    -127, /* 172 IRETURN */
    -127, /* 173 LRETURN */
    -127, /* 174 FRETURN */
    -127, /* 175 DRETURN */
    -127, /* 176 ARETURN */
    -127, /* 177 RETURN */
    127, /* 178 GETSTATIC */
    127, /* 179 PUTSTATIC */
    127, /* 180 GETFIELD */
    127, /* 181 PUTFIELD */
    127, /* 182 INVOKEVIRTUAL */
    127, /* 183 INVOKESPECIAL */
    127, /* 184 INVOKESTATIC */
    127, /* 185 INVOKEINTERFACE */
    127, /* 186 INVOKEDYNAMIC */
    1, /* 187 NEW */
    0, /* 188 NEWARRAY */
    0, /* 189 ANEWARRAY */
    0, /* 190 ARRAYLENGTH */
    -1, /* 191 ATHROW */
    0, /* 192 CHECKCAST */
    0, /* 193 INSTANCEOF */
    -1, /* 194 MONITORENTER */
    -1, /* 195 MONITOREXIT */
    127, /* 196 WIDE */
    127, /* 197 MULTIANEWARRAY */
    -1, /* 198 IFNULL */
    -1, /* 199 IFNONNULL */
    -127, /* 200 GOTO_W */
    -127/* 201 JSR_W */
};

const char *Constants::ArrayType[] = {"", "", "", "", "boolean", "char", "float", "double", "byte", "short", "int", "long"};