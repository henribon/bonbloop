# BONBLOOP

**BONBLOOP** é um firmware para o **M-VAVE FM-1**, feito a partir do [SLOOP 2.8.5](https://github.com/isod89/sloop-fm1)
(que por sua vez é baseado no [Felucca](https://github.com/hugelton/Felucca)). Ele é **idêntico ao SLOOP** —
mesmos 4 tracks, sequenciador, bateria, efeitos, projetos, editor web — e acrescenta o **MODO SYNTH**: um
sintetizador de **3 osciladores** (wavetables e formas analógicas, com **reverse**), filtro, envelope, LFO e uma
cadeia de efeitos própria (drive estilo Decapitator, phaser, filtro, chorus, delay, reverb).

Licença: GPL-3.0-only, como o SLOOP e o Felucca (ver `LICENSE` e `LICENSING.md`).

**Versão 1.5** — um **EQ de 4 bandas** com a curva na tela, como o de uma DAW (FX CHAIN → EQ); o **REC grava em tempo
real** de fábrica (o QUANT da tela REC READY liga a quantização).

**1.4.2** — tercinas do REPEAT gravadas no tempo exato, na grade de 1/16 (o micro-timing de cada passo),
sem mudar a grade do padrão.

**1.4.1** — tercinas do REPEAT gravadas no lugar; REPEAT com 1/4T e 1/2T; o **TRANSIENT saiu** (sobrava pouco
uso para a flash que ocupava: 424 bytes). Os presets WT PLUCK, WT BELLS e DIGI BASS, que o usavam, foram reajustados. E as caudas
já quase mudas de dois samples foram encurtadas (BASS: a partir de 0,8 s, abaixo de -27 dB; VIBES F3: a partir de
0,7 s, abaixo de -40 dB): 7 KB de flash livres para o que vem (o EQ do synth).

**1.4** — **REPEAT** na bateria (toque no ARP); o ARP com **1/2, 1/2T, 1/4T e 1/32T**; o delay com
**todas as tercinas** e um **MODE SYNC / MS**; as divisões em ordem musical no knob; o modo synth **começa num seno puro**.

**1.3.1** — correção: nas páginas OSC2 ENV e OSC3 ENV o desenho do envelope agora acompanha os knobs.

**1.3** — no MODO SYNTH não existe mais o envelope da voz: **cada oscilador tem o seu envelope completo**
(ATK · DEC · SUS · REL, igual ao do SLOOP).
(1.2: LFO por oscilador, cadeia de efeitos que você monta, DRIVE mais pesado, delay 1/2, 1/2T, livre em ms e FIX,
voicings de acorde no SEL, REC + SAVE para limpar o synth. 1.1: 3 osciladores, 16 formas de onda, REV, efeitos do
synth, o modo segura a tela.)

---

## O MODO SYNTH

### Entrar e sair

| Gesto | O que faz |
| --- | --- |
| **Segure ENV ~0,7 s** | Liga o MODO SYNTH. A tela mostra **SYNTH MODE** e abre a página **OSC 1**. |
| **Segure ENV de novo** | Desliga e volta ao SLOOP normal (página ENV). O som do track continua. |

Ao ligar:

- o **track selecionado** passa a tocar o engine **WAVE** no som **SINE** (1.4): só o OSC 1, numa senoide, OSC 2
  e 3 desligados, filtro aberto, sem efeitos e sem LFO — a menos que já esteja em WAVE: aí o som é mantido;
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
grupo; o **SELECT** percorre todas as páginas que estão à mostra.

| Botão | Página | KNOB 1 · KNOB 2 · KNOB 3 · KNOB 4 |
| --- | --- | --- |
| **EDIT** | OSC 1 | WAVE · POS · LVL · REV |
| | OSC 2 | WAVE · POS · LVL · REV |
| | OSC 3 | WAVE · POS · LVL · REV |
| | TUNE | SEMI2 · FINE2 · SEMI3 · FINE3 |
| | FILTER | CUT · RES · TYPE (LP/BP/HP) · FLT (quanto o envelope abre o filtro) |
| | VOICE | WARP · VCE (POLY/MONO/LEG/UNI) · DTUNE (abertura do unison) · GLD |
| **ENV** | OSC1 ENV | ATK · DEC · SUS · REL — **o envelope do OSC 1** (1.3) |
| | OSC2 ENV | ATK · DEC · SUS · REL — **o envelope do OSC 2** |
| | OSC3 ENV | ATK · DEC · SUS · REL — **o envelope do OSC 3** |
| | ENV DEST | FLT · PIT · **POS** · SYNC (o envelope do OSC 1 movendo filtro, pitch e POS; SYNC vale para os três) |
| **LFO** | LFO | RATE · WAVE · PHS · FADE |
| | LFO 2 | SYNC · TRIG · POL |
| | LFO DEST | PIT · FLT · **POS** · AMP |
| | OSC1 LFO · OSC2 LFO · OSC3 LFO | RATE · DEPTH · DEST (POS / PITCH / LVL) — **o LFO de cada oscilador** (1.2) |
| **FX** | FX CHAIN | SLOT1 · SLOT2 · SLOT3 · SLOT4 — **a sua cadeia de efeitos** (1.2) |
| | CHAIN 2 | SLOT5 · SLOT6 · SLOT7 |
| | DRIVE | DST · STYLE (A E N T P) · TONE · PUNSH |
| | PHASER | MIX · RATE · DEPTH · FDBK |
| | DJ FILT | FILT (filtro DJ do track) |
| | EQ 1 | LOW (Hz) · GAIN · MID1 (Hz) · GAIN — com a curva (1.5) |
| | EQ 2 | MID2 (Hz) · GAIN · HIGH (Hz) · GAIN — com a curva |
| | CHORUS | CHO (envio) · CRT · CDP |
| | DELAY | DLY (envio) · TIME (1/2 … 1/32T, ou os ms no MODE MS) · FDBK · **MODE** (SYNC / MS) |
| | DELAY 2 | COLR · MIX |
| | REVERB | REV (envio) · SIZE · DAMP |

As páginas de cada efeito **só aparecem quando ele está na cadeia** (ver abaixo).

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

### Um envelope para cada oscilador (1.3)

No MODO SYNTH **não existe envelope principal**: o ENV abre **OSC1 ENV**, **OSC2 ENV** e **OSC3 ENV**, cada um com
**ATK · DEC · SUS · REL** e o desenho do envelope, exatamente como o ENV do SLOOP (mesmas faixas, mesmas curvas, e o
**SYNC** da página ENV DEST vale para os três).

- O volume de cada oscilador segue **só o envelope dele**: um pad pode entrar devagar no OSC 2 enquanto o OSC 1 dá
  o ataque, ou o OSC 3 soar só no começo, como um clique.
- A nota dura até o **último** oscilador terminar o release: um REL longo no OSC 2 continua soando depois que o
  OSC 1 já parou.
- Um preset começa com os três envelopes iguais (o do preset); daí cada um vai para o seu lado.
- O envelope do OSC 1 é o ENV do track (P_ATK … P_REL): fora do modo synth, a página ENV do SLOOP num track WAVE
  mexe nele. É ele também que move filtro, pitch e POS (ENV DEST).
- Com **REV**, o envelope daquele oscilador vira o swell ao contrário (sobe na velocidade do DEC dele até o topo e
  cai no SUS dele).

### LFO de cada oscilador (1.2)

Além do LFO da voz (que vale para todos), cada oscilador tem o seu, na página **OSCn LFO**: **RATE** (0–15),
**DEPTH** (−8 a +7; 0 desliga) e **DEST**: **POS** (varre a tabela só daquele oscilador), **PITCH** (vibrato só nele)
ou **LVL** (tremolo só nele).

### A cadeia de efeitos (1.2)

No MODO SYNTH **todo efeito começa desligado**. A primeira página do FX é a **FX CHAIN**: 7 slots (4 aqui, 3 em
CHAIN 2, um por efeito), cada um com **--** ou um efeito — DRIVE, PHASR, FILTR, CHORS, DELAY, REVRB, EQ.

- Escolher um efeito num slot **liga** o efeito (numa quantidade que já se ouve) e faz a página de detalhes dele
  aparecer depois de CHAIN 2. Voltar o slot para **--** desliga o efeito e esconde a página.
- **DRIVE, PHASER e FILTER** tocam **na ordem dos slots** (ex.: PHASER antes do DRIVE soa diferente de DRIVE antes
  do PHASER). CHORUS, DELAY e REVERB são envios (depois de todos).
- Um efeito ligado por fora do modo (nas páginas normais do SLOOP, ou num projeto antigo) entra sozinho no primeiro
  slot livre: a cadeia sempre mostra tudo o que está soando.

| Efeito | O que é |
| --- | --- |
| **DRIVE** | Estilo **Decapitator**, num track WAVE: 5 estilos — **A** fita, **E** válvula (harmônicos pares), **N** transformador, **T** triodo, **P** pentodo —, **TONE** (escuro ↔ aberto) e **PUNSH**. No 1.2 ficou bem mais pesado: mais ganho em todo o curso do DST e o PUNSH soma +18 dB. |
| **PHASER** | O INSERT do SLOOP como PHASR (MIX · RATE · DEPTH · FDBK). |
| **FILTER** | O filtro DJ do track (passa-baixa para a esquerda, passa-alta para a direita). |
| **CHORUS, DELAY, REVERB** | Os envios do SLOOP e os ajustes globais deles, na mesma página. |

### EQ de 4 bandas (1.5)

Ponha **EQ** num slot da FX CHAIN e aparecem as páginas **EQ 1** e **EQ 2**, com a **curva** do EQ desenhada como numa
DAW: o ganho (±15 dB) em função da frequência (30 Hz a 16 kHz), com linhas em 0 dB e em 100 Hz, 1 kHz e 10 kHz.

| Banda | Tipo | Knobs |
| --- | --- | --- |
| **LOW** | shelf (graves) | EQ 1: KNOB 1 frequência, KNOB 2 ganho |
| **MID1** | sino | EQ 1: KNOB 3 frequência, KNOB 4 ganho |
| **MID2** | sino | EQ 2: KNOB 1 frequência, KNOB 2 ganho |
| **HIGH** | shelf (agudos) | EQ 2: KNOB 3 frequência, KNOB 4 ganho |

- Cada banda é um **ponto** no gráfico, na frequência e no ganho dela, na cor do seu knob de frequência (azul KNOB 1,
  amarelo KNOB 3); as da outra página aparecem em cinza.
- O EQ começa **plano** (100 Hz, 500 Hz, 2 kHz, 8 kHz, todos em 0 dB) e fica na ordem da cadeia junto com DRIVE,
  PHASER e FILTER. Tirar o EQ da cadeia desliga, sem perder os ajustes.
- Os filtros são os SVF do Andrew Simper (os mesmos do filtro do synth): shelves sem o "afundamento" depois do corte.
  Uma banda em 0 dB não gasta nada.
- O EQ fica salvo com o som: nos projetos e nos presets de usuário.

### Delay: MODE SYNC / MS (1.4)

O delay tem um **MODE** (KNOB 4 da página DELAY; fora do modo synth, FX → DLY 2):

- **SYNC**: o **TIME** é uma figura, no BPM: **1/2, 1/2T, 1/4, 1/4T, 1/4D, 1/8, 1/8T, 1/8D, 1/16, 1/16T, 1/16D,
  1/32, 1/32T** — o knob anda nessa ordem, uma figura por clique.
- **MS**: o **TIME** vira **ms** (10–1480) e não muda com o BPM. Ao passar para MS, o tempo que estava tocando vira
  ms (é o antigo FIX: mude o andamento e o eco fica onde estava). De volta a SYNC, volta a figura de antes.

### Limpar o synth: REC + SAVE (1.2)

No MODO SYNTH, segure **SAVE** e **REC** juntos: depois de ~0,7 s aparece o anel **clear / synth sound**; segure
até ele fechar (~1,3 s) e o track volta ao **SINE**, com todos os efeitos desligados, a cadeia
vazia, sem LFO nos osciladores e os três envelopes iguais ao do som. **As notas do track ficam** (para apagar as notas, REC sozinho segurado,
como no SLOOP). Soltou antes: nada acontece.

### Sem gastar flash nem RAM

O flash do FM-1 está quase cheio, então as formas de onda **não ficam no flash**: cada tabela escolhida é
**calculada** dentro da "arena" que os engines GRAIN/FM6/PHYS já compartilham por track — uma tabela para cada
oscilador, 8 frames de 128 amostras —, aos poucos no áudio (~23 ms por tabela), substituindo frame a frame a
que está tocando; trocar de tabela nunca deixa o som mudo. Nenhum byte de RAM a mais (um `_Static_assert`
garante isso).

Os 25 parâmetros do synth cabem **empacotados nos 8 parâmetros do engine** (16 bits cada): projetos, presets de
usuário e p-locks os guardam como sempre. Os do 1.2 (envelope e LFO de cada oscilador, a cadeia: 12 bytes por track,
e o MS do delay) vão num registro ao lado de cada projeto e de cada banco de presets de usuário na flash, que o
SLOOP nunca lê. As páginas do synth editam cada campo; o **editor web** mostra os 8
números brutos e não edita esses campos.

Custo de CPU, medido no PC: 1 oscilador de tabela ≈ 0,9× uma voz do ANALOG; 3 osciladores de tabela ≈ 1,5×.

### Os 16 sons

WT BASIC, WT SWEEP, PPG PAD, VOX SWEEP, WT CHOIR, PWM STRNG, WT ORGAN, WT PLUCK, WT BELLS, RESO BASS,
DIGI BASS, FM GROWL, FOLD LEAD, SUPER WAVE, CHIP WAVE e **REV SWELL** — no navegador de PRESETS por categoria
(BASS, ORGN, PAD, LEAD, PLCK), com volume equalizado aos do ANALOG (`WAVE_TRIM`, a ±2 dB).

---

## Fora do modo synth

### REC em tempo real (1.5)

O **REC grava em tempo real** de fábrica: cada nota fica no passo mais perto e o **micro-timing** do passo (1/64 de um
passo, ~2 ms a 120 BPM em 1/16) guarda o tempo em que você tocou. Para gravar **quantizado**, na tela **REC READY**
(REC com o transporte parado) gire o **KNOB 4 (QUANT)** para a direita: **ON**; para a esquerda volta a **OFF**. A
escolha fica salva no FM-1. Um passo tem um micro-timing só: num acorde, ou com dois sons da bateria no mesmo passo,
vale o da primeira nota.

### REPEAT da bateria (1.4)

Com a **bateria** (track 4) selecionada, um **toque no ARP** liga o **REPEAT** (o ARP fica aceso): cada tecla
segurada repete no tempo, como o *note repeat* de uma drum machine, sem precisar segurar nada. Com **REC** ligado,
as batidas são gravadas (mais rápidas que o passo viram ratchets). Outro toque no ARP desliga.

A divisão é a do roll: **ARP + KNOB 1** — **1/2T, 1/4, 1/4T, 1/8, 1/8T, 1/16, 1/16T, 1/32, 1/32T, 1/64** (1.4.1: 1/4T
e 1/2T). Segurar ARP + teclas continua sendo o roll de sempre, em qualquer track.

**Tercinas gravadas (1.4.2).** O REPEAT toca cada figura no tempo exato (a 480 ticks por semínima: 1/2T = 640,
1/4T = 320, 1/8T = 160, 1/16T = 80). Ao gravar, cada batida vai para o passo mais perto e o **micro-timing (nudge)**
desse passo guarda o tempo dela: **1/2T, 1/4T e 1/8T ficam exatas na grade normal de 1/16**. Um passo tem um nudge só:
se ele já tem outra batida, a nova fica na grade e a tela avisa **TRIPLETS: DIV 16T**; o 1/16T põe duas batidas num
passo de 1/16, então para ele use PATTERN → DIV 16T. 1/32T grava certo em 1/16 (três por passo, ratchet x3). A grade
do padrão nunca muda sozinha (a 1.4.1 mudava para 16T, e aí até o 1/16 tocava em tercina).

### ARP: todas as tercinas (1.4)

O **RATE** do arpejador ganhou **1/2, 1/2T, 1/4T e 1/32T** e anda em ordem musical: 1/2, 1/2T, 1/4, 1/4T, 1/8, 1/8T,
1/16, 1/16T, 1/32, 1/32T. Os valores antigos continuam com o mesmo número (projetos abrem iguais).

### VOICING (1.2)

Com o **CHORD** ligado (SEL → página SEL 2), o antigo **VLEAD** virou **VOIC**, o voicing do acorde:

| VOIC | Um C maior (C E G) vira |
| --- | --- |
| OFF | como tocado |
| LEAD | cada acorde na inversão mais perto do anterior (o VLEAD ON de antes) |
| INV1 / INV2 | 1ª / 2ª inversão (E G C / G C E) |
| DROP2 / DROP3 | a 2ª / 3ª voz de cima uma oitava abaixo (E3 C G / C3 E G) |
| SPRD | aberto: vozes alternadas uma oitava acima (C G E5) |
| +BASS | mais a fundamental uma oitava abaixo (C3 C E G) |

Funciona nas teclas, nos passos e com os modificadores das teclas pretas.

---

## Compatibilidade

- Projetos, presets de usuário, samples e ajustes do SLOOP continuam valendo.
- Sons WAVE salvos com o BONBLOOP 1.0 tocam diferente no 1.1 (os parâmetros do engine mudaram de significado).
- Do 1.1 para o 1.2: os sons e projetos abrem iguais; os efeitos que estavam ligados aparecem sozinhos na cadeia.
  Os presets de fábrica WAVE agora começam sem efeitos (no 1.1 vinham com o phaser no INSERT).
- Do 1.2 para o 1.3: o envelope ON/OFF de 4 bits por oscilador virou um envelope completo; os extras salvos no 1.2
  (LFO dos osciladores, cadeia, MS) não são lidos: o som abre com os três envelopes iguais e os efeitos ligados na cadeia.
- Um projeto salvo no 1.3 aberto no SLOOP ou num BONBLOOP anterior perde só os extras (envelopes do OSC 2 e 3, LFO
  dos osciladores, cadeia, MS): o OSC 1 segue o ENV do track.
- Um projeto com um track em WAVE (engine 12), aberto num SLOOP oficial, cai no som padrão daquele track.
- O editor web lê a lista de engines do próprio aparelho, então o WAVE aparece nele (com os números brutos).
- A identidade do pacote continua `FM-1_900` (build de desenvolvimento), instalável pelo mesmo instalador.

## Instalar

Pelo navegador (Chrome ou Edge), com o FM-1 ligado no computador pelo USB:
**[bonbap.com.br/bonbloop](https://bonbap.com.br/bonbloop/)** → **INSTALL**. O site (pasta `docs/`, em
inglês) é gerado por `web/make_site.py` e instala direto no FM-1 conectado, pelo Web MIDI.

Imagens para divulgação: `assets/promo/` (`tools/gen_bonbloop_brand.py promo`, com a fonte e as telas do próprio FM-1).

## Compilar e instalar pela linha de comando

Igual ao SLOOP (ver [BUILDING.md](BUILDING.md)): toolchain JieLi (Linux x86-64, via WSL ou Docker) e três
arquivos do SDK AC79. No Windows com WSL:

```
python tools/build_windows.py --distro Ubuntu --toolchain /root/.jieli/toolchain --sdk build/deps/ac79 --slice 0
python tools/fm1_install.py build/felucca.fwsc
```

Tamanho do 1.5: 576.164 de 581.564 bytes de flash (5,3 KB livres); RAM 59.740 de 98.304 bytes, pool igual ao do SLOOP.

## Testes

`tests/run_tests.sh` roda tudo, incluindo os dois testes novos:

- `tests/wave_test.c` — o engine sozinho: os 24 campos empacotados, todas as tabelas em cada oscilador, todos os
  presets (limites, DC, volume), as formas varridas, REV, DRIVE, extremos, custo de CPU; grava
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
