#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include <inttypes.h>

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
#include <avr/wdt.h>  

static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x29};
uint8_t myip[4] = {192,168,0,110};
char serverip[16] = "192.168.0.100";
#define MYWWWPORT 80
uint16_t SERVERPORT = 80;
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
#define BUFFER_SIZE 5000
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
	//PRR&=~(1<<PRTIM2); // write power reduction register to zero
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

uint16_t http200ok(void)
{
	return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n")));
}

char charNumber[24];
char* toChar(int number)
{
	itoa(number, charNumber, 24);
	return charNumber;
}

uint32_t print_webpage(uint8_t *buf)
{
	uint32_t plen;
	plen=http200ok();
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<pre>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<style>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("div.center {\nposition: absolute;\ntop: 50%;\nleft: 50%;\nmargin-right: -50%;\ntransform: translate(-50%, -50%)\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("table, th, td {\nborder: 1px solid black;\nborder-collapse: collapse;\npadding: 15px;\ntext-align: center;\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("td.button {\npadding: 0px;\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("h2, hr {\ntext-align: center;\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("body {\nbackground-color: powderblue;\n}\n"));

	plen=fill_tcp_data_p(buf,plen,PSTR("</style>\n"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<meta charset=UTF-8>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<body>\n<div class=center>\n<h2>Weather Station</h2>\n<table>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<th>Information</th>\n</tr>"));

	plen=fill_tcp_data_p(buf,plen,PSTR("\n<tr>\n<td>Temperature: "));
	plen=fill_tcp_data(buf,plen,tempChar);
	plen=fill_tcp_data_p(buf,plen,PSTR(" &#x2103;</td>\n</tr>"));
	
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<tr>\n<td>Humidity: "));
	plen=fill_tcp_data(buf,plen,humidChar);
	plen=fill_tcp_data_p(buf,plen,PSTR(" %</td>\n</tr>"));
	
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<tr><td>Pressure: "));
	plen=fill_tcp_data(buf,plen,pressChar);
	plen=fill_tcp_data_p(buf,plen,PSTR(" Pa</td>\n</tr>"));
	
	//
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<tr>\n<td>Wind Angle: "));
	plen=fill_tcp_data(buf,plen,anglChar);
	plen=fill_tcp_data_p(buf,plen,PSTR("&#xb0;</td>\n</tr>"));

	//
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<tr>\n<td>Wind Speed: "));
	plen=fill_tcp_data(buf,plen,speedChar);
	plen=fill_tcp_data_p(buf,plen,PSTR(" m/s</td>\n</tr>"));
	
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<tr>\n<td>Box Status: "));
	if(magnetVal >= halleff)
		plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"red\">Closed</font>"));
	else
		plen=fill_tcp_data_p(buf,plen,PSTR("<font color=\"green\">Open</font>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</td>\n</tr>"));

	plen=fill_tcp_data_p(buf,plen,PSTR("\n</table>\n</form>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=\"submit\" value=\"Settings\" style=\"width:100%\" onclick=\"window.location='/settings';\" />\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=\"submit\" value=\"RTC Settings\" style=\"width:100%\" onclick=\"window.location='/rtc';\" />\n"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<td class=button><input type=submit value=\"Confirm\" style=\"width:100%\"></td>\n</tr>\n</table></form>"));
	
	//plen=fill_tcp_data_p(buf,plen,PSTR("\n</form>\n<form action=/ method=get><input type=hidden name=\"rr\" value=\"1\"><td class=button><input type=submit value=\"Reset\" style=\"width:100%\"></td></form>\n</tr>\n</table>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<br><hr>by Daniels Fi&#353ers</hr></br>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</div>\n</body>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</pre>"));

	
	return(plen);
}
uint32_t print_settings(uint8_t *buf)
{
	char charMyIP[16], charGWIP[16];
	uint32_t plen;
	char vstr[5];
	plen=http200ok();
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<pre>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<style>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("div.center {\nposition: absolute;\ntop: 50%;\nleft: 50%;\nmargin-right: -50%;\ntransform: translate(-50%, -50%)\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("table, th, td {\nborder: 1px solid black;\nborder-collapse: collapse;\npadding: 15px;\ntext-align: center;\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("td.button {\npadding: 0px;\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("h2, hr {\ntext-align: center;\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("body {\nbackground-color: powderblue;\n}\n"));

	plen=fill_tcp_data_p(buf,plen,PSTR("</style>\n"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<meta charset=UTF-8>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<body>\n<div class=center>\n<h2>Weather Station</h2>\n<table>\n"));
	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<th>Settings</th>\n</tr>\n"));

	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td><form action=/ method=get>Weather Station IP: <input type=text size=12 name=myip value=\""));
	sprintf(charMyIP, "%d.%d.%d.%d", myip[0], myip[1], myip[2], myip[3]);
	plen=fill_tcp_data(buf,plen,charMyIP);
	plen=fill_tcp_data_p(buf,plen,PSTR("\" /></td></tr>\n"));
	
	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Destination IP: <input type=text size=12 name=servip value=\""));
	plen=fill_tcp_data(buf,plen,serverip);
	plen=fill_tcp_data_p(buf,plen,PSTR("\" /></td></tr>\n"));
	
	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Gateway IP: <input type=text size=12 name=gwip value=\""));
	sprintf(charGWIP, "%d.%d.%d.%d", gwip[0], gwip[1], gwip[2], gwip[3]);
	plen=fill_tcp_data(buf,plen,charGWIP);
	plen=fill_tcp_data_p(buf,plen,PSTR("\" /></td></tr>\n"));
	//
 	char port[8];
 	// 	plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<td>\nPort: <input type=text size=12 name=port value="));
 	dtostrf(SERVERPORT,1,0,port);
 	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Port: <input type=text size=6 name=port value=\""));
 	plen=fill_tcp_data(buf,plen,port);
 	plen=fill_tcp_data_p(buf,plen,PSTR("\" /></td></tr>\n"));
	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Timeout (Seconds): <input type=text size=12 name=timeout value=\""));
	itoa(timeout,vstr,10);
	plen=fill_tcp_data(buf,plen,vstr);
	plen=fill_tcp_data_p(buf,plen,PSTR("\" /></td></tr>\n"));
	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Hall Sensor: <input type=text size=12 name=hall value=\""));
 	itoa(halleff,vstr,10);
 	plen=fill_tcp_data(buf,plen,vstr);
	plen=fill_tcp_data_p(buf,plen,PSTR("\" /></td></tr>\n"));
	
	
	//
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td class=button><input type=submit value=\"Confirm\" style=\"width:100%\"></td></tr>\n</form>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td class=button><input type=\"submit\" value=\"Restart\" style=\"width:100%\" onclick=\"window.location='/reset';\" /></td></tr>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</table>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=\"submit\" value=\"Home\" style=\"width:100%\" onclick=\"window.location='/home';\" />\n"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<input type=\"submit\" value=\"Information\" style=\"width:100%\" onclick=\"window.location='/';\" />\n</table></form>"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<td class=button><input type=submit value=\"Confirm\" style=\"width:100%\"></td>\n</tr>\n</table></form>"));
	
	//plen=fill_tcp_data_p(buf,plen,PSTR("\n</form>\n<form action=/ method=get><input type=hidden name=\"rr\" value=\"1\"><td class=button><input type=submit value=\"Reset\" style=\"width:100%\"></td></form>\n</tr>\n</table>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<br><hr>by Daniels Fi&#353ers</hr></br>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</div>\n</body>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</pre>"));

	
	return(plen);
}
uint32_t print_rtc(uint8_t *buf)
{
	char year[8],month[8],day[8],hour[8],minute[8],second[8];
	uint32_t plen;
	plen=http200ok();
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<pre>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<style>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("div.center {\nposition: absolute;\ntop: 50%;\nleft: 50%;\nmargin-right: -50%;\ntransform: translate(-50%, -50%)\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("table, th, td {\nborder: 1px solid black;\nborder-collapse: collapse;\npadding: 15px;\ntext-align: center;\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("td.button {\npadding: 0px;\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("h2, hr {\ntext-align: center;\n}\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("body {\nbackground-color: powderblue;\n}\n"));

	plen=fill_tcp_data_p(buf,plen,PSTR("</style>\n"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<meta charset=UTF-8>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<body>\n<div class=center>\n<h2>Weather Station</h2>\n<table>\n"));
	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<th colspan=\"2\">Real Time Clock Settings</th>\n</tr>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<th>Date</th><th>Time</th>\n</tr>"));

	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<td><form action=/ method=get>\nYear: <input type=text size=12 name=year value="));
	dtostrf(rtc.year,1,0,year);
	plen=fill_tcp_data(buf,plen,year);
	plen=fill_tcp_data_p(buf,plen,PSTR("></td>"));
	
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<td><form action=/ method=get>\nHour: <input type=text size=12 name=hour value="));
	dtostrf(rtc.hour,1,0,hour);
	plen=fill_tcp_data(buf,plen,hour);
	plen=fill_tcp_data_p(buf,plen,PSTR("></td>\n</tr>"));
	
	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<td><form action=/ method=get>\nMonth: <input type=text size=12 name=month value="));
	dtostrf(rtc.month,1,0,month);
	plen=fill_tcp_data(buf,plen,month);
	plen=fill_tcp_data_p(buf,plen,PSTR("></td>"));
	
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<td><form action=/ method=get>\nMinute: <input type=text size=12 name=min value="));
	dtostrf(rtc.min,1,0,minute);
	plen=fill_tcp_data(buf,plen,minute);
	plen=fill_tcp_data_p(buf,plen,PSTR("></td>\n</tr>"));
	
	
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<td><form action=/ method=get>\nDay: <input type=text size=12 name=day value="));
	dtostrf(rtc.date,1,0,day);
	plen=fill_tcp_data(buf,plen,day);
	plen=fill_tcp_data_p(buf,plen,PSTR("></td>"));
	//
	plen=fill_tcp_data_p(buf,plen,PSTR("\n<td><form action=/ method=get>\nSecond: <input type=text size=12 name=sec value="));
	dtostrf(rtc.sec,1,0,second);
	plen=fill_tcp_data(buf,plen,second);
	plen=fill_tcp_data_p(buf,plen,PSTR("></td>\n</tr>"));

	//
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<td class=button colspan=\"2\"><input type=submit value=\"Confirm\" style=\"width:100%\"></td>\n</tr>\n</form>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("\n</table>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=\"submit\" value=\"Home\" style=\"width:100%\" onclick=\"window.location='/home';\" />\n"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<input type=\"submit\" value=\"Information\" style=\"width:100%\" onclick=\"window.location='/';\" />\n</table></form>"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<tr>\n<td class=button><input type=submit value=\"Confirm\" style=\"width:100%\"></td>\n</tr>\n</table></form>"));
	
	//plen=fill_tcp_data_p(buf,plen,PSTR("\n</form>\n<form action=/ method=get><input type=hidden name=\"rr\" value=\"1\"><td class=button><input type=submit value=\"Reset\" style=\"width:100%\"></td></form>\n</tr>\n</table>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<br><hr>by Daniels Fi&#353ers</hr></br>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</div>\n</body>\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</pre>"));

	
	return(plen);
}

