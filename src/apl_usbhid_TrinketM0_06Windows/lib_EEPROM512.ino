//
#include "lib_EEPROM512.h"
/*
 * boolean sub_EEPROM512_init()
 * sub_EEPROM512_plot()
 * void sub_EEPROM512_ReadLogHeader(unsigned int* p_s, unsigned int* p_e, unsigned int* p_c, unsigned int* p_n)
 * void sub_EEPROM512_ReadLogData(int pCount, int16_t* p_t, int16_t* p_h, int16_t* p_b, int8_t* p_c)
 * void sub_EEPROM512_WriteLogHeader(char p_se) 
 * void sub_EEPROM512_WriteLogData(int16_t p_t, int16_t p_h, int16_t p_p, int8_t p_c)
 * void sub_EEPROM512_commandOUT(int8_t pNum)
 * void sub_EEPROM512_commandIN(int8_t pNum)
 */
/*
 * 周期と時間
 *  log0=960b,log1=960b,kig2=6k=1024*6=4096+2048=6144
 *  log0,1では、4バイトが240個保存可。log2では、4バイトが1536個保存可。
 *  1分周期なら、60分で 60個。24時間で1440個。
 */
#define APPL_PLOT
//#define I2CEEPROM512 0x50   //64kbit=8kByte
//#define I2CEEPROM512 0x56     //512kbit=64kByte の半分
#define I2CEEPROM512 0x57     //512kbit=64kByte の半分
static unsigned long gCount;
static int8_t gPlotNumber;
static int8_t gNextNumber;
//
  u2 gEINData[501][4];

