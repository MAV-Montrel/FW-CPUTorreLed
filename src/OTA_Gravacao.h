#ifndef OTA_GRAVACAOH
#define OTA_GRAVACAO_H

void OTA_init(void);
void OTA_handleUpload(void);


#endif




  //  if(Tecla == 0){
  //       static int cont = 0;
  //       while(Tecla == 0){
  //           cont++;
  //           if(cont > 2000){
  //               OTA_init();
  //               while (1)
  //               {
  //                   OTA_handleUpload();
  //                   delay(10);                    
  //               }                
  //           }
  //       }
  //   }