# BONBLOOP

**BONBLOOP** é um firmware para o **M-VAVE FM-1**, feito a partir do [SLOOP 2.8.5](https://github.com/isod89/sloop-fm1)
(que por sua vez é baseado no [Felucca](https://github.com/hugelton/Felucca)). Ele é **idêntico ao SLOOP** —
mesmos 4 tracks, sequenciador, bateria, efeitos, projetos, editor web — e acrescenta o **MODO SYNTH**: um
sintetizador de **3 osciladores** (wavetables e formas analógicas, com **reverse**), filtro, envelope, LFO e uma
cadeia de efeitos própria (drive estilo Decapitator, transient shaper, phaser, chorus, delay, reverb).

Licença: GPL-3.0-only, como o SLOOP e o Felucca (ver `LICENSE` e `LICENSING.md`).

**Versão 1.1** — 3 osciladores, 16 formas de onda, REV, efeitos do synth, e o modo agora segura a tela.

---

## O MODO SYNTH

### Entrar e sair

| Gesto | O que faz |
| --- | --- |
| **Segure ENV ~0,7 s** | Liga o MODO SYNTH. A tela mostra **SYNTH MODE** e abre a página **OSC 1**. |
| **Segure ENV de novo** | Desliga e volta ao SLOOP normal (página ENV). O som do track continua. |

Ao ligar:

- o **track selecionado** passa a tocar o engine **WAVE** (o primeiro som dele, *WT BASIC*), a menos que já
  esteja em WAVE — aí o som é mantido;
- se a **bateria** (track 4) estiver selecionada, o modo pega o **track 1**;
- o cabeçalho mostra o selo **SYNTH** na cor do track e a linha sob ele fica colorida.

Um toque rápido em ENV continua abrindo a página ENV, como sempre; só **segurar** liga e desliga o modo.

### O modo segura a tela (1.1)

Dentro do MODO SYNTH você **não sai sem querer**:

- **ARP, SEQ, SEL, GLO e SAVE** tocados rapidamente não abrem nada (a tela avisa *HOLD ENV: SYNTH OFF*).
  **Segurados**, as camadas deles funcionam como sempre (passos no SEQ, mute/solo no GLO, seções no SAVE, roll
  no ARP...) e, ao soltar, você volta ao synth.
- **HOME** volta à página OSC 1 (segurar HOME ainda abre o menu).
- **ALGORITHM** (troca de track) só passa pelos tracks de synth 1 a 3.
- Se algo trocar a tela (carregar um projeto, o editor web), ela volta ao synth.
- PLAY, REC, OCT−/OCT+, o teclado e o sequenciador funcionam normalmente.

### Os botões dentro do modo

Quatro botões abrem os grupos de páginas do synth. Tocar de novo no mesmo botão passa para a próxima página do
grupo; o **SELECT** percorre as 18 páginas.

| Botão | Página | KNOB 1 · KNOB 2 · KNOB 3 · KNOB 4 |
| --- | --- | --- |
| **EDIT** | OSC 1 | WAVE · POS · LVL · REV |
| | OSC 2 | WAVE · POS · LVL · REV |
| | OSC 3 | WAVE · POS · LVL · REV |
| | TUNE | SEMI2 · FINE2 · SEMI3 · FINE3 |
| | VOICE | WARP · VCE (POLY/MONO/LEG/UNI) · DTUNE (abertura do unison) · GLD |
| **ENV** | ENV | ATK · DEC · SUS · REL |
| | ENV DEST | FLT · PIT · **POS** · SYNC |
| **LFO** | LFO | RATE · WAVE · PHS · FADE |
| | LFO 2 | SYNC · TRIG · POL |
| | LFO DEST | PIT · FLT · **POS** · AMP |
| **FX** | FILTER | CUT · RES · TYPE (LP/BP/HP) · FILT (filtro DJ do track) |
| | DRIVE | DST (quantidade) · STYLE (A E N T P) · TONE · PUNSH |
| | TRANSNT | ATTCK · SUSTN |
| | PHASER | MIX · RATE · DEPTH · FDBK |
| | INSERT | TYPE (PHASR, e também FLANG, CHOR, SOFT, HARD, FOLD, FUZZ, CRUSH) |
| | SENDS | CHO · DLY · REV (envios para o chorus, delay e reverb) |
| | DLY | tempo, feedback, cor e mix do delay (global) |
| | REV/CHO | tamanho e amortecimento do reverb, velocidade e profundidade do chorus (globais) |

- **PRESETS** (o encoder), numa página do synth: passa pelos 16 sons WAVE.
- No engine WAVE, os destinos **SHP** do envelope e do LFO aparecem como **POS**: movem a posição de **todos** os
  osciladores ao mesmo tempo (o *wavetable sweep*).
- Fora do modo, as páginas EDIT do SLOOP mostram, num track WAVE, os campos de verdade (OSC 1 e 2; filtro e OSC 3).

### A tela

As páginas OSC 1, 2 e 3 desenham **o próprio oscilador**: numa tabela, os 8 frames um atrás do outro (cinza) e, na
frente, na cor do track, a forma de onda **na posição que está tocando** (com a modulação, enquanto uma nota
soa); numa forma analógica, dois ciclos dela (no SQR, com a largura de pulso do POS).

---

## O engine WAVE

### Os 3 osciladores

Cada oscilador tem:

| Campo | O que faz |
| --- | --- |
| **WAVE** | `OFF`, `SIN`, `TRI`, `SAW`, `SQR`, `NOISE`, ou uma das 10 tabelas (abaixo). |
| **POS** | Numa tabela: o frame (1 a 8, lido entre dois). No `SQR`: a largura do pulso (50 % → 6 %). No `NOISE`: a cor (branco → escuro). |
| **LVL** | O volume dele na mistura. |
| **REV** | Toca o oscilador **ao contrário** (abaixo). |
| **SEMI / FINE** | Só OSC 2 e 3: afinação de −24 a +24 semitons e −16 a +15 cents em relação ao OSC 1. |

`SAW` e `SQR` são formas analógicas sem aliasing (polyBLEP, como as do engine ANALOG): graves cheios. As tabelas
têm até 24 harmônicos — o caráter dos wavetables dos anos 80.

| Tabela | Do frame 1 ao 8 |
| --- | --- |
| **BASIC** | seno → triângulo → dente de serra → quadrada → pulso 25 % |
| **PWM** | pulso de 50 % até 5 % |
| **HARM** | órgão: um harmônico a mais a cada frame |
| **RESO** | um pico ressonante subindo por uma dente de serra |
| **VOCAL** | vogais A, A-E, E, I, I-O, O, U, ER |
| **DIGI** | espectros digitais estilo PPG |
| **FOLD** | uma senoide dobrada (wavefolder) cada vez mais forte |
| **FM** | seno modulado a 3×, índice subindo |
| **BELL** | parciais esparsos, vítreos |
| **CHIP** | pulsos e degraus de 4 bits (com aliasing de propósito) |

**WARP** (página VOICE) dobra a fase de leitura de todas as tabelas: a primeira metade do ciclo é espremida (mais
brilho, um "PWM" em qualquer tabela). O DC que isso criaria é compensado.

### REV — tocar ao contrário

Num sampler, tocar ao contrário transforma um som que decai num som que **cresce e para de repente**. Num
oscilador de um só ciclo, ler a onda de trás para frente quase não muda o timbre — o que o ouvido percebe como
"reverse" é a **evolução da nota ao contrário**. Por isso o REV faz as duas coisas:

1. lê o ciclo do oscilador de trás para frente;
2. inverte o decaimento: o oscilador **começa em −40 dB e cresce** no tempo do **DEC** até o volume cheio, e aí
   **para** (cai para o nível do SUS — com SUS 0, silêncio, como o fim de um sample invertido). Ao soltar a
   tecla, ele some junto com os outros (no tempo do REL).

Com um oscilador normal e outro em REV, você tem o clássico "pluck + reverse". O som **REV SWELL** mostra isso.

### Filtro

**CUT** e **RES** (o filtro SVF do SLOOP), **TYPE** passa-baixa, passa-banda ou passa-alta, com 50 % de key
tracking; o envelope e o LFO (→ FLT) movem o CUT. Na mesma página, o **FILT** é o filtro DJ do track (passa-baixa
para a esquerda, passa-alta para a direita), depois de todos os efeitos de insert. O CC 74 de um teclado MIDI
move o filtro DJ.

### Efeitos do synth

| Efeito | De onde vem |
| --- | --- |
| **DRIVE** | Novo, estilo **Decapitator**: num track WAVE, o DIST do SLOOP vira um saturador de 5 estilos — **A** fita (suave, simétrica), **E** válvula (assimétrica: harmônicos pares), **N** transformador (mais ganho), **T** triodo (mais sujo), **P** pentodo (meio corte duro) —, **TONE** (escuro ↔ aberto) e **PUNSH** (+12 dB de drive). |
| **TRANSIENT** | Novo: **ATTCK** sobe até +12 dB os primeiros ~25 ms de cada nota (ou, negativo, faz a nota entrar suave); **SUSTN** sobe até +6 dB o corpo da nota (ou, negativo, deixa só o ataque). |
| **PHASER** | O INSERT do SLOOP, já em PHASR nos sons WAVE (MIX 0 = desligado). Na página INSERT dá para trocar por FLANG, CHOR ou um dos drives. |
| **CHORUS, DELAY, REVERB** | Os envios do SLOOP (página SENDS) e os ajustes globais (DLY, REV/CHO). |
| **FILTER** | O filtro da voz (acima) e o filtro DJ do track. |

A ordem: osciladores → filtro → transient (por nota) → DRIVE → PHASER/INSERT → filtro DJ → volume/pan → envios
(chorus, delay, reverb).

### Sem gastar flash nem RAM

O flash do FM-1 está quase cheio, então as formas de onda **não ficam no flash**: cada tabela escolhida é
**calculada** dentro da "arena" que os engines GRAIN/FM6/PHYS já compartilham por track — uma tabela para cada
oscilador, 8 frames de 128 amostras —, aos poucos no áudio (~23 ms por tabela), substituindo frame a frame a
que está tocando; trocar de tabela nunca deixa o som mudo. Nenhum byte de RAM a mais (um `_Static_assert`
garante isso).

Os 25 parâmetros do synth cabem **empacotados nos 8 parâmetros do engine** (16 bits cada): projetos, presets de
usuário e p-locks os guardam como sempre. As páginas do synth editam cada campo; o **editor web** mostra os 8
números brutos e não edita esses campos.

Custo de CPU, medido no PC: 1 oscilador de tabela ≈ 0,9× uma voz do ANALOG; 3 osciladores de tabela ≈ 1,5×.

### Os 16 sons

WT BASIC, WT SWEEP, PPG PAD, VOX SWEEP, WT CHOIR, PWM STRNG, WT ORGAN, WT PLUCK, WT BELLS, RESO BASS,
DIGI BASS, FM GROWL, FOLD LEAD, SUPER WAVE, CHIP WAVE e **REV SWELL** — no navegador de PRESETS por categoria
(BASS, ORGN, PAD, LEAD, PLCK), com volume equalizado aos do ANALOG (`WAVE_TRIM`, a ±2 dB).

---

## Compatibilidade

- Projetos, presets de usuário, samples e ajustes do SLOOP continuam valendo.
- Sons WAVE salvos com o BONBLOOP 1.0 tocam diferente no 1.1 (os parâmetros do engine mudaram de significado).
- Um projeto com um track em WAVE (engine 12), aberto num SLOOP oficial, cai no som padrão daquele track.
- O editor web lê a lista de engines do próprio aparelho, então o WAVE aparece nele (com os números brutos).
- A identidade do pacote continua `FM-1_900` (build de desenvolvimento), instalável pelo mesmo instalador.

## Compilar e instalar

Igual ao SLOOP (ver [BUILDING.md](BUILDING.md)): toolchain JieLi (Linux x86-64, via WSL ou Docker) e três
arquivos do SDK AC79. No Windows com WSL:

```
python tools/build_windows.py --distro Ubuntu --toolchain /root/.jieli/toolchain --sdk build/deps/ac79 --slice 0
python tools/fm1_install.py build/felucca.fwsc
```

Tamanho do 1.1: 575.972 de 581.564 bytes de flash (5,5 KB livres); RAM e pool iguais aos do SLOOP.

## Testes

`tests/run_tests.sh` roda tudo, incluindo os dois testes novos:

- `tests/wave_test.c` — o engine sozinho: os 24 campos empacotados, todas as tabelas em cada oscilador, todos os
  presets (limites, DC, volume), as formas varridas, REV, TRANSIENT, DRIVE, extremos, custo de CPU; grava
  `wave_presets.wav` e `wave_sweep.wav` para ouvir.
- `tests/synth_mode_test.c` — a interface real: segurar ENV, os 4 grupos e as 18 páginas, cada knob mexendo
  um campo só, os 3 osciladores tocando, os efeitos, o modo segurando a tela (ARP/SEQ/SEL/GLO/SAVE/HOME/ALGO),
  sair, a bateria selecionada; grava as telas em `synth-*.ppm`.

A suíte de regressão (`tests/regress.c`) confirma que **nenhum som do SLOOP mudou**: as 182 renderizações batem
bit a bit com o `golden.txt`; as 16 do WAVE foram atualizadas.

## Créditos

- **SLOOP** — isod89 e colaboradores (GPL-3.0).
- **Felucca** — Leo Kuroshita / Hügelton Instruments (GPL-3.0).
- **Decapitator** é da Soundtoys; o DRIVE do BONBLOOP só se inspira nos estilos dele.
- **BONBLOOP** — o MODO SYNTH e o engine WAVE.
