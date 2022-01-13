#include <Servo.h> //Servo header file
#include <EEPROM.h> //EEPROM header file

//Declare object for 4 Servo Motors and buttons
Servo Servo_0;
Servo Servo_1;
Servo Servo_2; 
Servo Gripper;

const int Rbutton = 2; //Record button connected to digital pin 8
const int Pbutton = 4; //Play button connected to digital pin 9

int Rflag = 0;
int Pflag = 0;
int LED = 13;

//Global Variable Declaration 
int StartRecord = 0; // variable for reading the pushbutton status
int StartPlay = 0; // variable for reading the pushbutton status

int S0_pos, S1_pos,  S2_pos, G_pos; 
int P_S0_pos, P_S1_pos, P_S2_pos, P_G_pos;
int C_S0_pos, C_S1_pos, C_S2_pos, C_G_pos;
int POT_0,POT_1,POT_2,POT_4; //POT_4 -> for the gripper

int saved_data[250]; //Array for saving recorded data
int saved_data_eeprom[250]; //Array for saving recorded data in EEPROM so that array from program is not read
                            //passed by pointer to Read function

int array_index = 0;
char incoming = 0;

int action_pos;
int action_servo;

const int STARTING_EEPROM_ADDRESS = 0;

void writeIntArrayIntoEEPROM(int address, int numbers[], int arraySize){
 
 int addressIndex = address;
 for (int i = 0; i < arraySize; i++){
   EEPROM.write(addressIndex, numbers[i] >> 8);
   EEPROM.write(addressIndex + 1, numbers[i] & 0xFF);
   addressIndex += 2; //since each integer occupies two bytes
 }
}

void readIntArrayFromEEPROM(int address, int numbers[], int arraySize)
{
 int addressIndex = address;
 for (int i = 0; i < arraySize; i++){
   numbers[i] = (EEPROM.read(addressIndex) << 8) + EEPROM.read(addressIndex + 1);
   addressIndex += 2;
 }
}

void setup() 
{
  Serial.begin(9600); //Serial Monitor for Debugging
  
  //TAKING INPUT FOR BUTTONS
  pinMode(Rbutton,INPUT);
  pinMode(Pbutton,INPUT);

  //OUTPUT LED
  pinMode(LED,OUTPUT);
  
  //Declare the pins to which the Servo Motors are connected to 
Servo_0.attach(3);
Servo_1.attach(9);
Servo_2.attach(10);
Gripper.attach(11);
  
 
}

void Read_POT() //Function to read the Analog value from POT and map it to Servo value
{
   POT_0 = analogRead(A0); 
   POT_1 = analogRead(A2); 
   POT_2 = analogRead(A3); 
   POT_4 = analogRead(A5); //Read the Analog values form all five POT
   S0_pos = map(POT_0,0,1024,10,170); //Map it for 1st Servo (Base motor)
   S1_pos = map(POT_1,0,1024,10,170); //Map it for 2nd Servo (Hip motor)
   
   S2_pos = map(POT_2,0,1024,10,170); //Map it for 3th Servo (Neck motor)
   G_pos  = map(POT_4,0,1024,10,170);  //Map it for 4th Servo (Gripper motor)
}

void Record() //Function to Record the movements of the Robotic Arm
{
  digitalWrite(13, LOW); //to show recording
  Read_POT(); //Read the POT values  for 1st time
  
  //Save the POT values in a variable initially to compare it later
     P_S0_pos = S0_pos;
     P_S1_pos = S1_pos;
     
     P_S2_pos = S2_pos;
     P_G_pos  = G_pos;
     
  Read_POT(); //Read the POT value for 2nd time
    
     if (P_S0_pos == S0_pos) //If 1st and 2nd value are same
     {
      Servo_0.write(S0_pos); //Control the 1st servo
      
      if (C_S0_pos != S0_pos) //If the POT has been turned 
      {
        saved_data[array_index] = S0_pos + 0; //Save the new position to the array. Zero is added for zeroth motor (for understanding purpose)
        array_index++; //Increase the array index 
      }
      
      C_S0_pos = S0_pos; //Saved the previous value to check if the POT has been turned 
     }
  
  //Similarly, repeat for other 3 servo Motors
     if (P_S1_pos == S1_pos)
     {
      Servo_1.write(S1_pos);
      
      if (C_S1_pos != S1_pos)
      {
        saved_data[array_index] = S1_pos + 1000; //1000 is added for 2nd servo motor as differentiator 
        array_index++;
      }
      
      C_S1_pos = S1_pos;
     }
     
  
     if (P_S2_pos == S2_pos)
     {
      Servo_2.write(S2_pos); 
      
      if (C_S2_pos != S2_pos)
      {
        saved_data[array_index] = S2_pos + 2000; //2000 is added for 3rd servo motor as differentiator 
        array_index++;
      }
      
      C_S2_pos = S2_pos;   
     }
  
     if (P_G_pos == G_pos)
     {
      Gripper.write(G_pos);
      
      if (C_G_pos != G_pos)
      {
        saved_data[array_index] = G_pos + 3000; //3000 is added for 4th servo motor (gripper) as differentiator  
        array_index++;
      }
      
      C_G_pos = G_pos;
     }
     
    delay(100); 

}

void Play() //Function to play the recorded movements on the Robotic ARM
{
  digitalWrite(13, LOW); //show a new repeat
  delay(100);
  digitalWrite(13, HIGH);
  for (int Play_action=0; Play_action<array_index; Play_action++) //Navigate through every saved element in the array 
  {
    action_servo = saved_data_eeprom[Play_action] / 1000; //The first character of the array element is split for knowing the servo number
    action_pos = saved_data_eeprom[Play_action] % 1000; //The last three characters of the array element is split to know the servo position 

    switch(action_servo){ //Check which servo motor should be controlled 
      case 0: //If zeroth(1st) motor
        Servo_0.write(action_pos);
        delay(500);
      break;

      case 1://If 1st(2nd) motor
        Servo_1.write(action_pos);
        delay(500);
      break;

      case 2://If 3rd(3rd) motor
        Servo_2.write(action_pos);
        delay(500);
      break;

      case 3://If 4th motor (gripper)
        Gripper.write(action_pos);
        delay(500);
      break;
    }

    
  }
}

void loop(){
  
    Rflag = digitalRead(Rbutton); //Read input from Record button
  
    while(Rflag == 1) //If user has selected Record mode 
    {
      Serial.println("Robotic Arm Recording Started......");
      Record();
      //Write to EEPROM (store the array of positions)
      writeIntArrayIntoEEPROM(STARTING_EEPROM_ADDRESS,saved_data,array_index);
      
      Pflag = digitalRead(Pbutton); //Read input from Play button
      
      if(Pflag == 1)
        break;
    }
 
    while(Pflag == 1) //If user has selected Play Mode 
    {
      Serial.println("Playing Recorded sequence");
    
      digitalWrite(LED,LOW); //LED HIGH
      delay(500);
      digitalWrite(LED,HIGH); //LED LOW
     readIntArrayFromEEPROM(STARTING_EEPROM_ADDRESS,saved_data_eeprom,array_index); //reads array saved in EEPROM 
      Play();      
    }
}
