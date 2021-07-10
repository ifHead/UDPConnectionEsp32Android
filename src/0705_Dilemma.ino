#include <MsTimer2.h>
#include <Arduino.h>
#include "pin.h"
#include "Gvar.h"
#include "Link.h"
#include "CommThread.h"

#include <avr/interrupt.h>
#include<avr/io.h>

#include "HardwareSerial.h"
#include "com_input.h"
#include "com_output.h"

#define DUMMYJUMP 16
#define DUMMYSTUCK 17

int timer_cnt = 0;
int timer_scale = 0;
char str[100];

unsigned long before_millis = 0;
 
CommThread_Running thread_manager(false);
CommThread comwrite;
CommThread_mini com_1_th;

ISR(USART1_RX_vect){
    //char rx = UDR1;
    thread_manager.recive( UDR1 );
}


void TimerISR(){ //50ms 마다 실행 (20fps)
    
  if(++timer.scale >= 5){
      if(++timer.cnt == 2){
          
          set_bit(LED_SYSTEM_DDR, LED_SYSTEM_PIN);
          timer.cnt = 0;
      }else{
        clr_bit(LED_SYSTEM_DDR, LED_SYSTEM_PIN);
      }
    timer.scale = 0;
  }
  //--------------------------
    timer.en = true;
    
}


// ------------------------

void setup() {
  pin_init();
  serial_init();

  delay(1500);
  sprintf(str, "ISC_MAIN_v%d.%d Start..." , PROGRAM_VER / 10 , PROGRAM_VER % 10);   DEBUG.println(str);
  
  delay(1500);
  sprintf(str, "DEVICE Scan start!!" );   DEBUG.println(str);

  start_up_device();
  device_writer_schedule(); //여기서 장치 값 넣음

  MsTimer2::set(50, TimerISR);
  MsTimer2::start();

  delay(4000);

  
  before_millis = millis();
  comwrite.idx_reset();
}

//보내고 수신이 하든 안하든 false out이나서 false 출력함.
//Thread[0] 은 무조건 input 요청임