void software_reset()
{
	wdt_reset();
	wdt_enable(WDTO_250MS);
	exit (1);  // loop forever
}

int restart = 0;

int8_t analyse_get_url(char *str)
{
	uint8_t mn=0;
	char kvalstrbuf[10];
	// the first slash:
	if(str[0] == '/' && str[1] == 'r' && str[3] == 'c' && str[4] == ' ')
	{
		return (4);
	}
	if(str[0] == '/' && str[1] == 's' && str[8] == 's' && str[9] == ' ')
	{
		return (3);
	}
	if ((str[0] == '/' && str[1] == ' ') || (str[0] == '/' && str[1] == 'h')){
		// end of url, display just the web page
		return(2);
	}
	if(str[0] == '/' && str[1] == 'r' && str[5] == 't' && str[6] == ' ')
	{
		restart = 1;
		mn = 1;
	}
// 	if (find_key_val(str,kvalstrbuf,16,"settings")){
// 		mn = 3;
// 	}
	// str is now something like ?pw=secret&mn=0 or just end of url
	if (find_key_val(str,kvalstrbuf,16,"myip")){
		stringToIntArray(myip, kvalstrbuf, (uint8_t*)0x20);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,16,"servip")){
		sprintf(serverip, "%s", kvalstrbuf);
		eeprom_write_block((const void*)kvalstrbuf, (void*)0x30, 16);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,16,"gwip")){
		stringToIntArray(gwip, kvalstrbuf, (uint8_t*)0x40);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,10,"port")){
		SERVERPORT = atoi(kvalstrbuf);
		eeprom_write_word((uint16_t*)0x70, (uint16_t)SERVERPORT);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,10,"timeout")){
		timeout = atoi(kvalstrbuf);
		eeprom_write_word((uint16_t*)0x10, (uint16_t)timeout);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,10,"hall")){
		halleff = atoi(kvalstrbuf);
		eeprom_write_word((uint16_t*)0x60, (uint16_t)halleff);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,8,"year")){
		rtc.year = atoi(kvalstrbuf);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,8,"month")){
		rtc.month = atoi(kvalstrbuf);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,8,"day")){
		rtc.date = atoi(kvalstrbuf);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,8,"hour")){
		rtc.hour = atoi(kvalstrbuf);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,8,"min")){
		rtc.min = atoi(kvalstrbuf);
		mn = 1;
	}
	if (find_key_val(str,kvalstrbuf,8,"sec")){
		rtc.sec = atoi(kvalstrbuf);
		mn = 1;
	}
	if(mn == 1)
	{
		RTC_SetDateTime(&rtc);
		return(1);
	}
	// browsers looking for /favion.ico, non existing pages etc...
	return(-1);
}

