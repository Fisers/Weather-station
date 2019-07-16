
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include <inttypes.h>
// http://www.nongnu.org/avr-libc/changes-1.8.html:
#define __PROG_TYPES_COMPAT__
#include <avr/pgmspace.h>
#include "ip_arp_udp_tcp.h"
#include "websrv_help_functions.h"
#include "enc28j60.h"
#include "timeout.h"
#include "net.h"
#include "dnslkup.h"
#include "../../Options.h"
#include "../RTC/rtc.h"


static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x29};
uint8_t myip[4] = {192,168,0,101};
char serverip[16] = "192.168.0.100";
#define MYWWWPORT 80
uint8_t gwip[4] = {192,168,0,1};
//
// --- there should not be any need to changes things below this line ---
#define TRANS_NUM_GWMAC 1
static uint8_t gwmac[6];
#define TRANS_NUM_WEBMAC 2
static uint8_t otherside_www_gwmac[6];
static uint8_t otherside_www_ip[4]; // will be filled by dnslkup
//
//
//
#define BUFFER_SIZE 128
static uint8_t buf[BUFFER_SIZE+1];
volatile uint8_t sec=0;
static volatile uint8_t cnt2step=0;
int8_t sendingPacket = 0;
static int8_t start_web_client = 0;
static int8_t processing_state=0;




/* setup timer T2 as an interrupt generating time base.
* You must call once sei() in the main program */
void init_cnt2(void)
{
	cnt2step=0;
	PRR&=~(1<<PRTIM2); // write power reduction register to zero
	TIMSK2=(1<<OCIE2A); // compare match on OCR2A
	TCNT2=0;  // init counter
	OCR2A=244; // value to compare against
	TCCR2A=(1<<WGM21); // do not change any output pin, clear at compare match
	// divide clock by 1024: 12.5MHz/128=12207 Hz
	TCCR2B=(1<<CS22)|(1<<CS21)|(1<<CS20); // clock divider, start counter
	// 12207/244=50Hz
}

// called when TCNT2==OCR2A
// that is in 50Hz intervals
ISR(TIMER2_COMPA_vect){
	cnt2step++;
	if (cnt2step>50){
                cnt2step=0;
                sec++; // stepped every second
	}
}

// the __attribute__((unused)) is a gcc compiler directive to avoid warnings about unsed variables.
void browserresult_callback(uint16_t webstatuscode,uint16_t datapos __attribute__((unused)), uint16_t len __attribute__((unused))){
// 	if(webstatuscode == 200) {
// 		sendingPacket = 0;
// 		start_web_client = 1;
// 	}
}

// the __attribute__((unused)) is a gcc compiler directive to avoid warnings about unsed variables.
void arpresolver_result_callback(uint8_t *ip __attribute__((unused)),uint8_t transaction_number,uint8_t *mac){
        uint8_t i=0;
        if (transaction_number==TRANS_NUM_GWMAC){
                // copy mac address over:
                while(i<6){gwmac[i]=mac[i];i++;}
        }
}




void Ether_SendPacket(char* text){
	uint16_t dat_p,plen;
	
	sendingPacket = 1;
	plen=enc28j60PacketReceive(BUFFER_SIZE, buf);
	buf[BUFFER_SIZE]='\0';
	dat_p=packetloop_arp_icmp_tcp(buf,plen);
	if(plen == 0){
		if(start_web_client == 1)
		{
			sec = 0;
			start_web_client = 0;
			
			client_tcpSend(text,&browserresult_callback,otherside_www_ip,gwmac,(uint8_t*)80);
		}
	}
	if(sec > timeout)
	{
		sendingPacket = 0;
		start_web_client = 1;
	}
	
	if(dat_p==0){ // plen!=0
		                         // check for incomming messages not processed
		                         // as part of packetloop_arp_icmp_tcp, e.g udp messages
		                         udp_client_check_for_dns_answer(buf,plen);
		                         return;
		                 }
		if (strncmp("POST / timeout",(char *)&(buf[dat_p]),14)==0){
			sscanf((char *)&(buf[dat_p]), "POST / timeout %d", &timeout);
			eeprom_write_word((uint16_t*)0x10, (uint16_t)timeout);
		}
		else if (strncmp("POST / setrtc",(char *)&(buf[dat_p]),13)==0){
			//rtc_t tempRtc;
			sscanf((char *)&(buf[dat_p]), "POST / setrtc %"SCNu8"/%"SCNu8"/%"SCNu8" %"SCNu8"/%"SCNu8"/%"SCNu8, &rtc.date, &rtc.month, &rtc.year, &rtc.hour, &rtc.min, &rtc.sec);
			RTC_SetDateTime(&rtc);
		}
}


void Ether_init()
{
	 uint8_t i;
	 uint16_t plen;

	 // Set the clock speed to "no pre-scaler" (8MHz with internal osc or
	 // full external speed)
	 // set the clock prescaler. First write CLKPCE to enable setting
	 // of clock the next four instructions.
	 // Note that the CKDIV8 Fuse determines the initial
	 // value of the CKKPS bits.
	 CLKPR=(1<<CLKPCE); // change enable
	 CLKPR=0; // "no pre-scaler"
	 _delay_loop_1(0); // 60us

	 /*initialize enc28j60*/
	 enc28j60Init(mymac);
	 enc28j60clkout(1); // change clkout from 6.25MHz to 12.5MHz
	 _delay_loop_1(0); // 60us
	 
	 init_cnt2();
	 sei();

	 /* Magjack leds configuration, see enc28j60 datasheet, page 11 */
	 // LEDB=yellow LEDA=green
	 //
	 // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
	 // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
	 enc28j60PhyWrite(PHLCON,0x476);

	 
	 //init the web server ethernet/ip layer:
	 init_udp_or_www_server(mymac,myip);
	 www_server_port(MYWWWPORT);

	 get_mac_with_arp(gwip,TRANS_NUM_GWMAC,&arpresolver_result_callback);
	 while(get_mac_with_arp_wait()){
		 // to process the ARP reply we must call the packetloop
		 plen=enc28j60PacketReceive(BUFFER_SIZE, buf);
		 packetloop_arp_icmp_tcp(buf,plen);
	 }

	 parse_ip(otherside_www_ip,serverip);
	 processing_state=2; // no need to do any dns look-up
	 
	 while(processing_state != 4)
	 {
		 if (processing_state==2){
			 if (route_via_gw(otherside_www_ip)){
				 // otherside_www_ip is behind the GW
				 i=0;
				 while(i<6){
					 otherside_www_gwmac[i]=gwmac[i];
					 i++;
				 }
				 processing_state=4;
				 }else{
				 get_mac_with_arp(otherside_www_ip,TRANS_NUM_WEBMAC,&arpresolver_result_callback);
				 processing_state=3;
			 }
			 continue;
		 }
		 if (processing_state==3 && get_mac_with_arp_wait()==0){
			 processing_state=4;
		 }
	 }
}


