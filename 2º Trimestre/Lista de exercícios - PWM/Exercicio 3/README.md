# Variação de Intensidade RGB com PWM em Tempo Real - Exercício 3

Este exercício demonstra como gerar variações suaves de brilho em LEDs RGB utilizando **PWM** no **Timer 8**, com atualizações temporizadas por **interrupções do Timer 10**. A intensidade de cada LED segue uma função cossenoidal defasada no tempo, simulando um efeito de transição contínua de cor.

---

## Funcionamento Geral

O programa varia automaticamente as intensidades dos LEDs RGB com base no tempo, sem interação do usuário.

- O **Timer 10** gera interrupções a cada 10 ms.
- A cada interrupção, a variável `time` é incrementada.
- O `main()` verifica uma flag (`updateFlag`) e, quando ela é ativada, chama a função `updateIntensities()`.
- Essa função atualiza os registradores `CCR` dos canais do Timer 8 com base em funções trigonométricas que dependem do tempo.

---

## Detalhamento do Funcionamento

### 1. Contador de tempo (baseado em Timer 10)

- O Timer 10 está configurado para gerar interrupções a cada 10 milissegundos.
- A função `HAL_TIM_PeriodElapsedCallback()` é chamada automaticamente, e dentro dela:
  ```c
  time++;          // tempo em centésimos de segundo (cs)
  updateFlag = 1;  // sinaliza que está na hora de atualizar o PWM
  ```

---

### 2. Loop principal (main loop)

No `while(1)` do `main()`, o código espera pela flag `updateFlag` ser ativada:

```c
while(!updateFlag);  // espera o próximo ciclo de atualização
updateIntensities(); // atualiza os três canais PWM
updateFlag = 0;      // limpa a flag
```

---

### 3. Atualização dos sinais PWM

A função `updateIntensities()` calcula novos valores para os registradores `CCR` dos três canais do Timer 8 (TIM8_CH1, CH2, CH3). Cada canal representa uma cor (vermelho, verde, azul). O procedimento é:

1. Parar o canal PWM com `HAL_TIM_PWM_Stop(...)`
2. Atualizar o valor de `CCR` com a nova intensidade
3. Reativar o canal com `HAL_TIM_PWM_Start(...)`

Isso é feito para cada cor independentemente:

```c
TIM8->CCR1 = intensityFunc('r');  // vermelho
TIM8->CCR2 = intensityFunc('g');  // verde
TIM8->CCR3 = intensityFunc('b');  // azul
```

---

### 4. Função de intensidade

A função `intensityFunc(char colour)` define como o brilho do LED varia ao longo do tempo. Ela usa uma equação baseada em **cosseno** para gerar uma onda suave entre 0 e 255:

```c
aux = (M_PI / 250) * time;   // frequência base (tempo em cs)
```

- Para o **verde**, a frequência é aumentada com um coeficiente multiplicador:
  ```c
  aux *= greenFrequencyCoeficient; // 1.1
  ```
- Para o **azul**, a frequência é reduzida:
  ```c
  aux *= blueFrequencyCoeficient;  // 0.8
  ```

A fórmula completa é:

```c
aux = -127.5 * cos(aux);
aux += 127.5;
```

Com isso, a saída oscila entre 0 e 255 em forma de onda suave, ideal para controlar brilho com transição contínua.

---

## Mapeamento entre Canais e Cores

- `TIM8_CH1` → LED Vermelho (PC6)
- `TIM8_CH2` → LED Verde (PC7)
- `TIM8_CH3` → LED Azul (PC8)

---

## Observações

- Nenhuma interação do usuário é necessária neste exercício.
- O código mostra como usar **dois timers em conjunto**:
  - Um para gerar PWM (Timer 8)
  - Outro para controle de tempo com interrupção (Timer 10)
- Cada cor pode ter sua própria frequência de oscilação, simulando **modulação independente**.
