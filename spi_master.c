#include "spi_master.h"
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "Driver_SPI.h"

#define SS_PIN 16 // Assuming the slave select pin is connected to P0.16
extern ARM_DRIVER_SPI Driver_SPI2;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI2;

uint8_t SPI_Rx(void) {
    uint8_t data;
    SPIdrv->Receive(&data, sizeof(data));
	ARM_SPI_STATUS state = SPIdrv->GetStatus();
    do {
        state = SPIdrv->GetStatus();
    }while(state.busy == 1);
	return data;
}

void SPI_Tx(uint8_t data) {
    //SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE); // to chyba mozna wykomentowac
    SPIdrv->Send(&data, 1);
    ARM_SPI_STATUS state = SPIdrv->GetStatus();
    do {
        state = SPIdrv->GetStatus();
    } while(state.busy == 1);
}

uint8_t SPI_TxRx(uint8_t data) {
    SPI_Tx(data);
    return SPI_Rx();
}

//unused
void SPI_RxBuffer(uint8_t *buffer, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = SPI_Rx();
    }
}
//unused
void SPI_TxBuffer(uint8_t *buffer, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        SPI_Tx(buffer[i]);
    }
}

void SPI_Select(void) {
    // Set SS_PIN low
    SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
}

void SPI_Deselect(void) {
    // Set SS_PIN high
	SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
}