void loop() {
    if(before_millis + 100 <= millis() ){
        get_internal_data(); //내부핀 읽어오기 ><
        thread_manager.command_raw_loop(); //앞으로 이친구가 관리할것 입니다 하하.
        read_schedule();
        before_millis = millis(); //update
    }

    if(esp32_jump == true){
        comwrite.jump(); //점프해랑 ><
        esp32_jump = false;
    }

   device_writer_schedule(); //여기서 장치 값 넣음
   commend_esp32_recive(); //esp32 에서 값 읽음

    //샘플코드
    // comwrite.wait(p1, 131900); p1++;
    // comwrite.com_sound(p1, 1, D_MUSIC_BGM + 1, 1,1,1,1); p1++;
    // comwrite.finish(p1, false);
    // comwrite.com_relay_raw(p1, 1, D_RELAY_SET_1, S_RELAY_OFF, read_data_list[15],1,1,1); p1++;
                             //아래의 앤퍼센드는 스레드 특징임
    // com_12_th.com_motor_raw(0, &comwrite.dev.motor_raw[1], D_MOTOR_SET_4, S_MOTOR_OPEN, read_data_list[12], 1,1,1);
    // com_12_th.finish(1, false);


   // A   /  로비  중국 비디오  /   꺼지면 
    int p1 = 0;
    ///*점프더미*/ comwrite.com_relay_raw(p1, 2, D_RELAY_SET_7, S_RELAY_ON, read_data_list[DUMMYJUMP],1,1,1); p1++; // 벽조명 켜짐 

    // esp 32 받는 코드는 link 파일에 있고
    // esp 32 보내는 코드는 output에 있다.
    // 안드로이드에서 보내는 스트링은 #ISC,1,1,1,1, 마지막에 쉽표 붙여야 함.
    
    comwrite.wait(p1, 30000); p1++;
    /*원본코드*/comwrite.com_relay_raw(p1, 2, D_RELAY_SET_7, S_RELAY_ON, read_data_list[0],1,1,1); p1++; // 벽조명 켜짐 
/* 1 */comwrite.com_sound_ch_raw(p1, 1, 0b00000001, 1, 1, 1, 1); p1++; // 라운지 스피커 켜기
/* 2 */comwrite.com_sound(p1, 1, D_MUSIC_BGM + 1, 1,1,1,1); p1++; // BGM1 시작 

    // B   /   휴대폰 상자의 자석스위치 닫아서 / LOW
    /*점프더미*/ comwrite.com_relay_raw(p1, 1, D_RELAY_SET_1, S_RELAY_ON, read_data_list[DUMMYJUMP],1,1,1); p1++;
/* 3 */comwrite.com_relay_raw(p1, 1, D_RELAY_SET_1, S_RELAY_ON, read_data_list[1],1,1,1); p1++; // 이엠락 1번 잠김 
/* 4 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_8, S_RELAY_ON, 1,1,1,1);  p1++; // 라운지 홀조명 두개 켜짐
/* 5 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_7, S_RELAY_OFF, 1,1,1,1);  p1++; // 라운지 벽조명 꺼짐
/* 6 */comwrite.com_sound(p1, 1, D_MUSIC_EVENT + 2, 1,1,1,1);  p1++; // 라운지 스피커에서 따르릉 네~입니다 소리 들림 --------- !!!!!!!!!!!!!!!!이벤트 코드 필요!!!!!!!!!!!!!!!!
/* 7 */comwrite.com_sound(p1, 1, D_MUSIC_EVENT + 2, 1,1,1,1);  p1++; // 라운지 스피커에서 따르릉 네~입니다 소리 들림 --------- !!!!!!!!!!!!!!!!이벤트 코드 필요!!!!!!!!!!!!!!!!
/* 8 */comwrite.wait(p1, 2000); p1++; // 이벤트 기다리기
/* 9 */comwrite.com_relay_raw(p1, 1, D_RELAY_SET_2, S_RELAY_OFF, 1,1,1,1);  p1++; // 멘트 끝나면, 엔틱 이엠락 2 열림 ((((놓침))))  

    // C   /   가정용 기성품 스위치를 켜면 / ????
    //comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_1, S_MOTOR_OPEN, read_data_list[DUMMYJUMP],1,1,1);  p1++;// 강가문 열린다.
/* 10 */comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_1, S_MOTOR_OPEN, read_data_list[2],1,1,1);  p1++;// 강가문 열린다.
/* 11 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_8, S_RELAY_OFF, 1,1,1,1); p1++;// 라운지 홀조명 두 개 오프
/* 12 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_2, S_RELAY_ON, 1,1,1,1); p1++;// 강가 태양 조명 ON
/* 13 */comwrite.com_sound(p1, 1, D_MUSIC_BGM + 2, 1,1,1,1); p1++;// 강가 스피커에서 나올 BGM2 켜기
/* 14 */comwrite.com_sound_ch_raw(p1, 1, 0b00010000, 1, 1, 1, 1); p1++;// 라운지 스피커 오프 & 강가 스피커 ON 
    

    //태블릿 있음

    // D   /   강가자물쇠 풀고 기성품 스위치 켜기 / ???? 
    //comwrite.com_sound(p1, 1, D_MUSIC_BGM + 1, read_data_list[DUMMYJUMP],1,1,1); p1++;//bgm은 1번으로 
/* 15 */comwrite.com_sound(p1, 1, D_MUSIC_BGM + 1, read_data_list[3],1,1,1); p1++;//bgm은 1번으로     
/* 16 */comwrite.com_sound_ch_raw(p1, 1, 0b00000001, 1,1,1,1); p1++;// C의 강가 스피커는 끄고 라운지 스피커는 켜기
/* 17 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_2, S_RELAY_OFF/*OFF로 바꿀 것!!!!*/, 1,1,1,1); p1++; // C의 강가 태양조명 꺼짐 !!!!!!!@#!@#!@#!# 5/29 공사용으로 항상 켜짐
/* 18 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_8, S_RELAY_ON, 1,1,1,1); p1++; // C의 라운지 홀조명 두 개 켜짐
/* 19 */comwrite.com_relay_raw(p1, 1, D_RELAY_SET_1, S_RELAY_OFF, 1,1,1,1); p1++; // B의 이엠1번이 열린다.
/* 20 */comwrite.wait(p1, 10000); p1++; // 10초 딜레이 
/* 21 */comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_1, S_MOTOR_CLOSE/*CLOSE로 바꿀 것!!!!*/, 1,1,1,1); p1++; // 강가 문 닫힘 !!!!!!!@#!@#!@#!# 5/29 공사용으로 항상 열림
 
    /////// E (삭제된 시퀀스임)

    // F   /   라운지 중국 비디오 꺼지면 / LOW
    //comwrite.com_relay_raw(p1, 2, D_RELAY_SET_5, S_RELAY_ON, read_data_list[DUMMYJUMP],1,1,1); p1++; //라운지 LED바 (초기값)OFF -> ON
