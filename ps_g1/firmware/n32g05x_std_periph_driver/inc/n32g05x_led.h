/**
*     Copyright (c) 2023, Nations Technologies Inc.
* 
*     All rights reserved.
*
*     This software is the exclusive property of Nations Technologies Inc. (Hereinafter 
* referred to as NATIONS). This software, and the product of NATIONS described herein 
* (Hereinafter referred to as the Product) are owned by NATIONS under the laws and treaties
* of the People's Republic of China and other applicable jurisdictions worldwide.
*
*     NATIONS does not grant any license under its patents, copyrights, trademarks, or other 
* intellectual property rights. Names and brands of third party may be mentioned or referred 
* thereto (if any) for identification purposes only.
*
*     NATIONS reserves the right to make changes, corrections, enhancements, modifications, and 
* improvements to this software at any time without notice. Please contact NATIONS and obtain 
* the latest version of this software before placing orders.

*     Although NATIONS has attempted to provide accurate and reliable information, NATIONS assumes 
* no responsibility for the accuracy and reliability of this software.
* 
*     It is the responsibility of the user of this software to properly design, program, and test 
* the functionality and safety of any application made of this information and any resulting product. 
* In no event shall NATIONS be liable for any direct, indirect, incidental, special,exemplary, or 
* consequential damages arising in any way out of the use of this software or the Product.
*
*     NATIONS Products are neither intended nor warranted for usage in systems or equipment, any
* malfunction or failure of which may cause loss of human life, bodily injury or severe property 
* damage. Such applications are deemed, "Insecure Usage".
*
*     All Insecure Usage shall be made at user's risk. User shall indemnify NATIONS and hold NATIONS 
* harmless from and against all claims, costs, damages, and other liabilities, arising from or related 
* to any customer's Insecure Usage.

*     Any express or implied warranty with regard to this software or the Product, including,but not 
* limited to, the warranties of merchantability, fitness for a particular purpose and non-infringement
* are disclaimed to the fullest extent permitted by law.

*     Unless otherwise explicitly permitted by NATIONS, anyone may not duplicate, modify, transcribe
* or otherwise distribute this software for any purposes, in whole or in part.
*
*     NATIONS products and technologies shall not be used for or incorporated into any products or systems
* whose manufacture, use, or sale is prohibited under any applicable domestic or foreign laws or regulations. 
* User shall comply with any applicable export control laws and regulations promulgated and administered by 
* the governments of any countries asserting jurisdiction over the parties or transactions.
**/

/**
*\*\file n32g05x_led.h
*\*\author Nations
*\*\version v1.0.1
*\*\copyright Copyright (c) 2023, Nations Technologies Inc. All rights reserved.
**/

#ifndef __N32G05X_LED_H__
#define __N32G05X_LED_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "n32g05x.h"
#include "n32g05x_rcc.h"
#include "n32g05x_gpio.h"
#include "n32g05x_spi.h"

/*** LED Structure Definition Start ***/
#define SPI_LED         SPI3
#define SPIx_CLK        RCC_APB2_PERIPH_SPI3

#define SPIx_LED_CLK    (RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOD)

#define SPIx_LED_GCLK_PORT   GPIOD      
#define SPIx_LED_GCLK_PIN    GPIO_PIN_1 /* PD1 */

#define SPIx_LED_SCK_PORT    GPIOD
#define SPIx_LED_SCK_PIN     GPIO_PIN_3 /* PD3 */
#define SPIx_LED_SCK_AFIO    GPIO_AF0_SPI3

#define SPIx_LED_NSS_PORT    GPIOD 
#define SPIx_LED_NSS_PIN     GPIO_PIN_2 /* PD2 */
#define SPIx_LED_NSS_AFIO    GPIO_AF0_SPI3

#define SPIx_LED_MOSI_PORT   GPIOA      
#define SPIx_LED_MOSI_PIN    GPIO_PIN_0 /* PA0 */
#define SPIx_LED_MOSI_AFIO   GPIO_AF4_SPI3

#define SPIx_LED_MISO_PORT   GPIOA 
#define SPIx_LED_MISO_PIN    GPIO_PIN_1 /* PA1 */
#define SPIx_LED_MISO_AFIO   GPIO_AF0_SPI3



/*** LED COM PORT Defination ***/
#define COM0_2_PORT       GPIOB
#define COM3_PORT         GPIOC
#define COM4_PORT         GPIOA
#define COM5_9_PORT       GPIOD

 /*** LED COM PIN Defination ***/
#define COM0_PIN        GPIO_PIN_12
#define COM1_PIN        GPIO_PIN_14
#define COM2_PIN        GPIO_PIN_15
#define COM3_PIN        GPIO_PIN_8
#define COM4_PIN        GPIO_PIN_15
#define COM5_PIN        GPIO_PIN_10
#define COM6_PIN        GPIO_PIN_9
#define COM7_PIN        GPIO_PIN_8
#define COM8_PIN        GPIO_PIN_7
#define COM9_PIN        GPIO_PIN_6


/*** LED SEG PORT Defination ***/
#define SEG0_2PORT      GPIOB
#define SEG3_PORT       GPIOC
#define SEG4_PORT       GPIOB
#define SEG5_15_PORT    GPIOC


 /*** LED SEG PIN Defination ***/
