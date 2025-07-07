const int FLR_BUTTON[3] = {11, 12, 13};
const int FLR_LED[3] = {8, 9, 10};
const int ELV_LED[7] = {A0, 2, 3, 4, 5, 6, 7};

int currentFloor = 0;
int targetFloor = -1;
int currentIndex = 0;  // 0(1층), 1, 2, 3(2층), 4, 5, 6(3층)
int targetIndex = -1;
int direction = 0;     // 정지: 0, 위: 1, 아래: -1
bool call[3] = {false, false, false};
bool prevButtonState[3] = {false, false, false};
bool isMoving = false;

unsigned long lastMoveTime = 0;

void setup()
{
  Serial.begin(9600);
  for (int i = 0; i < 3; i++)
  {
    pinMode(FLR_BUTTON[i], INPUT);
    pinMode(FLR_LED[i], OUTPUT);
    digitalWrite(FLR_LED[i], LOW);
  }
  for (int i = 0; i < 7; i++)
  {
    pinMode(ELV_LED[i], OUTPUT);
    digitalWrite(ELV_LED[i], LOW);
  }
  digitalWrite(ELV_LED[currentIndex], HIGH);  // 1층에서 시작
}

void loop()
{
  detectCall();

  if (!isMoving)
  {
    direction = determineDir();
    targetFloor = findTargetFloor();
    if (targetFloor != -1)
    {
      targetIndex = targetFloor * 3;
      isMoving = true;
    }
  }
  else
  {
    moveElevator();
  }
}

// 호출 감지 및 LED 제어
void detectCall()
{
  for (int i = 0; i < 3; i++)
  {
    bool currButtonState = digitalRead(FLR_BUTTON[i]);
    if (prevButtonState[i] == LOW && currButtonState == HIGH)
    {
      call[i] = !call[i];
      if (call[i])
      {
        digitalWrite(FLR_LED[i], HIGH);
      }
      else
      {
        digitalWrite(FLR_LED[i], LOW);
      }
    }
    prevButtonState[i] = currButtonState;
  }
}

// 호출 상태에 따라 방향 결정
int determineDir()
{
  for (int i = currentFloor + 1; i < 3; i++)
  {
    if (call[i])
    {
      return 1;
    }
  }
  for (int i = currentFloor - 1; i >= 0; i--)
  {
    if (call[i]) 
    {
      return -1;
    }
  }
  return 0;
}

// 방향에 따라 가장 가까운 호출층을 반환
int findTargetFloor()
{
  if (direction == 0)  // 정지 상태면 전체 순차 탐색
  {
    for (int i = 0; i < 3; i++)
    {
      if (call[i])
      {
        return i;
      }
    }
  }
  else if (direction == 1)  // 위로 이동 중
  {
    for (int i = currentFloor + 1; i < 3; i++)
    {
      if (call[i])
      {
        return i;
      }
    }
    for (int i = currentFloor - 1; i >= 0; i--)
    {
      if (!call[i])
      {
        return i;
      }
    }
  }
  else if (direction == -1)  // 아래로 이동 중
  {
    for (int i = currentFloor - 1; i >= 0; i--)
    {
      if (call[i])
      {
        return i;
      }
    }
    for (int i = currentFloor + 1; i < 3; i++)
    {
      if (!call[i])
      {
        return i;
      }
    }
  }
  return -1;
}

void moveElevator()
{
  unsigned long currentTime = millis();
  if (currentTime - lastMoveTime < 500) 
  {
    return;
  }

  // 엘베LED 한 칸 이동
  digitalWrite(ELV_LED[currentIndex], LOW);
  if (direction == 1 && currentIndex < 6)
  {
    currentIndex++;
  }    
  else if (direction == -1 && currentIndex > 0)
  {
    currentIndex--;
  }    
  digitalWrite(ELV_LED[currentIndex], HIGH);

  // 층 위치에 도달한 경우
  if (currentIndex % 3 == 0)
  {
    currentFloor = currentIndex / 3;    

    // 도착한 층에 호출이 있으면 정차
    if (call[currentFloor])
    {
      call[currentFloor] = false;
      digitalWrite(FLR_LED[currentFloor], LOW);

      direction = determineDir();
      targetFloor = findTargetFloor();
      if (targetFloor != -1)
      {
        targetIndex = targetFloor * 3;        
      }
      else
      {
        targetFloor = -1;
        targetIndex = -1;
        direction = 0;
        isMoving = false;
      }
    }

    // 호출이 취소된 경우 경로 재탐색 또는 정지
    if (targetFloor != -1 && !call[targetFloor])
    {
      direction = determineDir();
      targetFloor = findTargetFloor();
      if (targetFloor != -1)
      {
        targetIndex = targetFloor * 3;        
      }
      else
      {
        targetFloor = -1;
        targetIndex = -1;
        direction = 0;
        isMoving = false;
      }
    }
  }

  lastMoveTime = currentTime;
}