/* 22 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_5, S_RELAY_ON, read_data_list[0],1,1,1); p1++; //라운지 LED바 (초기값)OFF -> ON
/* 23 */comwrite.com_sound(p1, 1, D_MUSIC_BGM + 3, 1,1,1,1); p1++; //라운지 스피커 음악 변경 BGM1이 -> BGM3으로 변경
/* 24 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_6, S_RELAY_ON, 1,1,1,1); p1++;//라운지 가장 안쪽 원탁등 (초기값 OFF)->켜짐
/* 25 */comwrite.com_relay_raw(p1, 1, D_RELAY_SET_3, S_RELAY_OFF, 1,1,1,1); p1++; //원탁등 왼쪽에 있는 이엠락 3번이 (초기값 닫힘) -> /*  */
    ////// G (중국시퀀스이므로 생략)

    // H   /   중국 IC5구 / LOW
    //comwrite.com_relay_raw(p1, 2, D_RELAY_SET_6, S_RELAY_OFF, read_data_list[DUMMYJUMP],1,1,1); p1++; // 원탁등 꺼짐
/* 26 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_6, S_RELAY_OFF, read_data_list[6],1,1,1); p1++; // 원탁등 꺼짐
/* 27 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_8, S_RELAY_OFF, 1,1,1,1); p1++; // C의 라운지 홀조명 두 개 꺼짐
/* 28 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_5, S_RELAY_OFF, 1,1,1,1); p1++; // F2에서 켜진 LED바가 뚝 꺼진다.
/* 29 */comwrite.com_sound_ch_raw(p1, 1, 0b00000000, 1,1,1,1); p1++;// 라운지 BGM3스피커 꺼짐
    // 중국 비디오가 중국 비디오 스위치에 의해 재생된다.  
    //comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_2, S_MOTOR_OPEN, read_data_list[DUMMYJUMP],1,1,1); p1++;// ((##162번줄의 레거시코드임##))중국 영상 끝나면, 2차 HIGH -> LOW 받아서, 라운지 대형 액추에이터 축소되어있던게 확장된다. 
/* 30 */comwrite.wait(p1, 130000); p1++; //중국영상 나오는 동안 기다림
/* 31 */comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_2, S_MOTOR_OPEN, 1,1,1,1); p1++;// 중국 영상 중간쯤에, 라운지 대형 액추에이터 캡슐방 열어줌
/* 32 */comwrite.com_sound(p1, 1, D_MUSIC_BGM + 4, 1,1,1,1); p1++; // BGM4 준비
/* 33 */comwrite.com_sound_ch_raw(p1, 1, 0b00000010, 1,1,1,1); p1++;// 캡슐호텔방 천장 스피커 BGM4번 켜짐
/* 34 */comwrite.wait(p1, 30000); p1++; 
/* 35 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_8, S_RELAY_ON, 1,1,1,1); p1++; // C의 라운지 홀조명 두 개 켜짐


    ///// I (중국시퀀스이므로 생략)

    // J   /   말풍선 서랍 자석스위치 떨어지면 / HIGH
    //comwrite.com_relay_raw(p1, 2, D_RELAY_SET_1, S_RELAY_ON, read_data_list[DUMMYSTUCK],1,1,1); p1++; // 더미
/* 36 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_1, S_RELAY_ON, read_data_list[7],1,1,1); p1++; // 더미
    // 장비가 태블릿에 신호를 준다.    ---------------------<<----------------------------------<<-----------------------------------태블릿--------------<<------------------------<<---------------------
    // 캡슐방 태블릿 (초기값)대기화면이 검정색임 -> 켜짐
    comwrite.com_esp32(p1, 1/*1 캡슐 태블릿*/, 1/*활성화 해라*/, 1/*조건*/,1,1,1); p1++;

    // K   /   중국 시계 맞추면 / LOW
    // comwrite.com_relay_raw(p1, 1, D_RELAY_SET_4, S_RELAY_OFF, read_data_list[DUMMYJUMP],1,1,1); p1++;// 캡슐호텔 네 칸 중 1층 우측 칸, 서랍에 이엠락 닫혀있다가 열림.
