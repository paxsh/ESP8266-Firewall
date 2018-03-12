#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = <Your SSID>;
const char* password = <Your password>;
const int LED_PIN = 5;

String conn_ip="";
String blocked_ip="";
int counter = 0;
unsigned long start_time = millis();
unsigned long current_time;
//End of Definition

ESP8266WebServer server(80);
WiFiServer server2(80);
//Check if header is present and correct
bool is_authentified(){
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")){
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin(){
  String msg;
  if (server.hasHeader("Cookie")){
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")){
    Serial.println("Disconnection");
    server.sendHeader("Location","/login");
    server.sendHeader("Cache-Control","no-cache");
    server.sendHeader("Set-Cookie","ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")){
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "admin" ){
      server.sendHeader("Location","/");
      server.sendHeader("Cache-Control","no-cache");
      server.sendHeader("Set-Cookie","ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
  msg = "Wrong username/password! try again.";
  Serial.println("Log in Failed");
  }
  String content = "<html><body><br> <center><font size=+4> Network Security Lab Mid Term </font></center> <br><br><form action='/login' method='POST'><br>";
  content += "<center><font size=+2>User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></font></center></form>" + msg + "<br>";
  content += "</body></html>";
  /*content += "You also can go <a href='/inline'>here</a></body></html>";
  */
  server.send(200, "text/html", content);
}

//root page can be accessed only if authentification is ok
void handleRoot(){
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()){
    server.sendHeader("Location","/login");
    server.sendHeader("Cache-Control","no-cache");
    server.send(301);
    return;
  }
  String content = "<html><body><H2><center>LED Control</center></H2><br>";
  
  if (server.hasHeader("User-Agent")){
    content += "<center><font size=+2> Welcome to the Home page <br><br>";
  }
  content += " You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  content += " <br>Turn an LED <a href=/led/1> On </a> or <a href=/led/0> Off </a> <br></font></center>";
  
  
  server.send(200, "text/html", content);
}

//no need authentification
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  String req=server.uri();
  if(req.indexOf("led/1")!=-1)
  {
      digitalWrite(LED_PIN,LOW);
      server.send(200,"text/html","<html> <br> <font size=3> <center> The led is turned on </center> </font> </html>");
  }
  else
   {
      digitalWrite(5,HIGH);
       server.send(200,"text/html","<html> <br> <font size=3>  <center> The led is turned off </center> </font> </html>");
   }
  //server.send(404, "text/plain", message);

 }


void setup(void){
  Serial.begin(9600);
  initHardware();
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/", handleRoot);
 
  server.on("/login", handleLogin);
  /*server.on("/inline", [](){
    server.send(200, "text/plain", "this works without need of authentification");
  }*/
  
  

  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  Serial.println("HTTP server started");
}

void ledfuncon()
{
  digitalWrite(LED_PIN,HIGH);
  
}

void ledfuncoff()
{
  digitalWrite(LED_PIN,LOW);
}
void loop(void){
  WiFiClient client = ESP8266WiFi::server.available();
  if (!client) {
    return;
  }
  conn_ip = client.remoteIP().toString();
  if (conn_ip!=blocked_ip)
  {
    server.handleClient();
     WiFiClient client = server2.available();
   /* server.on("/led/1",ledfuncon);
    server.on("/led/0",ledfuncoff);
  */
    counter+=1;
    if (counter >= 15 && current_time - start_time > 15 )
    { blocked_ip = conn_ip;}  
  }
  else
  { Serial.print(blocked_ip + " was blocked");
    start_time = millis();
  }
}


void initHardware()
{
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}