#define SEG0_PIN        GPIO_PIN_6
#define SEG1_PIN        GPIO_PIN_8
#define SEG2_PIN        GPIO_PIN_9
#define SEG3_PIN        GPIO_PIN_4
#define SEG4_PIN        GPIO_PIN_7
#define SEG5_PIN        GPIO_PIN_11
#define SEG6_PIN        GPIO_PIN_10
#define SEG7_PIN        GPIO_PIN_9
#define SEG8_PIN        GPIO_PIN_5
#define SEG9_PIN        GPIO_PIN_6
#define SEG10_PIN       GPIO_PIN_7
#define SEG11_PIN       GPIO_PIN_2
#define SEG12_PIN       GPIO_PIN_3
#define SEG13_PIN       GPIO_PIN_0
#define SEG14_PIN       GPIO_PIN_1
#define SEG15_PIN       GPIO_PIN_12


/** LED Init structure definition **/
typedef struct
{
		/* LED Init Config */
    uint8_t GCLK_LowLevelNum;		/* Specifies the GCLK low level number , the value must be greater than 1 */

    uint8_t COM_WorkNum;				/* Specifies the total number of COM-IO eable to work , the value must be greater than 1. 
																		eg : when COMWorkNumer = 4 it means COM0,COM1,COM2,COM3 will eable to work*/
    uint8_t SEG_WorkCurrent;		/* Specifies LED SEG working current,the value 0 ~30 to 5~20mA */

    uint16_t SEG_WorkStatus;		/* Specifies which SEG-IO is enable to work,Bit0 - SEG0,Bit1 - SEG1...Bit15 - SEG15
                                    eg :when SEG_WorkStatus = 0x1841 it means SEG0,SEG6,SEG11,SEG15 will eable to work*/

    uint8_t Scan_Mode;					/* Specifies LED display mode  */

    uint8_t Work_Mode;					/* Specifies LED is enabled or disabled. */
		/* *************** */
	
		/* LED operating status */
    uint8_t Cur_ScanCOM;				/* LED current scan COM. */
		/* *************** */
    /* LED SEG duty */
    uint8_t SEG_Duty[16];				/* LED current SEGx duty. */
		/* *************** */
	
    /* LED Trim Value */
    uint8_t Trim_Rset;					/* LED Rset trim. */

    uint8_t Trim_BG;						/* LED BG trim. */
                                    
    uint8_t Trim_SEG[6];				/* LED SEGx trim. */
		/* *************** */
} LED_InitType;

