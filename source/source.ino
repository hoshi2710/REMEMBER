#include <Event.h>
#include <Timer.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>


LiquidCrystal_I2C lcd(0x3F, 16, 2); //LCD 기본 설정

byte life_bar[8] = { //라이프 바 블럭 문자
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};
byte blank[8] = { //빈칸 문자
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

int btn_led_pin[] = {52,50,48,46,44,42,40,38,36};   //버튼 LED 핀 번호가 포함된 배열
int btn_input_pin[] = {53,51,49,47,45,43,41,39,37,35};    //버튼 INPUT 핀 번호가 포함된 배열

Timer update_life_bar_Timer;
Timer main_game_Timer;
Timer alert_buzzer_Timer;

////////////////////벨런스 조절은 여기 하슈///////////////////
int life = 10; //생명바
int score = 0;//점수를 담는 변수
int _round_ = 1;//게임 라운드
float score_x = 1.0; //점수 가산 배수 (한번이라도 오답 발생시 다시 1.0으로 초기화)
float add_score_x = 0.1; //정답을 맞추는 콤보가 올라갈때마다 추가할 점수가산배수
int random_button_input_getsu = 5; //메모리 게임시 랜덤으로 누르게 할 버튼 갯수

int correct_life_up = 3; //정답을 맞췄을때 추가할 생명바 갯수(최대 10)
int wrong_down_life = 2; //오답일때 감소시킬 생명바 갯수(최대10)
int add_random_button_getsu = 1; //stageup_round변수에서 설정한 라운드마다 문제의 갯수를 몇개씩 추가할지 설정함
int stageup_round = 5;//몇 라운드마다 문제출제 갯수를 늘릴지 설정함
int delay_time = 300;//문제 출제시 문제마다의 지연 간격을 설정함

boolean easteregg_mode_trigger_1 = false; //이스터에그(피아노모드)로 진입하기위한 트리거 1
boolean easteregg_mode_trigger_2 = false; //이스터에그(피아노모드)로 진입하기위한 트리거 2
boolean easteregg_mode_trigger_3 = false; //이스터에그(피아노모드)로 진입하기위한 트리거 3
int easteregg_mode_code_1[] = {2,5,5,7,3};//이스터에그 접속코드 1
int easteregg_mode_code_2[] = {3,7,2,7,6};//이스터에그 접속코드 2

boolean easteregg_mode_on = false;
//////////////////////////////////////////////////////////


void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));
  // I2C LCD를 초기화 합니다..
  lcd.init();
  // I2C LCD의 백라이트를 켜줍니다.
  lcd.backlight();

  main_intro();

  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print(" Life ");
  lcd.setCursor(0, 1);
  lcd.print("Score  ");
  lcd.setCursor(6,1);
  lcd.print(String(score));
  lcd.createChar(1,life_bar); //LCD 한칸을 꽉채우는 생명바 문자
  lcd.createChar(0,blank); //빈칸으로 채우기 위한 문자

  for(int i=0; i<sizeof(btn_input_pin); i++)
  {
    pinMode(btn_input_pin[i],INPUT_PULLUP);
  }
  set_life();
  update_life_bar_Timer.every(2000,life_refresh);
  main_game_Timer.every(10,main_game);
  alert_buzzer_Timer.every(10,life_bar_alert_sound);
}




boolean main_game_timer_sw = false;
void loop() {
  if(main_game_timer_sw)
    main_game_Timer.update();
}

void set_life() //생명바를 새로고침 하는 함수 파라미터 int remain은 남은 생명의 값이 입력되게 됨
{
  lcd.setCursor(6,0); //생명바가 있는곳으로 커서 이동
  for(int i=0; i<10; i++) //생명바를 모두지우고 생명바가 업데이트 될 수 있는 공간을 만듬
  {
    lcd.write(byte(0));  //빈칸문자 입력
  }
  lcd.setCursor(6,0); //생명바가 있는곳으로 커서 이동
  for(int i=0; i<life; i++) //생명바를 현재 남은 생명값만큼 채우기
  {
    lcd.write(byte(1)); //체력바를 입력
  }
  delay(100);
}

