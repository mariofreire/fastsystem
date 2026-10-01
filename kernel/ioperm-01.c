#include <stdio.h>
#include <sys/io.h> // Necessário para ioperm, inb e outb

#define PORT_ADDRESS 0x3F8 // Exemplo: Porta serial COM1

int main() {
    // Solicita permissão para 1 porta no endereço 0x3F8
    if (ioperm(PORT_ADDRESS, 1, 1) < 0) {
        perror("Erro no ioperm (você é root?)");
        return 1;
    }

    // Agora o processo pode usar instruções de E/S diretamente
    unsigned char value = inb(PORT_ADDRESS); // Lê da porta
    outb(value, PORT_ADDRESS);               // Escreve na porta

    // Opcional: revoga a permissão ao fechar
    ioperm(PORT_ADDRESS, 1, 0);

    return 0;
}