/*LED GCLK_LowLevelNum*/
#define	GCLK_LOWLEVELNUM_2	((uint8_t)	2U)
#define	GCLK_LOWLEVELNUM_3	((uint8_t)	3U)
#define	GCLK_LOWLEVELNUM_4	((uint8_t)	4U)
#define	GCLK_LOWLEVELNUM_5	((uint8_t)	5U)
#define	GCLK_LOWLEVELNUM_6	((uint8_t)	6U)
#define	GCLK_LOWLEVELNUM_7	((uint8_t)	7U)
#define	GCLK_LOWLEVELNUM_8	((uint8_t)	8U)
#define	GCLK_LOWLEVELNUM_9	((uint8_t)	9U)
#define	GCLK_LOWLEVELNUM_10	((uint8_t)	10U)
#define	GCLK_LOWLEVELNUM_11	((uint8_t)	11U)
#define	GCLK_LOWLEVELNUM_12	((uint8_t)	12U)
#define	GCLK_LOWLEVELNUM_13	((uint8_t)	13U)
#define	GCLK_LOWLEVELNUM_14	((uint8_t)	14U)
#define	GCLK_LOWLEVELNUM_15	((uint8_t)	15U)
#define	GCLK_LOWLEVELNUM_16	((uint8_t)	16U)
#define	GCLK_LOWLEVELNUM_17	((uint8_t)	17U)
#define	GCLK_LOWLEVELNUM_18	((uint8_t)	18U)
#define	GCLK_LOWLEVELNUM_19	((uint8_t)	19U)
#define	GCLK_LOWLEVELNUM_20	((uint8_t)	20U)
#define	GCLK_LOWLEVELNUM_21	((uint8_t)	21U)
#define	GCLK_LOWLEVELNUM_22	((uint8_t)	22U)
#define	GCLK_LOWLEVELNUM_23	((uint8_t)	23U)
#define	GCLK_LOWLEVELNUM_24	((uint8_t)	24U)
#define	GCLK_LOWLEVELNUM_25	((uint8_t)	25U)
#define	GCLK_LOWLEVELNUM_26	((uint8_t)	26U)
#define	GCLK_LOWLEVELNUM_27	((uint8_t)	27U)
#define	GCLK_LOWLEVELNUM_28	((uint8_t)	28U)
#define	GCLK_LOWLEVELNUM_29	((uint8_t)	29U)
#define	GCLK_LOWLEVELNUM_30	((uint8_t)	30U)
#define	GCLK_LOWLEVELNUM_31	((uint8_t)	31U)
#define	GCLK_LOWLEVELNUM_32	((uint8_t)	32U)
#define	GCLK_LOWLEVELNUM_33	((uint8_t)	33U)
#define	GCLK_LOWLEVELNUM_34	((uint8_t)	34U)
#define	GCLK_LOWLEVELNUM_35	((uint8_t)	35U)
#define	GCLK_LOWLEVELNUM_36	((uint8_t)	36U)
#define	GCLK_LOWLEVELNUM_37	((uint8_t)	37U)
#define	GCLK_LOWLEVELNUM_38	((uint8_t)	38U)
#define	GCLK_LOWLEVELNUM_39	((uint8_t)	39U)
#define	GCLK_LOWLEVELNUM_40	((uint8_t)	40U)
#define	GCLK_LOWLEVELNUM_41	((uint8_t)	41U)
#define	GCLK_LOWLEVELNUM_42	((uint8_t)	42U)
#define	GCLK_LOWLEVELNUM_43	((uint8_t)	43U)
#define	GCLK_LOWLEVELNUM_44	((uint8_t)	44U)
#define	GCLK_LOWLEVELNUM_45	((uint8_t)	45U)
#define	GCLK_LOWLEVELNUM_46	((uint8_t)	46U)
#define	GCLK_LOWLEVELNUM_47	((uint8_t)	47U)
#define	GCLK_LOWLEVELNUM_48	((uint8_t)	48U)
#define	GCLK_LOWLEVELNUM_49	((uint8_t)	49U)
#define	GCLK_LOWLEVELNUM_50	((uint8_t)	50U)
#define	GCLK_LOWLEVELNUM_51	((uint8_t)	51U)
#define	GCLK_LOWLEVELNUM_52	((uint8_t)	52U)
#define	GCLK_LOWLEVELNUM_53	((uint8_t)	53U)
#define	GCLK_LOWLEVELNUM_54	((uint8_t)	54U)
#define	GCLK_LOWLEVELNUM_55	((uint8_t)	55U)
#define	GCLK_LOWLEVELNUM_56	((uint8_t)	56U)
#define	GCLK_LOWLEVELNUM_57	((uint8_t)	57U)
#define	GCLK_LOWLEVELNUM_58	((uint8_t)	58U)
#define	GCLK_LOWLEVELNUM_59	((uint8_t)	59U)
#define	GCLK_LOWLEVELNUM_60	((uint8_t)	60U)
#define	GCLK_LOWLEVELNUM_61	((uint8_t)	61U)
#define	GCLK_LOWLEVELNUM_62	((uint8_t)	62U)
#define	GCLK_LOWLEVELNUM_63	((uint8_t)	63U)
#define	GCLK_LOWLEVELNUM_64	((uint8_t)	64U)
#define	GCLK_LOWLEVELNUM_65	((uint8_t)	65U)
#define	GCLK_LOWLEVELNUM_66	((uint8_t)	66U)
#define	GCLK_LOWLEVELNUM_67	((uint8_t)	67U)
#define	GCLK_LOWLEVELNUM_68	((uint8_t)	68U)
#define	GCLK_LOWLEVELNUM_69	((uint8_t)	69U)
#define	GCLK_LOWLEVELNUM_70	((uint8_t)	70U)
#define	GCLK_LOWLEVELNUM_71	((uint8_t)	71U)
#define	GCLK_LOWLEVELNUM_72	((uint8_t)	72U)
#define	GCLK_LOWLEVELNUM_73	((uint8_t)	73U)
#define	GCLK_LOWLEVELNUM_74	((uint8_t)	74U)
#define	GCLK_LOWLEVELNUM_75	((uint8_t)	75U)
#define	GCLK_LOWLEVELNUM_76	((uint8_t)	76U)
#define	GCLK_LOWLEVELNUM_77	((uint8_t)	77U)
#define	GCLK_LOWLEVELNUM_78	((uint8_t)	78U)
#define	GCLK_LOWLEVELNUM_79	((uint8_t)	79U)
#define	GCLK_LOWLEVELNUM_80	((uint8_t)	80U)
#define	GCLK_LOWLEVELNUM_81	((uint8_t)	81U)
#define	GCLK_LOWLEVELNUM_82	((uint8_t)	82U)
#define	GCLK_LOWLEVELNUM_83	((uint8_t)	83U)
#define	GCLK_LOWLEVELNUM_84	((uint8_t)	84U)
#define	GCLK_LOWLEVELNUM_85	((uint8_t)	85U)
#define	GCLK_LOWLEVELNUM_86	((uint8_t)	86U)
#define	GCLK_LOWLEVELNUM_87	((uint8_t)	87U)
#define	GCLK_LOWLEVELNUM_88	((uint8_t)	88U)
#define	GCLK_LOWLEVELNUM_89	((uint8_t)	89U)
#define	GCLK_LOWLEVELNUM_90	((uint8_t)	90U)
#define	GCLK_LOWLEVELNUM_91	((uint8_t)	91U)
#define	GCLK_LOWLEVELNUM_92	((uint8_t)	92U)
#define	GCLK_LOWLEVELNUM_93	((uint8_t)	93U)
#define	GCLK_LOWLEVELNUM_94	((uint8_t)	94U)
#define	GCLK_LOWLEVELNUM_95	((uint8_t)	95U)
#define	GCLK_LOWLEVELNUM_96	((uint8_t)	96U)
#define	GCLK_LOWLEVELNUM_97	((uint8_t)	97U)
#define	GCLK_LOWLEVELNUM_98	((uint8_t)	98U)
#define	GCLK_LOWLEVELNUM_99	((uint8_t)	99U)
#define	GCLK_LOWLEVELNUM_100	((uint8_t)	100U)
#define	GCLK_LOWLEVELNUM_101	((uint8_t)	101U)
#define	GCLK_LOWLEVELNUM_102	((uint8_t)	102U)
#define	GCLK_LOWLEVELNUM_103	((uint8_t)	103U)
#define	GCLK_LOWLEVELNUM_104	((uint8_t)	104U)
#define	GCLK_LOWLEVELNUM_105	((uint8_t)	105U)
#define	GCLK_LOWLEVELNUM_106	((uint8_t)	106U)
#define	GCLK_LOWLEVELNUM_107	((uint8_t)	107U)
#define	GCLK_LOWLEVELNUM_108	((uint8_t)	108U)
#define	GCLK_LOWLEVELNUM_109	((uint8_t)	109U)
#define	GCLK_LOWLEVELNUM_110	((uint8_t)	110U)
#define	GCLK_LOWLEVELNUM_111	((uint8_t)	111U)
#define	GCLK_LOWLEVELNUM_112	((uint8_t)	112U)
#define	GCLK_LOWLEVELNUM_113	((uint8_t)	113U)
#define	GCLK_LOWLEVELNUM_114	((uint8_t)	114U)
#define	GCLK_LOWLEVELNUM_115	((uint8_t)	115U)
#define	GCLK_LOWLEVELNUM_116	((uint8_t)	116U)
#define	GCLK_LOWLEVELNUM_117	((uint8_t)	117U)
#define	GCLK_LOWLEVELNUM_118	((uint8_t)	118U)
#define	GCLK_LOWLEVELNUM_119	((uint8_t)	119U)
#define	GCLK_LOWLEVELNUM_120	((uint8_t)	120U)
#define	GCLK_LOWLEVELNUM_121	((uint8_t)	121U)
#define	GCLK_LOWLEVELNUM_122	((uint8_t)	122U)
#define	GCLK_LOWLEVELNUM_123	((uint8_t)	123U)
#define	GCLK_LOWLEVELNUM_124	((uint8_t)	124U)
#define	GCLK_LOWLEVELNUM_125	((uint8_t)	125U)
#define	GCLK_LOWLEVELNUM_126	((uint8_t)	126U)
#define	GCLK_LOWLEVELNUM_127	((uint8_t)	127U)
#define	GCLK_LOWLEVELNUM_128	((uint8_t)	128U)
#define	GCLK_LOWLEVELNUM_129	((uint8_t)	129U)
#define	GCLK_LOWLEVELNUM_130	((uint8_t)	130U)
#define	GCLK_LOWLEVELNUM_131	((uint8_t)	131U)
#define	GCLK_LOWLEVELNUM_132	((uint8_t)	132U)
#define	GCLK_LOWLEVELNUM_133	((uint8_t)	133U)
#define	GCLK_LOWLEVELNUM_134	((uint8_t)	134U)
#define	GCLK_LOWLEVELNUM_135	((uint8_t)	135U)
#define	GCLK_LOWLEVELNUM_136	((uint8_t)	136U)
#define	GCLK_LOWLEVELNUM_137	((uint8_t)	137U)
#define	GCLK_LOWLEVELNUM_138	((uint8_t)	138U)
#define	GCLK_LOWLEVELNUM_139	((uint8_t)	139U)
#define	GCLK_LOWLEVELNUM_140	((uint8_t)	140U)
#define	GCLK_LOWLEVELNUM_141	((uint8_t)	141U)
#define	GCLK_LOWLEVELNUM_142	((uint8_t)	142U)
#define	GCLK_LOWLEVELNUM_143	((uint8_t)	143U)
#define	GCLK_LOWLEVELNUM_144	((uint8_t)	144U)
#define	GCLK_LOWLEVELNUM_145	((uint8_t)	145U)
#define	GCLK_LOWLEVELNUM_146	((uint8_t)	146U)
#define	GCLK_LOWLEVELNUM_147	((uint8_t)	147U)
#define	GCLK_LOWLEVELNUM_148	((uint8_t)	148U)
#define	GCLK_LOWLEVELNUM_149	((uint8_t)	149U)
#define	GCLK_LOWLEVELNUM_150	((uint8_t)	150U)
#define	GCLK_LOWLEVELNUM_151	((uint8_t)	151U)
#define	GCLK_LOWLEVELNUM_152	((uint8_t)	152U)
#define	GCLK_LOWLEVELNUM_153	((uint8_t)	153U)
#define	GCLK_LOWLEVELNUM_154	((uint8_t)	154U)
#define	GCLK_LOWLEVELNUM_155	((uint8_t)	155U)
#define	GCLK_LOWLEVELNUM_156	((uint8_t)	156U)
#define	GCLK_LOWLEVELNUM_157	((uint8_t)	157U)
#define	GCLK_LOWLEVELNUM_158	((uint8_t)	158U)
#define	GCLK_LOWLEVELNUM_159	((uint8_t)	159U)
#define	GCLK_LOWLEVELNUM_160	((uint8_t)	160U)
#define	GCLK_LOWLEVELNUM_161	((uint8_t)	161U)
#define	GCLK_LOWLEVELNUM_162	((uint8_t)	162U)
#define	GCLK_LOWLEVELNUM_163	((uint8_t)	163U)
#define	GCLK_LOWLEVELNUM_164	((uint8_t)	164U)
#define	GCLK_LOWLEVELNUM_165	((uint8_t)	165U)
#define	GCLK_LOWLEVELNUM_166	((uint8_t)	166U)
#define	GCLK_LOWLEVELNUM_167	((uint8_t)	167U)
#define	GCLK_LOWLEVELNUM_168	((uint8_t)	168U)
#define	GCLK_LOWLEVELNUM_169	((uint8_t)	169U)
#define	GCLK_LOWLEVELNUM_170	((uint8_t)	170U)
#define	GCLK_LOWLEVELNUM_171	((uint8_t)	171U)
#define	GCLK_LOWLEVELNUM_172	((uint8_t)	172U)
#define	GCLK_LOWLEVELNUM_173	((uint8_t)	173U)
#define	GCLK_LOWLEVELNUM_174	((uint8_t)	174U)
#define	GCLK_LOWLEVELNUM_175	((uint8_t)	175U)
#define	GCLK_LOWLEVELNUM_176	((uint8_t)	176U)
#define	GCLK_LOWLEVELNUM_177	((uint8_t)	177U)
#define	GCLK_LOWLEVELNUM_178	((uint8_t)	178U)
#define	GCLK_LOWLEVELNUM_179	((uint8_t)	179U)
#define	GCLK_LOWLEVELNUM_180	((uint8_t)	180U)
#define	GCLK_LOWLEVELNUM_181	((uint8_t)	181U)
#define	GCLK_LOWLEVELNUM_182	((uint8_t)	182U)
#define	GCLK_LOWLEVELNUM_183	((uint8_t)	183U)
#define	GCLK_LOWLEVELNUM_184	((uint8_t)	184U)
#define	GCLK_LOWLEVELNUM_185	((uint8_t)	185U)
#define	GCLK_LOWLEVELNUM_186	((uint8_t)	186U)
#define	GCLK_LOWLEVELNUM_187	((uint8_t)	187U)
#define	GCLK_LOWLEVELNUM_188	((uint8_t)	188U)
#define	GCLK_LOWLEVELNUM_189	((uint8_t)	189U)
#define	GCLK_LOWLEVELNUM_190	((uint8_t)	190U)
#define	GCLK_LOWLEVELNUM_191	((uint8_t)	191U)
#define	GCLK_LOWLEVELNUM_192	((uint8_t)	192U)
#define	GCLK_LOWLEVELNUM_193	((uint8_t)	193U)
#define	GCLK_LOWLEVELNUM_194	((uint8_t)	194U)
#define	GCLK_LOWLEVELNUM_195	((uint8_t)	195U)
#define	GCLK_LOWLEVELNUM_196	((uint8_t)	196U)
#define	GCLK_LOWLEVELNUM_197	((uint8_t)	197U)
#define	GCLK_LOWLEVELNUM_198	((uint8_t)	198U)
#define	GCLK_LOWLEVELNUM_199	((uint8_t)	199U)
#define	GCLK_LOWLEVELNUM_200	((uint8_t)	200U)
#define	GCLK_LOWLEVELNUM_201	((uint8_t)	201U)
#define	GCLK_LOWLEVELNUM_202	((uint8_t)	202U)
#define	GCLK_LOWLEVELNUM_203	((uint8_t)	203U)
#define	GCLK_LOWLEVELNUM_204	((uint8_t)	204U)
#define	GCLK_LOWLEVELNUM_205	((uint8_t)	205U)
#define	GCLK_LOWLEVELNUM_206	((uint8_t)	206U)
#define	GCLK_LOWLEVELNUM_207	((uint8_t)	207U)
#define	GCLK_LOWLEVELNUM_208	((uint8_t)	208U)
#define	GCLK_LOWLEVELNUM_209	((uint8_t)	209U)
#define	GCLK_LOWLEVELNUM_210	((uint8_t)	210U)
#define	GCLK_LOWLEVELNUM_211	((uint8_t)	211U)
#define	GCLK_LOWLEVELNUM_212	((uint8_t)	212U)
#define	GCLK_LOWLEVELNUM_213	((uint8_t)	213U)
#define	GCLK_LOWLEVELNUM_214	((uint8_t)	214U)
#define	GCLK_LOWLEVELNUM_215	((uint8_t)	215U)
#define	GCLK_LOWLEVELNUM_216	((uint8_t)	216U)
#define	GCLK_LOWLEVELNUM_217	((uint8_t)	217U)
#define	GCLK_LOWLEVELNUM_218	((uint8_t)	218U)
#define	GCLK_LOWLEVELNUM_219	((uint8_t)	219U)
#define	GCLK_LOWLEVELNUM_220	((uint8_t)	220U)
#define	GCLK_LOWLEVELNUM_221	((uint8_t)	221U)
#define	GCLK_LOWLEVELNUM_222	((uint8_t)	222U)
#define	GCLK_LOWLEVELNUM_223	((uint8_t)	223U)
#define	GCLK_LOWLEVELNUM_224	((uint8_t)	224U)
#define	GCLK_LOWLEVELNUM_225	((uint8_t)	225U)
#define	GCLK_LOWLEVELNUM_226	((uint8_t)	226U)
#define	GCLK_LOWLEVELNUM_227	((uint8_t)	227U)
#define	GCLK_LOWLEVELNUM_228	((uint8_t)	228U)
#define	GCLK_LOWLEVELNUM_229	((uint8_t)	229U)
#define	GCLK_LOWLEVELNUM_230	((uint8_t)	230U)
#define	GCLK_LOWLEVELNUM_231	((uint8_t)	231U)
#define	GCLK_LOWLEVELNUM_232	((uint8_t)	232U)
#define	GCLK_LOWLEVELNUM_233	((uint8_t)	233U)
#define	GCLK_LOWLEVELNUM_234	((uint8_t)	234U)
#define	GCLK_LOWLEVELNUM_235	((uint8_t)	235U)
#define	GCLK_LOWLEVELNUM_236	((uint8_t)	236U)
#define	GCLK_LOWLEVELNUM_237	((uint8_t)	237U)
#define	GCLK_LOWLEVELNUM_238	((uint8_t)	238U)
#define	GCLK_LOWLEVELNUM_239	((uint8_t)	239U)
#define	GCLK_LOWLEVELNUM_240	((uint8_t)	240U)
#define	GCLK_LOWLEVELNUM_241	((uint8_t)	241U)
#define	GCLK_LOWLEVELNUM_242	((uint8_t)	242U)
#define	GCLK_LOWLEVELNUM_243	((uint8_t)	243U)
#define	GCLK_LOWLEVELNUM_244	((uint8_t)	244U)
#define	GCLK_LOWLEVELNUM_245	((uint8_t)	245U)
#define	GCLK_LOWLEVELNUM_246	((uint8_t)	246U)
#define	GCLK_LOWLEVELNUM_247	((uint8_t)	247U)
#define	GCLK_LOWLEVELNUM_248	((uint8_t)	248U)
#define	GCLK_LOWLEVELNUM_249	((uint8_t)	249U)
#define	GCLK_LOWLEVELNUM_250	((uint8_t)	250U)
#define	GCLK_LOWLEVELNUM_251	((uint8_t)	251U)
#define	GCLK_LOWLEVELNUM_252	((uint8_t)	252U)
#define	GCLK_LOWLEVELNUM_253	((uint8_t)	253U)
#define	GCLK_LOWLEVELNUM_254	((uint8_t)	254U)
#define	GCLK_LOWLEVELNUM_255	((uint8_t)	255U)