void life_refresh()
{
  if(!easteregg_mode_on)
  {
    if(life != 0) 
    {
       life--;
       set_life();
    }
    else
    {
      game_over();
    }
  }
}
void main_intro()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Remember");
  lcd.setCursor(0,1);
  lcd.print("Press to Start");
   for(int i=0; i<sizeof(btn_led_pin)+1; i++)
    {
      pinMode(btn_led_pin[i],OUTPUT);
    }
     for(int j=0; j<sizeof(btn_led_pin); j++)
    {
      digitalWrite(btn_led_pin[j],HIGH); 
    }
  int a = input_btn_main_intro();
  for(int j=0; j<sizeof(btn_led_pin); j++)
    {
      digitalWrite(btn_led_pin[j],LOW); 
    }
  if(a == 4)
  {
    easteregg_mode_trigger_1 = true; //이스터에그 첫번째 트리거 활성화
    Serial.println("easteregg_mode_trigger 1 ON!");
  }
    
  main_game_timer_sw = true;
}

void refresh_score()
{
  lcd.setCursor(6,1);
  lcd.print(String(score));
}

void game_over()    //게임 오버시 작동할 함수
{
  main_game_timer_sw = false;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GAME OVER");
  lcd.setCursor(0,1);
  lcd.print("Score : " + String(score));
  owari();
  int a = input_btn_main_intro();
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  
}

void correct_sign_led()  //정답일때 LED 불빛 점멸
{
  int correct_led_pin[] = {52,50,48,46,42,40,38,36};
  tone(3,1046,100);
  delay(100);
  tone(3,1046,500);
  for(int x=0; x<2; x++)
  {
    for(int i=0; i<5; i++)
    {
      pinMode(correct_led_pin[i],OUTPUT);
    }
    for(int j=0; j<sizeof(correct_led_pin); j++)
    {
      digitalWrite(correct_led_pin[j],HIGH); 
    }
    delay(500);
    for(int j=0; j<sizeof(correct_led_pin); j++)
    {
      digitalWrite(correct_led_pin[j],LOW); 
    }
    delay(500);
  }  
}
void wrong_sign_led() //오답일때 LED불 빛 점멸
{
  int wrong_led_pin[] = {52,48,44,40,36};
  tone(3,174,100);
  delay(200);
  tone(3,174,100);
   for(int i=0; i<5; i++)
    {
      pinMode(wrong_led_pin[i],OUTPUT);
    }
  for(int x=0; x<2; x++)
  {

    for(int j=0; j<5; j++)
    {
      digitalWrite(wrong_led_pin[j],HIGH); 
    }
    delay(500);
    for(int j=0; j<5; j++)
    {
      digitalWrite(wrong_led_pin[j],LOW); 
    }
    delay(500);
  }  
}


int round_pushed_btn = 0;// 한 라운드당 눌린 버튼

int main_game() //버튼 입력 관리
{


  ////////////////문제 출제 부분////////////////
  if(int(_round_%stageup_round) == 0)
  {
    random_button_input_getsu+=add_random_button_getsu;
  }
    
    
int correct_answer[random_button_input_getsu];//정답이 들어있는 배열
  for(int j=0; j<random_button_input_getsu; j++) //랜덤한 값을 입력함
  {
    correct_answer[j] = random(0,9);
    pinMode(btn_led_pin[correct_answer[j]],OUTPUT);
    delay(delay_time);
    digitalWrite(btn_led_pin[correct_answer[j]],HIGH);
    Serial.println(String(correct_answer[j]));
    delay(delay_time);
    digitalWrite(btn_led_pin[correct_answer[j]],LOW);
   
  }
  
 
  ///////////////////////////////////////////

  ////////////////정답 입력 부분////////////////
  int user_answer[random_button_input_getsu];
//  delay(200);
  for(int a=0; a<random_button_input_getsu; a++)
  {
      user_answer[a] = input_btn();
      Serial.println(user_answer[a]);
  }
  Serial.println("==============================");
  boolean iscorrect = true;
  int dontgiveup = 0;
  int rememberus = 0;
  for(int a=0; a<random_button_input_getsu; a++)
  {
    if(user_answer[a] != correct_answer[a])
    {
      iscorrect = false;
      break;
    }
  }
  for(int a=0; a<random_button_input_getsu; a++)
  {
    if(user_answer[a] == easteregg_mode_code_1[a] && _round_ == 1) 
    {
      dontgiveup++;
    }
    if(user_answer[a] == easteregg_mode_code_2[a] && _round_ == 2)
    {
      rememberus++;
    }
  }

  if(dontgiveup == 5 && _round_ == 1)
  {
    easteregg_mode_trigger_2 = true;  //이스터에그 두번째 트리거 활성화
    Serial.println("easteregg_mode_trigger 2 ON!");
  }
  if(rememberus == 5 && _round_ == 2)
  {
    easteregg_mode_trigger_3 = true;  //이스터에그 세번째 트리거 활성화
    Serial.println("easteregg_mode_trigger 3 ON!");
    if(easteregg_mode_trigger_1 == true && easteregg_mode_trigger_2 == true && easteregg_mode_trigger_3 == true)
    {
      main_game_timer_sw = false;
      easteregg_mode_on = true;
      easteregg_mode_function();
    }
  }
  if(iscorrect) //정답인가요?
  {
  //정다다다다아압
    correct_sign_led();
    score += int(100*score_x);
    score_x += add_score_x;
    if(life==10)
    {
      life = 10;
    }
    else
    {
      life += correct_life_up; 
    }
    set_life();
    refresh_score();
  }
  else
  {
    //오답....ㅠㅠ
    wrong_sign_led();
    life -= wrong_down_life;
    score_x = 1.0;
    set_life();
    refresh_score();
    if (life <= 0)
    {
      game_over();
    }
  }
  _round_++; //다음 라운드
  
  ////////////////////////////////////////////
}

