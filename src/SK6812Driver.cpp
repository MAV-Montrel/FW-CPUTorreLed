#include "SK6812Driver.h"

#define NS_TO_US(ns) ((ns) / 1000)  // Conversão de nanossegundos para microssegundos


SK6812Driver::SK6812Driver(uint16_t numLeds, uint8_t dataPin) {
    this->numLeds = numLeds;
    this->dataPin = dataPin;
    this->ledBuffer = new uint8_t[numLeds * 4];
}

void SK6812Driver::begin() {
    pinMode(dataPin, OUTPUT);
}

void SK6812Driver::setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    if (index < numLeds) {
        uint16_t pixelOffset = index * 4;
        ledBuffer[pixelOffset] = r;
        ledBuffer[pixelOffset + 1] = g;
        ledBuffer[pixelOffset + 2] = b;
        ledBuffer[pixelOffset + 3] = w;
    }
}

// void SK6812Driver::sendBit(bool bitValue) {

//     digitalWrite(dataPin, 1);
//     if (bitValue == 0) {        
//       digitalWrite(dataPin, 0);
//       asm volatile("nop");
//       //delayMicroseconds(3);
//     } else {   
//       //delayMicroseconds(3);  
//       asm volatile("nop");  
//       digitalWrite(dataPin, 0);            
//     }
//      digitalWrite(dataPin, 1);


//   // if (bitValue == 0) {
//   //   // Send a '1' bit
//   //   //  Serial.print("1"); 
//   //   digitalWrite(dataPin, HIGH);
//   //   delayMicroseconds(300);  // T1H  LOW  HIGH
//   //   // for (int i = 0; i < 10; i++)
//   //   // {
//   //   //   asm volatile("nop");
//   //   // }    
//   //   //delayMicroseconds(NS_TO_US(600));  // T1H
//   //   digitalWrite(dataPin, LOW);
 
//   //   delayMicroseconds(900);  // T1L
//   //   //delayMicroseconds(NS_TO_US(600));  // T1L
//   //   // for (int i = 0; i < 10; i++)
//   //   // {
//   //   //   asm volatile("nop");
//   //   // }
//   // } 
//   // else {
//   //   // Send a '0' bit
//   //   //Serial.print("0"); 
//   //   digitalWrite(dataPin, HIGH);
//   //   delayMicroseconds(600);  // T0H    
//   //   // for (int i = 0; i < 5; i++)
//   //   // {
//   //   //   asm volatile("nop");
//   //   // }
//   //   //delayMicroseconds(NS_TO_US(300));  // T0H              
//   //   digitalWrite(dataPin, LOW);
//   //   delayMicroseconds(600); // T0L
    
//   //   // for (int i = 0; i < 15; i++)
//   //   // {
//   //   //   asm volatile("nop");
//   //   // }
//   //   //delayMicroseconds(NS_TO_US(900)); // T0L
//   // }

// }

void SK6812Driver::sendBit(bool bitValue) {
  digitalWrite(dataPin, 1);
  if (bitValue == 0) {        
    digitalWrite(dataPin, 0);
    asm volatile("nop");
    //delayMicroseconds(3);
  } else {   
    //delayMicroseconds(3);  
    asm volatile("nop");  
    digitalWrite(dataPin, 0);            
  }
  digitalWrite(dataPin, 1);
}
void SK6812Driver::show() {
  noInterrupts();  // Disable interrupts temporarily
  digitalWrite(dataPin, LOW);
  delayMicroseconds(80);
  //  Serial.print("r ");  // Red 
    for (int j = 8; j > 0; j--) {
      sendBit(ledBuffer[0] & (1 << j));
    }
  //  //  Serial.print(" g ");  // Green
  //   for (int j = 8; j > 0; j--) {
  //     sendBit(ledBuffer[1] & (1 << j));
  //   }

  //  // Serial.print(" b ");  // Blue
  //   for (int j = 8; j > 0; j--) {
  //     sendBit(ledBuffer[2] & (1 << j));
  //   } 
  //   // // //Serial.print(" w ");  // White
  //   for (int j = 8; j > 0; j--) {
  //     sendBit(ledBuffer[3] & (1 << j));
  //   }   
  interrupts();  // Re-enable interrupts
}


// void SK6812Driver::show() {
//   noInterrupts();  // Disable interrupts temporarily
//   digitalWrite(dataPin, LOW);
//   delayMicroseconds(80);

//   //   uint32_t teste = 0xFFFFFFFF;

//   //   static int teste2 = 0;

//   //  //  Serial.print("r ");  // Red 
//   //   for (int j = teste2; j > 0; j++) {

//   //     sendBit(teste & (1 << j));

//   //   }

//   //   teste2++;
//   //   if(teste2 > 32){
//   //     teste2 = 0;
//   //   }


//   //  Serial.print("r ");  // Red 
//     for (int j = 8; j > 0; j--) {
//       sendBit(ledBuffer[0] & (1 << j));
//     }
//   //  //  Serial.print(" g ");  // Green
//   //   for (int j = 8; j > 0; j--) {
//   //     sendBit(ledBuffer[1] & (1 << j));
//   //   }

//   //  // Serial.print(" b ");  // Blue
//   //   for (int j = 8; j > 0; j--) {
//   //     sendBit(ledBuffer[2] & (1 << j));
//   //   } 
//   //   // // //Serial.print(" w ");  // White
//   //   for (int j = 8; j > 0; j--) {
//   //     sendBit(ledBuffer[3] & (1 << j));
//   //   }    

    
   
//   //    Serial.println();   

//   // for (uint16_t i = 0; i < numLeds; i++) {
//   //   uint16_t pixelOffset = i * 4;
//   //   uint8_t *pixelData = &ledBuffer[pixelOffset];
          
//   //   Serial.print("r ");  // Red
//   //   for (int j = 7; j > 0; j--) {
//   //     sendBit(pixelData[0] & (1 << j));
//   //   }

//   //   // Serial.print(pixelData[1]); 
//   //    Serial.print(" g ");  // Green
//   //   for (int j = 7; j > 0; j--) {
//   //     sendBit(pixelData[1] & (1 << j));
//   //   }

//   //   // Serial.print(pixelData[1]); 
//   //    Serial.print(" b ");  // Blue
//   //   for (int j = 7; j > 0; j--) {
//   //     sendBit(pixelData[2] & (1 << j));
//   //   }

//   //   // Serial.print(pixelData[1]); 
//   //    Serial.print(" w ");  // White
//   //   for (int j = 7; j > 0; j--) {
//   //     sendBit(pixelData[3] & (1 << j));
//   //   }      

//   //   // for (int j = 31; j >= 1; j--) {
//   //   //         sendBit(*pixelData & (1 << j));
//   //   //     }


//       Serial.println();
//   // }
    
//   // digitalWrite(dataPin, LOW);
//   // delayMicroseconds(80);
//   interrupts();  // Re-enable interrupts
// }