/*LED COM_WorkNum*/
#define	LED_COM_WORKNUM_2	((uint8_t)	2U)
#define	LED_COM_WORKNUM_3	((uint8_t)	3U)
#define	LED_COM_WORKNUM_4	((uint8_t)	4U)
#define	LED_COM_WORKNUM_5	((uint8_t)	5U)
#define	LED_COM_WORKNUM_6	((uint8_t)	6U)
#define	LED_COM_WORKNUM_7	((uint8_t)	7U)
#define	LED_COM_WORKNUM_8	((uint8_t)	8U)
#define	LED_COM_WORKNUM_9	((uint8_t)	9U)
#define	LED_COM_WORKNUM_10	((uint8_t)	10U)


/*LED SEG_WorkCurrent*/
#define     LED_WORKCURRENT_5_MA          	 ((uint8_t) 0U)
#define     LED_WORKCURRENT_5_P5_MA          ((uint8_t) 1U)
#define     LED_WORKCURRENT_6_MA          	 ((uint8_t) 2U)
#define     LED_WORKCURRENT_6_P5_MA          ((uint8_t) 3U)
#define     LED_WORKCURRENT_7_MA          	 ((uint8_t) 4U)
#define     LED_WORKCURRENT_7_P5_MA          ((uint8_t) 5U)
#define     LED_WORKCURRENT_8_MA          	 ((uint8_t) 6U)
#define     LED_WORKCURRENT_8_P5_MA          ((uint8_t) 7U)
#define     LED_WORKCURRENT_9_MA          	 ((uint8_t) 8U)
#define     LED_WORKCURRENT_9_P5_MA          ((uint8_t) 9U)
#define     LED_WORKCURRENT_10_MA          	 ((uint8_t) 10U)
#define     LED_WORKCURRENT_10_P5_MA         ((uint8_t) 11U)
#define     LED_WORKCURRENT_11_MA          	 ((uint8_t) 12U)
#define     LED_WORKCURRENT_11_P5_MA         ((uint8_t) 13U)
#define     LED_WORKCURRENT_12_MA          	 ((uint8_t) 14U)
#define     LED_WORKCURRENT_12_P5_MA         ((uint8_t) 15U)
#define     LED_WORKCURRENT_13_MA          	 ((uint8_t) 16U)
#define     LED_WORKCURRENT_13_P5_MA         ((uint8_t) 17U)
#define     LED_WORKCURRENT_14_MA          	 ((uint8_t) 18U)
#define     LED_WORKCURRENT_14_P5_MA         ((uint8_t) 19U)
#define     LED_WORKCURRENT_15_MA          	 ((uint8_t) 20U)
#define     LED_WORKCURRENT_15_P5_MA         ((uint8_t) 21U)
#define     LED_WORKCURRENT_16_MA          	 ((uint8_t) 22U)
#define     LED_WORKCURRENT_16_P5_MA         ((uint8_t) 23U)
#define     LED_WORKCURRENT_17_MA          	 ((uint8_t) 24U)
#define     LED_WORKCURRENT_17_P5_MA         ((uint8_t) 25U)
#define     LED_WORKCURRENT_18_MA          	 ((uint8_t) 26U)
#define     LED_WORKCURRENT_18_P5_MA         ((uint8_t) 27U)
#define     LED_WORKCURRENT_19_MA          	 ((uint8_t) 28U)
#define     LED_WORKCURRENT_19_P5_MA         ((uint8_t) 29U)
#define     LED_WORKCURRENT_20_MA          	 ((uint8_t) 30U)