//
//
//
boolean sub_EEPROM512_init() {
//EEPROMのアドレス
// 今回版では最新データの受け渡しはEEPROMではなくメモリ渡しにするので、
//  以下のラベル領域は使わない。 
/* 8Kの小さなEEPROMの場合
// あちゃー　64kbyte と思ったら64kbit=8kbyteだった。 0x2000
//一次元データ 64x2 ログ6k,1k-128,1k-128
// 0x0400が1kbyte。　0x0400,0x0800,0x0c00,0x1000,0x1400,0x1800,0x1c00,0x2000
// log0:0x0080 - 0x03FF までの 896(128+256+256+256)
// log1:0x0480 - 0x07FF までの 896(128+256+256+256)
// log2:0x0800 - 0x1FFF まで 6kbyte を当てる
// 0x0000 0x0400 0x0780
//memo 共通ヘッダーは0x0000-0x003F
//まだない 
//memo 一次元データは0x0040-0x007F
#define MAPDT     0x0040
#define MAPDH     0x0042
#define MAPDP     0x0044
#define MAPDC     0x0046
#define MAPDY     0x0047
//memo 空き 0x0048-0x007F
//memo ログヘッダーは 0x0400-0x047F
// LOG Header 16バイトx３セット  +0,+0x10,+0x20
#define MAPFLG  0x0400
#define MAPYOBI1 0x0402
#define MAPSTART 0x0404
#define MAPEND   0x0408
#define MAPCYCLE 0x040c
#define MAPCOUNT 0x040e
//ログヘッダー 管理情報
#define MAPNEXT 0x0430
#define MAPPLOT 0x0431
// 空き 0x0432-0x047F
// LOG 896,896,6k(例：4byte/1分 周期で6kは、約25時間  
#define MAPLOGK     0x0080
#define MAPLOGL     0x0082
//#define MAPLOGM     0x0084
#define MAPLOGC     0x0083
//#define MAPLOGD     0x4007  //ダミー。EEPROMのパディングで必要？　あったほうがよさそう 
//log 温度2,湿度１、制御１＝４バイト 
*/
/* 64Kx2の大きなEEPROMの場合 */
//一次元データ 64x2 ログ16k x 3
// 0x0400が1kbyte。　0x0400,0x0800,0x0c00,0x1000,0x1400,0x1800,0x1c00,0x2000
// log0:0x4000 - 0x7FFF までの 16kbyte
// log1:0x8000 - 0xBFFF までの 16kbyte
// log2:0xC000 - 0xFFFF まで 16kbyte を当てる
//memo 共通ヘッダーは0x0000-0x00FF
//まだない 
//memo 一次元データは0x0100-0x1FFF
#define MAPDT     0x0140
#define MAPDH     0x0142
#define MAPDP     0x0144
#define MAPDC     0x0146
#define MAPDY     0x0147
//memo 空き 0x2000-0x37FF
//memo ログヘッダーは 0x3F00-0x3FFF
// LOG Header 16バイトx３セット  +0,+0x10,+0x20
#define MAPFLG  0x3F00
#define MAPYOBI1 0x3F02
#define MAPSTART 0x3F04
#define MAPEND   0x3F08
#define MAPCYCLE 0x3F0c
#define MAPCOUNT 0x3F0e
//ログヘッダー 管理情報
#define MAPNEXT 0x3F30
#define MAPPLOT 0x3F31
// 空き 0x3F32-0x3FFF
// LOG 16k x 3 (例：4byte/1分周期でサイズが16kは、16*1024/60/4=2*1024/30=2048/30=68時間)
#define MAPLOGK     0x0000
#define MAPLOGL     0x0002
//#define MAPLOGM     0x0004
#define MAPLOGC     0x0003
//log 温度2,湿度１、制御１＝４バイト 
//
//
  //Wire.setClock(100000);
  Wire.setClock(400000);
  //Wire.setClock(800000);
#ifndef APPL_PLOT
  Serial.println("EEPROM512_test setClock 400kHdz");
#endif
  strcpy(tableInfo[0].label , "Dtemp");
  tableInfo[0].address = MAPDT;
  tableInfo[0].len = 2;
  strcpy(tableInfo[1].label , "Dhum");
  tableInfo[1].address = MAPDH;
  tableInfo[1].len = 2;
  strcpy(tableInfo[2].label , "Dpress");
  tableInfo[2].address = MAPDP;
  tableInfo[2].len = 2;
  strcpy(tableInfo[3].label , "Dctl");
  tableInfo[3].address = MAPDC;
  tableInfo[3].len = 1;
  strcpy(tableInfo[4].label , "Dyobi");
  tableInfo[4].address = MAPDY;
  tableInfo[4].len = 1;
  tableInfoCount = 5;
#ifndef APPL_PLOT
  Serial.printf("sub_EEPROM512 Info sizeof(%d) tableInfoCount(%d)"
      , sizeof(INFO), tableInfoCount);  
  Serial.println("");
#endif
  //フラグ 0x01,0xfe が有効 
  strcpy(tableHeaderInfo[0].label , "f0");
  tableHeaderInfo[0].address = MAPFLG;
  tableHeaderInfo[0].len = 2;
  //予備 
  strcpy(tableHeaderInfo[1].label , "y0");
  tableHeaderInfo[1].address = MAPYOBI1;
  tableHeaderInfo[1].len = 2;
  //日付。時刻不明のため　当面は　unsigned long mills()
  strcpy(tableHeaderInfo[2].label , "s0");
  tableHeaderInfo[2].address = MAPSTART;
  tableHeaderInfo[2].len = 4;
  //日付。時刻不明のため　当面は　unsigned long mills()
  strcpy(tableHeaderInfo[3].label , "e0");
  tableHeaderInfo[3].address = MAPEND;
  tableHeaderInfo[3].len = 4;
  //周期　unsigned long 秒
  strcpy(tableHeaderInfo[4].label , "c0");
  tableHeaderInfo[4].address = MAPCYCLE;
  tableHeaderInfo[4].len = 2;
  //データ点数。int16
  strcpy(tableHeaderInfo[5].label , "n0");
  tableHeaderInfo[5].address = MAPCOUNT;
  tableHeaderInfo[5].len = 2;

    //フラグ 0x01,0xfe が有効 
  strcpy(tableHeaderInfo[6].label , "f1");
  tableHeaderInfo[6].address = MAPFLG + 16;
  tableHeaderInfo[6].len = 2;
  //予備 
  strcpy(tableHeaderInfo[7].label , "y1");
  tableHeaderInfo[7].address = MAPYOBI1 + 16;
  tableHeaderInfo[7].len = 2;
  //日付。時刻不明のため　当面は　unsigned long mills()
  strcpy(tableHeaderInfo[8].label , "s1");
  tableHeaderInfo[8].address = MAPSTART + 16;
  tableHeaderInfo[8].len = 4;
  //日付。時刻不明のため　当面は　unsigned long mills()
  strcpy(tableHeaderInfo[9].label , "e1");
  tableHeaderInfo[9].address = MAPEND + 16;
  tableHeaderInfo[9].len = 4;
  //周期　unsigned long 秒
  strcpy(tableHeaderInfo[10].label , "c1");
  tableHeaderInfo[10].address = MAPCYCLE + 16;
  tableHeaderInfo[10].len = 2;
  //データ点数。int16
  strcpy(tableHeaderInfo[11].label , "n1");
  tableHeaderInfo[11].address = MAPCOUNT + 16;
  tableHeaderInfo[11].len = 2;

  //フラグ 0x01,0xfe が有効 
  strcpy(tableHeaderInfo[12].label , "f2");
  tableHeaderInfo[12].address = MAPFLG + 32;
  tableHeaderInfo[12].len = 2;
  //予備 
  strcpy(tableHeaderInfo[13].label , "y2");
  tableHeaderInfo[13].address = MAPYOBI1 + 32;
  tableHeaderInfo[13].len = 2;
  //日付。時刻不明のため　当面は　unsigned long mills()
  strcpy(tableHeaderInfo[14].label , "s2");
  tableHeaderInfo[14].address = MAPSTART + 32;
  tableHeaderInfo[14].len = 4;
  //日付。時刻不明のため　当面は　unsigned long mills()
  strcpy(tableHeaderInfo[15].label , "e2");
  tableHeaderInfo[15].address = MAPEND + 32;
  tableHeaderInfo[15].len = 4;
  //周期　unsigned long 秒
  strcpy(tableHeaderInfo[16].label , "c2");
  tableHeaderInfo[16].address = MAPCYCLE + 32;
  tableHeaderInfo[16].len = 2;
  //データ点数。int16
  strcpy(tableHeaderInfo[17].label , "n2");
  tableHeaderInfo[17].address = MAPCOUNT + 32;
  tableHeaderInfo[17].len = 2;
  //管理情報  、次回書き込みログ番号
  strcpy(tableHeaderInfo[18].label , "ln");
  tableHeaderInfo[18].address = MAPNEXT;
  tableHeaderInfo[18].len = 1;
  //管理情報  PLOT対象ログ番号
  strcpy(tableHeaderInfo[19].label , "lp");
  tableHeaderInfo[19].address = MAPPLOT;
  tableHeaderInfo[19].len = 1;
  //
  tableHeaderInfoCount = 20;  // 6 x 3set + 2
#ifndef APPL_PLOT
  Serial.printf("sub_EEPROM512 Info sizeof(%d) tableHeaderInfoCount(%d)"
      , sizeof(INFO), tableHeaderInfoCount);
  Serial.println("");
#endif
  //24時間の測定ログを保持する info
  strcpy(tableLogInfo[0].label , "LK");
  tableLogInfo[0].address = MAPLOGK;
  tableLogInfo[0].len = 2;
  strcpy(tableLogInfo[1].label , "LL");
  tableLogInfo[1].address = MAPLOGL;
  tableLogInfo[1].len = 1;
  //strcpy(tableLogInfo[2].label , "LM");
  //tableLogInfo[2].address = MAPLOGM;
  //tableLogInfo[2].len = 2;
  strcpy(tableLogInfo[2].label , "LC");  //controll値　エアコン on/off
  tableLogInfo[2].address = MAPLOGC;
  tableLogInfo[2].len = 1;  // int8_t とする 
  //strcpy(tableLogInfo[4].label , "LD");  //ダミー
  //tableLogInfo[4].address = MAPLOGD;
  //tableLogInfo[4].len = 1;  // int8_t とする 
  tableLogInfoCount = 4;
  tableLogSize = 4;  //=2+1+1
  /* 8Kの小さなEEPROMの場合
  tableLogRepeat[0] = 223;
  tableLogRepeat[1] = 223;
  tableLogRepeat[2] = 1535; //周期の　最大繰り返し回数
  tableLogStartAddr[0] = 0x0000;
  tableLogStartAddr[1] = 0x0400;
  tableLogStartAddr[2] = 0x0780;
  */
  /* 64K x 2 の大きなEEPROMの場合 */
  tableLogRepeat[0] = 4095;  // 16k / 4
  tableLogRepeat[1] = 4095;
  tableLogRepeat[2] = 4095; //周期の　最大繰り返し回数
  tableLogStartAddr[0] = 0x4000;
  tableLogStartAddr[1] = 0x8000;
  tableLogStartAddr[2] = 0xC000;
  
#ifndef APPL_PLOT
  Serial.printf("sub_EEPROM512 Info sizeof(%d) tableLogInfoCount(%d) tableLogSize(%d)"
      , sizeof(INFO), tableLogInfoCount, tableLogSize);
  Serial.println("");
  Serial.printf("sub_EEPROM512 Info tableLogRepeat(%d) tableLogPage(%x)"
      , ttableLogRepeat, tableLogPage);
  Serial.println("");
#endif
  //
  //device check
  Wire.beginTransmission(I2CEEPROM512);
  byte e = Wire.endTransmission();
  if (e != 0 ) {
    Serial.printf("I2CEEPROM512 (%d) e(0x%x) ERROR!!!\n", I2CEEPROM512, e);
    return false;
  }

  //ログは３つある。PLOT対象ログ番号、次回書き込みログ番号はEEPROMを読んで求める
  int8_t wP;
  int8_t wN;
  sub_EEPROM512_ReadLogCommon(&wP, &wN);
  //Serial.printf("sub_EEPROM512 Info wP(%d) wN(%d)", wP, wN);
  //Serial.println("");

  //LogData のread/write について、 
  //Plot は読む時、 Next は書く時と限定していいか？ =>よし、いいとして進む 
  //
  gPlotNumber = wP;
  gNextNumber = wN;
  //
  return true;
}
//
//EEPROMを読んで、Serial.print でPLOTする 
// Serial.printをデバッグ で使用しては駄目 
// 初めにY軸の名称を出す。名称に周期や時間幅などを含める
//
// ログは最大３つある。そのうち、PLOTするログ番号を設定後、PLOTすること
void sub_EEPROM512_plot(char pPage) {
  static boolean s_ONFlag = false;
  unsigned long p_s;
  unsigned long p_e;
  int16_t p_c;
  int16_t p_n;
  int16_t p_t;
  int16_t p_h;
  int16_t p_b;
  int8_t p_count;
  int wmax;
  int w_start;
  int w_end;
  int w_page;

  //gPlotNumberはグローバル変数で常に最新。入力パラメータ。0,1,2 のいづれか。 
  sub_EEPROM512_ReadLogHeader(gPlotNumber,&p_s, &p_e, &p_c, &p_n);

  //周期は、当面固定
  //Serial.println("Y_temp_10sec, Y_humi_10sec, Y_balo_10sec, Y_ONOFF_10sec");  //Y軸出力 
  Serial.println("Y_temp_20sec, Y_ONOFFLevel_20sec");  //Y軸出力 
  wmax = p_n;
  w_page = pPage - 48; //数字に変換。 char to int
  //
  //page オーバー
  if (((wmax - 1) / 500 + 1) < w_page) {
    Serial.printf("sub_EEPROM512_plot page over ");
    Serial.println("");
    return;
  }
  //500区切り
  w_start = w_page * 500 - 500;
  w_end =  w_page * 500 - 1;
  if (w_end > wmax) w_end = wmax;
  int w_kari = 20;
  int j;
  //前ページからの引き継ぎ
  if (s_ONFlag == true) w_kari = 21;
  else w_kari = 20;

  for (j=w_start; j<w_end; j++) {
    sub_EEPROM512_ReadLogData(j, &p_t, &p_h, &p_b, &p_count);
    float ft = float(p_t/100.0);
    float fh = float(p_h);
    //float fb = float(p_b/100.0); //値のレンジをそろえるために　1013->10.13 にした 
    //Serial.print( j);  //DEBUG
    //Serial.print(", ");  //DEBUG
    Serial.print( ft, 2);
    Serial.print(", ");
    //Serial.print( fh, 0);
    //Serial.print(", ");
    //Serial.print( fb, 2);
    //Serial.print(", ");
    if (p_count == 1) {
       w_kari = 21;
    } else if (p_count == 2) {
      w_kari = 20;   
    }
    //Serial.printf("%d", p_count + 15);  //下駄上げして、グラフの見栄えがよくならないか？
    //Serial.print(", ");
    Serial.printf("%d", w_kari);  //edge ではなく、レベルで表示表示してみる 
    Serial.println("");  
  }
  //次ページのためにONFlag 保管
  if (w_kari == 21 ) s_ONFlag = true;
  else s_ONFlag = false;
  
  if (j >= wmax) {
    //最後のページは ダミー出力しないとおもったがやってみる  
    // ゼロでもいいが、Y軸を広くみたいので、中心付近の値（15)にしてみた
    p_t = 20;
    p_h = 0;
    p_b = 0;
    p_count = 20;
    w_kari = 20;
    for (int k= (w_end - w_start); k<(500 - 1); k++) {
      Serial.printf("%d",p_t);
      Serial.print(", ");
      //Serial.print( p_h, 2);
      //Serial.print(", ");
      //Serial.print( p_b, 0);
      //Serial.print( p_b / 100, 2); //値のレンジをそろえるために　1013->10.13 にしてみた 
      //Serial.print(", ");
      //Serial.printf("%d", p_count);
      //Serial.print(", ");
      Serial.printf("%d", w_kari);
      Serial.println("");
    }
    return ;
  }
}
//
//Log共通情報を読む  
//
void sub_EEPROM512_ReadLogCommon(int8_t* p_Plot, int8_t* p_Next) {
  char p[16];
  int8_t* fi8;
  int8_t w;

  //plot
  sub_GetVal("lp", p);
  // １バイトなので型変換がなくてもうまくいく
  *p_Plot = p[0];
  //fi8 = (int8_t*)(p);
  //w = (*fi8);
  //*p_Plot = w;
  //next
  sub_GetVal("ln", p);
  *p_Next = p[0];
  //sub_GetVal("ln", p);
  //fi8 = (int8_t*)(p);
  //w = (*fi8);
  //*p_Next = w;

  //Serial.printf("sub_EEPROM512_ReadLogCommon  plot(%d) next(%d)", *p_Plot , *p_Next);
  //Serial.println("");

}
//
void sub_EEPROM512_WriteLogCommon(int8_t* p_Plot, int8_t* p_Next) {
  char val[2];
  
  //Serial.printf("sub_EEPROM512_WriteLogCommon  plot(%d) next(%d)", *p_Plot , *p_Next);
  //Serial.println("");
  val[0] = *p_Plot;
  val[1] = 0x00;
  sub_SetVal("lp", val);  
  val[0] = *p_Next;
  val[1] = 0x00;
  sub_SetVal("ln", val);  

  gPlotNumber = *p_Plot;
  gNextNumber = *p_Next;
  
}
//
//Logヘッダーを読む  
//
bool sub_EEPROM512_ReadLogHeader(int8_t p_lognum, unsigned long* p_s, unsigned long* p_e, int16_t* p_c, int16_t* p_n) {
  unsigned long s;
  unsigned long e;
  int16_t c;
  int16_t n;
  char p[128];
  unsigned long* fp;
  int16_t* fi;
  char wLabel[5][8] = {"sx", "ex", "cx", "nx", "fx"};

  // ログ番号は0,1,2 であること 
  if (( p_lognum < 0 ) || ( p_lognum > 2 )) {
    Serial.printf("sub_EEPROM512_ReadLogHeader slognum(%d) ERROR!!!", p_lognum);
    Serial.println(" ");
    return false;
  }
  //
  wLabel[0][1] = p_lognum + 48; //int値を　文字に変換
  wLabel[1][1] = p_lognum + 48; //int値を　文字に変換
  wLabel[2][1] = p_lognum + 48; //int値を　文字に変換
  wLabel[3][1] = p_lognum + 48; //int値を　文字に変換
  wLabel[4][1] = p_lognum + 48; //int値を　文字に変換
  //Serial.printf("sub_EEPROM512_ReadLogHeader  wLabel[0](%s) wLabel[1](%s)"
  //, wLabel[0], wLabel[1]);
  //Serial.println(" ");
  //Serial.printf("sub_EEPROM512_ReadLogHeader  wLabel[2](%s) wLabel[3](%s)"
  //, wLabel[2], wLabel[3]);
  //Serial.println(" ");

  /**  次回、ログヘッダを正しく書いた後から有効にする 
  //FLG
  sub_GetVal(wLabel[4], p);
  fi = (int16_t*)(p);
  n = (*fi);
  if (n != 0x01fe) {
    Serial.printf("sub_EEPROM512_ReadLogHeader slognum(%d) FLG(%x) ERROR!!!"
     , p_lognum, n);
    Serial.println(" ");
    return false;
  }
  ***/

  //EEPROMから読んだデータをアプリが型を意識して扱う 
  //s
  sub_GetVal(wLabel[0], p);
  fp = (unsigned long*)(p); //一旦 int* に代入するとうまくいく。直接 char*だとダメ
                    //たぶんリトルエンディアン、ビックエンディアンの変換場所の問題
  s = (*fp);
  //e
  sub_GetVal(wLabel[1], p);
  fp = (unsigned long*)(p);
  e = (*fp);
  //c
  sub_GetVal(wLabel[2], p);
  fi = (int16_t*)(p);
  c = (*fi);
  //n
  sub_GetVal(wLabel[3], p);
  fi = (int16_t*)(p);
  n = (*fi);

  gCount = n;
#ifndef APPL_PLOT
  Serial.printf("sub_EEPROM512_ReadLogHeader n (%d msec) end(%d msec) cycle(%d sec) n(%d)"
    ,s ,e ,c ,n);
  Serial.println("");
#endif
  //
  *p_s = s;
  *p_e = e;
  *p_c = c;
  *p_n = n;

  return true;
}
//
//Logヘッダーを書く 
//
void sub_EEPROM512_WriteLogHeader(char p_se) {
  //gNextNumber
  char* p;
  char val[3];
  char wLabel[6][8] = {"sx", "ex", "cx", "nx", "fx", "yx"};

  wLabel[0][1] = gNextNumber + 48; //int値を　文字に変換
  wLabel[1][1] = gNextNumber + 48; //int値を　文字に変換
  wLabel[2][1] = gNextNumber + 48; //int値を　文字に変換
  wLabel[3][1] = gNextNumber + 48; //int値を　文字に変換
  wLabel[4][1] = gNextNumber + 48; //int値を　文字に変換
  wLabel[5][1] = gNextNumber + 48; //int値を　文字に変換
  //初めに、開始時刻/周期を書いて、終了時刻と件数は初期値を書いておく。
  //終わりに、SWが押されたら　終了時刻／件数を書く。
  if (p_se == 's') {
    Serial.printf("sub_EEPROM512_WriteLogHeader p_se(%c) =s ",p_se);
    Serial.println("");
    //
    gCount = 0;
    //FLG
    //ヘッダーを読んだ時に有効かどうかチェックが必要だが未実装 
    val[0] = 0x01;   
    val[1] = 0xfe;
    val[2] = 0x00;
    sub_SetVal(wLabel[4], val);  
    //予備 
    val[0] = 0x00;   
    val[1] = 0x00;
    val[2] = 0x00;
    sub_SetVal(wLabel[5], val);     
    //開始時刻
    unsigned long s = millis();
    p = (char*)&s;
    sub_SetVal(wLabel[0], p); 
    //終了時刻　初期化
    unsigned long e = 0; //仮の値 
    p = (char*)&e;
    sub_SetVal(wLabel[1], p);     
    //周期
    int16_t c = 20; //20秒で三回／分。60秒なら1分周期 
    p = (char*)&c;
    sub_SetVal(wLabel[2], p); 
    //件数　初期化 
    int16_t n = 0; //仮の値
    p = (char*)&n;
    sub_SetVal(wLabel[3], p); 
  } else if (p_se == 'e') {
    //終了時刻　初期化
    unsigned long e = millis();
    p = (char*)&e;
    sub_SetVal(wLabel[1], p); 
    //件数
    int16_t n = gCount;
    p = (char*)&n;
    sub_SetVal(wLabel[3], p); 
    Serial.printf("sub_EEPROM512_WriteLogHeader p_se(%c) =e count(%d)",p_se,n);
    Serial.println("");
  } else {
    Serial.printf("sub_EEPROM512_WriteLogHeader p_se(%c) ERROR!!!",p_se);
    Serial.println("");
  }

}
//
//Logヘッダーを書く 
//
void sub_EEPROM512_WriteLogHeaderPara(int8_t p_Num, u4 p_Start, u4 p_End, int8_t p_Cycle, int16_t p_Count) {
  //gNextNumber
  char* p;
  char val[3];
  char wLabel[6][8] = {"sx", "ex", "cx", "nx", "fx", "yx"};

  //0,1,2 以外エラー
  Serial.printf("sub_EEPROM512_WriteLogHeaderPara p_Num(%d)",p_Num);
  Serial.println("");
  if ((p_Num < 0) || ( p_Num > 2)) {
    return;
  }
  //Label の２文字目（LOG番号に相当）置き換え。　例）sx=>s0
  wLabel[0][1] = p_Num + 48; //int値を　文字に変換
  wLabel[1][1] = p_Num + 48; //int値を　文字に変換
  wLabel[2][1] = p_Num + 48; //int値を　文字に変換
  wLabel[3][1] = p_Num + 48; //int値を　文字に変換
  wLabel[4][1] = p_Num + 48; //int値を　文字に変換
  wLabel[5][1] = p_Num + 48; //int値を　文字に変換
  //初めに、開始時刻/周期を書いて、終了時刻と件数は初期値を書いておく。
  //終わりに、SWが押されたら　終了時刻／件数を書く。

  //
  gCount = 0;
  //FLG
  //ヘッダーを読んだ時に有効かどうかチェックが必要だが未実装 
  val[0] = 0x01;   
  val[1] = 0xfe;
  val[2] = 0x00;
  sub_SetVal(wLabel[4], val);  
  //予備 
  val[0] = 0x00;   
  val[1] = 0x00;
  val[2] = 0x00;
  sub_SetVal(wLabel[5], val);
  //開始時刻
  unsigned long s = p_Start;
  p = (char*)&s;
  sub_SetVal(wLabel[0], p); 
  //終了時刻　初期化
  unsigned long e = p_End;
  p = (char*)&e;
  sub_SetVal(wLabel[1], p);     
  //周期
  int16_t c = p_Cycle;
  p = (char*)&c;
  sub_SetVal(wLabel[2], p); 
  //件数　初期化  何ページが決まらないので始めに全件数を書いてしまう
  int16_t n = p_Count;
  p = (char*)&n;
  sub_SetVal(wLabel[3], p);     
  //int16_t n = gCount;  
}