/* 37 */comwrite.com_relay_raw(p1, 1, D_RELAY_SET_4, S_RELAY_OFF, read_data_list[8],1,1,1); p1++;// 캡슐호텔 네 칸 중 1층 우측 칸, 서랍에 이엠락 닫혀있다가 열림.
/* 38 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_OFF, 1,1,1,1); p1++; // 캡슐호텔 LED 1개 (초기값 켜져있던 거) 꺼짐, 테마 입구와 가까운 쪽만 꺼짐. 

    // L   /   골목 좌측 입구 자석스위치 열리면 / HIGH
    // 골목 좌측 입구의 자석스위치가 초기값은 닫혀있음.
    // 아날로그 퍼즐을 풀면 자석스위치가 열리면,
    // 자석스위치 닫히면, ((정리하면, 자석스위치 HIGH -> LOW -> HIGH ))
/*39  */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_1, S_RELAY_ON, read_data_list[15],1,1,1); p1++; // 좌측 입구 닫힘(초기값) (릴레이2-1은 더미)
    // comwrite.com_relay_raw(p1, 2, D_RELAY_SET_1, S_RELAY_ON, read_data_list[DUMMYJUMP],1,1,1); p1++;// 좌측 입구 스위치 열림이면 (더미)
/* 40 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_1, S_RELAY_ON, read_data_list[9],1,1,1); p1++;// 좌측 입구 스위치 열림이면 (더미)
/* 41 */comwrite.com_relay_raw(p1, 1, D_RELAY_SET_5, S_RELAY_ON, 1,1,1,1); p1++;// 좌측 입구 이엠 닫고
/* 42 */comwrite.com_sound_ch_raw(p1, 1, 0b00000100, read_data_list[15],1,1,1); p1++;//좌측 입구 다시 닫히면, 캡슐의 스피커 꺼지고 펍스피커 켜짐
/* 43 */comwrite.com_sound(p1, 1, D_MUSIC_BGM + 5, 1,1,1,1); p1++; //펍의 스피커에서 BGM5이 나옴

    ////// 중국시퀀스 혹은 생략 M,N,O

    // O `   /   펍 상자 안 기성품스위치 / ????
    // 앱에서 나온 힌트로, 상자 퍼즐을 풀어서, 그 안에 들어있던 기성품 스위치를 OFF->ON
    // 펍 태블릿이 신호를 받는다.
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!상자 안 기성품 스위치가 장비에 연결되지 않은 것 같음!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //---------------------<<----------------------------------<<-----------------------------------태블릿--------------<<------------------------<<-------------------------------<<-------------------
    comwrite.com_esp32(p1, 2/*C 펍 태블릿*/, /*무엇을 보낼지*/1, 1/*조건*/,1,1,1); p1++;
    
    // P   /   골목 창문 열면 (오류있음 무조건 실행)
    // comwrite.com_relay_raw(p1, 2, D_RELAY_SET_1, S_RELAY_OFF, read_data_list[DUMMYJUMP],1,1,1); p1++; // 더미
/* 44 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_1, S_RELAY_OFF, read_data_list[14],1,1,1); p1++; // 더미
/* 45 */comwrite.com_sound_ch_raw(p1, 1, 0b000001010, 1,1,1,1); p1++; // 펍 스피커 끄고, 캡슐방스피커와 골목쪽스피커 bgm 6
/* 46 */comwrite.com_sound(p1, 1, D_MUSIC_BGM + 6, 1,1,1,1); p1++; // bgm 6번 켜기 //

    // Q   /   골목 IC5구 / LOW // 오류있음: LOW든 HIGH든 상관없이 실행됨
/* 47 */comwrite.com_relay_raw(p1, 1, D_RELAY_SET_6, S_RELAY_OFF, read_data_list[11],1,1,1); p1++; // 골목 우측 입구 이엠락7번 (초기값닫혀있던게) 열림

    // R   /   캡슐 2층 우측 IC3구 / HIGH뜨면 // 오류있음 : 뽑았다 끼웠다 해줘야 실행됨 LOW든 HIGH든
