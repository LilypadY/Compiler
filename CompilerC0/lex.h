#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED
#include"compile.h"

/*reserved words*/
#define RES_LIST_LEN 13
#define RES_ZONE_ST  (1)
#define RES_ZONE_END (RES_ZONE_ST+RES_LIST_LEN)

#define CONSTSY (RES_ZONE_ST+0)
#define INTSY (RES_ZONE_ST+1)
#define CHARSY (RES_ZONE_ST+2)
#define VOIDSY (RES_ZONE_ST+3)
#define MAINSY (RES_ZONE_ST+4)
#define IFSY (RES_ZONE_ST+5)
#define DOSY (RES_ZONE_ST+6)
#define WHILESY (RES_ZONE_ST+7)
#define SWITCHSY (RES_ZONE_ST+8)
#define CASESY (RES_ZONE_ST+9)
#define RETURNSY (RES_ZONE_ST+10)
#define SCANFSY (RES_ZONE_ST+11)
#define PRINTFSY (RES_ZONE_ST+12)

/*separators */
#define SPT_LIST_LEN (20)
#define SPT_ZONE_ST  (RES_ZONE_END)
#define SPT_ZONE_END  (SPT_ZONE_ST+SPT_LIST_LEN)

#define 	LPARSY	(	SPT_ZONE_ST	    )//(
#define 	RPARSY	(	SPT_ZONE_ST+1	)//)
#define 	PLUSSY	(	SPT_ZONE_ST+2	)//+
#define 	MINUSSY	(	SPT_ZONE_ST+3	)//-
#define 	MULSY	(	SPT_ZONE_ST+4	)//*
#define 	DIVSY	(	SPT_ZONE_ST+5	)//\/
#define 	COLONSY	(	SPT_ZONE_ST+6	)//:
#define 	SMCLSY	(	SPT_ZONE_ST+7	)//;
#define 	COMMASY	(	SPT_ZONE_ST+8	)//,
#define 	LBRCSY	(	SPT_ZONE_ST+9	)//{
#define 	RBRCSY	(	SPT_ZONE_ST+10	)//}
#define 	LBRKSY	(	SPT_ZONE_ST+11	)//[
#define 	RBRKSY	(	SPT_ZONE_ST+12	)//]
#define 	LSTSY	(	SPT_ZONE_ST+13	)//<
#define 	GTHSY	(	SPT_ZONE_ST+14	)//>
#define 	ASNSY	(	SPT_ZONE_ST+15	)//=
#define 	NEQSY	(	SPT_ZONE_ST+16	)//\!=
#define 	LEQSY	(	SPT_ZONE_ST+17	)//<=
#define 	GEQSY	(	SPT_ZONE_ST+18	)//>=
#define 	EQUSY	(	SPT_ZONE_ST+19	)//==

#define     ISCMP(x)   ((x)>=LSTSY&&(x)<=EQUSY&&(x)!=ASNSY)

/*constant values*/
#define VAL_ZONE_ST  (SPT_ZONE_END)
#define VAL_ZONE_END  (VAL_ZONE_ST+5)
#define INTVALSY (SPT_ZONE_END+0)
#define CHARVALSY (SPT_ZONE_END+1)
#define STRVALSY (SPT_ZONE_END+2)
#define IDSY (SPT_ZONE_END+3)
#define ZEROSY (SPT_ZONE_END+4)

#endif // LEX_H_INCLUDED
