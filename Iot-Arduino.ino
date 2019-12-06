#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/*-------------WIFI------------------*/
const char *ssid =  "IoT";     // replace with your wifi ssid and wpa2 key
const char *pass =  "1t3s0IoT18";
WiFiClient client;
/*-------------VARIABLES-------------*/
//Volumen
volatile int NumPulsos; //variable para la cantidad de pulsos recibidos
int PinSensor = 2;    //Sensor conectado en el pin 2 --- NODEMCU pin D8
float factor_conversion= 7.5;//7.11; //para convertir de frecuencia a caudal
float volumen=0;
long dt=0; //variación de tiempo por cada bucle
long t0=0; //millis() del bucle anterior

// VOLUMEN FUNCTION
//---Función que se ejecuta en interrupción---------------
//se anade la parte de ICHACHE_RAM_ATTR
void ContarPulsos ()  
{ 
  NumPulsos++;  //incrementamos la variable de pulsos
} 

//---Función para obtener frecuencia de los pulsos--------
int ObtenerFrecuecia() 
{
  int frecuencia;
  NumPulsos = 0;   //Ponemos a 0 el número de pulsos
  interrupts();    //Habilitamos las interrupciones
  delay(1000);   //muestra de 1 segundo
  noInterrupts(); //Deshabilitamos  las interrupciones
  frecuencia=NumPulsos; //Hz(pulsos por segundo)
  return frecuencia;
}

//PH Sensor
const int analogInPin = 17;  // PIN NODEMCU 2
int sensorValue = 0; 
unsigned long int avgValue; 
float b;
int buf[10],temp;

//COLOR
//
// Cableado de TCS3200 a Arduino
//
#define S0 5
#define S1 4
#define S2 12
#define S3 13
#define salidaSensor 14
 
// Para guardar las frecuencias de los fotodiodos
int frecuenciaRojo = 0;
int frecuenciaVerde = 0;
int frecuenciaAzul = 0;
int colorRojo;
int colorVerde;
int colorAzul;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "mx.pool.ntp.org", 3600);