/*LED Scan_Mode*/
#define     LED_SCAN_MODE_8SEG_8SEG          ((uint8_t) 0U)
#define     LED_SCAN_MODE_16SEG              ((uint8_t) 1U)

/*LED Work_Mode*/
#define     LED_WORK_MODE_DISABLE             ((uint8_t) 0U)
#define     LED_WORK_MODE_ENABLE              ((uint8_t) 1U)

/*LED Cur_ScanCOM*/
#define     LED_CUR_SCANCOM_COM0              ((uint8_t) 0U)
#define     LED_CUR_SCANCOM_COM1              ((uint8_t) 1U)
#define     LED_CUR_SCANCOM_COM2              ((uint8_t) 2U)
#define     LED_CUR_SCANCOM_COM3              ((uint8_t) 3U)
#define     LED_CUR_SCANCOM_COM4              ((uint8_t) 4U)
#define     LED_CUR_SCANCOM_COM5              ((uint8_t) 5U)
#define     LED_CUR_SCANCOM_COM6              ((uint8_t) 6U)
#define     LED_CUR_SCANCOM_COM7              ((uint8_t) 7U)
#define     LED_CUR_SCANCOM_COM8              ((uint8_t) 8U)
#define     LED_CUR_SCANCOM_COM9              ((uint8_t) 9U)
#define     LED_CUR_SCANCOM_SHUTDOWM          ((uint8_t) 0x0FU)