//
//一周期のログをログを読む 
//
void sub_EEPROM512_ReadLogData(int pCount, int16_t* p_t, int16_t* p_h, int16_t* p_b, int8_t* p_c) {
  int16_t t;
  int16_t h;
  int16_t b;
  int8_t c;
  int16_t* fp;
  int8_t* ip;
  char p[8];
  
  sub_GetLogVal(pCount, "LK", p);
  fp = (int16_t*)(p);
  //Serial.printf("temp(%x%x )",p[0],p[1]);

  t = (*fp);
  sub_GetLogVal(pCount, "LL", p);
  ip = (int8_t*)(p);
  //Serial.printf("humi(%x%x )",p[0],p[1]);
  //Serial.println("");
  h = (*ip);
  //sub_GetLogVal(pCount, "LM", p);
  //fp = (int16_t*)(p);
  //b = (*fp);
  sub_GetLogVal(pCount, "LC", p);
  ip = (int8_t*)(p);
  c = (*ip);

#ifndef APPL_PLOT
  Serial.printf("sub_EEPROM512_ReadLogData c(%d)",c);
  Serial.println("");
#endif
  //EEPROMの値がおかしいとき、とりあえずゼロにする。 
  if ((t > 9990) || ( t < 10)) {
    *p_t = 0;
  } else {
    *p_t = t;    
  }
  // 湿度は１バイト。0%-100%
  if ((h > 9990) || ( h < 1)) {
    *p_h = 0;
  } else {
    *p_h = h;
  }
  //*p_b = b;
  *p_c = c;

}
//
//一周期のログを書く
//
void sub_EEPROM512_WriteLogData(int16_t p_t, int16_t p_h, int16_t p_p, int8_t p_c) {
  char* p;
  char buf[8];
  int8_t w;

  if (gCount >= tableLogRepeat[gNextNumber]) {
    Serial.printf("sub_EEPROM512_WriteLogData gCount(%d) MAX over!!!",gCount);
    Serial.println("");
    return;
  }
  //gCount で何周期目かを計算
  p = (char*)&p_t;
  sub_SetLogVal(gCount, "LK", p);
  w = p_h / 100; // 100.00% -> 100%
  p = (char*)&w;
  sub_SetLogVal(gCount, "LL", p);
  //p = (char*)&p_p;
  //sub_SetLogVal(gCount, "LM", p);
  //if ((p_c >= 0 ) && (p_c < 11)) {
  //  ;
    //Serial.printf("sub_EEPROM512_WriteLogData p_c(%d)",p_c);
    //Serial.println("");
  //} else {
  //  Serial.printf("sub_EEPROM512_WriteLogData p_c(%d) ERROR!!!",p_c);
  //    Serial.println("");
  //}
  p = (char*)&p_c;
  sub_SetLogVal(gCount, "LC", p);  //制御値　ON/OFF２情報。1バイト       
  gCount += 1;
}
//EOUT
//plotと似ている
void sub_EEPROM512_commandOUT(int8_t pNum, int8_t pPage) {
#define D500 500
//EEPROMを読んでSerialPrint する
//番号から先頭カウントを求める
//まずは、plot同様に、ラベルを使って1ch づつread
//できればplot も合わせて、連続read するほうが早いし EEPROM寿命もいい
//1chづつ　write&改行
 /* u1 x;
  int j;
  char buf[16];
 */
  //static boolean s_ONFlag = false;
  unsigned long p_s;
  unsigned long p_e;
  int16_t p_c;
  int16_t p_n;
  int16_t p_t;
  int16_t p_h;
  int16_t p_b;
  int8_t p_count;
  int wmax;
  int w_start;
  int w_end;
  int w_page;
  int8_t wPlotNumber;

  //グローバル変数退避(LOGを読む処理で随所に使っている）
  wPlotNumber = gPlotNumber;
  gPlotNumber = pNum;
  Serial.printf("sub_EEPROM512_commandOUT: n(%d) page(%d)", pNum, pPage); 
  Serial.println("");

  //pNumは入力パラメータ。0,1,2 のいづれか。 
  sub_EEPROM512_ReadLogHeader(pNum,&p_s, &p_e, &p_c, &p_n);

  //LOG番号、ページ番号、開始、終了、周期、データ件数
  Serial.printf("EOUT %d,%d,%d,%d,%d,%d", pNum, pPage, p_s, p_e, p_c, p_n );
  Serial.println("");
  wmax = p_n;
  w_page = pPage;
  //
  //page オーバー
  if (((wmax - 1) / D500 + 1) < w_page) {
    Serial.printf("sub_EEPROM512_commandOUT page over ");
    Serial.println("");
    //グローバル変数戻す(LOGを読む処理で随所に使っている）
    gPlotNumber = wPlotNumber;
    return;
  }
  //500区切り
  w_start = w_page * D500 - D500;
  w_end =  w_page * D500 - 1;
  if (w_end > wmax) w_end = wmax;
  int j;
  for (j=w_start; j<w_end; j++) {
    sub_EEPROM512_ReadLogData(j, &p_t, &p_h, &p_b, &p_count);
    //float ft = float(p_t/100.0);
    float ft = float(p_t);
    float fh = float(p_h);
    //float fb = float(p_b/100.0); //値のレンジをそろえるために　1013->10.13 にした 
    //Serial.print( j);  //DEBUG
    //Serial.print(", ");  //DEBUG
    //Serial.print( ft, 2);
    Serial.print( ft, 0);
    Serial.print(",");
    Serial.print( fh, 0);
    Serial.print(",");
    //Serial.print( fb, 2);
    //Serial.print(", ");
    //if (p_count == 1) {
    //   w_kari = 21;
    //} else if (p_count == 2) {
    //  w_kari = 20;   
    //}
    Serial.printf("%d", p_count);  //下駄上げしない
    //Serial.print(", ");
    //Serial.printf("%d", w_kari);  //edge ではなく、レベルで表示表示してみる 
    Serial.println("");  
  }
  //グローバル変数戻す(LOGを読む処理で随所に使っている）
  gPlotNumber = wPlotNumber;
  //DEBUG
  Serial.printf("sub_EEPROM512_commandOUT: w_start(%d) w_end(%d)", w_start, w_end); 
  Serial.println("");

}
//IN
//page=1 のみ、ヘッダー情報も入力されるので、それをヘッダーに書く。
//Serialをread して、カンマ区切り、改行区切りで内部変数に読み込む。
//内部変数から値を取り出して、EEPROMに書く。ラベル指定で1ch つづ。
// 周期の数　x ch数　をループする。
// 連続書き込みは 改善項目
void sub_EEPROM512_commandIN(int8_t pNum, int8_t pPage) {
#define TIMEOUT_SEND           (2000000)
  u1 x;
  int j,jj;
  char buf[128];
  int8_t wNextNumber;
  int32_t p_EINStart;
  int32_t p_EINEnd;
  int8_t p_EINCycle;
  int16_t p_EINCount;
  u4 tmp = 0;
  u4 index = 0;

  //グローバル変数退避(LOGを読む処理で随所に使っている）
  wNextNumber = gNextNumber;
  gNextNumber = pNum;

  Serial.printf("sub_EEPROM512_commandIN: IN n(%d) page(%d)", pNum, pPage); 
  Serial.println("");
  
  //まず先頭行を読む。改行(と思ったら空白だった)まで
  //EOUTのデータをメモ帳に貼り付けた後、コピーすれば改行コードも含まれる
  //各ページにヘッダー行あり。２ページ目以降は、s,e,周期は使わない。件数は使う
  if (pPage > 0 ) {
    j = 0;
    while (1) {
      if(Serial.available()){
        x=Serial.read();
        if(x>='0' && x<='9'){
          // 数字を受信した場合
          tmp *= 10;
          tmp += x - '0';
        } else {
          // 数字以外を受信した場合
          if((tmp == 0) && (index == 0)){
            // 最初の一文字目は読み飛ばす
          } else {
            if (index == 0) p_EINStart = tmp;
            else if (index == 1) p_EINEnd = tmp;
            else if (index == 2) p_EINCycle = (u1)tmp;
            else if (index == 3) p_EINCount = tmp;
            index++;
          }
          tmp = 0;
        }
        //if (x == 0xa) break;
        if (x == 0x20) break;
        j = j + 1;
        if (j>90) break; //フェールセール対応
      } else {
        break;
      }
    }
    //DEBUG
    Serial.printf("s(%d) e(%d) c(%d) n(%d)", 
      p_EINStart, p_EINEnd, p_EINCycle, p_EINCount);
    Serial.println("");
  }
  //
  //値の取得
  //次に２行目から最終行まで、改行(空白)ごとに読みながら、データ取得
  //gEINData[] に格納する
  int16_t pRecordCount;
  sub_EEPROM512_getSerial(&pRecordCount);
  //Serialで読み込んだレコード数は、500または最終ページの件数のはずだが、
  //それ以外が来た場合の処理をどうするか？　エラーでわかりやすくしたほうがいいかな？
  Serial.println("sub_EEPROM512_commandIN: TODO recordCount not 500");
  Serial.printf("sub_EEPROM512_commandIN: recordCount(%d)",
    pRecordCount);
  Serial.println("");
  //page オーバー
  if (((p_EINCount - 1) / 500 + 1) < pPage) {
    Serial.printf("sub_EEPROM512_commandIN page(%d) over ",pPage);
    Serial.println("");
    //グローバル変数戻す(LOGを読む処理で随所に使っている）
    gNextNumber=wNextNumber;
    return;
  }
  
  //header
  if (pPage == 1) {
    Serial.println("sub_EEPROM512_commandIN only page=1");
    Serial.printf("sub_EEPROM512_commandIN num(%d) start(%d) end(%d) cycle(%d) count(%d)",pNum, p_EINStart, p_EINEnd, p_EINCycle, p_EINCount);
    Serial.println("");
    
    sub_EEPROM512_WriteLogHeaderPara(pNum, p_EINStart, p_EINEnd, p_EINCycle, p_EINCount);   
  }
  // chデータを書く。最大５００件
  // まず、全ページ中の位置を求める。　例）1000-1299
  int w_start = pPage * 500 - 500;
  int w_end =  pPage * 500 - 1;
  if (w_end > p_EINCount) w_end = p_EINCount;
  Serial.printf("sub_EEPROM512_commandIN 500 s(%d) e(%d)",w_start, w_end);
  Serial.println("");
  // 関数内でgCountをつかっている。呼び出し前に正しくセットするように。
  gCount = w_start;  //これは、EEPROM内の位置 例）　０，５００，１０００、、、
  //Serialから入力されたデータはゼロから５００以内。
  w_end = w_end - w_start; //500以内の端数に置き換える。
  w_start = 0; //先頭は必ずゼロ
  if (w_end > p_EINCount) w_end = p_EINCount;
  Serial.printf("sub_EEPROM512_commandIN 0 s(%d) e(%d)",w_start, w_end);
  Serial.println("");
  //
  int16_t wDummy = 0;
  for (j=w_start; j<w_end; j++) {
    //gEINData
    sub_EEPROM512_WriteLogData(gEINData[j][0],gEINData[j][1],wDummy,gEINData[j][2]);
  }
  //グローバル変数戻す(LOGを読む処理で随所に使っている）
  gNextNumber=wNextNumber;
}

