#include "UART.h"
#include "stm32F4xx_usart.h"
using namespace HAL;
namespace STM32F4
{
	class F4UART:public UART
	{
	#define TX_BUFFER_SIZE 1024
	#define RX_BUFFER_SIZE 512
	private:
		USART_TypeDef * USARTx;
		int baudrate;
		//DMA var:
		uint32_t DMA_Channel;
		uint8_t DMA_Stream_IRQ;
		DMA_Stream_TypeDef* DMAy_Streamx;
		uint32_t RCC_AHB1Periph;
		//USART buffer var:
		int start;
		int end;
		int tx_start;
		int tx_end;
		int ongoing_tx_size;
		int dma_running; 
		int end_sentence;
		char tx_buffer[TX_BUFFER_SIZE];
		char buffer[RX_BUFFER_SIZE];		// circular buffer
	public:
		F4UART(USART_TypeDef * USARTx);
		~F4UART(){};
		virtual int set_baudrate(int baudrate);
		virtual int write(const void *data, int count);
		virtual int read(void *data, int max_count);
		virtual void dma_init();
		virtual void DMA1_Steam4_IRQHandler();
		virtual int dma_handle_queue();
		virtual void UART4_IRQHandler(void);
		virtual int UART4_SendPacket(const void *buf, int size);
		virtual int UART4_ReadPacket(void *out, int maxsize);

	};
}