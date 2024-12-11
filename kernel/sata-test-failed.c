#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define AHCI_BASE 0x400000

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_CMD_ST 0x0001
#define HBA_PORT_CMD_FRE 0x0010
#define HBA_PORT_CMD_FR 0x4000
#define HBA_PORT_CMD_CR 0x8000

#define PORT_START 0x400
#define PORT_REG_COMMAND 0x18
#define CMD_PORT_RESET 0x08
#define BIT_PORT_CR 0x8000
#define PORT_REG_TASK_FILE_DATA 0x20
#define CMD_PORT_FRE 0x10

void write_port_register(uint32_t port, uint32_t reg, uint32_t value) {
    *(volatile uint32_t*)(port + reg) = value;
}

uint32_t read_port_register(uint32_t port, uint32_t reg) {
    return *(volatile uint32_t*)(port + reg);
}

void configure_port(uint32_t port) {
    write_port_register(port, PORT_REG_COMMAND, CMD_PORT_RESET);
    write_port_register(port, PORT_REG_COMMAND, CMD_PORT_RESET | BIT_PORT_CR);
}

void start_port(uint32_t port) {
    write_port_register(port, PORT_REG_COMMAND, CMD_PORT_FRE | CMD_PORT_RESET | BIT_PORT_CR);
}

void probe_sata_port(uint32_t port) {
    // Probe SATA port implementation
    write_port_register(port, PORT_REG_TASK_FILE_DATA, CMD_PORT_FRE);
}

/*
void configure_port(uint32_t port) {
    write_port_register(port, PORT_REG_COMMAND, CMD_PORT_RESET);
}

void start_port(uint32_t port) {
    write_port_register(port, PORT_REG_COMMAND, BIT_PORT_CR);
}

void init_ahci_sata_port(uint32_t port) {
    configure_port(port);
    start_port(port);
}

void probe_sata_port(uint32_t port) {
    write_port_register(port, PORT_REG_TASK_FILE_DATA, CMD_PORT_FRE);
}
*/

/*
void init_ahci_sata_port(uint32_t port) {
    configure_port(port);
    start_port(port);
}

typedef struct {
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t rsv0;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sctl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t fbs;
    uint32_t rsv1[11];
    uint32_t vendor[4];
} hba_port_t;

void write_port_register(hba_port_t *port, uint32_t reg, uint32_t val) {
    switch (reg) {
        case 0:
            port->clb = val;
            break;
        case 1:
            port->clbu = val;
            break;
        case 2:
            port->fb = val;
            break;
        case 3:
            port->fbu = val;
            break;
        case 4:
            port->is = val;
            break;
        case 5:
            port->ie = val;
            break;
        case 6:
            port->cmd = val;
            break;
        default:
            break;
    }
}

uint32_t read_port_register(hba_port_t *port, uint32_t reg) {
    switch (reg) {
        case 0:
            return port->clb;
        case 1:
            return port->clbu;
        case 2:
            return port->fb;
        case 3:
            return port->fbu;
        case 4:
            return port->is;
        case 5:
            return port->ie;
        case 6:
            return port->cmd;
        default:
            return 0;
    }
}

void configure_port(hba_port_t *port) {
    port->cmd |= HBA_PORT_CMD_ST;
    port->cmd |= HBA_PORT_CMD_FRE;
    port->cmd |= HBA_PORT_CMD_FR;
    port->cmd |= HBA_PORT_CMD_CR;
}

void start_port(hba_port_t *port) {
    port->cmd |= HBA_PORT_CMD_ST;
}

void init_ahci_sata_port(hba_port_t *port) {
    // Initialization steps for AHCI SATA port
    configure_port(port);
    start_port(port);
}
*/

typedef struct {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_ports;
    uint32_t em_loc;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;
} ahci_regs_t;

ahci_regs_t *ahci = (ahci_regs_t *)AHCI_BASE;

void list_sata_devices() {
    // List SATA devices implementation
}

void init_ahci_sata_port() {
    // Code to initialize AHCI SATA port goes here
    // Example code snippet:
    
    // Reset the port
    write_port_register(PORT_START, PORT_REG_COMMAND, CMD_PORT_RESET);
    
    // Wait for port to be ready
    while (read_port_register(PORT_START, PORT_REG_COMMAND) & CMD_PORT_RESET);
    
    // Enable FIS receive
    write_port_register(PORT_START, PORT_REG_COMMAND, CMD_PORT_FRE);
    
    // Check if the port is ready
    while (!(read_port_register(PORT_START, PORT_REG_TASK_FILE_DATA) & BIT_PORT_CR));
    
    // Configure the port
    configure_port(PORT_START);
    
    // Start the port
    start_port(PORT_START);
}


int main() {
	
    // Init AHCI SATA ports
	init_ahci_sata_port();
	
    // Read AHCI registers
    uint32_t cap_register = ahci->cap;
    uint32_t pi_register = ahci->pi;

    // Probe SATA ports
    probe_sata_port(0);
    probe_sata_port(1);

    // List SATA devices
    list_sata_devices();

    return 0;
}