#define     PROTOCOL_HEAD_CMD                ((uint16_t)0x8000U )
#define     PROTOCOL_BASE_CMD                ((uint16_t)0x0000U )
#define     PROTOCOL_TRIM_CMD                ((uint16_t)0x1000U )
#define     PROTOCOL_GLOBAL_CMD              ((uint16_t)0x2000U )
#define     PROTOCOL_SEG_DUTY_CMD            ((uint16_t)0x3000U )

/*LED Base Command Definition*/
#define     LED_CMD_COM_FORCE_SHUTDOWM       ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_BASE_CMD | 0x0000U))
#define     LED_CMD_COM_SCAN_RESET           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_BASE_CMD | 0x0100U))
#define     LED_CMD_COM_SCAN_ON              ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_BASE_CMD | 0x0200U))
#define     LED_CMD_READ_REG                 ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_BASE_CMD | 0x0300U))
#define     LED_CMD_READ_OFF                 ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_BASE_CMD | 0x0400U))
#define     LED_CMD_CHIP_WAKEUP              ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_BASE_CMD | 0x0700U))

/*LED Triming Command Definition*/
#define     LED_CMD_SEG_TRIM_REG0_WRITE      ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_TRIM_CMD | 0x0000U))
#define     LED_CMD_SEG_TRIM_REG1_WRITE      ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_TRIM_CMD | 0x0100U))
#define     LED_CMD_SEG_TRIM_REG2_WRITE      ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_TRIM_CMD | 0x0200U))
#define     LED_CMD_SEG_TRIM_REG3_WRITE      ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_TRIM_CMD | 0x0300U))
#define     LED_CMD_SEG_TRIM_REG4_WRITE      ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_TRIM_CMD | 0x0400U))
#define     LED_CMD_SEG_TRIM_REG5_WRITE      ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_TRIM_CMD | 0x0500U))
#define     LED_CMD_RSET_TRIM_REG_WRITE      ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_TRIM_CMD | 0x0600U))
#define     LED_CMD_BG_TRIM_REG_WRITE        ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_TRIM_CMD | 0x0700U))

