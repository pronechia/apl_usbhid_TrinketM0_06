//
//info 情報定義 
//
typedef struct _INFO {
   char label[8];       //値の識別用
   uint16_t  address;   //EEPROM保存時の場所。先頭から何バイト目か？
   uint16_t  len;       //1,2,4byte
   //int type;          //1:FLG 2:char 3:int8 4:int16 5:uint32 6:float
                        //=>type はアプリが意識すればいいのでなくす
                        // 他に　単位、小数点位置、等が考えられる
 } INFO;
//
#define INFOMAX 5
int tableInfoCount = INFOMAX;
INFO tableInfo[INFOMAX];
//測定ログを保持する保持するヘッダー info
#define HEADERMAX 20
INFO tableHeaderInfo[HEADERMAX];
int tableHeaderInfoCount = HEADERMAX;
//
#define LOGRMAX 5
INFO tableLogInfo[LOGRMAX];
//
int tableLogInfoCount;
int tableLogSize = 0;     //1周期のデータバイト
int tableLogRepeat[3]; //周期の　最大繰り返し回数 
uint16_t tableLogStartAddr[3]; //先頭アドレス
//