/* 48 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_OFF, read_data_list[10],1,1,1); p1++; // 캡슐 LED바 테마 입구에서 먼 쪽 1개 꺼짐.
/* 49 */comwrite.wait(p1, 20000); p1++; // 딜레이 20초
/* 50 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_ON, 1,1,1,1); p1++;// 캡슐 LED바 2개가 깜빡 깜빡 
/* 51 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_ON, 1,1,1,1); p1++;
/* 52 */comwrite.wait(p1, 1600);p1++;
/* 53 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_OFF, 1,1,1,1); p1++;
/* 54 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_OFF, 1,1,1,1); p1++;
/* 55 */comwrite.wait(p1, 1600);p1++;
/* 56 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_ON, 1,1,1,1); p1++;
/* 57 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_ON, 1,1,1,1); p1++;
/* 58 */comwrite.wait(p1, 1600);p1++;
/* 59 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_OFF, 1,1,1,1); p1++;
/* 60 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_OFF, 1,1,1,1); p1++;
/* 61 */comwrite.wait(p1, 1600);p1++;
/* 62 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_ON, 1,1,1,1); p1++;// 캡슐 LED바 2개가 깜빡 깜빡 
/* 63 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_ON, 1,1,1,1); p1++;
/* 64 */comwrite.wait(p1, 1600);p1++;
/* 65 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_OFF, 1,1,1,1); p1++;
/* 66 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_OFF, 1,1,1,1); p1++;
/* 67 */comwrite.wait(p1, 1600);p1++;
/* 68 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_ON, 1,1,1,1); p1++;
/* 69 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_ON, 1,1,1,1); p1++;
/* 70 */comwrite.wait(p1, 1600);p1++;
/* 71 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_OFF, 1,1,1,1); p1++;
/* 72 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_OFF, 1,1,1,1); p1++;
/* 73 */comwrite.wait(p1, 1600);p1++;
/* 74 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_ON, 1,1,1,1); p1++;
/* 75 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_ON, 1,1,1,1); p1++;
/* 76 */comwrite.wait(p1, 1600);p1++;
/* 77 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_OFF, 1,1,1,1); p1++;
/* 78 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_OFF, 1,1,1,1); p1++; 
    // 키패드 여기임
    ////// S  / 로비 비디오센서 2차가 신호를 주면 작동(문제없음)
/* 79 */comwrite.com_relay_raw(p1, 1, D_RELAY_SET_7, S_RELAY_OFF, read_data_list[0],1,1,1); p1++; // 2층 좌측 캡슐방의 서랍 em 열기
    // comwrite.com_relay_raw(p1, 1, D_RELAY_SET_7, S_RELAY_OFF, read_data_list[DUMMYJUMP],1,1,1); p1++; // 2층 좌측 캡슐방의 서랍 em 열기
/* 80 */comwrite.com_sound_ch_raw(p1, 1, 0b00000000, 1,1,1,1); p1++; //캡슐 BGM6번을 끄는 의미로 스피커를 전부 끈다. 
    // T   /   캡슐 2층 좌측 방에 있는 가정용 스위치 / ????
/* 81 */comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_2, S_MOTOR_CLOSE, read_data_list[12],1,1,1); p1++; // 캡슐의 중국 영상 끝나서 LOW 받으면 대형 액추에이터 펼쳐진 것이 -> 접힌다.
    // comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_2, S_MOTOR_CLOSE, read_data_list[DUMMYJUMP],1,1,1); p1++; // 캡슐의 중국 영상 끝나서 LOW 받으면 대형 액추에이터 펼쳐진 것이 -> 접힌다.
/* 82 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_2, S_RELAY_ON, 1,1,1,1); p1++; // 강가 태양조명&강가 계단 조명 켜짐
/* 83 */comwrite.com_sound(p1, 1, D_MUSIC_BGM + 2, 1,1,1,1); p1++;// bgm2 재생
/* 84 */comwrite.com_sound_ch_raw(p1, 1, 0b00010000, 1,1,1,1); p1++;// 강가스피커를 켠다.
/* 85 */comwrite.wait(p1, 5000); p1++;// 딜레이 5초 (액추에이터 기다리는 시간)
    
    

    // U   /   강가 2층 벽면에 레버가 정답이 되고 영상이 플레이된 후 LOW가 뜨면
    // comwrite.com_sound_ch_raw(p1, 1, 0b00000000, read_data_list[13],1,1,1); p1++;// 강가 스피커 BGM2꺼짐
/* 86 */comwrite.com_sound_ch_raw(p1, 1, 0b00000000, read_data_list[DUMMYJUMP],1,1,1); p1++;// 강가 스피커 BGM2꺼짐
/* 87 */comwrite.com_relay_raw(p1, 2, D_RELAY_SET_2, S_RELAY_OFF, 1,1,1,1); p1++; // 강가 태양조명&계단조명 꺼짐

    //END
