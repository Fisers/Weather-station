int BMP_CS;
int32_t t_fine;

typedef struct {
	uint16_t dig_T1; /**< dig_T1 cal register. */
	int16_t dig_T2;  /**<  dig_T2 cal register. */
	int16_t dig_T3;  /**< dig_T3 cal register. */

	uint16_t dig_P1; /**< dig_P1 cal register. */
	int16_t dig_P2;  /**< dig_P2 cal register. */
	int16_t dig_P3;  /**< dig_P3 cal register. */
	int16_t dig_P4;  /**< dig_P4 cal register. */
	int16_t dig_P5;  /**< dig_P5 cal register. */
	int16_t dig_P6;  /**< dig_P6 cal register. */
	int16_t dig_P7;  /**< dig_P7 cal register. */
	int16_t dig_P8;  /**< dig_P8 cal register. */
	int16_t dig_P9;  /**< dig_P9 cal register. */

	uint8_t dig_H1; /**< dig_H1 cal register. */
	int16_t dig_H2; /**< dig_H2 cal register. */
	uint8_t dig_H3; /**< dig_H3 cal register. */
	int16_t dig_H4; /**< dig_H4 cal register. */
	int16_t dig_H5; /**< dig_H5 cal register. */
	int8_t dig_H6;  /**< dig_H6 cal register. */
} bmp280_calib_data;
bmp280_calib_data _bmp280_calib;


extern void bmp280_init(int pin);
extern float bmp280_readPressure();