/*LED Global Command Definition*/
#define     LED_CMD_SEG_CUR_DISP_MODE_CHIP_EN      ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_GLOBAL_CMD | 0x0000U))
#define     LED_CMD_COM_NUM_COM0_1_WORK_STATUS     ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_GLOBAL_CMD | 0x0100U))
#define     LED_CMD_COM2_9_WORK_STATUS             ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_GLOBAL_CMD | 0x0200U))
#define     LED_CMD_SEG0_7_WORK_STATUS             ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_GLOBAL_CMD | 0x0300U))
#define     LED_CMD_SEG8_15_WORK_STATUS            ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_GLOBAL_CMD | 0x0400U))

/*LED Seg Duty Data Command Definition*/
#define     LED_CMD_SEG0_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0000U))
#define     LED_CMD_SEG1_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0100U))
#define     LED_CMD_SEG2_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0200U))
#define     LED_CMD_SEG3_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0300U))
#define     LED_CMD_SEG4_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0400U))
#define     LED_CMD_SEG5_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0500U))
#define     LED_CMD_SEG6_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0600U))
#define     LED_CMD_SEG7_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0700U))
#define     LED_CMD_SEG8_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0800U))
#define     LED_CMD_SEG9_DUTY_DATA           ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0900U))
#define     LED_CMD_SEG10_DUTY_DATA          ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0A00U))
#define     LED_CMD_SEG11_DUTY_DATA          ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0B00U))
#define     LED_CMD_SEG12_DUTY_DATA          ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0C00U))
#define     LED_CMD_SEG13_DUTY_DATA          ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0D00U))
#define     LED_CMD_SEG14_DUTY_DATA          ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0E00U))
#define     LED_CMD_SEG15_DUTY_DATA          ((uint16_t)(PROTOCOL_HEAD_CMD | PROTOCOL_SEG_DUTY_CMD | 0x0F00U))

