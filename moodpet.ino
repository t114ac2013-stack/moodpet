const int RledPin = 9;
const int GledPin = 10;
const int BledPin = 11;
const int buttonPin = 2;

// 如果你的 RGB LED 是「0 亮、255 滅」，改成 true
const bool commonAnode = false;

int mood = 10;
const int neutralMood = 10;

int lastReading = LOW;
int buttonState = LOW;

unsigned long debounceTimer = 0;
const unsigned long debounceInterval = 30;

unsigned long touchedTimer = 0;
unsigned long reducedTimer = 0;
const unsigned long unTouchInterval = 5000;
const unsigned long reducedInterval = 1000;

// 燈光效果
int previousMode = -1;
unsigned long effectStart = 0;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
}

void loop() {
  unsigned long now = millis();

  // 按鈕防彈跳，避免按一下被算成很多次
  int reading = digitalRead(buttonPin);

  if (reading != lastReading) {
    debounceTimer = now;
  }
  lastReading = reading;

  if (now - debounceTimer >= debounceInterval) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        mood = min(mood + 1, 20);

        touchedTimer = now;
        reducedTimer = now;
      }
    }
  }

  // 5 秒沒有按下後開始扣分，之後每秒扣 1 分
  if (now - touchedTimer >= unTouchInterval &&
      now - reducedTimer >= reducedInterval) {
    mood = max(mood - 1, 0);
    reducedTimer = now;
  }

  showLEDState(mood, now);
}

// 這裡統一使用：0 = 不亮，255 = 最亮
void setColor(int red, int green, int blue) {
  if (commonAnode) {
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  }

  analogWrite(RledPin, red);
  analogWrite(GledPin, green);
  analogWrite(BledPin, blue);
}

void showLEDState(int state, unsigned long now) {
  // 0：普通，1：開心，2：生氣
  int mode = 0;

  if (state > 15) {
    mode = 1;
  }
  else if (state < 5) {
    mode = 2;
  }

  // 切換心情區間時，重新開始燈光效果
  if (mode != previousMode) {
    previousMode = mode;
    effectStart = now;
  }

  unsigned long elapsed = now - effectStart;

  if (mode == 1) {
    // 開心：每 2 秒完成一次漸亮、漸暗
    unsigned long phase = elapsed % 2000;
    int brightness;

    if (phase < 1000) {
      brightness = map(phase, 0, 1000, 20, 255);
    }
    else {
      brightness = map(phase, 1000, 2000, 255, 20);
    }

    // 保留原本開心時，綠色往藍色變化的配色
    int blue = map(state, 10, 20, 0, 255);
    int green = 255 - blue;

    setColor(
      0,
      (long)green * brightness / 255,
      (long)blue * brightness / 255
    );
  }
  else if (mode == 2) {
    // 生氣：紅燈亮 250 毫秒、滅 250 毫秒
    if (elapsed % 500 < 250) {
      setColor(255, 0, 0);
    }
    else {
      setColor(0, 0, 0);
    }
  }
  else {
    // 普通：維持原本的心情顏色
    if (state >= neutralMood) {
      int blue = map(state, 10, 20, 0, 255);
      setColor(0, 255 - blue, blue);
    }
    else {
      int red = map(state, 0, 10, 255, 0);
      setColor(red, 255 - red, 0);
    }
  }
}