/* 88 */comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_2, S_MOTOR_OPEN, read_data_list[4],1,1,1); p1++;// 테마 입구 오른쪽 벽면에 있는 스위치로, 대형 액추에이터를 접는다
/* 89 */comwrite.finish(p1, false);

    int p2 = 0;
    com_1_th.com_motor_raw(p2, &comwrite.dev.motor_raw[1], D_MOTOR_SET_1, S_MOTOR_OPEN, read_data_list[4], 1,1,1); p2++;

  //---------아래는 샘플코드---------------------------------

    //comwrite.com_relay_raw(0, 1, D_RELAY_SET_1, S_RELAY_OFF, read_data_list[15],1,1,1);
    //comwrite.com_relay_raw(1, 1, D_RELAY_SET_2, S_RELAY_OFF, read_data_list[0],1,1,1);
    // comwrite.com_sound_ch_raw(0, 1, 0b10101111, 1, 1, 1, 1);
    // comwrite.com_sound(1, 1, D_MUSIC_BGM + 1, 1,1,1,1); 
    // comwrite.wait(2, 2500);
    // comwrite.com_sound_ch_raw(3, 1, 0b00000011, 1, 1, 1, 1);
    // comwrite.com_sound(4, 1, D_MUSIC_EVENT + 2, 1,1,1,1); 
    // comwrite.wait(5, 4500);
    // comwrite.com_sound_ch_raw(6, 1, 0b00001111, 1, 1, 1, 1);

    // comwrite.finish(7, false);


    //comwrite.com_sound(4, 1, D_MUSIC_BGM + 2, 1,1,1,1); 
    //comwrite.com_relay_pwm_all(5, &comwrite.dev.relay_pwm_all[1], MODE_DIMMIG, 1, read_data_list[4],1,1,1); 
  //_------------------------------------------------------------------
}

void device_writer_schedule(){
  //장비 목록 : 인풋 1개, 릴레이 2개, 모터 1개, 사운드 1개 
    thread_manager.add_raw_schedule(0, ISC_INPUT, 1, I_INPUT_RAW, 1); // 인풋보드 1개
    
    thread_manager.add_raw_schedule(1, ISC_RELAY, 1, W_RELAY_RAW, comwrite.dev.relay_raw[1] ); // 릴레이보드 2개
    thread_manager.add_raw_schedule(2, ISC_RELAY, 2, W_RELAY_RAW, comwrite.dev.relay_raw[2] );
    
    thread_manager.add_raw_schedule(3, ISC_AUDIO, 1, W_SOUND_BGM, comwrite.dev.sound_mp3[1]); // 오디오 1개
    thread_manager.add_raw_schedule(4, ISC_AUDIO, 1, W_SOUND_SET_CH, comwrite.dev.sound_ch[1] );
    
    thread_manager.add_raw_schedule(5, ISC_MOTOR, 1, W_MOTOR_RAW, comwrite.dev.motor_raw[1] ); // 모터 1개
    
    load_input(0, thread_manager.get_parm1(0), thread_manager.get_parm2(0) ); //인풋보드 1개
    //thread_manager.add_raw_schedule(0, ISC_INPUT, 1, I_INPUT_RAW, 1); // 인풋보드가 한 개 이므로.

    //thread_manager.add_raw_schedule(1, ISC_RELAY, 1, W_PWM_CH_ALL, comwrite.dev.relay_pwm_all[1] );
    //thread_manager.add_raw_schedule(2, ISC_RELAY, 1, W_RELAY_RAW, comwrite.dev.relay_raw[1] );
    //thread_manager.add_raw_schedule(0, ISC_AUDIO, 1, W_SOUND_BGM, comwrite.dev.sound_mp3[1] );
    //thread_manager.add_raw_schedule(1, ISC_AUDIO, 1, W_SOUND_SET_CH, comwrite.dev.sound_ch[1] ); //채널

    //thread_manager.add_raw_schedule(7, ISC_VIDEO, 1, W_VIDEO_PLAY, comwrite.dev.video_play[1] );
    //thread_manager.add_raw_schedule(8, ISC_MOTOR, 1, W_MOTOR_RAW, comwrite.dev.motor_raw[1] );
        
    //load_input(0, thread_manager.get_parm1(0), thread_manager.get_parm2(0) ); //외부 인풋1
}

