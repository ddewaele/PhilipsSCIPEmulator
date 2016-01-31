void setup() {
  Serial.begin(9600);
}

const bool DEBUG = 0;

byte cmdBuffer[100];
byte msgBuffer[50];

bool cmdFound = false;
int i = 0;

byte requests[][10] = {  
                        {0x06,0x01,0x00,0xA2,0x00,0xA5},    // get SCIP platform version
                        {0x05,0x01,0x00,0x19,0x1D},         // get powerstate
                        {0x05,0x01,0x00,0xAD,0xA9},         // get inputsource
                        {0x05,0x01,0x00,0x2F,0x2B},         // get temperature
                        {0x05,0x01,0x00,0x62,0x66},         // get fan speed
                        {0x06,0x01,0x00,0x18,0x02,0x1D},    // set power on
                        {0x06,0x01,0x00,0x18,0x01,0x1E},    // set power off
                        {0x06,0x01,0x00,0x0F,0x02,0x0A}     // get operating hours
                    };


byte responses[][10] = {
                        {0x09,0x01,0x01,0xA2,0x31,0x2E,0x38,0x38,0xB4},
                        {0x06,0x01,0x01,0x19,0x02,0x1D},
                        {0x09,0x01,0x01,0xAD,0x06,0x00,0x01,0x00,0xA3},
                        {0x06,0x01,0x01,0x2F,0x21,0x08},
                        {0x06,0x01,0x01,0x62,0x04,0x60},
                        {0x06,0x01,0x01,0x00,0x06,0x00},
                        {0x06,0x01,0x01,0x00,0x06,0x00},                        
                        {0x07,0x01,0x00,0x0F,0x4D,0x00,0x44}
                     };
  
  


int msgLength = 0;

void loop() 
{
   
    cmdBuffer[i] = '\0'; // Null terminate the string
    
    
  while(Serial.available() > 0) {
    char c = Serial.read();
    cmdBuffer[i] = c;     // Store the current character
    if (DEBUG) { debugChar(i,c);  }
    if (DEBUG) { printBuffer(cmdBuffer,10); Serial.println(""); }
    
    if (i!=0 && c==0x01) {
        msgLength = cmdBuffer[i-1];
        if (DEBUG) { Serial.print("FOUND CTRL MSG WITH LENGTH = "); Serial.println(msgLength); }  // if a msg is found with length 5, we are at i=1 and need to continue to i=5-1
        msgBuffer[0]=msgLength;
        msgBuffer[1]=0x01;
        msgBuffer[2] = '\0';
    } else if (i>0) {
      if (DEBUG) { Serial.print("Storing "); printHex(c); Serial.print(" into msgBuffer as element "); Serial.println(i); }
      msgBuffer[i]=c;
      msgBuffer[i+1] = '\0';
    }
    
    if (i==msgLength-1) {
        byte expectedChecksum = getCheckSum(msgBuffer,msgLength);

        if ((byte)c==(byte)expectedChecksum) {
           if (DEBUG) { Serial.println("checksum OK"); printBuffer(msgBuffer,msgLength);  }
          sendCorrectResponse(msgBuffer,msgLength);
          i=-1;     
        } else {
           if (DEBUG) { Serial.print("checksum NOK : expecting "); printHex(expectedChecksum); Serial.print(" but was "); printHex(c); Serial.println(""); }
           i=-1;     
        }
    } 

    i++;
    
  }
}

void sendCorrectResponse(byte* msgBuffer,int msgLength) {

  int result = -1;
  
  for (int i = 0; i < sizeof(requests) ; i++) {
      if (array_cmp(requests[i], msgBuffer, msgLength, msgLength)==true) {
        result = i;
        break;
      }
  }
  
  if (result != -1) {
    Serial.write(responses[result],responses[result][0]);  // how to get the real size of the array ? got lucky here because size is encoded as first byte.
  }
  
}


void debugChar(int i, char c) {
    Serial.print(" [");
    Serial.print(i);
    Serial.print(". ");
    printHex(c);
    Serial.print("] ");
    i++;
}

void printHex(byte c) {
    char tmp[16];
    sprintf(tmp, "0x%.2X",c); 
    Serial.print(tmp);    
} 

void printBuffer(byte* buffer,int size) {
   Serial.print("CMD BUFFER = [");
   for (int n=0;n<size;n++) {   // why is the sizeof(buffer) not working here ?
     printHex(buffer[n]);
    Serial.print(" ");    
   }
   Serial.println("]");
   
}

//
// check if 2 arrays are the same
//
boolean array_cmp(byte *a, byte *b, int len_a, int len_b){
     int n;

     if (len_a != len_b) return false;

     for (n=0;n<len_a;n++) if (a[n]!=b[n]) return false;

     return true;
}


//
// Calculates the checksum of the byte array with the provided length.
// The last byte of the message is the checksum byte, so it needs to be excluded from the check. (msgLength-1)
// Need to investigate why the dynamic sizing (malloc) didn't work here.
//
char getCheckSum(byte *string,int msgLength)
{
  char XOR = 0;	
  if (DEBUG) { Serial.print("CHECKSUM MSG LENGTH = "); Serial.println(sizeof(string)); }
  for (int i = 0; i < msgLength-1; i++) 
  {
    if (DEBUG) { Serial.print("CHECKSUM BYTE : "); printHex(string[i]); Serial.println(""); }
    XOR = XOR ^ string[i];
  }
  return XOR;
}