void sub_EEPROM512_getSerial(int16_t* pN) {
  
  s1 state = STATE_NONE;
  u1 x;
  u4 tmp = 0;
  u4 index = 0;
  u1 line = 0;
  u4 us = 0;
  
  us = micros();
  
  while(state == STATE_NONE){
    if(Serial.available() == 0){
      if((micros() - us) > TIMEOUT_SEND){
        state = STATE_TIMEOUT;
        break;
      }
    } else {
      x = Serial.read();
      //Serial.printf("%x ",x);
      if(x>='0' && x<='9'){
        // 数字を受信した場合
        tmp *= 10;
        tmp += x - '0';
      } else {
        // 数字以外を受信した場合
        if((tmp == 0) && (index == 0) && (line == 0)){
          // 最初の一文字目は読み飛ばす
        } else {
          gEINData[index][line] = (u2)tmp;
          //if (index < 5) {
          //  Serial.printf("index(%d) line(%d) val(%d)",index, line, tmp);
          //  Serial.println("");
          //}
          line++;
        }
        tmp = 0;
      }
      //
      if (x == 0x20) {
        if ( line != 3 ) {
          Serial.printf("sub_EEPROM512_getSerial: line not 3 (%d %d)ERROR!!!", index, line);
          Serial.println("");
        }
        index++; //一件追加
        line = 0; //戻す
        if (index > 500 ) {
          state = STATE_TIMEOUT; //timeout ではないけど、異常対応
          break;
        }
      }

    }
  }
  /*
  //異常処理。シリアルのデータをすべて読み飛ばす
  int jj = 0;
  while (1) {
    if(Serial.available()){
      x=Serial.read();
      jj++;
    }
  }
  */
  //なぜかシリアルがつまつ感じなので、2秒あけてみた
  delay(2000);
  //
  *pN = (int16_t)index;
  //DEBUG
  for (int j=0; j<6; j++) {
    Serial.printf("%d , %d , %d", gEINData[j][0], gEINData[j][1], gEINData[j][2]);    
    Serial.println("");
  }
  Serial.printf("sub_EEPROM512_getSerial: pN(%d)", *pN);
  Serial.println("");

  return;
}



