/*
 * test_web_client.h
 *
 * Created: 09/07/2019 13:09:08
 *  Author: daniels
 */ 


#ifndef TEST_WEB_CLIENT_H_
#define TEST_WEB_CLIENT_H_

extern uint8_t sendingPacket;
extern volatile uint8_t sec;
void Ether_init();
void Ether_SendPacket(char* text);
void init_cnt2(void);



#endif /* TEST_WEB_CLIENT_H_ */