void setup() 
{ 
  Serial.begin(9600); 
  //wifi
  delay(10);
               
  Serial.println("Connecting to ");
  Serial.println(ssid); 
 
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());   
  //-------------------------------
  timeClient.begin();
  
  //pinMode(PinSensor, INPUT);
  pinMode(PinSensor,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2),ContarPulsos,RISING);
  //attachInterrupt(0,ContarPulsos,RISING);//(Interrupción 0(Pin2),función,Flanco de subida)
  //Serial.println ("Envie 'r' para restablecer el volumen a 0 Litros"); 
  t0=millis();
  //COLOR SETUP
  // Definiendo las Salidas
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  
  // Definiendo salidaSensor como entrada
  pinMode(salidaSensor, INPUT);
  
  // Definiendo la escala de frecuencia a 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
  
} 
float prom = 0.0;
void loop ()    
{ 
  /*-----------------------------------------------------------*/
  /*----------------------VOLUME-------------------------------*/
  /*-----------------------------------------------------------*/
  if (Serial.available()) {
    //if(Serial.read()=='r')volumen=0;//restablecemos el volumen si recibimos 'r'
  }
  float frecuencia=ObtenerFrecuecia(); //obtenemos la frecuencia de los pulsos en Hz
  float caudal_L_m=frecuencia/factor_conversion; //calculamos el caudal en L/m
  prom = (prom + caudal_L_m)/2;
  //prom = sqrt(prom + caudal_L_m);
  dt=millis()-t0; //calculamos la variación de tiempo
  t0=millis();
  //Serial.print ("Caudal PROMEDIO: "); 
  //Serial.print (prom,3);
  volumen=volumen+(prom/60)*(dt/1000); // volumen(L)=caudal(L/s)*tiempo(s)
  
  //volumen=volumen+(caudal_L_m/60)*(dt/1000); // volumen(L)=caudal(L/s)*tiempo(s)

   //-----Enviamos por el puerto serie---------------
//  Serial.print ("\tCaudal: "); 
//  Serial.print (caudal_L_m,3);
//  Serial.print ("L/min \tMilitros/s ");
float caudal_Ml_s= caudal_L_m / 0.06;
//  Serial.print (caudal_Ml_s,3);
//  Serial.print ("\tVolumen: "); 
//  Serial.print (volumen,3); 
//  Serial.println (" L");
  /*-----------------------------------------------*/
  /*-------------------------PH--------------------*/
  /*------------------------------------------------*/
  for(int i=0;i<10;i++) 
 { 
  buf[i]=analogRead(analogInPin);
  delay(10);
 }
 for(int i=0;i<9;i++)
 {
  for(int j=i+1;j<10;j++)
  {
   if(buf[i]>buf[j])
   {
    temp=buf[i];
    buf[i]=buf[j];
    buf[j]=temp;
   }
  }
 }
 avgValue=0;
 for(int i=2;i<8;i++)
 avgValue+=buf[i];
 float pHVol=(float)avgValue*5.0/1024/6;
 float phValue = -5.70 * pHVol + 21.34;
// Serial.print("sensor = ");
// Serial.println(phValue);

// delay(20);
 /*------------------------------------------------*/
 /*--------------------COLOR-----------------------*/
 /*------------------------------------------------*/
 // Definiendo la lectura de los fotodiodos con filtro rojo
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  
  // Leyendo la frecuencia de salida del sensor
  frecuenciaRojo = pulseIn(salidaSensor, LOW);
 
  // Mapeando el valor de la frecuencia del ROJO (RED = R) de 0 a 255
  // Usted debe colocar los valores que registro. Este es un ejemplo: 
  // colorRojo = map(frecuenciaRojo, 100, 170, 255,0);
  colorRojo = map(frecuenciaRojo, 60, 130, 255,0);
  
  // Mostrando por serie el valor para el rojo (R = Red)
//  Serial.print("R = ");
//  Serial.print(colorRojo);
  delay(100);
  
  // Definiendo la lectura de los fotodiodos con filtro verde
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  
  // Leyendo la frecuencia de salida del sensor
  frecuenciaVerde = pulseIn(salidaSensor, LOW);
 
  // Mapeando el valor de la frecuencia del VERDE (GREEN = G) de 0 a 255
  // Usted debe colocar los valores que registro. Este es un ejemplo: 
  // colorVerde = map(frecuenciaVerde, 150, 230, 255,0);
  colorVerde = map(frecuenciaVerde, 110, 190, 255,0);
 
  // Mostrando por serie el valor para el verde (G = Green)
//  Serial.print("G = ");
//  Serial.print(colorVerde);
  delay(100);
 
  // Definiendo la lectura de los fotodiodos con filtro azul
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  
  // Leyendo la frecuencia de salida del sensor
  frecuenciaAzul = pulseIn(salidaSensor, LOW);
 
  // Mapeando el valor de la frecuencia del AZUL (AZUL = B) de 0 a 255
  // Usted debe colocar los valores que registro. Este es un ejemplo: 
  // colorAzul = map(frecuenciaAzul, 38, 84, 255, 0);
  colorAzul = map(frecuenciaAzul, 18, 34 , 255, 0);
  
//  // Mostrando por serie el valor para el azul (B = Blue)
//  Serial.print("B = ");
//  Serial.print(colorAzul);
  delay(100);

  //Variables finales a enviar
  // SEND VOLUMEN: volumen
  // SEND PH: phValue
  // SEND COLOR RGB: (colorRojo, colorVerde, colorAzul)

  timeClient.update();
  int phM = abs(int(phValue)%14);
  int red = abs(int(colorRojo)%255);
  int green = abs(int(colorVerde)%255);
  int blue = abs(int(colorAzul)%255);
  String messageToSend= "{\"vol\": " + 
                        String(int(volumen)) + 
                        ", \"ph\": " + 
                        String(phM) + 
                        ", \"color\":\" "+String(red)+" , "+String(green)+" , "+ String(blue) +"\",\"patientId\":1}";
    Serial.print(messageToSend);
//  Serial.print("VolumenSEND: ");
//  Serial.print(volumen,3); 
//  Serial.print("PhSEND: ");
//  Serial.print(phValue);
//  Serial.print("\tRGBSEND: (");
//  Serial.print(colorRojo);
//  Serial.print(" , ");
//  Serial.print(colorVerde);
//  Serial.print(" , ");
//  Serial.print(colorAzul);
//  Serial.print(" )");
//  Serial.print("\n");
  delay(100);

  /*-----------------------------------------------------------*/
  /*----------------------WIFI---------------------------------*/
  /*-----------------------------------------------------------*/
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
 
   HTTPClient https;    //Declare object of class HTTPClient
 
   //https.begin("https://ptsv2.com/t/kgrzh-1574818253/post","CA DF 3B 47 61 BF 7B 58 13 5D E2 18 36 11 5E FE FF 3D 46 AD");      //Specify request destination
   //https.addHeader("Content-Type", "text/plain");  //Specify content-type header

   https.begin("https://3t7lgm2vdi.execute-api.us-east-1.amazonaws.com/dev/data","72 D4 00 92 77 37 50 C9 9B A1 38 FA 21 8A 9B FD BA CF CD 49");      //Specify request destination
   https.addHeader("Content-Type", "text/plain");  //Specify content-type header
   https.addHeader("x-api-key","BhPMOkeApxa0UKb6kcvId5iNBPmVSGLZ93dzunmN");  //Specify content-type header 
   int httpCode = https.POST(messageToSend);   //Send the request
   String payload = https.getString();                  //Get the response payload

   //Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload
   //Serial.println(https.errorToString(httpCode).c_str());
   https.end();  //Close connection
 
 }else{
 
    Serial.println("Error in WiFi connection");   
 
 }
 
  delay(120000);  //Send a request every 120 seconds
 
}