void read_schedule(){
    read_data_list[0] = get_input_Internal(GET_PORT_1, IN_LOW); // 1번포트 : 로비모니터 / 영상끝나면 LOW 발생
    read_data_list[1] = get_input_Internal(GET_PORT_2, IN_LOW); // 2번포트 : 폰상자자석스위치 / LOW (상자 닫히면)
    read_data_list[2] = get_input_Internal(GET_PORT_3, IN_HIGH); // 3번포트 : 강가문 여는 스위치 / HIGH
    read_data_list[3] = get_input_Internal(GET_PORT_4, IN_HIGH); // 4번포트 : 강가방에 있는 스위치 / HIGH // 의심중!! 5/29 high-> low로 바꿈
    read_data_list[4] = get_input_Internal(GET_PORT_5, IN_HIGH); // 5번포트 : 대형액추에이터 탈출용 & 강가방 탈출용/ HIGH
    read_data_list[5] = get_input_Internal(GET_PORT_6, IN_HIGH); // 6번포트 : 펍 상자속 스위치 / HIGH // 중국용이라 안 씀!!!!!!!
    read_data_list[6] = get_input_Internal(GET_PORT_7, IN_LOW); // 7번포트 : 원탁 중국 IC 5구 / LOW
    read_data_list[7] = get_input_Internal(GET_PORT_8, IN_HIGH); // 8번포트 : 말풍선 서랍 자석스위치 / HIGH
    read_data_list[8] = get_input_Internal(GET_PORT_9, IN_LOW); // 9번포트 : 중국 시계 / LOW
    read_data_list[9] = get_input_Internal(GET_PORT_10, IN_HIGH); // 10번포트 : 골목 좌측 입구의 자석스위치 / HIGH
                                    //인풋보드의(load_input) idx다
    read_data_list[10] = get_input_ext(0, GET_PORT_1, IN_LOW); // 캡슐 2층 우측 IC 3구 2차 / LOW // 중국장비의 그라운드와 isc그라운드의 통일성 오류였음
    read_data_list[11] = get_input_ext(0, GET_PORT_2, IN_LOW); // 골목 IC 5구 2차 / LOW
    read_data_list[12] = get_input_ext(0, GET_PORT_3, IN_LOW); // 캡슐 2층 좌측방 토글 2차 / LOW
    read_data_list[13] = get_input_ext(0, GET_PORT_4, IN_LOW); // 강가 레버 비디오 2차 / LOW
    read_data_list[14] = get_input_ext(0, GET_PORT_5, IN_HIGH); // 골목 창문 자석스위치 / 열리면 HIGH

    read_data_list[15] = get_input_Internal(GET_PORT_10, IN_LOW); // ([9]의 반대 골목좌측입구 스위치 닫힘상태

    // read_data_list[16] = get_esp32_req(1); // 상수는 내맴 (0~50)
    // read_data_list[17] = get_esp32_req(2);

    /*무조건 뚫리는 더미인풋*/read_data_list[DUMMYJUMP] = get_input_Internal(GET_PORT_12, IN_LOW); //무조건 뚫리는 더미인풋
    /*무조건 막히는 더미인풋*/read_data_list[DUMMYSTUCK] = get_input_Internal(GET_PORT_12, IN_HIGH); //무조건 뚫리는 더미인풋


  //----------- 아래는 log 추적용
  // sprintf(str, "<in0> %d %d %d %d %d %d %d %d", read_data_list[0], read_data_list[1], read_data_list[2], read_data_list[3],
  // read_data_list[4], read_data_list[5], read_data_list[6], read_data_list[7]  ); DEBUG.println(str);

  // sprintf(str, "<in8> %d %d %d %d %d %d %d %d %d ", read_data_list[8], read_data_list[9], read_data_list[10], read_data_list[11],
  // read_data_list[12], read_data_list[13], read_data_list[14], read_data_list[15] , read_data_list[16]   ); DEBUG.println(str);

  // sprintf(str, "<key> %d %d %d %d S%d ", read_data_list[17], read_data_list[18], read_data_list[19], read_data_list[20] , read_data_list[21] ); DEBUG.println(str);
}

