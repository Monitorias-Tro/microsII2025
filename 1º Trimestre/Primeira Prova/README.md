# Prova

Prova aplicada no dia 23/04/2025, com duração de 2 horas e 30 minutos. Válida pelo primeiro trimestre do 4º ano.

---

## Funcionamento do Código

O objetivo principal é controlar o piscar de três LEDs (no meu caso, vermelho, amarelo e verde) com diferentes frequências, que podem ser configuradas dinamicamente pelo usuário por meio de dois botões físicos ou comandos via UART.

### Estrutura do Código

- Cada LED é representado por uma estrutura (`LEDTimer_t`) que armazena:
  - Um contador interno (`counter`)
  - O valor de reinício do contador (`kickoff`), relacionado à frequência desejada
  - O pino GPIO associado ao LED
  - O nome do LED para exibição no terminal

- O timer TIM10 é utilizado para gerar interrupções periódicas e controlar o tempo de piscar dos LEDs. Cada vez que a interrupção ocorre:
  - O contador de cada LED ativo é decrementado.
  - Quando o contador atinge zero, o LED correspondente tem seu estado alternado (toggle), e o contador é reiniciado com o valor `kickoff`.

- A frequência de piscar de cada LED é determinada pela variável de controle `ctrl`, com os seguintes significados:

| Ctrl | Frequência (Hz) | Período de toggle (Hz) | Kickoff |
|------|------------------|------------------------|---------|
| 0    | Desligado        | -                      | -1      |
| 1    | 2 Hz             | 4 Hz                   | 260     |
| 2    | 5 Hz             | 10 Hz                  | 104     |
| 3    | 13 Hz            | 26 Hz                  | 40      |
| 4    | 16 Hz            | 32 Hz                  | 32      |

> **Nota:** O timer TIM10 foi configurado para gerar interrupções com frequência de aproximadamente 1041 Hz (ou um período de ~960,7 μs). Os valores de `kickoff` foram calculados com base nisso. Por exemplo, para gerar um período de toggle de 4Hz deve-se alternar o estado do LED a cada 260 interrupções do TIM10.

---

## Interação com o Sistema

### Botões Físicos

- **Botão BF (Botão de Frequência):** Incrementa o valor de `ctrl`, modificando a frequência de piscar do LED atualmente selecionado.
- **Botão BS (Botão de Seleção):** Alterna o índice do LED selecionado entre as cores (vermelho, amarelo, verde).

Ambos os botões contam com lógica de debounce por software e espera pela liberação do botão antes de aceitar novas entradas.

### UART

O projeto também permite controle via interface serial. O usuário pode enviar dois caracteres:

1. O primeiro indica qual LED deve ser configurado:
   - `'r'` para vermelho
   - `'y'` para amarelo
   - `'g'` para verde
2. O segundo caractere é um número de `'0'` a `'4'`, indicando o valor de `ctrl` (frequência desejada).

Exemplo: Enviar `"r3"` ajusta o LED vermelho para piscar a 13 Hz.

Após o envio, o sistema responde com um eco da mensagem e mensagens de validação ou erro conforme apropriado.