/*LED GCLK Prescale Definition*/
#define     LED_GCLK_DIV1                    RCC_LEDCLK_DIV1   
#define     LED_GCLK_DIV32                   RCC_LEDCLK_DIV32   
#define     LED_GCLK_DIV40                   RCC_LEDCLK_DIV40   
#define     LED_GCLK_DIV50                   RCC_LEDCLK_DIV50   
#define     LED_GCLK_DIV64                   RCC_LEDCLK_DIV64   
#define     LED_GCLK_DIV80                   RCC_LEDCLK_DIV80   
#define     LED_GCLK_DIV160                  RCC_LEDCLK_DIV160   

/*LED GCLK clock source Definition*/
#define     LED_GCLK_SRC_HSI                 RCC_LEDCLK_SRC_HSI   
#define     LED_GCLK_SRC_HSE                 RCC_LEDCLK_SRC_HSE  

#define     SPI_DUMMY_DATA                   (0xFFFFU)


#define     MAX_LED_SEG_ID                   (0x10U)
#define     MAX_LED_SEG_DUTY                 (0x003FU)
/* this value must be fix at 130 */
#define     LED_CLOCK_CYCLE_NUMBER           (0x82U) 
/* GCLK_MNUM Close */
#define     LED_MCLOCK_CYCLE_0               (0xFFU)
/* GCLK_NNUM Close */ 
#define     LED_NCLOCK_CYCLE_0               (0x01U) 

#define     NVR_ADDR_90H_TRIM                (0x1FFFF938U) 
#define     NVR_ADDR_91H_TRIM                (0x1FFFF940U) 
#define     NVR_ADDR_92H_TRIM                (0x1FFFF948U) 
#define     NVR_ADDR_93H_TRIM                (0x1FFFF950U) 
#define     NVR_ADDR_94H_TRIM                (0x1FFFF958U) 
#define     NVR_ADDR_95H_TRIM                (0x1FFFF960U) 
#define     NVR_ADDR_96H_TRIM                (0x1FFFF968U) 
#define     NVR_ADDR_97H_TRIM                (0x1FFFF970U)

#define		GCLK_ENABLE							ENABLE
#define		GCLK_DISABLE						DISABLE
#define		GCLKDMA_ENABLE						ENABLE
#define		GCLKDMA_DISABLE						DISABLE

/*** LED Macro Definition End ***/
                                            
                                            

/*** LED Driving Functions Declaration ***/
void LED_TrimGet(LED_InitType *led_status_get);
void LED_TrimLoad(LED_InitType *led_status_set);
void LED_DeInit(void);
void LED_InitStruct(LED_InitType* LED_InitStruct);
void LED_Init(LED_InitType* LED_InitStruct);
void LED_SEG_DutyDataSet(uint8_t SegId, uint8_t duty);

ErrorStatus Get_LED_Scan_COM_Status(uint8_t COM_num, uint8_t *com_scan_list);
void LED_Get_Cur_ScanCOM_List(uint8_t COM_num, uint8_t index, uint8_t *com_scan_list);
ErrorStatus LED_Check_Cur_ScanCOM(uint8_t COM_num, uint8_t index);
void LED_Get_RegData(LED_InitType *led_status_get);
uint16_t SPI_SendDataToLED(uint16_t data) ;

void LED_GclkConfiguration(uint32_t LED_clksrc, uint32_t LED_prescaler);
void LED_SpiInit(void);
void LED_RccConfiguration(void);
void LED_GpioConfiguration(uint8_t COM_WorkNum, uint16_t SEG_WorkStatus);
                                            
#ifdef __cplusplus
}
#endif

#endif /* __N32G05X_LED_H__ */