int input_btn_main_intro()  //메인 화면에서 버튼 입력 받기
{
  boolean ok = false;
  while(!ok)
  {
    for(int i=0; i<sizeof(btn_input_pin); i++)
    {
      pinMode(btn_input_pin[i],INPUT_PULLUP);
    }
    if (digitalRead(btn_input_pin[0]) == LOW)
    {
      delay(300);
      tone(3,523,100);
      ok = true;
      return 0;
    }
   else if (digitalRead(btn_input_pin[1]) == LOW)
    {
      delay(200);
      tone(3,587,100);
      ok = true;
     return 1;
    }
   else if (digitalRead(btn_input_pin[2]) == LOW)
    {
      delay(200);
      tone(3,659,100);
      ok = true;
     return 2;
    }
   else if (digitalRead(btn_input_pin[3]) == LOW)
    {
      delay(200);
      tone(3,698,100);
      ok = true;
     return 3;
    }
   else if (digitalRead(btn_input_pin[4]) == LOW)
    {
      delay(200);
      tone(3,783,100);
      ok = true;
     return 4;
    }
    else if (digitalRead(btn_input_pin[5]) == LOW)
    {
      delay(200);
      tone(3,880,100);
      ok = true;
     return 5;
    }
    else if (digitalRead(btn_input_pin[6]) == LOW)
    {
      delay(200);
      tone(3,987,100);
      ok = true;
     return 6;
    }
    else if (digitalRead(btn_input_pin[7]) == LOW)
    {
      delay(200);
      tone(3,1046,100);
      ok = true;
     return 7;
    }
    else if (digitalRead(btn_input_pin[8]) == LOW)
    {
      delay(200);
      tone(3,1174,100);
      ok = true;
     return 8;
    }
  }
    
}
unsigned long time_now = 0;
void caution1()
{
  
//  delay(2000);

if(millis() >= time_now + 2000){
        time_now += 2000;
        tone(2,488,100);
        Serial.println("caution1");
    }
    
}

void caution2()
{
  if(millis() >= time_now + 1000){
        time_now += 1000;
        tone(2,550,100);
    }
}
void emergency()
{
  if(millis() >= time_now + 200){
        time_now += 200;
      tone(2,976,100);
  }
//  delay(200);
}
void owari()
{
  tone(2,976,5000);
}

void life_bar_alert_sound()
{
  if(!easteregg_mode_on)
  {
    if(life<=2)
    {
      emergency();
    }
    else if(life<=4)
    {
      caution2();
    }
    else if(life<=7)
    {
      caution1();
    }
  }
}

void easteregg_mode_function()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Don't Give Up");
  lcd.setCursor(0,1);
  lcd.print("Just Do IT!");
  while(true)
  {
    int a = input_btn_main_piano();  
  }
}

