#include <LiquidCrystal.h>
#include <ManageDisplay.h>

LiquidCrystal lcd(50, 51, 48, 46, 42, 40);

ManageDisplay::ManageDisplay(){
  lcd.begin(16,2);
  lcd.createChar(0, batteryFull);
  lcd.createChar(1, batteryThreeQuarters);
  lcd.createChar(2, batteryHalf);
  lcd.createChar(3, batteryOneQuarter);
  lcd.createChar(4, batteryTenPercent);
  lcd.createChar(5, batteryEmpty);
  lcd.createChar(6, batteryError);
}

//Queues the Next Message to be displayed
void ManageDisplay::updateMessage(String m){
  message.push(m);
}

//Returns the next message in the queue
String ManageDisplay::getNextMessage(){
  return message.pop();
}

//Checks if the queue is empty
bool ManageDisplay::messageEmpty(){
  return message.isEmpty();
}

//Writes the battery percentage to the screen
void ManageDisplay::writeBatteryPercentage(int batteryCharge){
  //shows battery power.
  if ( batteryCharge >= 90 && batteryCharge <=100) {
    //Display LCD character for FULL battery.
    lcd.setCursor(15,0);                                   // Battery symbol will display in top right corner.
    lcd.write(byte(0));
    lcd.noCursor();
    updateMessage("Battery Charged.");
    }

  else if (batteryCharge < 90 && batteryCharge >=75) {
      //Display LCD character for FULL-1 battery.
      lcd.setCursor(15,0);                                   // Battery symbol will display in top right corner.
      lcd.write(byte(1));
      lcd.noCursor();
    }

  else if (batteryCharge <75 && batteryCharge >= 50) {
      // Display 66% Battery Symbol
      lcd.setCursor(15,0);                                   // Battery symbol will display in top right corner.
      lcd.write(byte(2));
      lcd.noCursor();
    }

  else if (batteryCharge <50 && batteryCharge >= 25) {
        //Display 50% Battery Symbol
        lcd.setCursor(15,0);                                   // Battery symbol will display in top right corner.
        lcd.write(byte(3));
        lcd.noCursor();
      }

  else if (batteryCharge <25 && batteryCharge > 10) {
        //Display 10% Battery Symbol
        lcd.setCursor(15,0);                                   // Battery symbol will display in top right corner.
        lcd.write(byte(4));
        lcd.noCursor();
      }

  else if (batteryCharge <=10 && batteryCharge >0) {
    // Display Empty battery Symbol
      // Display/Send Error message saying 'critically low battery level
      lcd.setCursor(15,0);                                   // Battery symbol will display in top right corner.
      lcd.write(byte(5));
      lcd.noCursor();
    updateMessage("Warning: Low Battery.");
  }
  else{
        lcd.setCursor(15,0);                                   // Battery symbol will display in top right corner.
        lcd.write(byte(6));
        lcd.noCursor();
        updateMessage("Error: Invalid format for Battery Percentage.");

    // NOTE: If Scrolling issue cannot be resolved, create Table of Errors
    //(i.e. Error 1: Invalid Entry, Error 2: No connection, etc.)
  }
}

//Writes the message to the screen
void ManageDisplay::writeMessageToDisplay(){
  if (!messageEmpty()){
    lcd.setCursor(0,1);
    lcd.print("               ");
    String m = getNextMessage();
    lcd.setCursor(0,1);
    delay(1200);
    int ScrollStart = 0;
    int ScrollEnd = 15;
    int scroll = m.length();
    if (scroll < 16)  // Change to Less than or equal to ?
      lcd.print(m);
    else{
      for(int j=0;j<(scroll-14);j++){
        lcd.setCursor(0,1);
        lcd.print(m.substring(ScrollStart,ScrollEnd));
        ScrollStart++;
        ScrollEnd++;
        delay(700);
        // This should display the string characters from 0 to 15,
        //then each loop, shift one character to the right until the end.
      }
      lcd.setCursor(0,1);
      lcd.print(m.substring(0,15));
    }
  }
}

void ManageDisplay::updateDisplay(String time){
  lcd.setCursor(0,0);
  lcd.print(time);
  writeMessageToDisplay();
}