// - - - - - - - - - - 以下、不要な関数も含まれるがまだ精査できていない - - - - - - -
//
//
//変数 info に従って、データ読み出し 
void sub_val_read() {
  float k;
  float l;
  float m;
  char p[128];
  float* fp;

  //EEPROMから読んだデータをアプリが型を意識して扱う 
  sub_GetVal("k", p);
  fp = (float*)(p); //一旦 float* に代入するとうまくいく。直接 char*だとダメ
                    //たぶんリトルエンディアン、ビックエンディアンの変換場所の問題
  k = (*fp);
  sub_GetVal("l", p);
  fp = (float*)(p);
  l = (*fp);
  sub_GetVal("m", p);
  fp = (float*)(p);
  m = (*fp);

  //Serial.print( ps2.j, DEC);
  //Serial.println(" us");
  Serial.println("- - - - - - - - - - - - - - - - - - - - ");
  Serial.print( k, 2);
  Serial.println("  'C");
  Serial.print( l, 2);
  Serial.println(" %");
  Serial.print( (m) / 100, 2);
  Serial.println(" mmbar");
}
//---------------------------------
// ここからは、アプリのデータに依存しない関数。infoを定義してあればよい。 
//
//labelで指定された変数の値を 読む 
//
void sub_GetVal(char* pLabel, char* ps2) {
  char buf[16];

  //Serial.printf("sub_GetVal label(%s) ",pLabel);
  //Serial.println("");

  //tableInfo
  for (int j=0; j<tableInfoCount; j++) {
    //Serial.printf("(%d) label(%s) ",j,tableInfo[j].label);
    if (strcmp(pLabel,tableInfo[j].label) == 0) {
      EEPROM512_read(tableInfo[j].address, tableInfo[j].len, buf);
      //Serial.printf("sub_GetVal label(%c)... j(%d) matched val(0x",pLabel[0], j);
      for (int k=0; k<tableInfo[j].len; k++) {
        *ps2 = buf[k];
        ps2 += 1;
        //Serial.printf("%02x ",buf[k]);
      }
      //Serial.println(")");
      return;
    }
  }
  //tableHeaderInfo
  for (int j=0; j<tableHeaderInfoCount; j++) {
    //Serial.printf("(%d) label(%s) ",j,tableHeaderInfo[j].label);
    if (strcmp(pLabel,tableHeaderInfo[j].label) == 0) {
      EEPROM512_read(tableHeaderInfo[j].address, tableHeaderInfo[j].len, buf);
//#ifndef APPL_PLOT
      //Serial.printf("sub_GetVal label(%c) j(%d) matched val(0x",pLabel[0], j);
//#endif
      for (int k=0; k<tableHeaderInfo[j].len; k++) {
        *ps2 = buf[k];
        ps2 += 1;
//#ifndef APPL_PLOT
        //Serial.printf("%02x ",buf[k]);
//#endif
      }
//#ifndef APPL_PLOT
      //Serial.println(")");
//#endif
      /*      
      //DEBUG Cycleは必ず６０のはずだが、1011(0x3f3)となるバグ調査 
      if ((tableHeaderInfo[j].address == MAPCYCLE) ||
          (tableHeaderInfo[j].address == MAPCYCLE + 16) ||
          (tableHeaderInfo[j].address == MAPCYCLE + 32)) {
        Serial.printf("sub_GetVal label(%c) j(%d) matched val(0x",pLabel[0], j);
        for (int k=0; k<tableHeaderInfo[j].len; k++) {
          Serial.printf("%02x ",buf[k]);
        }
        Serial.println(")");
      }
      */
      //
      return;
    }
  }
  Serial.printf("sub_GetVal label(%s) not found ERROR!!! ",pLabel);
  Serial.println("");
}