int input_btn()//버튼 입력 검사
{
  while(round_pushed_btn < random_button_input_getsu)
  {
    for(int i=0; i<sizeof(btn_input_pin); i++)
    {
      pinMode(btn_input_pin[i],INPUT_PULLUP);
    }
    update_life_bar_Timer.update();
    if(life <=7)
    {
      life_bar_alert_sound();
    }
    
//    alert_buzzer_Timer.update();
    if (digitalRead(btn_input_pin[0]) == LOW)
    {
      delay(300);
      tone(3,523,100);
    return 0;
    }
   else if (digitalRead(btn_input_pin[1]) == LOW)
    {
      delay(200);
      tone(3,587,100);
     return 1;
    }
   else if (digitalRead(btn_input_pin[2]) == LOW)
    {
      delay(200);
      tone(3,659,100);
     return 2;
    }
   else if (digitalRead(btn_input_pin[3]) == LOW)
    {
      delay(200);
      tone(3,698,100);
     return 3;
    }
   else if (digitalRead(btn_input_pin[4]) == LOW)
    {
      delay(200);
      tone(3,783,100);
     return 4;
    }
    else if (digitalRead(btn_input_pin[5]) == LOW)
    {
      delay(200);
      tone(3,880,100);
     return 5;
    }
    else if (digitalRead(btn_input_pin[6]) == LOW)
    {
      delay(200);
      tone(3,987,100);
     return 6;
    }
    else if (digitalRead(btn_input_pin[7]) == LOW)
    {
      delay(200);
      tone(3,1046,100);
     return 7;
    }
    else if (digitalRead(btn_input_pin[8]) == LOW)
    {
      delay(200);
      tone(3,1174,100);
     return 8;
    }
  }
}

int input_btn_main_piano()
{
  boolean ok = false;
  while(!ok)
  {
    for(int i=0; i<sizeof(btn_input_pin); i++)
    {
      pinMode(btn_input_pin[i],INPUT_PULLUP);
    }
    for(int i=0; i<sizeof(btn_led_pin); i++)
    {
      pinMode(btn_led_pin[i],OUTPUT);
    }
    if (digitalRead(btn_input_pin[0]) == LOW)
    {
      delay(150);
      tone(3,523,100);
      digitalWrite(btn_led_pin[0],HIGH);
      delay(200);
      digitalWrite(btn_led_pin[0],LOW);
      ok = true;
      return 0;
    }
   else if (digitalRead(btn_input_pin[1]) == LOW)
    {
      delay(150);
      tone(3,587,100);
      digitalWrite(btn_led_pin[1],HIGH);
      delay(200);
      digitalWrite(btn_led_pin[1],LOW);
      ok = true;
     return 1;
    }
   else if (digitalRead(btn_input_pin[2]) == LOW)
    {
      delay(150);
      tone(3,659,100);
      digitalWrite(btn_led_pin[2],HIGH);
      delay(200);
      digitalWrite(btn_led_pin[2],LOW);
      ok = true;
     return 2;
    }
   else if (digitalRead(btn_input_pin[3]) == LOW)
    {
      delay(150);
      tone(3,698,100);
      digitalWrite(btn_led_pin[3],HIGH);
      delay(200);
      digitalWrite(btn_led_pin[3],LOW);
      ok = true;
     return 3;
    }
   else if (digitalRead(btn_input_pin[4]) == LOW)
    {
      delay(150);
      tone(3,783,100);
      digitalWrite(btn_led_pin[4],HIGH);
      delay(200);
      digitalWrite(btn_led_pin[4],LOW);
      ok = true;
     return 4;
    }
    else if (digitalRead(btn_input_pin[5]) == LOW)
    {
      delay(150);
      tone(3,880,100);
      digitalWrite(btn_led_pin[5],HIGH);
      delay(200);
      digitalWrite(btn_led_pin[5],LOW);
      ok = true;
     return 5;
    }
    else if (digitalRead(btn_input_pin[6]) == LOW)
    {
      delay(150);
      tone(3,987,100);
      digitalWrite(btn_led_pin[6],HIGH);
      delay(200);
      digitalWrite(btn_led_pin[6],LOW);
      ok = true;
     return 6;
    }
    else if (digitalRead(btn_input_pin[7]) == LOW)
    {
      delay(150);
      tone(3,1046,100);
      digitalWrite(btn_led_pin[7],HIGH);
      delay(200);
      digitalWrite(btn_led_pin[7],LOW);
      ok = true;
     return 7;
    }
    else if (digitalRead(btn_input_pin[8]) == LOW)
    {
      delay(150);
      tone(3,1174,100);
      digitalWrite(btn_led_pin[8],HIGH);
      delay(200);
      digitalWrite(btn_led_pin[8],LOW);
      ok = true;
     return 8;
    }
  }
    
}