void Ether_SendPacket(char* text){
	uint16_t dat_p,plen;
	
	if(sec > timeout)
	{
		sendingPacket = 0;
		start_web_client = 1;
		return;
	}
	cli();
	sendingPacket = 1;
	plen=enc28j60PacketReceive(BUFFER_SIZE, buf);
	buf[BUFFER_SIZE]='\0';
	dat_p=packetloop_arp_icmp_tcp(buf,plen);
	if(plen == 0){
		if(start_web_client == 1)
		{
			sec = 0;
			start_web_client = 0;
			client_tcpSend(text,&browserresult_callback,otherside_www_ip,gwmac,SERVERPORT);
			//client_http_post(PSTR("/api/statuses/update.xml"),"",PSTR("192.168.0.100"),NULL,text,&browserresult_callback,otherside_www_ip,gwmac);
			return;
			//
			
		}
	}
	sei();
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
		if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
			//head, post and other methods:
						                         
				//for possible status codes see:
				///http:www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
				dat_p=http200ok();
				dat_p=fill_tcp_data_p(buf,dat_p,PSTR("<h1>200 OK</h1>"));
				goto SENDTCP;
		}
		int8_t cmd = analyse_get_url((char *)&(buf[dat_p+4]));
		if (cmd==-1){
			dat_p=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
			goto SENDTCP;
		}
		if(cmd == 1)
		{
			char charMyIP[16];
			sprintf(charMyIP, "%d.%d.%d.%d", myip[0], myip[1], myip[2], myip[3]);
			dat_p=http200ok();
			dat_p=fill_tcp_data_p(buf,dat_p,PSTR("<pre>\n"));
			dat_p=fill_tcp_data_p(buf,dat_p,PSTR("<script type=text/javascript>\n"));
			dat_p=fill_tcp_data_p(buf,dat_p,PSTR("location.href = 'http://"));
			dat_p=fill_tcp_data(buf,dat_p,charMyIP);
			dat_p=fill_tcp_data_p(buf,dat_p,PSTR("/home';\n</script>\n"));
			goto SENDTCP;
		}
		if(cmd == 3)
		{
			dat_p=http200ok();
			dat_p=print_settings(buf);
			goto SENDTCP;
		}
		if(cmd == 4)
		{
			dat_p=http200ok();
			dat_p=print_rtc(buf);
			goto SENDTCP;
		}
		dat_p=http200ok();
		dat_p=print_webpage(buf);
		if(restart) software_reset();
		goto SENDTCP;
						                 
	SENDTCP:
				www_server_reply(buf,dat_p);
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
	 sec = 0;
	 while(get_mac_with_arp_wait()){
		 if(sec > 10)
		 {
			 processing_state=4;
			 break;
		 }
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


