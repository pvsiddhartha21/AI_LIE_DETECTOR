#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

const int pulsePin = A0;
int threshold = 520;  // Start with this, adjust as needed
unsigned long lastBeatTime = 0;
int bpm = 0;
bool pulseDetected = false;
int rawValue;

void setup() {
  Serial.begin(9600);
  
  // Initialize OLED - Try both addresses if one fails
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println("OLED not found");
      while(1);
    }
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Place finger on");
  display.println("sensor firmly");
  display.display();
  
  // Auto-calibrate threshold
  long sum = 0;
  for(int i=0; i<100; i++){
    sum += analogRead(pulsePin);
    delay(10);
  }
  threshold = (sum/100) * 1.2;  // Set threshold 20% above average
}

void loop() {
  rawValue = analogRead(pulsePin);
  
  // Detect heartbeat
  if(rawValue > threshold && !pulseDetected) {
    pulseDetected = true;
    
    if(lastBeatTime > 0) {
      bpm = 60000 / (millis() - lastBeatTime);  // Calculate instant BPM
      bpm = constrain(bpm, 40, 200);  // Keep within realistic range
      
      // Update OLED
      display.clearDisplay();
      display.setCursor(0,0);
      display.print("BPM: ");
      display.setTextSize(2);
      display.println(bpm);
      display.setTextSize(1);
      display.println("------------");
      display.print("Raw: ");
      display.println(rawValue);
      display.print("Thresh: ");
      display.println(threshold);
      display.display();
      
      // Send to Python
      Serial.print("BPM:");
      Serial.println(bpm);
    }
    lastBeatTime = millis();
  } 
  else if(rawValue < threshold) {
    pulseDetected = false;
  }

  // Dynamic threshold adjustment
  static unsigned long lastAdjust = millis();
  if(millis() - lastAdjust > 5000) {  // Every 5 seconds
    threshold = rawValue * 1.1;  // Adjust to 110% of current reading
    lastAdjust = millis();
  }

  delay(10);  // Small delay for stability
}