void start_up_device(){ //---------------------------5/29 초기 세팅 모두 확인 완료

  DEBUG.println("RESET....");
  int p1 = 0;
  // comwrite.com_relay_all(p1, 1, S_RELAY_ON, 1,1,1,1); p1++; // 공사용 코드
  // comwrite.com_relay_all(p1, 2, S_RELAY_ON, 1,1,1,1); p1++; // 공사용 코드
  comwrite.com_relay_raw(p1, 1, D_RELAY_SET_1, S_RELAY_OFF, 1,1,1,1); p1++; // r1-1 이엠락 1번 폰상자 / 초기값 LOW
  comwrite.com_relay_raw(p1, 1, D_RELAY_SET_2, S_RELAY_ON, 1,1,1,1); p1++; // r1-2 이엠락 2번 앤틱 위에서 1번째 / HIGH
  comwrite.com_relay_raw(p1, 1, D_RELAY_SET_3, S_RELAY_ON, 1,1,1,1); p1++; // r1-3 이엠락 3번 원탁등 아치서랍 / HIGH
  comwrite.com_relay_raw(p1, 1, D_RELAY_SET_4, S_RELAY_ON, 1,1,1,1); p1++; // r1-4 이엠락 4번 캡슐 1층 우측칸 서랍 / HIGH
  comwrite.com_relay_raw(p1, 1, D_RELAY_SET_5, S_RELAY_OFF, 1,1,1,1); p1++; // r1-5 이엠락 5번 골목 좌측 입구 / LOW
  comwrite.com_relay_raw(p1, 1, D_RELAY_SET_6, S_RELAY_ON, 1,1,1,1); p1++; // r1-6 이엠락 6번 골목 우측 입구 / HIGH
  comwrite.com_relay_raw(p1, 1, D_RELAY_SET_7, S_RELAY_ON, 1,1,1,1); p1++; // r1-7 이엠락 7번 캡슐 2층 좌측 / HIGH
  comwrite.com_relay_raw(p1, 2, D_RELAY_SET_2, S_RELAY_OFF, 1,1,1,1); p1++; // r2-2 강가 태양조명 / LOW
  comwrite.com_relay_raw(p1, 2, D_RELAY_SET_3, S_RELAY_ON, 1,1,1,1); p1++; // r2-3 캡슐방 led바2번 / HIGH
  comwrite.com_relay_raw(p1, 2, D_RELAY_SET_4, S_RELAY_ON, 1,1,1,1); p1++; // r2-4 캡슐방 led바1번 / HIGH
  comwrite.com_relay_raw(p1, 2, D_RELAY_SET_5, S_RELAY_OFF, 1,1,1,1); p1++; // r2-5 라운지 LED바 / LOW
  comwrite.com_relay_raw(p1, 2, D_RELAY_SET_6, S_RELAY_OFF, 1,1,1,1); p1++; // r2-6 라운지 가장 안쪽 원탁등 / LOW
  comwrite.com_relay_raw(p1, 2, D_RELAY_SET_7, S_RELAY_OFF, 1,1,1,1); p1++; // r2-7 라운지 벽조명 / LOW
  comwrite.com_relay_raw(p1, 2, D_RELAY_SET_8, S_RELAY_OFF, 1,1,1,1); p1++; // r2-8 라운지홀조명 두 개 / LOW
  
  comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_1, S_MOTOR_CLOSE, 1,1,1,1); p1++; // m1-1 강가문 액추 / LONG
  comwrite.com_motor_raw(p1, 1, D_MOTOR_SET_2, S_MOTOR_CLOSE, 1,1,1,1); p1++; // m1-2 라운지 대형 액추 / SHORT

  comwrite.com_sound_ch_raw(p1, 1, 0b00000000, 1,1,1,1); p1++; // s1-1 라운지 스피커 / OFF (0b00 = 고정 | 뒤에 6칸은 포트다.)
                                                              // 이벤트를 특정한 스피커에서 재생하고 싶으면 다른 곳은 다 꺼둬야 한다.
  comwrite.finish(p1, false);
  // s1-2 캡슐방 천장 스피커 / OFF
  // s1-3 펍 스피커 / OFF
  // s1-4 골목 스피커 / OFF
  // s1-5 강가 스피커 / OFF

  // 아래는 샘플 코드
  //comwrite.com_relay_raw(p1, 3, D_RELAY_SET_8, S_RELAY_OFF, 1,1,1,1); p1++; 
  //comwrite.com_relay_raw(p1, 3, D_RELAY_SET_3, S_RELAY_ON, 1,1,1,1); p1++; // 길거리로 나가는 em락 닫아두
  //comwrite.com_video(p1, 1, COM_VIDEO_NOLOOP, 0, 1,1,1,1); p1++;

  DEBUG.println("RESET OK!");
  delay(1000);
}