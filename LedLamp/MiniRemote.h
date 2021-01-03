#include <IRremote.h>

class MiniRemote {

  private:
    IRrecv *_irrecv;
    uint32_t _previousResult = 0;
    int _flashPin;

  public:
    MiniRemote(int irPin, int flashPin) {
      _irrecv = new IRrecv(irPin);
      _irrecv->enableIRIn();
      _flashPin = flashPin;
    }

    void init() {
      _irrecv->enableIRIn();
    }
    
    const static uint32_t  CH = 0xFF629D;
    const static uint32_t  CH_MINUS = 0xFFA25Du;
    const static uint32_t  CH_PLUS = 0xFFE21Du;
    const static uint32_t  MINUS = 0xFFE01Fu;
    const static uint32_t  PLUS = 0xFFA857u;
    const static uint32_t  EQ = 0xFF906F;
    const static uint32_t  BTN_0 = 0xFF6897u;
    const static uint32_t  BTN_1 = 0xFF30CFu;
    const static uint32_t  BTN_2 = 0xFF18E7u;
    const static uint32_t  BTN_3 = 0xFF7A85u;
    const static uint32_t  BTN_4 = 0xFF10EFu;
    const static uint32_t  BTN_5 = 0xFF38C7u;
    const static uint32_t  BTN_6 = 0xFF5AA5u;
    const static uint32_t  BTN_7 = 0xFF42BDu;
    const static uint32_t  BTN_8 = 0xFF4AB5u;
    const static uint32_t  BTN_9 = 0xFF52ADu;

    void handleRepeatIR(decode_results *pResults) {
      if (pResults->value == 0xffffffffu)
        pResults->value = _previousResult;
      _previousResult = pResults->value;
    }

    void flashOnSuccess() {
      if (_flashPin != -1) {
        int oldPinVal = digitalRead(_flashPin);
        digitalWrite(_flashPin, HIGH);
        delay(100);
        digitalWrite(_flashPin, oldPinVal);
      }
    }

    bool readInput(char buttonName[], int *buttonVal) {
      decode_results results;
      bool success = true;

      if (!_irrecv->decode(&results)) {
        success = false;
      }
      else {

        handleRepeatIR(&results);
  
        if (results.decode_type == 9) {
          switch(results.value) {
            case CH:
              strcpy(buttonName, "CH");
              break;
            case CH_PLUS:
              strcpy(buttonName, "CH+");
              break;
            case CH_MINUS:
              strcpy(buttonName, "CH-");
              break;
            case PLUS:
              strcpy(buttonName, "PLUS");
              break;
            case MINUS:
              strcpy(buttonName, "MINUS");
              break;
            case EQ:
              strcpy(buttonName, "EQ");
              break;
            case BTN_0:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 0;
              break;
            case BTN_1:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 1;
              break;
            case BTN_2:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 2;
              break;
            case BTN_3:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 3;
              break;
            case BTN_4:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 4;
              break;
            case BTN_5:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 5;
              break;
            case BTN_6:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 6;
              break;
            case BTN_7:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 7;
              break;
            case BTN_8:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 8;
              break;
            case BTN_9:
              strcpy(buttonName, "DIGIT");
              *buttonVal = 9;
              break;
            default:
              Serial.print("Unknown value: ");
              Serial.println(results.value, HEX);
              strcpy(buttonName, "UNK_VAL");
              success = false;
          }
        }
        else {
          Serial.print("Unknown decode_type: ");
          Serial.println(results.decode_type);
          success = false; // Not a recognized result type.
        }

        if (success)
          flashOnSuccess();

        delay(50); // Debounce long presses
          
        _irrecv->resume();
      }
      return success;
    }
};
