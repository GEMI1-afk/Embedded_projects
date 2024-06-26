#include <avr/io.h>
#include<avr/interrupt.h>

unsigned char hours,minutes,seconds,speed_;
bool PIR_Pressed,display_;

void seven_seg_display(void){

  PORTA = (1<<PA0);
  PORTC = (PORTC&0xF0) | ( (hours/10)&0x0F);
  _delay_ms(2);

  PORTA = (1<<PA1);
  PORTC = (PORTC&0xF0) | ( (hours%10)&0x0F);
  _delay_ms(2);

  PORTA = (1<<PA2);
  PORTC = (PORTC&0xF0) | ((minutes/10)&0x0F);
  _delay_ms(2);

  PORTA = (1<<PA3);
  PORTC = (PORTC&0xF0) | ((minutes%10)&0x0F);
  _delay_ms(2);

  PORTA = (1<<PA4);
  PORTC = (PORTC&0xF0) | ((seconds/10)&0x0F);
  _delay_ms(2);
  
  PORTA = (1<<PA5);
  PORTC = (PORTC&0xF0) | ((seconds%10)&0x0F);
  _delay_ms(2); 
}


void Timer0_PWM_init(){
  TCNT0 = 0;
  OCR0 = speed_;//INTIALY 25%
  DDRB |= (1<<PB3);
  TCCR0 = (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS01);
  SREG |= (1<<7);//GLOBAL interrupts
}

void Timer1_ctc_init(){

  TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);//CTC, 1024 PRESCALAR
  TCCR1A = 0;// NON PWM
  OCR1A = 975;
  TIMSK |= (1<<OCIE1A);// LOCAL INT CTC MODE
  SREG |= (1<<7);//GLOBAL interrupts
}

void INT0_init(){
  MCUCR |= (1 << ISC01) & ~(1 << ISC00) ;
  GICR |= (1<<INT0);
  SREG |= (1<<7);//GLOBAL interrupts
}

void INT1_init(){
   MCUCR |= (1 << ISC11) | (1 << ISC10);
   GICR |= (1<<INT1);
   SREG |= (1<<7);//GLOBAL interrupts
}

void INT2_init(){
  MCUCSR &= ~(1 << ISC2);
  GICR |= (1<<INT2);
  SREG |= (1<<7);//GLOBAL interrupts
}

ISR(INT1_vect) {
if(PIR_Pressed==0){
    PIR_Pressed = !PIR_Pressed;
    display_ = !display_;
    Timer0_PWM_init();
    Timer1_ctc_init();
    INT0_init();
    INT2_init();
  }
  else{
    PIR_Pressed = !PIR_Pressed;
    DDRB  &= ~(1<<PB0) & ~(1<<PB1);
    TCCR1B &= (~(1<<CS10)) & (~(1<<CS12));
    TCCR0 &= (~(1<<CS01));
    hours=0,minutes=0,seconds=0,speed_=68;
    PIR_Pressed=0,display_ = 0;
    display_ = !display_;
    } 
  }



ISR(TIMER1_COMPA_vect){
  seconds++;
    
    if(seconds == 60){
      seconds = 0;
      minutes++;
    }
    if(minutes == 60){
      minutes = 0;
      hours++;
    }
    if(hours == 24){
      hours = 0;
    }
}


ISR(INT0_vect){
  if (speed_ < 255) {
        speed_+=10;
        OCR0 = speed_;
    }
}


ISR(INT2_vect) {
    if (speed_ > 0) {
        speed_-=10;
        OCR0 = speed_;
    }
}

int main(){

INT1_init();

PORTA = 0x00;
DDRD &= ~(1<<PD3);
DDRB &= ~(1<<PB2) & ~(1<<PB4) & ~(1<<PB5);
DDRB |= (1<<PB0) | (1<<PB1);
PORTB |= (1<<PB0);
DDRA = 0x3F;
DDRC = 0X0F;
PORTC = 0xF0;



while(1){
  if(display_)seven_seg_display();
  else{
    PORTC &= 0x00;
    PORTA = (PORTA)& 0x00;
  }
  //PIN 5B -> STOP , PIN6B -> RESUME
  if(PINB & (1<<PB5)){
    TCCR1B &= ~(1<<CS10) & ~(1<<CS12);// NO CLK
    //int speed_2 = speed_;
    while(OCR0){
      OCR0-=50;
    }
    PORTB &~ (1<<PB0);
  }
  while(PINB & (1<<PB5));
  if(PINB &(1<<PB4)){
    TCCR1B |= (1<<CS10) | (1<<CS12);
    OCR0 = speed_;
    PORTB |= (1<<PB0);
  }
  while(PINB & (1<<PB4));

}

  
}
