# Controle de Intensidade de LEDs RGB via PWM e UART - Exercício 2

Este exercício demonstra o controle da intensidade de LEDs RGB conectados ao microcontrolador utilizando **PWM** por meio do **Timer 8** e recebendo comandos via **UART (USART2)**.

---

## Funcionamento

O usuário envia, via terminal serial, uma mensagem com **4 caracteres**:

- 1ª letra: qual LED ajustar:
  - `'r'` para vermelho (TIM8_CH1, PC6)
  - `'g'` para verde (TIM8_CH2, PC7)
  - `'b'` para azul (TIM8_CH3, PC8)
- 3 dígitos: valor da intensidade (de `000` a `255`, por exemplo)

**Exemplos válidos:**

- `r127` → seta intensidade do LED vermelho para 127  
- `g255` → brilho máximo do verde  
- `b000` → desliga o LED azul

---

## Etapas de Processamento no Código

Quando o microcontrolador recebe um comando pela UART (ex: `r127`), as seguintes etapas ocorrem no código:

### 1. Eco do comando recebido

O vetor `rxData[4]` armazena os 4 bytes recebidos pela UART por meio da função `HAL_UART_Receive_IT`. Assim que os dados chegam, a função de callback `HAL_UART_RxCpltCallback` é chamada, ativando a flag `updateDutyCycle`.

Dentro do `main()`, o código aguarda essa flag. Assim que está ativa, o conteúdo de `rxData` é imediatamente transmitido de volta via `HAL_UART_Transmit`, seguido de uma quebra de linha (`\n\r`). Isso serve apenas como confirmação para o usuário de que os dados foram corretamente recebidos.

---

### 2. Conversão dos 3 últimos caracteres em número inteiro

Após o eco, o código extrai os três últimos caracteres de `rxData` (índices 1, 2 e 3), que representam os dígitos da intensidade. Esses caracteres ASCII são convertidos para inteiros subtraindo `'0'` (por exemplo, `'7' - '0' = 7`) e usados para montar o valor decimal com a fórmula:

```c
intensity += rxData[i] * pow(10, 3 - i); // para i de 1 a 3
```

Exemplos:

- `'0''0''3'` → 3  
- `'1''2''7'` → 127  
- `'2''5''5'` → 255  

O valor final é armazenado na variável `intensity`.

---

### 3. Atualização do registrador CCR do Timer

O primeiro caractere (`rxData[0]`) define qual canal do Timer 8 será atualizado:

- `'r'`: TIM8_CH1 → LED vermelho → `CCR1`
- `'g'`: TIM8_CH2 → LED verde → `CCR2`
- `'b'`: TIM8_CH3 → LED azul → `CCR3`

O código executa os seguintes passos:

1. O PWM do canal correspondente é **parado** com `HAL_TIM_PWM_Stop(...)`.
2. O valor de `intensity` é atribuído ao registrador `CCR` correspondente diretamente (`TIM8->CCR1`, `CCR2` ou `CCR3`).
3. O PWM é **reiniciado** com `HAL_TIM_PWM_Start(...)`.

Essa sequência garante que o novo valor de razão cíclica seja aplicado corretamente, evitando glitches visuais.

---

### 4. Atualização do Duty Cycle

Ao atualizar o valor de `CCR`, o tempo em que a saída permanece em nível alto durante um ciclo do PWM é modificado.

O duty cycle é dado por:

```c
duty_cycle = CCR / ARR
```

Se, por exemplo, `ARR = 255`:

- `CCR = 127` → duty cycle ≈ 50%  
- `CCR = 255` → duty cycle = 100%  
- `CCR = 0` → duty cycle = 0%

O brilho do LED é proporcional ao valor de `CCR`. Quanto maior, mais tempo o sinal permanece em nível alto, resultando em maior brilho.

---

## Configurações de Hardware

- **Timer 8:**
  - Canal 1 (CH1): LED Vermelho (PC6)
  - Canal 2 (CH2): LED Verde (PC7)
  - Canal 3 (CH3): LED Azul (PC8)
- Frequência do PWM definida indiretamente via `Prescaler` e `ARR` (valores definidos por `PSC_200Hz` e `ARR_RGB`).

---

## Observações

- O código usa **interrupções de recepção UART** (`HAL_UART_Receive_IT`) e a função de callback `HAL_UART_RxCpltCallback()` para sinalizar a chegada de novos dados.
- A modulação PWM é controlada manualmente por paradas e reinícios (`HAL_TIM_PWM_Stop/Start`) para aplicar os novos valores.
