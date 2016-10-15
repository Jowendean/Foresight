#include <Wire.h>
#include <rgb_lcd.h>

//assigns name to lcd
rgb_lcd lcd;

//sets intial backlight color
int colorR = 0;
int colorG = 0;
int colorB = 0;

int count = 1;
int currentStat;
double react = 3;
int x1 = 0; // starting point is arbitrary
double green;
double red;
double spd;
double spdDisplay;
int light;
double dist;
double nightMod = 2.0;
double duskMod = 1.5;
int iterations = 0;
int lastCheck = 0;
int timer1 = 0;
int timer2 = 0;
int timer3 = 0;
double distArray[51] = {0};
double distArrayTemp[51] = {0};

//pins assigned here
const int TP = 3;
const int EP = 2;
const int spdP = A0;
const int lightP = A2;
const int buzz = 5;
const int button = 8;


void setup() {
  pinMode(spd, INPUT); //sets potentiometer
  pinMode(A2, INPUT); //sets PVC
  pinMode(TP,OUTPUT); //sets trigger pin as output
  pinMode(EP,INPUT);  //sets echo pin as an input
  pinMode(buzz, OUTPUT); //sets buzzer as output
  pinMode(button, INPUT);

  //initializes lcd
  lcd.begin(16,2);
  lcd.setRGB(colorR,colorG,colorB);
  lcd.print("Hello");
  //SetTimer(0,0,0);
  Serial.begin(9600); //sets up serial port for communication  
  delay(500);
}

void loop() { 
  //variables for inputs
  light = analogRead(lightP);
  dist = UltrasonicDist();
  //speed in mph
  spdDisplay = (analogRead(spdP) * 100) / 1023;
  //speed in kmph
  spd = spdDisplay * 1.609344;
 //cleans out bad distance readings
  distArrayAdd(distArray, dist);
  assignArray(distArrayTemp, distArray);  
  dist = median(distArrayTemp);
//adjust stopping distance according to visual conditions
  if(light > 700){
    green = getStoppingDist(spd) * 1.2;
    red = getStoppingDist(spd) * .5;
  }else if(light > 500){
    green = (getStoppingDist(spd) * duskMod) * 1.2;
    red = getStoppingDist(spd) * duskMod * .5;
  }else{
    green = (getStoppingDist(spd) * nightMod) * 1.2;
    red = getStoppingDist(spd) * nightMod * .5;
  }

  //create variable that holds value to meet warning conditions
currentStat = warning(dist);
  
// use switch statement to create 3 cases 
// dependent on the time and currentStat
// variable to determine color and warning
// message of LCD
switch(currentStat) {
    case 1:{//green
      timer1++;
      lcd.setRGB(0,0,255);
      if(iterations % 20 == 0){
        lcd.clear();
        lcd.print(" Great Driving!!!");
      }
      if(lastCheck != 1 || iterations > 20000){
        
        iterations = 0;
        lastCheck = 1;
      }else{
        iterations++;
      }
      break;
    }
    case 2: {//yellow
      timer2++;
      lcd.setRGB(255,255,0);
      if(iterations % 20 == 0){
        lcd.clear();
        lcd.print("   Slow Down");
      }
      if(lastCheck != 2 || iterations > 20000){
        
        iterations = 0;
        lastCheck = 2;
        
      }else{
        iterations++;
      }
      break;
    }
    case 3:{ //red
      timer3++;
      lcd.setRGB(255,0,0);
      if(iterations % 20 == 0){
        lcd.clear();
        lcd.print("   STOP NOW!!!");
      }
      if(lastCheck != 3 || iterations > 20000){
        
        iterations = 0;
        lastCheck = 3;
      }else{
        iterations++;
      }
      if(UltrasonicDist() < red){
        beep(60);
      }
      break;
    }
  }
  if(digitalRead(button) == 1) {
    percent();
  }
}

//copies one array b into array a
void assignArray(double a[], double b[]){
  int i;
  for(i = 0; i < 51; i++){
    a[i] = b[i];
  }
}

// function recieves distance variable and calculates status of LED
int warning(double distance){
  if(distance >= green){
    return 1;
  }
  if(distance < green && distance > red){
    return 2;
  }
  if(distance <= red){
    return 3;
  }
}

//takes a speed in km/h and returns the stopping distance in meters
double getStoppingDist(double spd){
  double output;

  output = ((spd*spd)/177.8) + ((spd * 1000) / 3600) * react;

  return output;
  }

double UltrasonicDist () { //reads distance from ultrasonic sensor
  double duration;
  //clears trigger pin
  digitalWrite(TP, LOW);
  delayMicroseconds(2);

  //sets trigger pin on high state for 10 microseconds
  digitalWrite(TP, HIGH);
  delayMicroseconds(10);
  digitalWrite(TP, LOW);

  //reads echo pin and returns sound wave in microseconds
  duration = pulseIn(EP, HIGH);

  //calculates distance
  return duration*.034/2;
}

//shifts array to left and adds new distance input
void distArrayAdd(double distance[], double newdistance){
  
  int i;
  for(i = 0; i < 50; i++){
    distArray[i] = distArray[i + 1];
  }
  distArray[50] = newdistance;
  
}

//sorts distance array in order to find median
void mergesort(double a1[], double a2[], int left, int right) {
  int i, j, k, middle; 
  if (left < right) {
    middle = (left + right) / 2; 
    // Sort left half (using recursive call)
    mergesort(a1, a2, left, middle);
    // Sort right half (using recursive call)
    mergesort(a1, a2, middle+1, right);
    // Now merge the halves into auxiliary array
    k = i = left; j = middle+1;
    while (i <= middle && j <= right){
      if(a1[i] <= a1[j]){
        a2[k++] = a1[i++];
      }
      else{
        a2[k++] = a1[j++];
      }
    }
    // Get what still remains in left half
    while (i <= middle) a2[k++] = a1[i++];
    // Get what still remains in right half
    while (j <= right) a2[k++] = a1[j++];
    // Copy back from auxiliary array to main array
    for (i = left; i <= right; i++) a1[i] = a2[i];
  }
}  

//calls the merge sort funciton
void sort(double disA[]) {
    double auxArray[51];
    mergesort(disA, auxArray, 0, 50);
    
}
//find median of last 51 distance inputs
double median(double distance[]){
  sort(distance);
  return distance[25];
}

//sets beep rate for buzzer
void beep(unsigned char delayms) {
  analogWrite(buzz, 20);

  delay(delayms);
  analogWrite(buzz, 0);
}

// function calculates the percentage of time 
// spent in each of the categories and prints the
// results
void percent(){  
  int temp;
  int all = (timer1 + timer2 + timer3);
  lcd.setCursor(0,1);
  lcd.print("B");
  temp = 100 * timer1 / all;
  lcd.print(temp);
  lcd.print("% Y");
  temp = 100 * timer2 / all;
  lcd.print(temp);
  lcd.print("% R");
  temp = 100 * timer3 / all;
  lcd.print(temp);
  lcd.print("%");
  lcd.setCursor(0,0);
}