//labelで指定された変数の値を 読む 
void sub_GetLogVal(unsigned long pCount, char* pLabel, char* ps2) {
  char buf[16];
  //gPlotNumber <= Get時
  //gNextNumber <= Set時

  //tableLogInfo
  for (int j=0; j<tableLogInfoCount; j++) {
    if (strcmp(pLabel,tableLogInfo[j].label) == 0) {
      //EEPROMの絶対アドレスを計算する。何周期目の何番目の要素か？ 
      //3set対応 
      uint16_t addr = (tableLogSize * pCount) + tableLogInfo[j].address;
      if (addr >= 0x4000) {
        //アドレス計算エラー
        Serial.printf("sub_GetLogVal addr>=0x4000 ERROR!!! pCount(%d) label(%c%c) j(%d) matched addr(0x%x) val(0x"
        ,pCount , pLabel[0], pLabel[1], j, addr);
        Serial.println("");
        //
        //for (int k=0; k<tableLogInfo[j].len; k++) {
        //  *ps2 = 0x00; //ダミーデータ
        //  ps2 += 1;
        //}
        return;
      }
      addr =  addr + tableLogStartAddr[gPlotNumber];
      EEPROM512_read(addr, tableLogInfo[j].len, buf);
      //Serial.printf("sub_GetLogVal pCount(%d) label(%c%c) j(%d) matched addr(0x%x) val(0x"
      //  ,pCount , pLabel[0], pLabel[1], j, addr);
      for (int k=0; k<tableLogInfo[j].len; k++) {
        *ps2 = buf[k];
        ps2 += 1;
        //Serial.printf("%02x ",buf[k]);
      }
      //Serial.println(")");
      //DEBUG
      //if (strcmp(pLabel, "LL") == 0) {
      //  Serial.printf("LL c(%d) addr(0x%x) ",pCount ,addr);
      //}
      delay(5);  // I2Cの待ち。不要かもしれないが。。。 
      return;
    }
  }
}
//
//実装途中イメージ
//labelで指定された変数の値を 書く
void sub_SetVal(char* pLabel, char* ppc) {
  //float wfloat = pVal;
  //char* pc = (char*)&wfloat;
  char* pc = ppc;
  char buf[16];
  
  for (int j=0; j<tableInfoCount; j++) {
    if (strcmp(pLabel,tableInfo[j].label) == 0) {
      for (int k=0; k<tableInfo[j].len; k++) {
        buf[k] = *pc;
        pc += 1;
      }
      EEPROM512_write(tableInfo[j].address, tableInfo[j].len, buf);
#ifndef APPL_PLOT
      Serial.printf("sub_SetVal info_j(%d) labbel(%c) len(%d) buf( 0x",j ,pLabel[0] ,tableInfo[j].len);
      for (int k=0; k<tableInfo[j].len; k++) {
        Serial.printf("%02x ",buf[k]);
      }
      Serial.println(")");      
#endif
      delay(5);  // I2Cの待ち。不要かもしれないが。。。 
      return;
    }
  }
  //
  pc = ppc;
  for (int j=0; j<tableHeaderInfoCount; j++) {
    if (strcmp(pLabel,tableHeaderInfo[j].label) == 0) {
      //
      for (int k=0; k<tableHeaderInfo[j].len; k++) {
        buf[k] = *pc;
        pc += 1;
      }
      EEPROM512_write(tableHeaderInfo[j].address, tableHeaderInfo[j].len, buf);
#ifndef APPL_PLOT
      Serial.printf("sub_SetVal Header_j(%d) labbel(%c) len(%d) addr(0x%x) buf(0x"
       ,j ,pLabel[0] ,tableHeaderInfo[j].len ,tableHeaderInfo[j].address);
      for (int k=0; k<tableHeaderInfo[j].len; k++) {
        Serial.printf("%02x ",buf[k]);
      }
      Serial.println(")");
#endif
      /*
      //DEBUG Cycleは必ず６０のはずだが、1011(0x3f3)となるバグ調査 
      if ((tableHeaderInfo[j].address == MAPCYCLE) ||
          (tableHeaderInfo[j].address == MAPCYCLE + 16) ||
          (tableHeaderInfo[j].address == MAPCYCLE + 32)) {
        Serial.printf("sub_SetVal Header_j(%d) labbel(%c) len(%d) addr(0x%x) buf(0x"
         ,j ,pLabel[0] ,tableHeaderInfo[j].len ,tableHeaderInfo[j].address);
        for (int k=0; k<tableHeaderInfo[j].len; k++) {
          Serial.printf("%02x ",buf[k]);
        }
        Serial.println(")");        
      }
      */
      delay(5);  // I2Cの待ち。不要かもしれないが。。。 
      return;
    }
  }
}
//
//labelで指定された変数の値を 書く
//
void sub_SetLogVal(unsigned long pCount, char* pLabel, char* ps) {
  //gPlotNumber <= Get時
  //gNextNumber <= Set時

  char* pc = ps;
  char buf[16];

  for (int j=0; j<tableLogInfoCount; j++) {
    if (strcmp(pLabel,tableLogInfo[j].label) == 0) {
      for (int k=0; k<tableLogInfo[j].len; k++) {
        buf[k] = *pc;
        pc += 1;
      }
      //EEPROMの絶対アドレスを計算する。何周期目の何番目の要素か？ 
      uint16_t addr = (tableLogSize * pCount) + tableLogInfo[j].address;
      if (addr >= 0x4000) {
        //アドレス計算エラー
        return;
      }
      addr =  addr + tableLogStartAddr[gNextNumber];
      //
      EEPROM512_write(addr, tableLogInfo[j].len, buf);
      //DEBUG
      //Serial.printf("sub_SetLogVal  j(%d) labbel(%c%c) len(%d) addr(0x%x) buf(0x"
      //  ,j ,pLabel[0] ,pLabel[1] ,tableLogInfo[j].len ,addr);
      //for (int k=0; k<tableLogInfo[j].len; k++) {
      //  Serial.printf("%02x ",buf[k]);
      //}
      //Serial.println(")");
      //DEBUG
      //if (strcmp(pLabel, "LL") == 0) {
      //  Serial.printf("LL c(%d) addr(0x%x) ",pCount ,addr);
      //}
      delay(5);  // I2Cの待ち。不要かもしれないが。。。 
      return;
    }
  }
  Serial.printf("sub_SetLogVal label(%c%c) not found ERROR!!!",pLabel[0] ,pLabel[1]);
  Serial.println("");
}
//
//
//
void EEPROM512_read(uint16_t paddr, int plen, char* ps2) {
  char dat0;
  char* p2 = (char*) ps2;

  // 512Kbit = 64Kbyte => アドレス空間が2byteなら2**16=1k * 2**6 = 1k * 64 = 64kbyte 
  byte a1 = (paddr >>  8) & 0xFF;
  byte a0 = (paddr )      & 0xFF;
  //Serial.printf("Read  a1(0x%02x) a0(0x%02x) len(%d)\n", a1, a0, plen);
  
  Wire.beginTransmission(I2CEEPROM512);
  Wire.write(a1);
  Wire.write(a0);
  byte e = Wire.endTransmission();                    // stop transmitting
  if (e != 0 ) {
    Serial.printf("Read  a1(0x%02x) a0(0x%02x) e(0x%x) ERROR!!!\n", a1, a0, e);
    return;
  }
  delay(5);
  int j=0;
  Wire.requestFrom(I2CEEPROM512, plen);
  while (Wire.available()) {
    dat0 = Wire.read();                       // receive a byte as character
    j++;
    *p2 = dat0;
    p2++;
  }
  delay(5);  //for DEBUG
  //Serial.printf("Read  a1(0x%02x) a0(0x%02x) read_byte(%d) dat0 1 2(0x%02x 0x%02x 0x%02x)\n"
  //  , a1, a0, j, *(ps2 + 0), *(ps2 + 1), *(ps2 + 2));

}
//
//
//
void EEPROM512_write(uint16_t paddr, int plen, char* p) {

  // 512Kbit = 64Kbyte => アドレス空間が2byteなら2**16=1k * 2**6 = 1k * 64 = 64kbyte 
  byte a1 = (paddr >>  8) & 0xFF;
  byte a0 = (paddr )      & 0xFF;
  //Serial.printf("Write  a1(0x%02x) a0(0x%02x) len(%d)\n", a1, a0, plen);

  Wire.beginTransmission(I2CEEPROM512);
  Wire.write(a1);
  Wire.write(a0);
  for (int i = 0; i < plen; i++) {
    Wire.write(*(p + i));
  }
  byte e = Wire.endTransmission();                    // stop transmitting
  if (e != 0 ) {
    Serial.printf("Write  a1(0x%x) a0(0x%x) e(0x%x) ERROR!!!\n", a1, a0, e);
    return;
  }
  delay(5);
  //Serial.printf("Write  a1(0x%2x) a0(0x%2x) dat0 1 2(0x%2x 0x%2x 0x%2x)\n",
  //  a1, a0, *(p + 0), *(p + 1), *(p + 2));
}
