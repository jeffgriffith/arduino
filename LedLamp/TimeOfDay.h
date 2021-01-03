#ifndef TIME_OF_DAY
#define TIME_OF_DAY

class TimeOfDay {
  private:
    unsigned long _refMillis = 0;
    unsigned long _fudgeFactorMillis = 0;
    static const unsigned long _millisPerMinute = 60ul * 1000ul;
    static const unsigned long _millisPerHour = 60ul * _millisPerMinute;
    static const unsigned long _millisPerDay = 24ul * _millisPerHour;

  public:
    static unsigned long hoursToMillis(int hrs) {
      return (unsigned long)hrs * _millisPerHour;
    }

    static unsigned long minutesToMillis(int minutes) {
      return (unsigned long)minutes * _millisPerMinute;
    }

  private:
    static unsigned long calculateFudgeFactor(int refHrs, int refMinutes, int refSeconds, unsigned long refMillis) {
      unsigned long relativeOffset = refMillis % _millisPerDay;
      unsigned long givenOffset = hoursToMillis(refHrs) + minutesToMillis(refMinutes) + (unsigned long)refSeconds*1000ul;
      unsigned long timeDelta = givenOffset - relativeOffset;
      // Add millisPerDay and then modulo after to avoid negative values
      return (_millisPerDay + timeDelta) % _millisPerDay;
    }
  
  public:
    TimeOfDay(unsigned long refMillis) {
      _refMillis = refMillis;
      _fudgeFactorMillis = calculateFudgeFactor(0, 0, 0, refMillis); 
    }

    void setTime(int hrs, int minutes, int seconds, unsigned long refMillis) {
      _refMillis = refMillis;
      _fudgeFactorMillis = calculateFudgeFactor(hrs, minutes, seconds, refMillis);
    }

    void setTime(int hrs, int minutes, unsigned long refMillis) {
      setTime(hrs, minutes, 0, refMillis);
    }

    unsigned long getTimeOfDay(unsigned long now) {
      return (now + _fudgeFactorMillis) % _millisPerDay;
    }

    void getTimeOfDay(int *hr, int *minute, int *secs, unsigned long now) {
      unsigned long remainder = getTimeOfDay(now);

      int hours = remainder / _millisPerHour;
      remainder -= hours * _millisPerHour;

      int minutes = remainder / _millisPerMinute;
      remainder -= minutes * _millisPerMinute;

      int seconds = remainder / 1000;

      *hr = hours;
      *minute = minutes;
      *secs = seconds;
    }

    void getTimeOfDay(int *hr, int *minute, unsigned long now) {
      int seconds;
      getTimeOfDay(hr, minute, &seconds, now);
    }

    void incrementHour(unsigned long now, int increment) {
      int hr, min, sec;
      getTimeOfDay(&hr, &min, &sec, now);

      char s[80];
      sprintf(s, "INITIAL: %02d:%02d:%02d", hr, min, sec);
      Serial.println(s);
      sprintf(s, "Adding %d hours", increment);
      Serial.println(s);
      
      int newHour = hr;
      newHour += increment;
      newHour += (newHour < 0) ? 24 : 0;
      newHour %= 24;

      sprintf(s, "SETTING WITH: %02d:%02d:%02d", newHour, min, sec);
      Serial.println(s);

      setTime(newHour, min, sec, now);
      // New value
      getTimeOfDay(&hr, &min, &sec, now);
      sprintf(s, "NEW: %02d:%02d:%02d", hr, min, sec);
      Serial.println(s);
    }

    void incrementHour(unsigned long now) {
      incrementHour(now, 1);
    }

    void incrementMinute(unsigned long now, int increment) {
      int hr, min, sec;
      getTimeOfDay(&hr, &min, &sec, now);

      int newMin = min;
      newMin += increment;
      newMin += (newMin < 0) ? 60 : 0;
      newMin %= 60;
      
      setTime(hr, newMin, sec, now);
    }
    
    void incrementMinute(unsigned long now) {
      incrementMinute(now, 1);
    }

    void incrementSecond(unsigned long now) {
      int hr, min, sec;
      getTimeOfDay(&hr, &min, &sec, now);
      setTime(hr, min, (sec + 1) % 60, now);
    }
};

#endif
